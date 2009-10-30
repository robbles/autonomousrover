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
	
#if(EXTERNAL_INTERRUPTS)
	// Enable rising-edge external interrupts on INT0(pin 4) and INT1(pin 5)
	// Warning, INT1 conflicts with OC2B (servo PWM output)
	EICRA = _BV(ISC11) | _BV(ISC10) | _BV(ISC01) | _BV(ISC00);
	EIMSK = _BV(INT1) | _BV(INT0);
	
#endif
		
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
	
	// Set encoder count to zero
	encoderLeft = encoderRight = 0;
	
#if(TEST_EXT_INTERRUPTS)
	// Testing if we miss any interrupts
	_delay_ms(1000);
	uint32_t test = encoderLeft;
	DEBUG_NUMBER("ext. interrupts counted in 1s", test);
	
	encoderLeft = 0;
	for(i=0; i<10; i++) { _delay_ms(1000); }
	test = encoderLeft;
	DEBUG_NUMBER("ext. interrupts counted in 10s ", test);
#endif

}

int main(void) {
	init();
		
	_delay_ms(STARTUP_DELAY);
	
	DEBUG_STRING("\n\n\nStarting...");
	
	LED_ON();
	
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
		
		DEBUG_NUMBER("encoderLeft", encoderLeft);
		DEBUG_NUMBER("encoderRight", encoderRight);
		
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
	MOTORL_FORWARD(128);
	MOTORR_REVERSE(128);
	
	while((encoderLeft + encoderRight) < end) { }
	
	// Reset encoders to start value since the rover hasn't moved
	encoderLeft = encoderRight = (start / 2);
}

void turnLeftTo(int16_t degree) {
	DEBUG_STRING("turning left");
	uint32_t start = encoderLeft + encoderRight;
	uint32_t end = start + (2 * degree * TICKS_PER_DEGREE);
	MOTORL_REVERSE(128);
	MOTORR_FORWARD(128);
	
	while((encoderLeft + encoderRight) < end) { }
	
	// Reset encoders to start value since the rover hasn't moved
	encoderLeft = encoderRight = (start / 2);
}

void driveUntil(uint16_t distance) {
	DEBUG_NUMBER("Driving distance", distance);
	
	uint32_t start = encoderLeft + encoderRight;
	uint32_t end = start + (2 * distance);
	MOTORL_FORWARD(128);
	MOTORR_FORWARD(128);
	
	while((encoderLeft + encoderRight) < end) {
	// Adjust motors so the rover keeps going straight
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
		DEBUG_NUMBER("left speed", data);
		break;
		case SET_RIGHT_SPEED:
		DEBUG_NUMBER("right speed", data);
		break;
		case BRAKE:
		DEBUG_NUMBER("brake speed", data);
		break;
		case REVERSE_LEFT:
		DEBUG_NUMBER("left speed", -((int16_t)data));
		break;
		case REVERSE_RIGHT:
		DEBUG_NUMBER("right speed", -((int16_t)data));
		break;
	}

}


void twi_rx(uint8_t* buffer, int count) {
	DEBUG_NUMBER("twi: received ", count);
	
	if(count == 2) {
		command(buffer[0], buffer[1]);
	} else {
		DEBUG_STRING("Wrong # of bytes for command msg!");
	}
}

void twi_tx(void) {
	// Send data value back as little-endian value
	twi_transmit((char *)txValue, 2);
	DEBUG_NUMBER("twi: sending value", txValue);
}

#endif


#if(EXTERNAL_INTERRUPTS)

SIGNAL(INT0_vect) {
	encoderLeft++;
}

SIGNAL(INT1_vect) {
	encoderRight++;
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


