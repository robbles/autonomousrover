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
	
	MOTORL_DDR |= _BV(MOTORL1_PIN) | _BV(MOTORL2_PIN);
	MOTORR_DDR |= _BV(MOTORR1_PIN) | _BV(MOTORR2_PIN);
	
#if(TWI_ENABLED)

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
	
#if(SERIAL_ENABLED)

	uart_init(UART_BAUD_SELECT(9600, F_CPU));
	
	UCSR0B &= ~_BV(RXCIE0);
	
	DDRD &= ~_BV(0);
	DDRD |= _BV(1);
	
	sei();

#endif
	
	// Setup and start following path
	goal = track;
}

int main(void) {
	init();
		
	_delay_ms(STARTUP_DELAY);
	
	DEBUG_STRING("\n\n\nStarting...");
	
	LED_ON();
	
	while(goal->distance != 0) {
		
		// Turn to face the next checkpoint
		LED_ON();
		DEBUG_STRING("\nGoing to next checkpoint:");
		DEBUG_NUMBER("distance", goal->distance);
		DEBUG_NUMBER("angle", goal->angle);
		
		if(goal->angle > 0) {
			turnRightTo(goal->angle);
		} 
		else if(goal->angle < 0) {
			turnLeftTo(goal->angle);
		}
		
		LED_OFF();
		
		driveUntil(goal->distance, 128);
		
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
	
	MOTORL_FORWARD(128);
	MOTORR_BRAKE(128);
	
	while(degree-- > 0) {
		_delay_ms(MS_PER_DEGREE);
	}
}

void turnLeftTo(int16_t degree) {
	DEBUG_STRING("turning left");
	MOTORR_FORWARD(128);
	MOTORL_BRAKE(128);
	
	while(degree++ < 0) {
		_delay_ms(MS_PER_DEGREE);
	}
}

void driveUntil(uint16_t distance, uint8_t speed) {
	DEBUG_NUMBER("Driving at speed", speed);
	
	MOTORR_FORWARD(255);
	MOTORL_FORWARD(255);
	
	// TODO: adjust delay based on speed
	while(distance--) {
		_delay_ms(MS_PER_METRE);
	}
}

void brake() {
	DEBUG_STRING("Braking");
	MOTORR_BRAKE(255);
	MOTORL_BRAKE(255);
	_delay_ms(BRAKE_TIME);
}


#if(TWI_ENABLED)

/* TWI / Command stuff */
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

#endif

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



