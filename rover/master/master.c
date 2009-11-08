#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "master.h"
#include "twi.h"

#if(SERIAL_ENABLED)
#include "uart.h"
#include <stdlib.h>
#endif

#if defined(SQUARE_TRACK)
#include "squaretrack.h"
#elif defined(ZIGZAG_TRACK)
#include "zigzagtrack.h"
#elif defined(STRAIGHT_TRACK)
#include "straighttrack.h"
#endif


/* Setup registers, initialize sensors, etc. */
void init(void) {
		
	//Disable timer interrupts before setting up timers
	TIMSK0 = 0x00;
	TIMSK1 = 0x00;
	TIMSK2 = 0x00;
	
	// Setup 8-bit timer 2
	TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20); // Fast PWM, top at OCR2A
	TCCR2B = _BV(WGM22) | _BV(CS20); // clock speed
	OCR2A = 25; // 80 KHz wave
	OCR2B = 12; // 50% duty cycle
	DDRD |= _BV(6); // Enable output
	
	// Setup 16-bit timer 1
	TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11); // PWM output on OC1A, OC1B
	TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11); // clk / 8, 16-bit Fast PWM
	ICR1 = 50000; // Overflows every 20 ms
	OCR1A = OCR1B = 2500; // Both servos near center
	TIMSK1 = _BV(ICIE1); // Trigger interrupt when timer reaches TOP	
	DDRD |= _BV(4) | _BV(5); // Enable PWM outputs
		
	// Setup ADC
	ADMUX = MUX_RANGER1; // VRef = AREF, Right adjust result, src = ADC0
	ADCSRA = _BV(ADEN) | _BV(ADIE); // Enable ADC and interrupt, clk /2 speed
	ADCSRA |= _BV(ADATE);
	//ADCSRB = _BV(ADTS2) | _BV(ADTS1) | _BV(ADTS0); // Auto-trigger on Timer1 capture event
	ADCSRB = 0x00; // Free-running mode
	DIDR0 = 0x00; // Don't disable digital input on ADC pins
	//ADCSRA |= _BV(ADSC); // Start converting!
		

	// Setup TWI
	twi_init();	
	
	// Enable rising-edge external interrupts on INT0(pin 4) and INT1(pin 5)
	EICRA = _BV(ISC11) | _BV(ISC10) | _BV(ISC01) | _BV(ISC00);
	EIMSK = _BV(INT1) | _BV(INT0);

	
#if(SERIAL_ENABLED)

	uart_init(UART_BAUD_SELECT(9600, F_CPU));
	
	UCSR0B &= ~_BV(RXCIE0);
	
	DDRD &= ~_BV(0);
	DDRD |= _BV(1);
	
#endif
		
	// Setup and start following path
	goal = track;
	
	// Set encoder count to zero
	encoderLeft = encoderRight = 0;
		
	// Turn on interrupts
	sei();
	
	// Initialize LED outputs
	LEDL_DDR |= _BV(LEDL_PIN);
	LEDR_DDR |= _BV(LEDR_PIN);
}

int main(void) {
		
	_delay_ms(STARTUP_DELAY);
	
	init();
			
	DEBUG_STRING("\n\n\nmaster starting...");
	
	DDRD &= ~_BV(2);
	DDRD &= ~_BV(3);
		
		
	while(goal->distance != 0) {
		
		// Turn to face the next checkpoint
		DEBUG_STRING("\nGoing to next checkpoint:");
		DEBUG_NUMBER("distance", goal->distance);
		DEBUG_NUMBER("angle", goal->angle);
			
		if(goal->angle > 0) {
			turnRightTo(goal->angle);
		} 
		else if(goal->angle < 0) {
			turnLeftTo(goal->angle);
		}
						
		driveUntil(goal->distance);
		
		//DEBUG_NUMBER("encoderLeft", encoderLeft);
		//DEBUG_NUMBER("encoderRight", encoderRight);
		
		brake();
		
		goal++;
	}
	
	DEBUG_STRING("done track!");
	
	// Finished, do nothing
	while(1) {}
	
	return 0;
}

void command(uint8_t command, uint8_t value) {
	uint8_t err;
	cmd_data[0] = command;
	cmd_data[1] = value;
	do {
		// Repeat transmission until successful
		err = twi_writeTo(TWI_SLAVE, cmd_data, 2, 1);
	} while(err);
}

void turnRightTo(int16_t degree) {
	DEBUG_STRING("turning right");
	uint32_t start = encoderLeft + encoderRight;
	uint32_t end = start + (2 * degree / DEGREES_PER_TICK);
	DEBUG_NUMBER("end", end);
	command(TURN_RIGHT, 255);
	
	while((encoderLeft + encoderRight) < end) { 
		DEBUG_NUMBER("encoderLeft", encoderLeft);
		DEBUG_NUMBER("encoderRight", encoderRight);
	}
	
	// Reset encoders to start value since the rover hasn't moved
	encoderLeft = encoderRight = (start / 2);
}

void turnLeftTo(int16_t degree) {
	DEBUG_STRING("turning left");
	uint32_t start = encoderLeft + encoderRight;
	uint32_t end = start + (2 * degree / DEGREES_PER_TICK);
	DEBUG_NUMBER("end", end);
	command(TURN_LEFT, 255);

	while((encoderLeft + encoderRight) < end) { 
		DEBUG_NUMBER("encoderLeft", encoderLeft);
		DEBUG_NUMBER("encoderRight", encoderRight);
	}
	
	// Reset encoders to start value since the rover hasn't moved
	encoderLeft = encoderRight = (start / 2);
}

