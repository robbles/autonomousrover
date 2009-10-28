#ifndef _testing_
#define _testing_

//#define SQUARE_TRACK
#define ZIGZAG_TRACK

/* Clock speed of the AVR CPU */
#ifndef F_CPU
#define F_CPU 16000000
#endif

#define STARTUP_DELAY 2000

#define SERIAL_ENABLED 1

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

#define MS_PER_DEGREE 11
#define MS_PER_METRE 20
#define BRAKE_TIME 1000

/* Servo motor(s) */
#define SERVO_MIN 950 // TODO: recalculate this value for 8-bit control
#define SERVO1 OCR2B
#define SERVO_POSITION(x) (x + SERVO_MIN)

/* TWI Definitions */
#define TWI_ENABLED 0

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
#define BRAKE_LEFT 3
#define BRAKE_RIGHT 4
#define REVERSE_LEFT 5
#define REVERSE_RIGHT 6
#define GET_PATH_OFFSET 7

/* Data types */
struct checkpoint {
	uint16_t distance; /* Distance to this checkpoint (meters? centimetres?) */
	int16_t angle; /* Direction of the checkpoint from last in degrees */
	uint8_t radius; /* Distance from the checkpoint centre that's safe to turn in */
	uint8_t sensor_flags; /* Bitfield indicating sensors that can be trusted near checkpoint */
} __attribute__((__packed__));

/* Global variables */
// TWI buffer
volatile uint8_t rxBuffer[24];
volatile uint8_t i = 0;

// Current checkpoint
struct checkpoint *goal;

/* Function prototypes */
void init(void);

void turnRightTo(int16_t degree);
void turnLeftTo(int16_t degree);
void driveUntil(uint16_t distance, uint8_t speed);
void brake(void);

void twi_rx(uint8_t* buffer, int count);
void twi_tx(void);

void LED_ON(void);
void LED_OFF(void);

void DEBUG_STRING(const char *str);
void DEBUG_NUMBER(const char *name, uint16_t num);
#if(SERIAL_ENABLED)
#define DEBUG_CHAR(x) uart_putc(x)
#else
#define DEBUG_CHAR(x)
#endif









#endif /* end of include guard: _testing_ */
