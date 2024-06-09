/*
 * RS232.c
 *
 * Created: 22/07/2022 17:03:24
 *  Author: Katharina Böhm-Klamt
 */ 
#include "RS232.h"
#define IRcomm_on()  TCCR1B = ( (1<<WGM12) | (1<<CS10) )
#define  IRon() TCCR1A |= (1<<COM1B0)
#define IRoff() TCCR1A &=~(1<<COM1B0)
#define IRcomm_off() TCCR1B = ( (1<<WGM12) | (1<<CS10) )


// die 4 byte codes sind nur gültig, wenn count >= 4
#define RX_BUFFER_SIZE 64
#define RX_BUFFER_MASK ( RX_BUFFER_SIZE-1 )
#define RX_BUFFER_EMPTY 1


#ifdef  PROTCOLL_CHECKSUM_4

struct USART_buffer
{
	uint8_t data[RX_BUFFER_SIZE];		// data bytes
	uint8_t ind_read;					// actual read position
	uint8_t ind_read_1;					// actual read position+1
	uint8_t ind_read_2;					// actual read position+2
	uint8_t ind_read_3;					// actual read position+3
	uint8_t ind_write;					// actual write position
	uint8_t cnt;						// anzahl bytes in buffer
} rx_buffer = {{},0,1,2,3,0,0};

// USART Receiver interrupt service routine
ISR(USART0_RX_vect)
{
	uint8_t next = ( rx_buffer.ind_write+1 ) & RX_BUFFER_MASK;
	if ( (UCSR0A & ( (1<<FE0 ) | (1<<UPE0 ) | (1<<DOR0) ) ) ==0 )
	{
		if (next == rx_buffer.ind_read)  // Buffer full  => overwrite first read_entry alte Daten gehen verloren !
		{
			rx_buffer.ind_read = rx_buffer.ind_read_1;		// alle anderen +1
			rx_buffer.ind_read_1 = rx_buffer.ind_read_2;	// alle anderen +1
			rx_buffer.ind_read_2 = rx_buffer.ind_read_3;	// alle anderen +1
			rx_buffer.ind_read_3 = ( rx_buffer.ind_read_3+1)  & RX_BUFFER_MASK;
			rx_buffer.cnt--;	// ein Wert weniger da anschliessend immer ein inc erfolgt
		}
		rx_buffer.data[rx_buffer.ind_write]=UDR0;
		rx_buffer.ind_write = next;
		rx_buffer.cnt ++;
	}
	else next = UDR0;  // skip byte on error - unbedingt auslesen, sonst hängt sich RS232 bei Error ewig in INT auf!		
}

uint8_t RX_GetNext(uint8_t* in)
{
	cli();	// lock while buffer reading
	uint8_t err =0;
	if (rx_buffer.ind_write == rx_buffer.ind_read)
	{
		err= RX_BUFFER_EMPTY;
	}
	else
	{
		*in = rx_buffer.data[rx_buffer.ind_read];		// erstes byte holen
		rx_buffer.ind_read = rx_buffer.ind_read_1;		// alle anderen +1
		rx_buffer.ind_read_1 = rx_buffer.ind_read_2;	// alle anderen +1
		rx_buffer.ind_read_2 = rx_buffer.ind_read_3;	// alle anderen +1
		rx_buffer.ind_read_3 = ( rx_buffer.ind_read_3 +1)  & RX_BUFFER_MASK;
		rx_buffer.cnt --;	/// ein zeichen weniger
	}
	sei();
	return err;
}

void RX_ClearBuffer()
{
	cli();
	rx_buffer.ind_read =0;
	rx_buffer.ind_read_1 =1;
	rx_buffer.ind_read_2 =2;
	rx_buffer.ind_read_3 =3;
	rx_buffer.ind_write =0;
	rx_buffer.cnt =0;
	sei();
}

