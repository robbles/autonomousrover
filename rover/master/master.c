#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <string.h>
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
#elif defined(SPIN_TRACK)
#include "spintrack.h"
#endif


/* Setup registers, initialize sensors, etc. */
void init(void) {
		
	//Disable timer interrupts before setting up timers
	TIMSK0 = 0x00;
	TIMSK1 = 0x00;
	TIMSK2 = 0x00;
	
	// Setup 8-bit timer 2
	TCCR0A = _BV(COM0A1) | _BV(WGM01) | _BV(WGM00); // Fast PWM, top at OCR0A
	TCCR0B = _BV(WGM02) | _BV(CS00); // clock speed
	OCR0A = 25; // 80 KHz wave
	OCR0B = 12; // 50% duty cycle
	DDRB |= _BV(4); // Enable output on OC0B (PB4)
	
	// Setup 16-bit timer 1
	TCCR1A = _BV(WGM11); // No PWM output
	TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11); // clk / 8, 16-bit Fast PWM
	ICR1 = 50000; // Overflows every 20 ms
	//TIMSK1 = _BV(ICIE1); // Trigger interrupt when timer reaches TOP	
		
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
	DDRC &= ~_BV(0);
	DDRC &= ~_BV(1);
	PORTC |= _BV(0) | _BV(1); // Enable input pull-up resistors (~15K)
	
	
	// Enable rising-edge external interrupts on INT0(pin 4) and INT1(pin 5)
	EICRA = _BV(ISC11) | _BV(ISC10) | _BV(ISC01) | _BV(ISC00);
	EIMSK = _BV(INT1) | _BV(INT0);

	
#if(SERIAL_ENABLED)

	uart_init(UART_BAUD_SELECT(9600, F_CPU));
	
	UCSR0B &= ~_BV(RXCIE0);
	
	DDRD &= ~_BV(0);
	DDRD |= _BV(1);
	
#endif
	
	// Make PA3 an input	
	DDRA &= ~_BV(3);
	
	// Setup and start following path
	goal = track;

	// Turn on interrupts
	sei();
	
}

