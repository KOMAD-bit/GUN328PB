/*
 * GS_BaseSoft.h
 *
 * Created: 22/07/2022 17:04:44
 *  Author: Katharina Böhm-Klamt
 */ 

#include <avr/interrupt.h>



#ifndef GS_BASESOFT_H_
#define GS_BASESOFT_H_


void soft_timer_reset(uint8_t n);		
uint8_t soft_timer_is_stop(uint8_t n); 

void soft_chk_do();			

#define SOFT_MAX_TIMER 4

// Flags: [ 7 6 7 4 3 AltDisp Led ]
uint8_t soft_enable;			// Enable Timer (master)
uint8_t soft_enable_fsync;		// Enable sync function
uint8_t soft_enable_fasync;		// Enable async function


struct SOFT_ZEITEN
{
	uint16_t 	ms;				// actual interrupt counts
	uint16_t	ms_max;			// max count;
	void		(*fsync)();		// interrupt-function synchron
	void		(*fasync)();	// interrupt-function asynchron
} soft_s_zeiten[SOFT_MAX_TIMER];


#endif /* GS_BASESOFT_H_ */