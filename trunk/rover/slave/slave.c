#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include "slave.h"
#include "twi.h"

#if(SERIAL_ENABLED)
#include "uart.h"
#endif

#if defined(SQUARE_TRACK)
#include "squaretrack.h"
#elif defined(ZIGZAG_TRACK)
#include "zigzagtrack.h"
#endif


/* Setup registers, initialize sensors, etc. */
void init(void) {
		
	//Disable timer interrupts before setting up timers
	TIMSK0 = 0x00;
	TIMSK1 = 0x00;
	TIMSK2 = 0x00;
	
	// Set 8-bit timer 0 and PWM outputs OC0A(PD6) and OC0B(PD5) at clock speed
	TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00);
	TCCR0B = _BV(CS00); 
	MOTORL1 = MOTORL2 = 0;
	
	// Set 16-bit timer 1 and pwm outputs OC1A(PB1) and OC1B(PB2) at clock speed
	TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(CS10); // clock speed, 8-bit phase correct PWM
	MOTORR1 = MOTORR2 = 0;

	// Set 8-bit timer 2 and PWM output OC2B(PD3) at clock speed / 1024
	//TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
	//TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20); // timer will overflow every 16.32 ms (~61 Hz)
	// Values for OC2B should range from 23 - 30 for full rotation
	
	MOTORL_DDR |= _BV(MOTORL1_PIN) | _BV(MOTORL2_PIN);
	MOTORR_DDR |= _BV(MOTORR1_PIN) | _BV(MOTORR2_PIN);
	
	// Enable rising-edge external interrupts on INT0(pin 4) and INT1(pin 5)
	// Warning, INT1 conflicts with OC2B (servo PWM output)
	EICRA = _BV(ISC11) | _BV(ISC10) | _BV(ISC01) | _BV(ISC00);
	EIMSK = _BV(INT1) | _BV(INT0);
	
	encoderLeft = 1;
	encoderRight = 1;
	
		
#if(TWI_ENABLED)

	// Setup TWI
	twi_init();

	// Attach TWI interrupts
	twi_attachSlaveRxEvent(twi_rx);
	twi_attachSlaveTxEvent(twi_tx);
	
	// Set slave address
	twi_setAddress(TWI_SLAVE);
	
#endif
	
//#if(SERIAL_ENABLED)

	uart_init(UART_BAUD_SELECT(9600, F_CPU));
	
	//UCSR0B &= ~_BV(RXCIE0);
	
	DDRD &= ~_BV(0);
	DDRD |= _BV(1);
	
//#endif
	
	// Setup and start following path
	goal = track;
	
	// Set encoder count to zero
	encoderLeft = encoderRight = 0;

	sei();

}

int main(void) {
	init();
		
	DDRD &= ~_BV(2);
	DDRD &= ~_BV(3);
		
	_delay_ms(STARTUP_DELAY);
	
	DEBUG_STRING("\n\n\nStarting...");
	
	while(1) {
		DEBUG_NUMBER("encoderLeft", encoderLeft);
		DEBUG_NUMBER("encoderRight", encoderRight);
		_delay_ms(500);
	}
	
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



void turnRightTo(int16_t degree) {
	DEBUG_STRING("turning right");
	uint32_t start = encoderLeft + encoderRight;
	uint32_t end = start + (2 * degree * TICKS_PER_DEGREE);
	MOTORL_FORWARD(MOTOR_SPEED);
	MOTORR_REVERSE(MOTOR_SPEED);
	
	while((encoderLeft + encoderRight) < end) {
 	}
	
	// Reset encoders to start value since the rover hasn't moved
	//encoderLeft = encoderRight = (start / 2);
}

void turnLeftTo(int16_t degree) {
	DEBUG_STRING("turning left");
	uint32_t start = encoderLeft + encoderRight;
	uint32_t end = start + (2 * degree * TICKS_PER_DEGREE);
	MOTORL_REVERSE(MOTOR_SPEED);
	MOTORR_FORWARD(MOTOR_SPEED);
	
	while((encoderLeft + encoderRight) < end) {
 	}
	
	// Reset encoders to start value since the rover hasn't moved
	//encoderLeft = encoderRight = (start / 2);
}

void driveUntil(uint16_t distance) {
	DEBUG_NUMBER("Driving distance", distance);
	
	uint32_t start = encoderLeft + encoderRight;
	uint32_t end = start + (2 * distance);
	MOTORL_FORWARD(MOTOR_SPEED);
	MOTORR_FORWARD(MOTOR_SPEED);
	
	while((encoderLeft + encoderRight) < end) {
		// Adjust motors so the rover keeps going straight
		/*
		if(encoderLeft > encoderRight) {
			MOTORL_FORWARD(120);
			MOTORR_FORWARD(130);
		} else if(encoderRight > encoderLeft) {
			MOTORL_FORWARD(130);
			MOTORR_FORWARD(120);
		} else {
			MOTORL_FORWARD(128);
			MOTORR_FORWARD(128);
		}
		*/
		//DEBUG_NUMBER("encoderLeft", encoderLeft);
		//DEBUG_NUMBER("encoderRight", encoderRight);
		_delay_ms(500);
 	}
	
}

void brake() {
	DEBUG_STRING("Braking");
	MOTORR_BRAKE(MOTOR_SPEED);
	MOTORL_BRAKE(MOTOR_SPEED);
	_delay_ms(BRAKE_TIME);
}


#if(TWI_ENABLED)

/* TWI Commands */
void command(uint8_t command, uint8_t data) {
	int i;
	for(i=0; i<command; i++) {
		LED_ON();
		_delay_ms(50);
		LED_OFF();
		_delay_ms(50);
	}
	_delay_ms(500);
	for(i=0; i<data; i++) {
		LED_ON();
		_delay_ms(50);
		LED_OFF();
		_delay_ms(50);
	}
	switch(command) {
		case SET_LEFT_SPEED:
		break;
		case SET_RIGHT_SPEED:
		break;
		case BRAKE:
		break;
		case REVERSE_LEFT:
		break;
		case REVERSE_RIGHT:
		break;
	}

}


void twi_rx(uint8_t* buffer, int count) {
	LED_ON();
	if(count == 2) {
		command(buffer[0], buffer[1]);
	} else {
		LED_OFF();
	}
}

void twi_tx(void) {
	// Send data value back as little-endian value
	twi_transmit((char *)txValue, 2);
}

#endif



SIGNAL(INT0_vect) {
	encoderLeft++;
	LED_PORT ^= _BV(LED_PIN);
}

SIGNAL(INT1_vect) {
	encoderRight++;
	LED_PORT ^= _BV(LED_PIN);
}



/* Turns on status LED */
void LED_ON() {
	LED_DDR |= _BV(LED_PIN);
	LED_PORT |= _BV(LED_PIN);
}

// Turns off status LED
void LED_OFF() {
	LED_PORT &= ~_BV(LED_PIN);
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



