#ifndef _testing_
#define _testing_

#define SQUARE_TRACK
//#define ZIGZAG_TRACK
//#define STRAIGHT_TRACK
//#define SPIN_TRACK

#define STARTUP_DELAY 4000

#define SERIAL_ENABLED 1

#define TWI_ENABLED 1

/* Status LED */
#define LED_LEFT 0
#define LEDL_PORT PORTC
#define LEDL_DDR DDRC
#define LEDL_PIN 7
#define LED_RIGHT 1
#define LEDR_PORT PORTB
#define LEDR_DDR DDRB
#define LEDR_PIN 2


/* Motor timing */
#define MOTOR_SPEED_HIGH 255
#define MOTOR_SPEED_LOW 200
#define MOTOR_SPEED_HIGH2 200
#define MOTOR_SPEED_LOW2 150
#define TURN_SPEED 120

/* Measurements */
#define TICKS_PER_DEGREE 0.3909722
#define TICKS_PER_METRE 300
#define BRAKE_TIME 500
#define BRAKE_SPEED 80
#define MIN(x, y) ((x < y)? x : y)
#define MAX(x, y) ((x < y)? y : x)
#define CONSTRAIN(x, low, high) (MIN(high, MAX(low, x)))

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
#define FORWARD_LEFT 1
#define FORWARD_RIGHT 2 
#define BRAKE 3
#define REVERSE_LEFT 4
#define REVERSE_RIGHT 5

#define FORWARD 6
#define TURN_RIGHT 7
#define TURN_LEFT 8
#define REVERSE 9
unsigned char cmd_data[2];

/* Data types */
struct checkpoint {
	uint16_t angle; /* Direction of the checkpoint from last in degrees */
	uint8_t direction; /* Left: 1   Right: 2 */
	uint16_t distance; /* Distance to this checkpoint (meters? centimetres?) */
	uint8_t radius; /* Distance from the checkpoint centre that's safe to turn in */
	uint8_t sensor_flags; /* Bitfield indicating sensors that can be trusted near checkpoint */
} __attribute__((__packed__));

/* Global variables */

// Current checkpoint
struct checkpoint *goal;

// Encoder counts (signed)
volatile int32_t encoderLeft, encoderRight;
volatile int8_t leftDirection, rightDirection;

// Servo
#define SERVO_TURN 30
#define SERVO_START 2500
#define SERVO_END 5000
uint16_t servo_counter = 0;

// ADC result
volatile uint16_t adc_reading;
#define MUX_RANGER1 0x00
#define MUX_RANGER2 0x01
#define MUX_COMPASS1 0x02
#define MUX_COMPASS2 0x03
#define MUX_INFRARED1 0x04
#define MUX_INFRARED2 0x05
#define MUX_INFRARED3 0x06
#define MUX_ADC7 0x07

volatile uint16_t ranger1, ranger2;
volatile uint16_t compass1, compass2;

// Compass reading / resetting
#define COMPASS_SET {}
#define COMPASS_RESET {}

/* Function prototypes */
void init(void);

void twi_rx(uint8_t* buffer, int count);
void twi_tx(void);

uint16_t reset_compass(void);

void command(uint8_t command, uint8_t value);
void turnTo(uint16_t ticks);
void driveUntil(uint16_t distance);
void brake(uint8_t amount);

void LED_ON(uint8_t led);
void LED_OFF(uint8_t led);
void LED_TOGGLE(uint8_t led);

void DEBUG_STRING(const char *str);
void DEBUG_NUMBER(const char *name, uint16_t num);

uint8_t *eeprom_p = 0;

#if(SERIAL_ENABLED)
#define DEBUG_CHAR(x) uart_putc(x)
#else
#define DEBUG_CHAR(x)
#endif









#endif /* end of include guard: _testing_ */
