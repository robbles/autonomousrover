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

/* Servo motor(s) */
#define SERVO1 OCR1A
#define SERVO2 OCR1B
#define SERVO_POSITION(x) ((2*x) + 1000) // Input should be from 0-1000

/* TWI Definitions */
#define TWI_SLAVE 0x5A
#define TWI_BAD_LENGTH 1
#define TWI_ADDR_NACK 2
#define TWI_DATA_NACK 3
#define TWI_ERROR 4
#define TWI_BAD_DATA 5
#define TWI_WAIT 1
#define TWI_NOWAIT 0

/* Function prototypes */
void init(void);
void show_error(uint8_t code);


#endif /* end of include guard: _testing_ */
