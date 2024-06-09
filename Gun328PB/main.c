/*
 * GunLight328PB.c
 *
 * Created: 22/07/2022 17:01:22
 * Author : Katharina Böhm-Klamt
 */ 




#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <math.h>

#include "i2cmaster.h"
#include "LCD.h"
#include "RS232.h"
#include "Commands.h"

#include "GS_BaseSoft.h"

// define PORTs

#define GUN_RUMBLE 7
#define Continious 5
#define Burst	   6
#define Trigger	   1
#define	muzzelflash 0;

void shot(void);
void triggershotburst(void);
void magazinchangePIN(void);

 void init_Register(){
cli();

	//PORT B initialization
	
	PORTB = 0x00;
	DDRB  =0x04;
	//PORTB |=(1<<PB1);
	//PORTB |=(1<<PB2);
	// Pins :				0 0 0 0 O I O 1
	//                      7 6 5 4 3 2 1 0
	// PB7 = XTAL ----------^ | | | | | | |
	// PB6 = XTAL ------------^ | | | | | |
	// PB5 = SCK  --------------^ | | | | |
	// PB4 = MISO ----------------^ | | | |
	// PB3 = MOSI-------------------^ | | |
	// PB2 = MOD_TXD -----------------^ | |
	// PB1 = Trigger--------------------^ |
	// PB0 = led7-------------------------^
		
		
	  
	 // Port C initialization
	 PORTC= 0x00; //
	 DDRC = 0x38; // Pins :  0 O 1 1 1 0 O O
	 //                      7 6 5 4 3 2 1 0
	 // PC7 = ---------------^ | | | | | | |
	 // PC6 = -----------------^ | | | | | |
	 // PC5 = SCL LCD -----------^ | | | | |
	 // PC4 = SDA LCD--.-----------^ | | | |
	 // PC3 = LED_GREEN--------------^ | | |
	 // PC2 = -------------------------^ | |
	 // PC1 = Magazin--------------------^ |
	 // PC0 = Magazin y/n------------------^
	 
	 
	 // Port D initialization
	 PORTD=0x00;

	 DDRD=0x92;  // Pins :   1 O O 1 O O 1 0
	 //                      7 6 5 4 3 2 1 0
	 // PD7 = Rumble---------^ | | | | | | |
	 // PD6 = Burst------------^ | | | | | |
	 // PD5 = Continious---------^ | | | | |
	 // PD4 = Speak----------------^ | | | |
	 // PD3 = BAT_Alert--------------^ | | |
	 // PD2 = -------------------------^ | |
	 // PD1 = TX USART Out---------------^ |
	 // PD0 = RX USART In -----------------^
	 
	 // PORT E initialization
	 PORTE=0x00;
	 DDRE = 0x00;


	 
	 TCCR1B = (1<<WGM12)  |(1<<CS10); //
	 TCCR1A |= (1<<COM1B0);

	 OCR1A=230;			
	
	TCCR2B=0x03;
	TCNT2 =0x00;
	//	OCR2 =30;
	TIMSK2 = (1<<TOIE2);
	


	soft_s_zeiten[0].ms_max = 5;
	soft_s_zeiten[0].fsync = triggershotburst;

	// timer 1 = led-alternate display
	soft_s_zeiten[1].ms_max = 20;
	soft_s_zeiten[1].fasync = magazinchangePIN;
	
	soft_s_zeiten[2].ms_max = 99;
	soft_s_zeiten[2].fasync = shot;

	soft_enable_fsync	= 0b00000001;
	soft_enable_fasync	= 0b00000110;
	soft_enable			= 0b00000111;

 UCSR0A=0x00;
 UCSR0B=0xD8;
 UCSR0C=0x86;
 UBRR0H =BAUD_H;
 UBRR0L=BAUD_L;
	 
	 	SPCR0 = 0x00;
	
	 
sei();
 }
 
 //****************************************************************************************************
 //					BAT ALert
 //****************************************************************************************************
#define PRESSED_BATALERT (1<<PD3)

