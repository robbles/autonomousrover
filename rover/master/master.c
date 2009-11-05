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
#endif


/* Setup registers, initialize sensors, etc. */
void init(void) {
		
	//Disable timer interrupts before setting up timers
	TIMSK0 = 0x00;
	TIMSK1 = 0x00;
	TIMSK2 = 0x00;
	
	// Setup 8-bit timer0 and timer2
	TCCR0A = _BV(WGM01);
	TCCR0B = _BV(CS02) | _BV(CS01) | _BV(CS00); // Clocked from T0 pin
	OCR0A = 255; // one full rotation
	TIMSK0 = _BV(OCIE0A);
	
	ASSR |= _BV(AS2); // Enable asynchronous operation first to avoid corruption
	ASSR |= _BV(EXCLK); // External clock instead of crystal
	TCCR2A = _BV(WGM21); // CTC mode, wraps at OCR2A
	TCCR2B = _BV(CS20); // Timer actually clocked externally
	OCR2A = 255; // one full rotation
	TIMSK2 = _BV(OCIE2A);
	
	// Setup 16-bit timer 1
	TCCR1A = 0x00; // No PWM output
	TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12) | _BV(CS10); // clk/1024, 16-bit CTC
	ICR1 = 15625; // Overflow interrupt will trigger every 1 second
	OCR1A = OCR1B = 0; // Set these to trigger additional interrupts for analog capture ?
	TIMSK1 = _BV(ICIE1); // Trigger interrupt when timer reaches TOP	
		
	// Setup ADC
	ADMUX = MUX_RANGER1; // VRef = AREF, Right adjust result, src = ADC0
	ADCSRA = _BV(ADEN) | _BV(ADIE); // Enable ADC and interrupt, clk /2 speed
	ADCSRA |= _BV(ADATE);
	//ADCSRB = _BV(ADTS2) | _BV(ADTS1) | _BV(ADTS0); // Auto-trigger on Timer1 capture event
	ADCSRB = 0x00; // Free-running mode
	DIDR0 = 0x00; // Don't disable digital input on ADC pins
	ADCSRA |= _BV(ADSC); // Start converting!
		
#if(TWI_ENABLED)

	// Setup TWI
	twi_init();	

#endif
	
#if(SERIAL_ENABLED)

	uart_init(UART_BAUD_SELECT(9600, F_CPU));
	
	UCSR0B &= ~_BV(RXCIE0);
	
	DDRD &= ~_BV(0);
	DDRD |= _BV(1);
	
#endif
		
	// Turn on interrupts
	sei();

}

int main(void) {
	
	_delay_ms(STARTUP_DELAY);
		
	LED_ON();
	
	init();
			
	DEBUG_STRING("master starting...");
	
	while(1) {		
		DEBUG_NUMBER("encoderLeft", encoderLeft + ENCODER_LEFT);
		DEBUG_NUMBER("encoderRight", encoderRight + ENCODER_RIGHT);
		_delay_ms(500);
	}
		
		
		
		
		
		
		
	// Finished, do nothing
	while(1) {}
	
	return 0;
}


/* Interrupt Handlers */

/* Interrupt handler for CTC interrupt
	Should occur every 1 second */
SIGNAL(TIMER1_CAPT_vect) {
	//LED_PORT ^= _BV(LED_PIN);
}

SIGNAL(TIMER0_COMPA_vect) {
	LED_PORT ^= _BV(LED_PIN);
	encoderLeft += OCR0A;
}

SIGNAL(TIMER2_COMPA_vect) {
	LED_PORT ^= _BV(LED_PIN);
	encoderRight += OCR2A;
}

/* Interrupt handler for ADC */
SIGNAL(ADC_vect) {
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