void driveUntil(uint16_t distance) {
	uint32_t endLeft = encoderLeft + distance;
	uint32_t endRight = encoderRight + distance;
	
	int32_t diff = 0;
	
	while((encoderLeft < endLeft) && (encoderRight < endRight)) {
		if((encoderLeft - encoderRight) != diff) {
			diff = encoderLeft - encoderRight;
			command(FORWARD_LEFT, CONSTRAIN(255 - diff, 200, 255));
			command(FORWARD_RIGHT, CONSTRAIN(255 + diff, 200, 255));
		}
	}
}

/*
void driveUntil(uint16_t distance) {
	DEBUG_NUMBER("Driving distance", distance);
	uint8_t left_speed, right_speed;
	left_speed = 255;
	right_speed = 255;
	uint32_t start = encoderLeft + encoderRight;
	uint32_t end = start + (2 * distance);
	DEBUG_NUMBER("end", end);
	command(FORWARD, 255);
	
	while((encoderLeft + encoderRight) < end) { 
		
		if(encoderLeft > encoderRight) {
			right_speed = 255;
			if(left_speed > 128) {
				left_speed-=1;
			}
			command(FORWARD_LEFT, left_speed);
			command(FORWARD_RIGHT, right_speed);
		}
		else if(encoderRight > encoderLeft) {
			left_speed = 255;
			if(right_speed > 128) {
				right_speed-=1;
			}
			command(FORWARD_RIGHT, right_speed);
			command(FORWARD_LEFT, left_speed);
		}
		_delay_ms(10);
	}	
}
*/

void brake() {
	DEBUG_STRING("Braking");
	command(BRAKE, 255);
	_delay_ms(BRAKE_TIME);
}



SIGNAL(BADISR_vect) {
	while(1) {
	LED_TOGGLE(LED_RIGHT);
	_delay_ms(50);
	}
}



/* Interrupt Handlers */

/* Interrupt handler for Timer1 interrupt
	Should occur every 20 milliseconds */
SIGNAL(TIMER1_CAPT_vect) {
	//LED_TOGGLE(LED_RIGHT);
	
	if(servo_counter >= SERVO_TURN) {
		servo_counter = 0;
		OCR1B = (OCR1B == SERVO_START)? SERVO_END : SERVO_START;
	} else {
		servo_counter++;
	}
}



/* Interrupt handler for ADC */
SIGNAL(ADC_vect) {
	//LED_PORT ^= _BV(LED_PIN);
	adc_reading = (ADCH << 8) | ADCL;
	
	// Choose next MUX value based on previous
	switch(ADMUX) {
		case MUX_RANGER1:
			ADMUX = MUX_RANGER2;
			ranger1 = adc_reading;
			break;
		case MUX_RANGER2:
			ADMUX = MUX_COMPASS1;
			ranger2 = adc_reading;
			break;
		case MUX_COMPASS1:
			ADMUX = MUX_COMPASS2;
			compass1 = adc_reading;
			break;
		case MUX_COMPASS2:
			ADMUX = MUX_INFRARED1;
			compass2 = adc_reading;
			break;
		case MUX_INFRARED1:
			ADMUX = MUX_INFRARED2;
			break;
		case MUX_INFRARED2:
			ADMUX = MUX_INFRARED3;
			break;
		case MUX_INFRARED3:
			ADMUX = MUX_RANGER1;
			break;
	}
}

SIGNAL(INT0_vect) {
	encoderLeft++;
	LED_TOGGLE(LED_LEFT);
}

SIGNAL(INT1_vect) {
	encoderRight++;
	LED_TOGGLE(LED_RIGHT);
}


/*
	Sets the next ADC conversion to the compass, 
	and turns on the compass set/reset flag
*/

uint16_t reset_compass() {
	uint16_t reading1, reading2;
	// Turn off ADC interrupt and auto-trigger
	ADCSRA &= ~_BV(ADIE) & ~_BV(ADATE);
	
	// Wait for any other conversions to complete
	loop_until_bit_is_clear(ADCSRA, ADSC);
	
	// Set compass, start conversion and wait
	COMPASS_SET;
	ADCSRA |= _BV(ADSC);
	loop_until_bit_is_clear(ADCSRA, ADSC);
	reading1 = (ADCH << 8) | ADCL;
	
	// Reset compass, start conversion and wait for interrupt
	COMPASS_RESET;
	ADCSRA |= _BV(ADSC);
	loop_until_bit_is_clear(ADCSRA, ADSC);
	reading2 = (ADCH << 8) | ADCL;
	
	// Restore interrupts and restart
	ADCSRA |= _BV(ADIE) | _BV(ADATE) | _BV(ADSC);
	
	// Return the offset voltage
	return reading2 - reading1;
}


/* Turns on status LED */
void LED_ON(uint8_t led) {
	if(led == LED_LEFT) {
		LEDL_PORT |= _BV(LEDL_PIN);
	} else {
		LEDR_PORT |= _BV(LEDR_PIN);
	}
}

// Turns off status LED
void LED_OFF(uint8_t led) {
	if(led == LED_LEFT) {
		LEDL_PORT &= ~_BV(LEDL_PIN);
	} else {
		LEDR_PORT &= ~_BV(LEDR_PIN);
	}
}

// Toggles status LED
void LED_TOGGLE(uint8_t led) {
	if(led == LED_LEFT) {
		LEDL_PORT ^= _BV(LEDL_PIN);
	} else {
		LEDR_PORT ^= _BV(LEDR_PIN);
	}
}

void DEBUG_STRING(const char *str) {
#if(SERIAL_ENABLED)
	uart_puts(str);
	uart_puts("\n\r");
#endif
}

void DEBUG_NUMBER(const char *name, uint16_t num) {
#if(SERIAL_ENABLED)
	char snum[8];
	itoa(num, snum, 10);
	uart_puts(name);
	uart_putc('=');
	uart_puts(snum);
	uart_puts("\n\r");
#endif
}