#define LC709203F_I2CADDR_DEFAULT 0x0B     // i2c address
#define LC709203F_CMD_THERMISTORB 0x06     // Read/write thermistor B
#define LC709203F_CMD_INITRSOC 0x07        // Initialize RSOC calculation
#define LC709203F_CMD_CELLTEMPERATURE 0x08 // Read/write batt temp
#define LC709203F_CMD_CELLVOLTAGE 0x09     // Read batt voltage
#define LC709203F_CMD_APA 0x0B             // APA
#define LC709203F_CMD_RSOC 0x0D            // Read state of charge
#define LC709203F_CMD_CELLITE 0x0F         // Read batt indicator to empty
#define LC709203F_CMD_ICVERSION 0x11       // Read IC version
#define LC709203F_CMD_BATTPROF 0x12        // Set the battery profile
#define LC709203F_CMD_ALARMRSOC 0x13       // Alarm on percent 
#define LC709203F_CMD_ALARMVOLT 0x14       // Alarm on voltage 
#define LC709203F_CMD_POWERMODE 0x15       // Sets sleep/power 
#define LC709203F_CMD_STATUSBIT 0x16       // Temp obtaining method
#define LC709203F_CMD_PARAMETER 0x1A       // Batt profile code


/*!  Battery temperature source */
typedef enum {
	LC709203F_TEMPERATURE_I2C = 0x0000,
	LC709203F_TEMPERATURE_THERMISTOR = 0x0001,
} lc709203_tempmode_t;

/*!  Chip power state */
typedef enum {
	LC709203F_POWER_OPERATE = 0x0001,
	LC709203F_POWER_SLEEP = 0x0002,
} lc709203_powermode_t;

/*!  Approx battery pack size */
typedef enum {
	LC709203F_APA_100MAH = 0x08,
	LC709203F_APA_200MAH = 0x0B,
	LC709203F_APA_500MAH = 0x10,
	LC709203F_APA_1000MAH = 0x19,
	LC709203F_APA_2000MAH = 0x2D,
	LC709203F_APA_3000MAH = 0x36,
} lc709203_adjustment_t;

 /************************crc8 checksum*************/
static uint8_t crc8(uint8_t*databuffer, uint8_t len)
{
	uint8_t crc = 0x00;
	uint8_t remainder = 0; 
	uint8_t bit = 0;
	uint8_t byte= 0;
	
	for(byte =0; byte<len; ++byte)
	{
		remainder ^= (databuffer[byte]<< ((8* sizeof(crc))-8));
		
		for(bit = 8; bit>0; --bit)
		{
			if(remainder & (1<< ((8*sizeof(crc))-1)))
			{
				remainder = (remainder <<1) ^0x07;
			}
			else 
			{
				remainder = (remainder<<1);
			}
		}
	}
	return (remainder);
}


 /***********************write baterie chip*************/

void i2cBatteriewatch(uint8_t Komando, uint8_t high, uint8_t low){
	uint8_t fr[4]={ LC709203F_I2CADDR_DEFAULT*2, Komando, low, high};
		//i2c_start_wait(LC709203F_I2CADDR_DEFAULT*2);
 	unsigned char ret =i2c_start(0x16);
 	if (ret)
 	{i2c_stop();}
	else	{
		
			i2c_write(Komando);
			i2c_write(low);
			i2c_write(high);		
			i2c_write(crc8(fr,4));		
			i2c_stop();
			
		
		}
}

 /************************read batteriechip*************/