uint8_t RX_GetNextCommand(uint8_t* cmd, uint8_t* id, uint8_t* val)
{
	if (rx_buffer.cnt>=4)
	{
		if (!RX_GetNext(cmd)) 
		{

			if ( (*cmd & 0xF8) == IRCMD_HEADER )		// obere 5 bytes = Header
			{
					
				uint8_t chksum = *cmd;
				chksum = chksum ^ rx_buffer.data[rx_buffer.ind_read];
				chksum = chksum ^ rx_buffer.data[rx_buffer.ind_read_1];
				chksum = chksum ^ rx_buffer.data[rx_buffer.ind_read_2];
				if (!chksum)	// wenn chksum passt alle 4 bytes ausbufer lesen (chkbyte skippen
				{
					RX_GetNext(id);
					RX_GetNext(val);
					RX_GetNext(&chksum);
					return 0;	//gültige Werte !
				}
			}
		}
			
	}
	return RX_BUFFER_EMPTY;
}
#endif
// --- end : PROTCOLL_CHECKSUM_4 : cmd , id , value , checksum


#ifdef  PROTCOLL_INVERT_8

struct USART_buffer
{
	uint8_t data[RX_BUFFER_SIZE];		// data bytes
	uint8_t ind_read;					// actual read position
	uint8_t ind_read_1;					// actual read position+1
	uint8_t ind_read_2;					// actual read position+2
	uint8_t ind_read_3;					// actual read position+3
	uint8_t ind_read_4;					// actual read position+1
	uint8_t ind_read_5;					// actual read position+2
	uint8_t ind_read_6;					// actual read position+3
	uint8_t ind_read_7;					// actual read position+3
	
