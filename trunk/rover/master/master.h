#ifndef _testing_
#define _testing_

//#define SQUARE_TRACK
#define ZIGZAG_TRACK

#define STARTUP_DELAY 1000

#define SERIAL_ENABLED 1

#define TWI_ENABLED 1

/* Status LED */
#define LED_PORT PORTB
#define LED_DDR DDRB
#define LED_PIN 2

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

// Current checkpoint
struct checkpoint *goal;

// Encoder counts
uint32_t encoder0, encoder1;

// ADC result
uint16_t adc_reading;
#define MUX_RANGER1 0x00
#define MUX_RANGER2 0x01
#define MUX_COMPASS1 0x02
#define MUX_COMPASS2 0x03
#define MUX_INFRARED1 0x04
#define MUX_INFRARED2 0x05
#define MUX_INFRARED3 0x06
#define MUX_ADC7 0x07

uint16_t ranger1, ranger2;
uint16_t compass1, compass2;

// Compass reading / resetting
#define COMPASS_SET {}
#define COMPASS_RESET {}

/* Function prototypes */
void init(void);

void twi_rx(uint8_t* buffer, int count);
void twi_tx(void);

uint16_t reset_compass(void);

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
