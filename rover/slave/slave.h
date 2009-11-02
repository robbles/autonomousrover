#ifndef _testing_
#define _testing_

#define SQUARE_TRACK
//#define ZIGZAG_TRACK

/* Clock speed of the AVR CPU */
#ifndef F_CPU
#define F_CPU 16000000
#endif

#define STARTUP_DELAY 2000

#define SERIAL_ENABLED 1

#define TWI_ENABLED 1

/* Status LED */
#define LED_PORT PORTB
#define LED_DDR DDRB
#define LED_PIN 0

/* DC motors */
#define MOTORL1 OCR0A
#define MOTORL2 OCR0B
#define MOTORR1 OCR1A
#define MOTORR2 OCR1B

#define MOTORL_PORT PORTD
#define MOTORL_DDR DDRD
#define MOTORR_PORT PORTB
#define MOTORR_DDR DDRB

#define MOTORL1_PIN 6
#define MOTORL2_PIN 5
#define MOTORR1_PIN 1
#define MOTORR2_PIN 2

#define MOTORL_FORWARD(x) {MOTORL1 = x; MOTORL2 = 0;}
#define MOTORL_REVERSE(x) {MOTORL1 = 0; MOTORL2 = x;}
#define MOTORL_BRAKE(x) {MOTORL1 = x; MOTORL2 = x;}

#define MOTORR_FORWARD(x) {MOTORR1 = x; MOTORR2 = 0;}
#define MOTORR_REVERSE(x) {MOTORR1 = 0; MOTORR2 = x;}
#define MOTORR_BRAKE(x) {MOTORR1 = x; MOTORR2 = x;}

#define MOTOR_SPEED 255

#define TICKS_PER_DEGREE 1
#define TICKS_PER_METRE 300
#define BRAKE_TIME 1000

/* Servo motor(s) */
#define SERVO_MIN 950 // TODO: recalculate this value for 8-bit control
#define SERVO1 OCR2B
#define SERVO_POSITION(x) (x + SERVO_MIN)

/* TWI Definitions */
#define TWI_SLAVE 0x5A
#define TWI_BAD_LENGTH 1
#define TWI_ADDR_NACK 2
#define TWI_DATA_NACK 3
#define TWI_ERROR 4
#define TWI_BAD_DATA 5
#define TWI_WAIT 1
#define TWI_NOWAIT 0

/* TWI Commands */
#define SET_LEFT_SPEED 1
#define SET_RIGHT_SPEED 2 
#define BRAKE 3
#define REVERSE_LEFT 4
#define REVERSE_RIGHT 5

/* Data types */
struct checkpoint {
	uint16_t distance; /* Distance to this checkpoint (meters? centimetres?) */
	int16_t angle; /* Direction of the checkpoint from last in degrees */
	uint8_t radius; /* Distance from the checkpoint centre that's safe to turn in */
	uint8_t sensor_flags; /* Bitfield indicating sensors that can be trusted near checkpoint */
} __attribute__((__packed__));

/* Global variables */

// Output value to send back to master
volatile uint16_t txValue;

// Current checkpoint
struct checkpoint *goal;

// Encoder counts
uint32_t encoderLeft, encoderRight;

/* Function prototypes */
void init(void);

void turnRightTo(int16_t degree);
void turnLeftTo(int16_t degree);
void driveUntil(uint16_t distance);
void brake(void);

void twi_rx(uint8_t* buffer, int count);
void twi_tx(void);

void LED_ON(void);
void LED_OFF(void);
#define LED_FLASH(x) {LED_ON(); _delay_ms(x); LED_OFF(); _delay_ms(200);}

void DEBUG_STRING(const char *str);
void DEBUG_NUMBER(const char *name, uint16_t num);
#if(SERIAL_ENABLED)
#define DEBUG_CHAR(x) uart_putc(x)
#else
#define DEBUG_CHAR(x)
#endif









#endif /* end of include guard: _testing_ */
