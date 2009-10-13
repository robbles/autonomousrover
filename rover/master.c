#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "testing.h"
#include "twi.h"

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
	// TODO: modify to control servo accurately using appropriate output capture value
	TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(COM1B1) | _BV(COM1B0);
	TCCR1B = _BV(CS11) | _BV(WGM13) | _BV(WGM12); // CLK / 8, CTC with TOP at ICR1
	ICR1 = 40000; // Timer overflows every 20ms
	OCR1A = OCR1B = 3000; // Position both servos at center
	
	// Setup TWI
	twi_init();

	// Turn on interrupts
	sei();
}

int main(void) {

	int err;
	char *msg1 = "Speed up";

	char rxBuffer[24];

	init();

	/* 		Testing TWI 	*/
	// Write first message to slave device
	err = twi_writeTo(TWI_SLAVE, msg1, 9, TWI_WAIT);
	if(err) { show_error(err); }

	// Try to read it back
	twi_readFrom(TWI_SLAVE, rxBuffer, 9);

	for(int i=0; i<9; i++) {
		if(rxBuffer[i] != msg1[i]) {
			show_error(TWI_BAD_DATA);
		}
	}
	
	
	/* Testing servo motors */
	SERVO1 = SERVO_POSITION(0);
	_delay_ms(500);
	SERVO1 = SERVO_POSITION(1000);
	_delay_ms(500);
	SERVO1 = SERVO_POSITION(500);
	_delay_ms(1000);
	
	SERVO2 = SERVO_POSITION(0);
	_delay_ms(500);
	SERVO2 = SERVO_POSITION(1000);
	_delay_ms(500);
	SERVO2 = SERVO_POSITION(500);
	_delay_ms(1000);
	
	
	/* Testing DC motors */
	// Spin left motor in one direction
	MOTORL1 = 128;
	MOTORL2 = 0;
	
	// Spin right in the opposite
	MOTORR1 = 0;
	MOTORR2 = 128;
	
	// Brake both motors
	MOTORL1 = MOTORL2 = MOTORR1 = MOTORR2 = 255;
	
	// Stop both motors
	MOTORL1 = MOTORL2 = MOTORR1 = MOTORR2 = 0;


	while(1) {}


	return 0;
}


void show_error(uint8_t code) {
	// Blink LED on pin PB5(13) to show error
	DDRB |= _BV(5);
	while((code--) > 0) {
	PORTB |= _BV(5);
	_delay_ms(50);
	PORTB &= ~_BV(5);
	_delay_ms(100);
	}
}