int main(void) {
	// Initialize LED outputs
	LEDL_DDR |= _BV(LEDL_PIN);
	LEDR_DDR |= _BV(LEDR_PIN);
	
	LED_ON(LED_LEFT);
	_delay_ms(500);
	LED_OFF(LED_LEFT);
	LED_ON(LED_RIGHT);
	_delay_ms(500);
	LED_OFF(LED_RIGHT);
	_delay_ms(500);
	LED_ON(LED_LEFT);
	LED_ON(LED_RIGHT);
	_delay_ms(1000);
	LED_OFF(LED_LEFT);
	LED_OFF(LED_RIGHT);
	
		
	_delay_ms(STARTUP_DELAY);
	
	init();
			
	DEBUG_STRING("\n\n\nmaster starting...\n");
		
	// Disable outputs on INT0, INT1
	DDRD &= ~_BV(2);
	DDRD &= ~_BV(3);
	PORTD &= ~_BV(2);
	PORTD &= ~_BV(3);
		
	// Set encoder count to zero
	encoderLeft = encoderRight = 0;
	leftDirection = rightDirection = 1;
	
	while((goal->distance != 0) || (goal->angle != 0)) {
		
		// Turn to face the next checkpoint
		DEBUG_STRING("\nGoing to next checkpoint:\n");
		DEBUG_NUMBER("distance", goal->distance);
		DEBUG_NUMBER("angle", goal->angle);
			
		if(goal->direction == 1) {
			DEBUG_STRING("turning left\n");
			command(TURN_LEFT, TURN_SPEED);
			turnTo(goal->angle);
		} 
		else if(goal->direction == 2) {
			DEBUG_STRING("turning right\n");
			command(TURN_RIGHT, TURN_SPEED);
			turnTo(goal->angle);
		}
		
		brake(255);
				
		DEBUG_STRING("driving straight\n");
		driveUntil(goal->distance);
		
		//DEBUG_NUMBER("encoderLeft", encoderLeft);
		//DEBUG_NUMBER("encoderRight", encoderRight);
		
		brake(BRAKE_SPEED);
		
		goal++;
	}
	
	DEBUG_STRING("done track!\n");
	
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

void turnTo(uint16_t degrees) {
	uint32_t ticks = (uint32_t)((float)degrees * TICKS_PER_DEGREE);
	DEBUG_NUMBER("goal ticks", ticks);
	
	//uint32_t endLeft = encoderLeft + ticks;
	//uint32_t endRight = encoderRight + ticks;
	uint32_t endLeft = ticks;
	uint32_t endRight = ticks;
	
	while((encoderLeft < endLeft) && (encoderRight < endRight)) {
		DEBUG_NUMBER("encoderLeft", encoderLeft);
		DEBUG_NUMBER("encoderRight", encoderRight);
	}
	
	encoderLeft = encoderRight = 0;
	//encoderLeft = encoderLeft - endLeft;
	//encoderRight = encoderRight - endRight;
	
}

void driveUntil(uint16_t distance) {
	//uint32_t endLeft = encoderLeft + distance;
	//uint32_t endRight = encoderRight + distance;
	uint32_t endLeft = distance;
	uint32_t endRight = distance;
	
	DEBUG_NUMBER("goal distance", distance);
	uint8_t high_speed = MOTOR_SPEED_HIGH;
	uint8_t low_speed = MOTOR_SPEED_LOW;
	
	int32_t diff = 0;
	
	command(FORWARD, high_speed);
	
	while((encoderLeft < endLeft) && (encoderRight < endRight)) {
		DEBUG_NUMBER("encoderLeft", encoderLeft);
		DEBUG_NUMBER("encoderRight", encoderRight);
		
		if((encoderLeft - encoderRight) != diff) {
			diff = 2 * (encoderLeft - encoderRight);
			uint8_t lspeed = CONSTRAIN(high_speed - diff, low_speed, high_speed);
			uint8_t rspeed = CONSTRAIN(high_speed + diff, low_speed, high_speed);
			command(FORWARD_LEFT, lspeed);
			command(FORWARD_RIGHT, rspeed);
		}
		/*if((endLeft - encoderLeft < 100) || (endRight - encoderRight < 100)) {
			high_speed = MOTOR_SPEED_HIGH2;
			low_speed = MOTOR_SPEED_LOW2;
		}*/
	}
	
	encoderLeft = encoderRight = 0;
	//encoderLeft = encoderLeft - endLeft;
	//encoderRight = encoderRight - endRight;
}


void brake(uint8_t amount) {
	command(BRAKE, amount);
	_delay_ms(BRAKE_TIME);
	DEBUG_NUMBER("encoderLeft after stopping", encoderLeft);
	DEBUG_NUMBER("encoderRight after stopping", encoderRight);
	
	// Do error correction
	/*if((encoderLeft - encoderRight) > 3) {
		command(REVERSE_LEFT, 120);
		while((encoderLeft - encoderRight) > 3) {};
	}
	else if((encoderRight - encoderLeft) > 3) {
		command(REVERSE_RIGHT, 120);
		while((encoderRight - encoderLeft) > 3) {};
	}
	command(BRAKE, 255);
	_delay_ms(BRAKE_TIME);
	*/
	encoderLeft = encoderRight = 0;
}



SIGNAL(BADISR_vect) {
	while(1) {
	//LED_TOGGLE(LED_RIGHT);
	DEBUG_STRING("WHAT THE FUCK");
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
	//LED_TOGGLE(LED_LEFT);
	LEDL_PORT ^= _BV(LEDL_PIN);
}

SIGNAL(INT1_vect) {
	encoderRight++;
	//LED_TOGGLE(LED_RIGHT);
	LEDR_PORT ^= _BV(LEDR_PIN);
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
	if(PINA & _BV(3) && (eeprom_p <= (uint8_t *)1950)) {
		uint8_t len = strlen(str);
		eeprom_write_block(str, eeprom_p, len);
		eeprom_p += len;
	}
}

void DEBUG_NUMBER(const char *name, uint16_t num) {
	if(PINA & _BV(3) && (eeprom_p <= (uint8_t *)1950)) {
		char snum[8];
		snum[0] = '=';
		itoa(num, snum+1, 10);
		uint8_t len = strlen(name);
		eeprom_write_block(name, eeprom_p, len);
		eeprom_p += len;
		len = strlen(snum);
		eeprom_write_block(snum, eeprom_p, len);
		eeprom_p += len;
		eeprom_write_block("\n\r", eeprom_p, 2);
		eeprom_p += 2;
	}
}