static readbatterie(uint8_t Komando, uint16_t data1){
	uint8_t re [6] ={0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	re[0] = 0x16;
	re [1]= Komando;
	re[2] = re[0] | 0x1; ;
	i2c_start(re[0]);
	
	i2c_write(re[1]);
	i2c_start(re[2]);
	
	re[3]=i2c_readAck();
	re[4]=i2c_readAck();
	re[5]=i2c_readNak();
	
	uint8_t crc = crc8(re, 5);
	
	data1 = re[4];
	data1 = data1<<=8;
	data1|=re[3];
	i2c_stop();
	return data1;
	
}
 /************************setup baterie*************/
void setup_bat()
{
	i2cBatteriewatch(LC709203F_CMD_POWERMODE,0x00, 0x01);					//powermode
	//i2cBatteriewatch(LC709203F_CMD_APA, 0x00,0x36);							// set APA
	i2cBatteriewatch(LC709203F_CMD_APA, 0x42,0x42);							// set APA
	i2cBatteriewatch(LC709203F_CMD_BATTPROF,0x00,0x00);						//bat profil
	i2cBatteriewatch(LC709203F_CMD_STATUSBIT,0x00,0x00);					//statusbit set 0x00 i2c mode 0x01 constant of thermistor
	i2cBatteriewatch(LC709203F_CMD_CELLTEMPERATURE,0x0B, 0xA6);
	i2cBatteriewatch(LC709203F_CMD_ALARMVOLT,0x0B,0xB8);					// 3.000 Volt
}
//****************************************************************************************************
//					I2C/TWI/LCD
//****************************************************************************************************

#define DevSSD1306  0x78    // device address of SSD1306 OLED, uses 8-bit address (0x3c OX3D)78!!!
void setup_i2c(){

	
	i2c_write(0x80);
	i2c_write(0xAE);                    // Display Off

	i2c_write(0x00 | 0x0);            // low col = 0
	i2c_write(0x10 | 0x0);           // hi col = 0
	i2c_write(0x40 | 0x0);            // line #0

	i2c_write(0x80);
	i2c_write(0x81);                   // Set Contrast 0x81
	i2c_write(0xFF);					//je höher desto höher Kontrast
	
	// flips display
	i2c_write(0xA1);                    // Segremap - 0xA1
	
	i2c_write(0x80);
	i2c_write(0xC8);                    // COMSCAN DEC 0xC8 C0
	i2c_write(0xA6);                    // Normal Display 0xA6 (Invert A7)
	
	//	i2c_write(0xA4);                // DISPLAY ALL ON RESUME - 0xA4
	i2c_write(0xA8);                    // Set Multiplex 0xA8
	i2c_write(0x3F);                    // 1/64 Duty Cycle

	i2c_write(0xD3);                    // Set Display Offset 0xD3
	i2c_write(0x00);                     // no offset

	i2c_write(0xD5);                    // Set Display Clk Div 0xD5
	i2c_write(0x80);                    // Recommneded resistor ratio 0x80


	i2c_write(0xD9);                  // Set Precharge 0xd9
	i2c_write(0x11);

	i2c_write(0xDA);                    // Set COM Pins0xDA
	
	i2c_write(0x12);

	i2c_write(0xDB);                 // Set VCOM Detect - 0xDB
	i2c_write(0x40);

	
	
	i2c_write(0x20);                    // Set Memory Addressing Mode
	i2c_write(0x02);                    // 0x00 - Horizontal

	//	i2c_write(0x40 | 0x0);              // Set start line at line 0 - 0x40

	i2c_write(0x8D);                    // Charge Pump -0x8D
	i2c_write(0x14);

	//	i2c_write(0xA4);              //--turn on all pixels - A5. Regular mode A4
	i2c_write(0xAF); 
	//i2c_write(0xA5);               //--turn on oled panel - AF
}

// Rumble
#define gun_rumble_on()  PORTD |= (1<<PD7)
#define gun_rumble_off() PORTD &= ~(1<<PD7)

//muzzelflash
#define  muzzelflah_on() PORTB |= (1<<PB0);
#define	 muzzelflash_off() PORTB &=~(1<<PB0);
// Trigger
uint16_t triggerpressed = 0;
uint8_t	 wait_fire_release = 0;  // if set fire button has to be relesed befor next fire
uint8_t  burst_counter = 0;
uint8_t  contburstquestion =0;
#define MAX_BURST_COUNT 3-2					// # Schuss -2 (0, >)
#define TRIGGERPRESSED    (1<<PB1)
#define PRESSED_SINGLE (1<<PD5)
#define PRESSED_BURST     (1<<PD6)
#define PRESSED_MASK      ((1<<PD5) | (1<<PD6))
#define	BATALERT		  (1<<PD3)

//Magazin

#define PRESSED_MAGAZIN (1<<PC1)
#define PRESSED_MAGAZINSTART (1<<PC0)

// Gamesettings -- EEPROM , als Block

 struct S_SETTINGS
{
	uint16_t max_ammo;		//maximal shots
	uint8_t  PlayerNr;		//PlayerNumber
	uint8_t	 Delay               ;
}  E_gamesettings EEMEM   = {0,1,5};
	
struct S_SETTINGS gamesettings;



	// State definiition
	uint8_t     current_state;		// aktueller Zustand
	#define     STATE_ADMIN	0x00	// admin modus
	#define		STATE_GAME	0x01	// normaler Spielmodus
	#define		STATE_OUT	0x02	// gun gesperrt (keien Mun oder keien Energy)

	uint16_t munition         ;		
	uint16_t tristatefire	  ;
	uint8_t  magazinsize =40  ;		
	uint8_t	 magazin1	=3	  ;		
	uint8_t	restmagazin		  ;		
	uint8_t  magazinnumber	  ;		
	uint8_t	 restlasmagazin   ;		
	uint8_t	 helpmagazin	=0;		
	uint8_t  magazinchange    ; 
	uint8_t	 delay            ;		
	uint8_t  timeout_time  = 0;		
	uint8_t  energy			  ;
	uint16_t munitionbefore	  ;
	uint16_t batalertisr = 0  ;		//help in int1 state of bat ..
	uint16_t helpbat	= 0	  ;		//helpbat alert placeholder for displaing
	uint16_t baterievoltage=31;
//****************************************************************************************************
//					EEPORM /Settings read write 
//****************************************************************************************************
// reads / write STRUCT gamesettings to / from EEPORM

//----------------------------------------------------------------------------------------------------
void Write_EEPROM()
{
	cli();
	eeprom_write_block(&gamesettings,&E_gamesettings,sizeof(gamesettings));
	sei();			
}

//----------------------------------------------------------------------------------------------------
void Read_EEPROM()
{
	cli();
	eeprom_read_block(&gamesettings,&E_gamesettings,sizeof(gamesettings));
	sei();
	munition = gamesettings.max_ammo;	
}

void Init_Gameset()
{

	#ifdef VER_EEPROM
	cli();
	eeprom_read_block(&gamesettings,&E_gamesettings,sizeof(gamesettings));
	sei();
	#endif
	munition = gamesettings.max_ammo;
}

int main(void)
{
	init_Register();
	i2c_init();
	my_delay(50);

	 	
restart:
	 
	 Read_EEPROM();
	 RX_ClearBuffer();
	 i2c_init();
	 my_delay(50);
	 cleardisplay();
	 my_delay(200);
	 setup_bat();
	 magazin1=3;
	 current_state=STATE_GAME;
	 playerwriten();
	 my_delay(250);
	 digitdraw(gamesettings.PlayerNr);                 
	/* my_delay(500);
	 batalert();
	 my_delay(250);
	 batalertisr= readbatterie(0x09,0x00)/100;
	 digitdraw(batalertisr);*/
	 my_delay(1000);
	 
	
 /************************as long no magazin he blinking 999*************/

	 while ((magazinchange = PINC & PRESSED_MAGAZIN) == 2)
	 {
		 digitdraw(999);
		 my_delay(500);
	 }
	 
/************************asking if with or without magazin*************/
	

	
		if(PINC & PRESSED_MAGAZINSTART){
			magazin1=0;
		}
		else{magazin1=1; }
		
	
	 
	 helpmagazin =0; 
		if (magazin1 == 1)
		{
			restlasmagazin = munition- (munition/magazinsize)*(magazinsize);
			if (restlasmagazin > 0)
			{ magazinnumber=(munition/magazinsize)+1;
				if(magazinnumber<2){
					digitdraw(restmagazin=restlasmagazin);
					magazin(magazinnumber);							}
				else
				{
		 			digitdraw(magazinsize);
					magazin(magazinnumber);
					restmagazin=40;
				}
			}
			else
			{magazinnumber=munition/magazinsize;
				if(magazinnumber==0){digitdraw(0); magazin(magazinnumber);}
				else
				{
					digitdraw(magazinsize);
					magazin(magazinnumber);
					restmagazin=40;
				}
			}
			
		}
		else {digitdraw(munition);}
	
	my_delay(1000);
	if (munition == 0)
	{
				digitdraw(0);

	}
	/*************** Main while loop **********************************/
	 
    while (1) 
    {


		if(magazin1==0 || magazin1==1)
		{
			soft_chk_do();  //starting softwaretimer
		/*	
			if(batalertisr<baterievoltage) // batalert ´questioning
			{
				 batalert();
				 if(magazin1==1){magazin(magazinnumber);}
				batalertisr=baterievoltage;
				helpbat = 3; 
			}
		*/
			
			if (!RX_GetNextCommand(&serial_in))
			{
				switch (serial_in.cmd)
				{
							
					case IRCMD_STOG_INFO:							// send by shield if timeout
					{
						if (current_state & (STATE_GAME|STATE_OUT))
						{
							if (serial_in.id== gamesettings.PlayerNr)
							{
								if(serial_in.val)
								{
									if (serial_in.val==0x80)
									{
										cleardisplay();
										out();
										my_delay(500);
										if(energy>0) {energy=0;}
										
									}
									else
									{
										timeout_time = (serial_in.val & 0xF7);
										digitdraw(timeout_time | 0x4000);
									}
									current_state=STATE_OUT;
								}
								if(serial_in.val==0x00)
								{ 
									//if (current_state== STATE_GAME)
									//{
										if(munition>0 || (restmagazin>0 && magazinnumber>0))
										{
											if(magazin1==1)
											{
												digitdraw(restmagazin);magazin(magazinnumber);
											}
											if(magazin1==0)
											{
											digitdraw(munition);
												
											}
										/*if(helpbat ==3)
										{	batalert();
										}	*/
										if(energy>0)
										{heart(energy);}
										current_state=STATE_GAME;	
										}
								}
							}
						}
					}
					break;
					
					case  IRCMD_INVULNERABLE:
						if( current_state & (STATE_GAME |STATE_OUT))
						{
							if (serial_in.id== gamesettings.PlayerNr)
							{
								if(current_state==STATE_GAME)
								{
										if(serial_in.val>0)
										{	
											//if(helpbat ==3){batalert();	}
											if(magazin1==1){magazin(magazinnumber);}
											energy=serial_in.val;
											heart(serial_in.val);									
										}
										else
										{
											energy=0;
											out();
											current_state = STATE_OUT;
										}								
								}
								else 
								{
									if (serial_in.val>0)
									{	
									//	if(helpbat ==3){batalert();	}
										if(magazin1==1){magazin(magazinnumber);}
										energy=serial_in.val;
										heart(serial_in.val);
										if (munition) current_state = STATE_GAME;
									}
									else
									{
										energy=0;
										out();
										current_state = STATE_OUT;
									}
								}
									
							}
								
						
								
						}
						break;
							
						case IRCMD_FIRE:
						{
							if (current_state==STATE_GAME && ((magazinnumber>0 && restmagazin>0)|| munition>0))
							{	if (serial_in.id != gamesettings.PlayerNr && serial_in.val==IRCMD_NOP)
								{
									if(munition > 0)
									{
										gun_rumble_on();
										my_delay(100);
										gun_rumble_off();
										weaponhit();
										current_state=STATE_OUT;
										my_delay(gamesettings.Delay*1000);
										if(magazin1==1)
										{
											digitdraw(restmagazin);
											magazin(magazinnumber);
										}
										else
										{
											digitdraw(munition);
										}
										if(energy>0)
											{heart(energy);
											}
										current_state=STATE_GAME;
										RX_ClearBuffer();
									}
								}
							}
								
						}
						break;
									
						case IRCMD_REFILL_GUN_AMMO:	// 2 bytes (id hi, val low)
							{
								munition = serial_in.id;
								munition = munition << 8;
								munition += serial_in.val;
								
								if ( current_state & (STATE_GAME | STATE_OUT ) )
								{
									
									if (munition)	// mun=0 : reload old setting
									{
										//gamesettings.max_ammo=munition; // 99;
										if (magazin1 == 1)
										{
											restlasmagazin = munition-(munition/magazinsize)*(magazinsize);
											if (restlasmagazin > 0)
											{ magazinnumber=(munition/magazinsize)+1;
												if(magazinnumber<2){
													digitdraw(restmagazin=restlasmagazin);
													//if(helpbat ==3){batalert();	}
													magazin(magazinnumber);
												}
												else
												{
													digitdraw(magazinsize);
												//	if(helpbat ==3){batalert();	}
													magazin(magazinnumber);
													restmagazin=40;
												}
											}
											else
											{magazinnumber=munition/magazinsize;
												if(magazinnumber==0){digitdraw(0); magazin(magazinnumber);}
												else
												{
													digitdraw(magazinsize);
												//	if(helpbat ==3){batalert();	}
													magazin(magazinnumber);
													restmagazin=40;
												}
											}
											helpmagazin=0;
										}
										else
										{	digitdraw(munition);
										}
									}
									else if(munition==0)
									{
										//munition=gamesettings.max_ammo;
										if (magazin1 == 1)
										{
											restlasmagazin =munition- (munition/magazinsize)*(magazinsize);
											if (restlasmagazin > 0)
											{ magazinnumber=(munition/magazinsize)+1;
												if(magazinnumber<2){
													digitdraw(restmagazin=restlasmagazin);
												magazin(magazinnumber);		}
												else
												{
													digitdraw(magazinsize);
												//	if(helpbat ==3){batalert();	}
													magazin(magazinnumber);
													restmagazin=40;
												}
											}
											else
											{magazinnumber=munition/magazinsize;
												if(magazinnumber==0){digitdraw(0); magazin(magazinnumber);}
												else
												{
													digitdraw(magazinsize);
													magazin(magazinnumber);
													restmagazin=40;
												}
											}
											helpmagazin=0;
										}
										else
										{
											digitdraw(munition);
										}
									}
									
									if(energy>0)
									{heart(energy);}
									current_state=STATE_GAME;
								}
								else
								{
									if ( current_state == STATE_ADMIN )
									{
										gamesettings.max_ammo = munition;
										cleardisplay();
										digitdraw(gamesettings.max_ammo);
										if(energy>0)
										{heart(energy);}
									}
								}
							}
							break;
							
							case IRCMD_REFILL_BOTH_GUN:
							{
								munition = serial_in.id;
								munition = munition << 8;
								munition += serial_in.val;
								
								if ( current_state & (STATE_GAME | STATE_OUT ) )
								{
									
									if (munition)	// mun=0 : reload old setting
									{ 
										//gamesettings.max_ammo=munition; // 99;
										if (magazin1 == 1)
										{
											restlasmagazin = munition-(munition/magazinsize)*(magazinsize);
											if (restlasmagazin > 0)
											{ magazinnumber=(munition/magazinsize)+1;
												if(magazinnumber<2){
													digitdraw(restmagazin=restlasmagazin);
													magazin(magazinnumber);
												}
												else
												{
													digitdraw(magazinsize);
													magazin(magazinnumber);
													restmagazin=40;										
												}
											}
											else
											{magazinnumber=munition/magazinsize;
												if(magazinnumber==0){digitdraw(0); magazin(magazinnumber);}
												else
												{
													digitdraw(magazinsize);
													magazin(magazinnumber);
													restmagazin=40;
												}
											}
											helpmagazin=0;
										}
										else 
										{	digitdraw(munition);
										}
									}
									else if(munition ==0)
									{
										//munition=gamesettings.max_ammo;
										if (magazin1 == 1)
										{
											restlasmagazin =munition- (munition/magazinsize)*(magazinsize);
											if (restlasmagazin > 0)
											{ magazinnumber=(munition/magazinsize)+1;
												if(magazinnumber<2){
													digitdraw(restmagazin=restlasmagazin);
											//		if(helpbat ==3){batalert();	}
													magazin(magazinnumber);		}
												else
												{
													digitdraw(magazinsize);
													magazin(magazinnumber);
													restmagazin=40;
												}
											}
											else
											{magazinnumber=munition/magazinsize;
												if(magazinnumber==0){digitdraw(0); magazin(magazinnumber);}
												else
												{
													digitdraw(magazinsize);
													magazin(magazinnumber);
													restmagazin=40;
												}
											}
											helpmagazin=0;
										}
										else 
										{
											digitdraw(munition);
										}
									}
									
									if(energy>0)
									{heart(energy);}
									current_state=STATE_GAME;
								}
								else
								{
									if ( current_state == STATE_ADMIN )
									{
										gamesettings.max_ammo = munition;
										cleardisplay();
										digitdraw(gamesettings.max_ammo);
										if(energy>0)
										{heart(energy);}
									}
								}
							}
							break;
							
							
							case IRCMD_SET_ADMIN:
							if (serial_in.id==IRCMD_NOP)
							{
								if ( (serial_in.val == IRVAL_ADMIN))
								{
									current_state =  STATE_ADMIN;
									cleardisplay();
									admin();
									my_delay(1000);
								
								}
								else
								if (!(serial_in.val & IRVAL_ADMIN) && (current_state==STATE_ADMIN))
								{
										//digitdraw(munition);
										heart(energy);
										my_delay(1000);
										current_state =(1<<STATE_GAME);
								}
							}
							break;
							
							
							// nur im ADMIN Mode
							#pragma region SIGNAL AMDIN
							
							case IRCMD_HIT_GUN:
								if(serial_in.id == serial_in.val)
								{
									gamesettings.Delay = serial_in.id;
									digitdraw(gamesettings.Delay);
									my_delay(500);
									
								}
							break;
							
							case IRCMD_SET_PLAYER:			// player Nr setzen
								//if (current_state == (1<<STATE_ADMIN) )
								if (serial_in.id == serial_in.val)
								{
									gamesettings.PlayerNr = serial_in.id;
									cleardisplay();
									playerwriten();		  my_delay(250);
									digitdraw(gamesettings.PlayerNr); my_delay(500);
									
								}
							break;
							
							case IRCMD_EEPROM_BURN:			// Setze Player - nur wenn id==val!
							//	if (current_state ==  (1<<STATE_ADMIN) )
								if (serial_in.id == serial_in.val && serial_in.val == IRCMD_NOP)
								{
									Write_EEPROM();
									my_delay(500);
									cleardisplay();
									admin();
									my_delay(500);
									goto restart;
								}
							break;
				  
							
							#pragma endregion SIGNAL ADMIN
							
						}
				}
		}
    }
}
void triggershotburst(void)
{
	triggerpressed = (PINB & TRIGGERPRESSED);
	contburstquestion = (PIND & PRESSED_MASK);
	tristatefire = triggerpressed+contburstquestion;

	

}
void shot(void){
	if ((current_state == STATE_OUT) && timeout_time)
	{
		timeout_time--;
		{
			if (timeout_time == 0)	// reaktiviren
			{
				digitdraw(munition);
				current_state=STATE_GAME;
			}
			else
			{
				digitdraw(timeout_time | 0x4000);
			}
		}
	}
	
	/*if(helpbat==0)	{
	batalertisr= readbatterie(0x09,0x00)/100;}*/
	
	if(magazin1==0)    //we 
		{
			if(triggerpressed & TRIGGERPRESSED)
			{
				
				if (!wait_fire_release && (current_state == STATE_GAME) && munition &&energy)
				{
					TX_SendCommand(IRCMD_FIRE,gamesettings.PlayerNr,IRCMD_NOP);
					// rumble on
					gun_rumble_on();
					muzzelflah_on(); 
					displayoff();
				
					munitionbefore=munition;
					munition= munition-1;
				
				
					switch (tristatefire)
					{
						case 34 :
						wait_fire_release =1;
						break;
					
						case 98:
						wait_fire_release = 0;		// permanent fire no release necessary
						break;
					
						case 66:
						if ( burst_counter++ > MAX_BURST_COUNT )
						{
							wait_fire_release = 1;
							burst_counter = 0;
						}
						break;
					
					} // switch
				
				} // if munition
				else
				{
					muzzelflash_off();
					
					if(munitionbefore>munition){
						digitdraw(munition);
						if(energy>0)
						{heart(energy);}
						munitionbefore=munition;
					}
					
					if(munition==0) {strich();}
					gun_rumble_off();

				}
				
			}
			else
			{
				gun_rumble_off();
				muzzelflash_off();
				if(munitionbefore>munition){
					digitdraw(munition);
		
				
				if(energy>0)
				{heart(energy);}
					munitionbefore=munition;
				}	if(munition==0) {strich();}
				burst_counter =0;
				wait_fire_release = 0;
				
			}
		}
		
		else
		{
			if(triggerpressed & TRIGGERPRESSED)
			{
				if(!wait_fire_release && (current_state == STATE_GAME) && restmagazin && magazinnumber &&energy)	
				{
					
					TX_SendCommand(IRCMD_FIRE,gamesettings.PlayerNr,IRCMD_NOP);
					displayoff();
					gun_rumble_on();
					muzzelflah_on(); 
					munitionbefore=restmagazin;
					restmagazin = restmagazin-1;
				
						switch (tristatefire)
						{
							
							case 34 :
							wait_fire_release =1;
							break;
						
							case 98:
							wait_fire_release = 0;		// permanent fire no release necessary
							break;
						
							case 66:
							if ( burst_counter++ > MAX_BURST_COUNT )
							{
								wait_fire_release = 1;
								burst_counter = 0;
							}
							break;
						
						} // switch
				}
				else
				{
					muzzelflash_off();
					if((munitionbefore>restmagazin)){
						digitdraw(restmagazin);
						magazin(magazinnumber);
						if(energy>0)
						{heart(energy);}
						munitionbefore=restmagazin;
					}
					gun_rumble_off(); 
				}
			}
			else
			{
			
				if((munitionbefore>restmagazin)){
					digitdraw(restmagazin);
					magazin(magazinnumber);
					if(energy>0)
					{heart(energy);}
					munitionbefore=restmagazin;
				}
				if((restmagazin==0) && (magazinnumber==0)) {strich();}
				burst_counter =0;
				wait_fire_release = 0;
				gun_rumble_off();
				muzzelflash_off();
			
			}
		}
	
}


void magazinchangePIN (void) {//free tu use 
	if(current_state & (STATE_GAME | STATE_OUT ))
		{
			magazinchange = PINC & PRESSED_MAGAZIN;
			if ((magazinnumber>0) && (current_state=STATE_GAME))
			{		if(magazinchange==0 && helpmagazin==0){ helpmagazin=1; }	//magazin in
			if(magazinchange==2 && helpmagazin ==1){ helpmagazin=2; }  //magazin out
			if((magazinchange==0) && helpmagazin==2)					//magazin in
			{
				magazinnumber--;
				if(magazinnumber>0 && restlasmagazin==0 )
				{	restmagazin=40;
					digitdraw(restmagazin);
					magazin(magazinnumber);
					if(energy>0)
					{heart(energy);}

				}
				if(magazinnumber>1 && restlasmagazin>0){
					restmagazin=40;
					
					digitdraw(restmagazin);
					magazin(magazinnumber);
					if(energy>0)
					{heart(energy);}
					
				}
				if((magazinnumber ==1)&&(restlasmagazin>0))
				{	restmagazin=restlasmagazin;
					digitdraw(restmagazin);
					magazin(magazinnumber);if(energy>0)
					{heart(energy);}
				}
				if (magazinnumber==0)
				{
					digitdraw(0);
					strich();
				}
				
				helpmagazin=0;
			}
		}
	}
	

}



/*
ISR(INT1_vect){
//	batalertisr =1;
}*/