	uint8_t ind_write;					// actual write position
	uint8_t cnt;						// anzahl bytes in buffer
	} rx_buffer = {{},0,1,2,3,4,5,6,7,0,0};

	// Testwert in Buffer schreiben
	#ifdef DEBUG_STUDIO
	void RXPush(uint8_t val)
	{
		uint8_t next = ( rx_buffer.ind_write+1 ) & RX_BUFFER_MASK;
		{
			if (next == rx_buffer.ind_read)  
			{
				rx_buffer.ind_read = rx_buffer.ind_read_1;		// alle anderen +1
				rx_buffer.ind_read_1 = rx_buffer.ind_read_2;	// alle anderen +1
				rx_buffer.ind_read_2 = rx_buffer.ind_read_3;	// alle anderen +1
				rx_buffer.ind_read_3 = rx_buffer.ind_read_4;	// alle anderen +1
				rx_buffer.ind_read_4 = rx_buffer.ind_read_5;	// alle anderen +1
				rx_buffer.ind_read_5 = rx_buffer.ind_read_6;	// alle anderen +1
				rx_buffer.ind_read_6 = rx_buffer.ind_read_7;	// alle anderen +1
				rx_buffer.ind_read_7 = ( rx_buffer.ind_read_7+1)  & RX_BUFFER_MASK;
				rx_buffer.cnt--;	// ein Wert weniger da anschliessend immer ein inc erfolgt
			}
			rx_buffer.data[rx_buffer.ind_write]= val;
			rx_buffer.ind_write = next;
			rx_buffer.cnt ++;
		}
	}
	#endif
	
	// USART Receiver interrupt service routine
	ISR(USART0_RX_vect)
	{
		uint8_t next = ( rx_buffer.ind_write+1 ) & RX_BUFFER_MASK;
		if ( (UCSR0A & ( (1<<FE0 ) | (1<<UPE0 ) | (1<<DOR0) ) ) ==0 )
		{
			if (next == rx_buffer.ind_read)  // Buffer full  => overwrite first read_entry alte Daten gehen verloren !
			{
				rx_buffer.ind_read = rx_buffer.ind_read_1;		// alle anderen +1
				rx_buffer.ind_read_1 = rx_buffer.ind_read_2;	// alle anderen +1
				rx_buffer.ind_read_2 = rx_buffer.ind_read_3;	// alle anderen +1
				rx_buffer.ind_read_3 = rx_buffer.ind_read_4;	// alle anderen +1
				rx_buffer.ind_read_4 = rx_buffer.ind_read_5;	// alle anderen +1
				rx_buffer.ind_read_5 = rx_buffer.ind_read_6;	// alle anderen +1
				rx_buffer.ind_read_6 = rx_buffer.ind_read_7;	// alle anderen +1
				rx_buffer.ind_read_7 = ( rx_buffer.ind_read_7+1)  & RX_BUFFER_MASK;
				rx_buffer.cnt--;	// ein Wert weniger da anschliessend immer ein inc erfolgt
			}
			rx_buffer.data[rx_buffer.ind_write]=UDR0;
			rx_buffer.ind_write = next;
			rx_buffer.cnt ++;
		}
		else next = UDR0;  
	}

	uint8_t RX_GetNext(uint8_t* in)
	{
		cli();	// lock while buffer reading
		uint8_t err =0;
		if (rx_buffer.ind_write == rx_buffer.ind_read)
		{
			err= RX_BUFFER_EMPTY;
		}
		else
		{
			*in = rx_buffer.data[rx_buffer.ind_read];		// erstes byte holen
			rx_buffer.ind_read = rx_buffer.ind_read_1;		// alle anderen +1
			rx_buffer.ind_read_1 = rx_buffer.ind_read_2;	// alle anderen +1
			rx_buffer.ind_read_2 = rx_buffer.ind_read_3;	// alle anderen +1
			rx_buffer.ind_read_3 = rx_buffer.ind_read_4;	// alle anderen +1
			rx_buffer.ind_read_4 = rx_buffer.ind_read_5;	// alle anderen +1
			rx_buffer.ind_read_5 = rx_buffer.ind_read_6;	// alle anderen +1
			rx_buffer.ind_read_6 = rx_buffer.ind_read_7;	// alle anderen +1
			rx_buffer.ind_read_7 = ( rx_buffer.ind_read_7 +1)  & RX_BUFFER_MASK;
			rx_buffer.cnt --;	/// ein zeichen weniger
		}
		sei();
		return err;
	}

	void RX_ClearBuffer()
	{
		cli();
		rx_buffer.ind_read =0;
		rx_buffer.ind_read_1 =1;
		rx_buffer.ind_read_2 =2;
		rx_buffer.ind_read_3 =3;
		rx_buffer.ind_read_4 =4;
		rx_buffer.ind_read_5 =5;
		rx_buffer.ind_read_6 =6;
		rx_buffer.ind_read_7 =7;
		rx_buffer.ind_write =0;
		rx_buffer.cnt =0;
		sei();
	}

	
	uint8_t RX_GetNextCommand(struct S_SER_IN* serial_in)
	{
		uint8_t chk_ff_0;
		
		if (rx_buffer.cnt>=8) // ev wieder auf while ändern
		{
			if (!RX_GetNext(&chk_ff_0)) // sollte passen, da cnt>=4 abgefragt wurde
			{
				// 1. byte auf gültges cmd überprüfen
				// HHHH.HCCC : 5 bits Header 3 bits command
				if ( chk_ff_0  == 0xff )		// obere 5 bytes = Header
				{
					if (rx_buffer.data[rx_buffer.ind_read] == 0xff)
					{
						if (rx_buffer.data[rx_buffer.ind_read_1] == (uint8_t)~rx_buffer.data[rx_buffer.ind_read_2] )	// cmd
						{
							if (rx_buffer.data[rx_buffer.ind_read_3] == (uint8_t)~rx_buffer.data[rx_buffer.ind_read_4] ) // id
							{
								if (rx_buffer.data[rx_buffer.ind_read_5] == (uint8_t)~rx_buffer.data[rx_buffer.ind_read_6] ) // val
								{
									RX_GetNext(&chk_ff_0); // skip 2. 0xff
									//RX_GetNext(cmd);
									RX_GetNext(&(serial_in->cmd));
									RX_GetNext(&chk_ff_0); // skip !cmd
									RX_GetNext(&(serial_in->id));
									RX_GetNext(&chk_ff_0); // skip !id
									RX_GetNext(&(serial_in->val));
									RX_GetNext(&chk_ff_0); // skip !val
									return 0;	//gültige Werte !
								} // val = !val								
							} // id = !id			
						}	// cmd = ! cmd		
					}	// byte 1 = 0xff					
				} // f byte 0 = 0xff
			} // if getnext			
		} // if 8 bytes
		return RX_BUFFER_EMPTY;
	}
