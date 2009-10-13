#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "slave.h"
#include "twi.h"

volatile uint8_t rxBuffer[24];
volatile uint8_t i = 0;

/* Setup registers, initialize sensors, etc. */
void init(void) {
	//Disable timer interrupts before setting up timers
	TIMSK0 = 0x00;
	TIMSK1 = 0x00;
	TIMSK2 = 0x00;

	/*
	TODO : Set PWM outputs to be inverting or non-inverting for appropriate
	H-Bridge inputs
	*/

	//Set 8-bit timer 0 and PWM outputs OC0A(PD6) and OC0B(PD5) at clock speed
	TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00);
	TCCR0B = _BV(CS00); 
	MOTORL1 = 0;
	MOTORL2 = 0;

	//Set 8-bit timer 2 and PWM outputs OC2A(PB3) and OC2B(PD3) at clock speed
	TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM20);
	TCCR2B = _BV(CS20);
	MOTORR1 = 0;
	MOTORR2 = 0;

	//Set 16-bit timer 1 and pwm outputs OC1A(PB1) and OC1B(PB2) at clock speed / 8
	TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
	TCCR1B = _BV(CS11) | _BV(WGM13) | _BV(WGM12); // CLK / 8, CTC with TOP at ICR1
	ICR1 = 40000; // Timer overflows every 20ms
	
	// Setup TWI
	twi_init();

	// Attach TWI interrupts
	twi_attachSlaveRxEvent(twi_rx);
	twi_attachSlaveTxEvent(twi_tx);
	
	// Set slave address
	twi_setAddress(TWI_SLAVE);
	
	// Turn on interrupts
	sei();
	
	// Setup outputs
	DDRB |= 0xFF;
	DDRD |= 0xFF;
	
}

int main(void) {
	
	DDRB |= _BV(5);
	PORTB |= _BV(5);
	_delay_ms(200);
	PORTB &= ~_BV(5);

	init();
	
	/*
	while(1) {
		SERVO1 = SERVO_POSITION(0);
		_delay_ms(1000);
		SERVO1 = SERVO_POSITION(3000);
		_delay_ms(1000);

	}
	*/
	
	/* Testing servo motors */
	/*
	SERVO1 = SERVO_POSITION(0);
	_delay_ms(1000);
	SERVO1 = SERVO_POSITION(1000);
	_delay_ms(1000);
	SERVO1 = SERVO_POSITION(500);
	_delay_ms(1000);
	
	SERVO2 = SERVO_POSITION(0);
	_delay_ms(500);
	SERVO2 = SERVO_POSITION(1000);
	_delay_ms(500);
	SERVO2 = SERVO_POSITION(500);
	_delay_ms(1000);
	*/
	
	/* Testing DC motors */
	// Spin left motor in one direction
	MOTORL1 = 255;
	MOTORL2 = 0;

	DDRB |= _BV(5);
	PORTB |= _BV(5);
	
	while(1) {}


	return 0;
}

void command(uint8_t command, uint8_t data) {
	switch(command) {
		case SET_LEFT_SPEED:
		break;
		case SET_RIGHT_SPEED:
		break;
		case BRAKE_LEFT:
		break;
		case BRAKE_RIGHT:
		break;
		case REVERSE_LEFT:
		break;
		case REVERSE_RIGHT:
		break;
		case GET_PATH_OFFSET:
		break;
	}
	DDRB |= _BV(5);
	PORTB |= _BV(5);
	_delay_ms(1);
	PORTB &= ~_BV(5);
}


void twi_rx(uint8_t* buffer, int count) {
	while(count) {
		rxBuffer[i++] = buffer[count--];
		if(i == 2) {
			command(rxBuffer[0], rxBuffer[1]);
		}
	}
	DDRB |= _BV(5);
	PORTB |= _BV(5);
}

void twi_tx(void) {
	twi_transmit(rxBuffer, i);
	i = 0;
	PORTB &= ~_BV(5);
}







