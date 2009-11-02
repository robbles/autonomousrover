/*
twi.h - TWI/I2C library for Wiring & Arduino
Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef twi_h
#define twi_h

#include <inttypes.h>

//#define ATMEGA8

#ifndef CPU_FREQ
#define CPU_FREQ 16000000L
#endif

#ifndef TWI_FREQ
#define TWI_FREQ 100000L
#endif

#ifndef TWI_BUFFER_LENGTH
#define TWI_BUFFER_LENGTH 32
#endif

#define TWI_READY 0
#define TWI_MRX   1
#define TWI_MTX   2
#define TWI_SRX   3
#define TWI_STX   4

// Sets up TWI
void twi_init(void);

// Sets slave address
void twi_setAddress(uint8_t address);

// Master read
uint8_t twi_readFrom(uint8_t address, uint8_t* data, uint8_t length);

// Master write
uint8_t twi_writeTo(uint8_t address, uint8_t* data, uint8_t length, uint8_t wait);

// Slave write (for returning a buffer)
uint8_t twi_transmit(volatile uint8_t* data, volatile uint8_t length);

// Attach slave interrupts
void twi_attachSlaveRxEvent( void (*)(uint8_t*, int) );
void twi_attachSlaveTxEvent( void (*)(void) );

// Return ACK/NACK + byte to master
void twi_reply(uint8_t);

// Send stop condition
void twi_stop(void);

// Release TWI bus
void twi_releaseBus(void);

#endif

