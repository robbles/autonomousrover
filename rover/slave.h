#ifndef _testing_
#define _testing_

/* Clock speed of the AVR CPU */
#ifndef F_CPU
#define F_CPU 16000000
#endif

/* DC motors */
#define MOTORL1 OCR0A
#define MOTORL2 OCR0B
#define MOTORR1 OCR2A
#define MOTORR2 OCR2B

#define MOTORL_FORWARD(x) {MOTORL1 = x; MOTORL2 = 0;}
#define MOTORL_REVERSE(x) {MOTORL1 = 0; MOTORL2 = x;}
#define MOTORL_BRAKE(x) {MOTORL1 = x; MOTORL2 = x;}

#define MOTORR_FORWARD(x) {MOTORR1 = x; MOTORR2 = 0;}
#define MOTORR_REVERSE(x) {MOTORR1 = 0; MOTORR2 = x;}
#define MOTORR_BRAKE(x) {MOTORR1 = x; MOTORR2 = x;}

/* Servo motor(s) */
#define SERVO_MIN 950
#define SERVO1 OCR1A
#define SERVO2 OCR1B
#define SERVO_POSITION(x) (x + SERVO_MIN) // Input should be from 0-3000

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
#define BRAKE_LEFT 3
#define BRAKE_RIGHT 4
#define REVERSE_LEFT 5
#define REVERSE_RIGHT 6
#define GET_PATH_OFFSET 7

/* Function prototypes */
void init(void);
void show_error(uint8_t code);
void twi_rx(uint8_t* buffer, int count);
void twi_tx(void);


#endif /* end of include guard: _testing_ */
