/*
 * twimaster.c
 *
 * Created: 22/07/2022 17:04:00
 *  Author: Katharina Böhm-Klamt
 */ 
#include <inttypes.h>
#include <compat/twi.h>
#include "twimaster.h"

/* define CPU frequency in Mhz here if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 8000000
#endif

/* I2C clock in Hz */
#define SCL_CLOCK  1000000


void i2c_init(void)
{
  /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
  
  TWSR0 = 0;                         /* no prescaler */
 // TWBR = ((F_CPU/SCL_CLOCK)-16)/1;  /* must be > 10 for stable operation */
	TWBR0 =10;
}/* i2c_init */


signed char i2c_start(unsigned char address)
{
    uint8_t   twst;

	// send START condition
	TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR0 & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 2;

	// send device address
	TWDR0 = address;
	TWCR0 = (1<<TWINT) | (1<<TWEN);

	// wail until transmission completed and ACK/NACK has been received
	while(!(TWCR0 & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;

	return 0;

}/* i2c_start */


void i2c_start_wait(unsigned char address)
{
    uint8_t   twst;


    while ( 1 )
    {
	    // send START condition
	    TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

    	while(!(TWCR0 & (1<<TWINT)));
    
    	twst = TW_STATUS & 0xF8;
    	if ( (twst != TW_START) && (twst != TW_REP_START)) continue;
    
    	// send device address
    	TWDR0 = address;
    	TWCR0 = (1<<TWINT) | (1<<TWEN);
    
    	// wail until transmission completed
    	while(!(TWCR0 & (1<<TWINT)));
    
    	twst = TW_STATUS & 0xF8;
    	if ( (twst == TW_MT_SLA_NACK )||(twst ==TW_MR_DATA_NACK) ) 
    	{    	    
	        TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	        
	        while(TWCR0 & (1<<TWSTO));
	        
    	    continue;
    	}
    	break;
     }

}/* i2c_start_wait */

unsigned char i2c_rep_start(unsigned char address)
{
    return i2c_start( address );

}/* i2c_rep_start */

void i2c_stop(void)
{
	TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	



}/* i2c_stop */



unsigned char i2c_write( unsigned char data )
{	
    uint8_t   twst;

	TWDR0 = data;
	TWCR0 = (1<<TWINT) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR0 & (1<<TWINT))); 

	twst = TW_STATUS & 0xF8;
	if( twst != TW_MT_DATA_ACK) return 1;
	return 0;

}/* i2c_write */



unsigned char i2c_readAck(void)
{
	TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR0 & (1<<TWINT)));    

    return TWDR0;

}/* i2c_readAck */



unsigned char i2c_readNak(void)
{
	TWCR0 = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR0 & (1<<TWINT)));
	
    return TWDR0;

}
