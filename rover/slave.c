#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "slave.h"
#include "twi.h"

/* Setup registers, initialize sensors, etc. */
void init(void) {
	//Disable timer interrupts before setting up timers
	TIMSK0 = 0x00;
	TIMSK1 = 0x00;
	TIMSK2 = 0x00;
	
	//Set 8-bit timer 0 and PWM outputs OC0A(PD6) and OC0B(PD5) at clock speed
	TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00);
	TCCR0B = _BV(CS00); 
	MOTORL1 = MOTORL2 = 0;
	
	//Set 16-bit timer 1 and pwm outputs OC1A(PB1) and OC1B(PB2) at clock speed
	TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(CS10); // clock speed, 8-bit phase correct PWM
	MOTORR1 = MOTORR2 = 0;

	//Set 8-bit timer 2 and PWM output OC2B(PD3) at clock speed / 1024
	TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
	TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20); // timer will overflow every 16.32 ms (~61 Hz)
	

	
#ifdef USE_TWI

	// Setup TWI
	twi_init();

	// Attach TWI interrupts
	twi_attachSlaveRxEvent(twi_rx);
	twi_attachSlaveTxEvent(twi_tx);
	
	// Set slave address
	twi_setAddress(TWI_SLAVE);
	
	// Turn on interrupts
	sei();

#endif
	
	// Setup outputs
	DDRB |= 0xFF;
	DDRD |= 0xFF;
	
	// Setup and start following path
	path = malloc(5 * sizeof(struct checkpoint *));
	path[0] = malloc(sizeof(struct checkpoint));
	goal = path[0];
	goal->turn_angle = 50;
	goal->distance = 200;
	goal->radius = 50;
	goal->flags = 0;
	
}

int main(void) {
	
	DDRB |= _BV(5);
	PORTB |= _BV(5);
	_delay_ms(200);
	PORTB &= ~_BV(5);

	init();
	
	/* Testing DC motors */

	MOTORL_FORWARD(255);
	MOTORR_BRAKE(255);
	_delay_ms(1000);
	
	MOTORL_BRAKE(255);
	MOTORR_FORWARD(255);
	_delay_ms(1000);
	
	MOTORL_FORWARD(128);
	MOTORR_BRAKE(255);
	_delay_ms(1000);
	
	MOTORR_FORWARD(128);
	MOTORL_BRAKE(255);
	_delay_ms(1000);
	
	MOTORL_FORWARD(128);
	MOTORR_FORWARD(128);
	_delay_ms(1000);
	
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

}


void twi_rx(uint8_t* buffer, int count) {
	while(count) {
		rxBuffer[i++] = buffer[count--];
		if(i == 2) {
			command(rxBuffer[0], rxBuffer[1]);
		}
	}
}

void twi_tx(void) {
	twi_transmit(rxBuffer, i);
	i = 0;
}

/* Flashes LED_PORT(LED_PIN) for ms milliseconds */
void flash_LED(uint8_t ms) {
	DDRB |= _BV(5);
	PORTB |= _BV(5);
	_delay_ms(1);
	PORTB &= ~_BV(5);
	
}