#endif
	// --- end : PROTCOLL INVERT_8 = 0xff 0xff cmd ^cmd id ^id val ^val


#define TX_BUFFER_SIZE 64
#define TX_BUFFER_MASK ( TX_BUFFER_SIZE-1 )
#define TX_BUFFER_EMPTY 1

struct USART_TX_buffer
{
uint8_t data[TX_BUFFER_SIZE];		// data bytes
uint8_t ind_read;					// actual read position
uint8_t ind_write;					// actual write position
} tx_buffer = {{},0,0};


// USART Sender interrupt service routine
ISR(USART0_TX_vect)
{
	if (tx_buffer.ind_write != tx_buffer.ind_read) // buffer not empty
	{
		UDR0 = tx_buffer.data[tx_buffer.ind_read];						// nächstes byte holen
		tx_buffer.ind_read = ( tx_buffer.ind_read+1) & TX_BUFFER_MASK;	// inc buffer read
	}
	else
	{
		IRcomm_off(); // TX 16 kHz Auschalten
		IRoff();
	}
}

uint8_t TX_BufferEmpty()
{
	return (tx_buffer.ind_write == tx_buffer.ind_read);
}


void TX_SendChar(uint8_t c)
{
	uint8_t next = ( tx_buffer.ind_write+1 ) & TX_BUFFER_MASK;
	if (next == tx_buffer.ind_read) return;
	
	cli();	// interruptssperren
	IRcomm_on();
	IRon();
	if ((tx_buffer.ind_write != tx_buffer.ind_read) || ((UCSR0A & (1<<UDRE0) )==0))	// Wenn daten im Buffer oder in RS232 dann an nächste Bufferstelle schrieben
	{
		tx_buffer.data[tx_buffer.ind_write]=c;
		tx_buffer.ind_write = ( tx_buffer.ind_write+1 ) & TX_BUFFER_MASK;
	}
	else UDR0 = c;	// sonst direkt in RS232 schreiben (wenn leer)
	sei();	// interrupts wieder erlauben
}

#ifdef  PROTCOLL_CHECKSUM_4
// --- Protokoll 4-Bytes 
void TX_SendCommand(uint8_t cmd, uint8_t id, uint8_t val)
{
	TX_SendChar(cmd);
	TX_SendChar(id);
	TX_SendChar(val);
	TX_SendChar(cmd ^ id ^ val);
			
	while ((tx_buffer.ind_write != tx_buffer.ind_read) || ((UCSR0A & (1<<UDRE0) )==0)) {}	// warte bis alls leer
}
#endif 
#ifdef  PROTCOLL_INVERT_8
// --- Protokoll 4-Bytes
void TX_SendCommand(uint8_t cmd, uint8_t id, uint8_t val)
{
	TX_SendChar(0xff);
	TX_SendChar(0xff);
	TX_SendChar(cmd);
	TX_SendChar(~cmd);
	TX_SendChar(id);
	TX_SendChar(~id);
	TX_SendChar(val);
	TX_SendChar(~val);
	
	while ((tx_buffer.ind_write != tx_buffer.ind_read) || ((UCSR0A & (1<<UDRE0) )==0)) {}	// warte bis alls leer
}
#endif

#ifdef DEBUG_STUDIO
void my_delay(uint16_t __ms)
{
}
#else
void my_delay(uint16_t __ms)
{
	uint16_t __count;
	
	uint16_t _ticks  = (uint16_t) (__ms);
	while(_ticks)
	{
		//my_delay_loop_2(_count2);
		__count = 3920;
		__asm__ volatile (
		"1: sbiw %0,1" "\n\t"
		"brne 1b"
		: "=w" (__count)
		: "0" (__count)
		);
		_ticks --;
	}
}
#endif 


