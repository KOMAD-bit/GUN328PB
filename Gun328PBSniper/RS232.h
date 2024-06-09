/*
 * RS232.h
 *
 * Created: 22/07/2022 17:03:35
 *  Author: Katharina Böhm-Klamt
 */ 


#define RS232_H_






#include <avr/interrupt.h>

//****************************************************************************************************

void my_delay(uint16_t __ms);

#define IRCMD_HEADER				0xF0
#define PROTCOLL_INVERT_8

struct S_SER_IN
{
	uint8_t     cmd;            // Serieller Input
	uint8_t     id;             // Serieller Input
	uint8_t		val;			// Serieller Input valid
}; 


void RX_ClearBuffer();
uint8_t RX_GetNextCommand(struct S_SER_IN* serial_in);
uint8_t RX_GetNext(uint8_t* in);	// cable version

#ifdef DEBUG_STUDIO
	void RXPush(uint8_t val);
#endif 

//--------------------------------- TX --------------------------------------------------------

uint8_t TX_BufferEmpty();	// cable version
void TX_SendChar(uint8_t c); // cable version
void TX_SendCommand(uint8_t cmd, uint8_t id, uint8_t val);


