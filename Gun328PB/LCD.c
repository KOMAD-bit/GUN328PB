/*
 * LCD.c
 *
 * Created: 22/07/2022 17:03:11
 *  Author: Katharina Böhm-Klamt
 */ 
#include "LCD.h"
#include "twimaster.h"

#define  DevSSD1306 0x78
//#define DevSSD1306 0x3c

static uint16_t bufferpage [64]={

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	
};

static uint16_t bufferpagec [8] [20]={
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00
};

static uint16_t bufferpagea [64]={

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	
};
static	int Pages [8]={ 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7};

static	uint16_t herzoben  [13]=	{0xE0, 0xF8, 0xFC, 0xFC, 0xF8, 0xF0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFC, 0xF8, 0xE0};
static	uint16_t herzunten [13]=	{0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00};
uint16_t bufferherz[13]=	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00, 0x00};

void drawBufferPage(uint16_t *buffp, int page, int position, int ende, int start){
	unsigned char ret = i2c_start(DevSSD1306+I2C_WRITE);
	    // set device address and write mode+I2C_WRITE

	if ( ret ) { i2c_stop();}
	else
	{

		setup_i2c();
		i2c_write(page);
		i2c_write(position);          // hi col = 0
		i2c_write(0x40 | 0x0);            // line #0
		uint8_t twbrbackup = TWBR0;
		TWBR0 = 10;

		for (uint16_t i=start; i<ende; i++){

			i2c_start_wait(DevSSD1306+I2C_WRITE);   // set device address and write mode
			
			i2c_write(0x40);                        // set display RAM display start line register
			i2c_write(buffp[i]);
			i2c_stop();
			TWBR0 = twbrbackup;
			
		}
		i2c_stop();

	}
}

void cleardisplay(){
	
	for (int i=0 ; i<64;i++){
	bufferpage[i]=0x00;}
	for(int i=0; i<=7; i++){
		drawBufferPage(bufferpage, Pages[i], 0x10, 64,0);
		drawBufferPage(bufferpage, Pages[i], 0x14,64,0);

	}
	
}

void clearbuffer( int x, int y){for (int i=x ; i<y;i++){
bufferpage[i]=0x00;}}

void horizontalline(int x, int y){
	for(int i = x; i<y; i++){
	bufferpage[i]=0xF0;}
}


void verticalline(int x, int y){
	for(int i = x; i<y; i++){
	bufferpage[i]=0xFF;}
}


void position1(int dez, int leftstart, int leftend, int rightstart, int rightend, int fulll, int fullr, int position){
	int pos=0;
	if (position ==0x10)
	{pos=64;
	}
	if (position==0x12)
	{pos=64;
	}
	else
	{pos=44;
	}

	switch (dez)
	{
		case 0:{
			clearbuffer(fulll,fullr);
			horizontalline(leftend,rightstart);
			if(position == 0x15){for (int i=0 ; i<20; i++)
				{
					int j=44+i;
					bufferpage[j]=bufferpagec[3][i];
				}
			}
			drawBufferPage(bufferpage, 0xB2, position, pos, 0);
			if(position== 0x10){
				for (int i=0 ; i<20; i++)
				{bufferpagec[3][i]=bufferpage[i];
				}
			}
			
			
			clearbuffer(fulll,fullr);
			verticalline(leftstart,leftend);
			verticalline(rightstart,rightend);
			for(int i=0xB3; i<=0xB6;i++){
				if(position== 0x10){
					for (int i=0 ; i<20; i++)
					{bufferpagec[4][i]=bufferpage[i];
						bufferpagec[5][i]=bufferpage[i];
						bufferpagec[6][i]=bufferpage[i];
						bufferpagec[6][i]=bufferpage[i];
					}
				}
			drawBufferPage(bufferpage, i, position, pos, 0);}
			
			clearbuffer(fulll,fullr);
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB7, position, pos, 0);

			
			break;
		}
		
		case 1:{
			clearbuffer(fulll,fullr);
			for(int i=0xB2; i<=0xB7;i++){
			drawBufferPage(bufferpage, i, position, pos, 0);}
			
			verticalline(rightstart,rightend);
			for(int i=0xB2; i<=0xB7;i++){
				drawBufferPage(bufferpage, i, position, pos, 0);
			}
		break;}
		
		case 2:{
			
			clearbuffer(fulll,fullr);
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB2, position, pos, 0);

			clearbuffer(fulll,fullr);
			verticalline(rightstart,rightend);
			for(int i=0xB3; i<=0xB4;i++){
			drawBufferPage(bufferpage, i, position, pos, 0);}
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB4, position, pos, 0);
			
			clearbuffer(fulll,fullr);
			verticalline(leftstart,leftend);
			for(int i=0xB5; i<=0xB6;i++){
			drawBufferPage(bufferpage, i, position, pos, 0);}
			
			
			clearbuffer(fulll,fullr);
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB7, position, pos, 0);
		break;}
		
		case 3:{clearbuffer(fulll,fullr);
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB2, position, pos, 0);

			clearbuffer(fulll,fullr);
			verticalline(rightstart,rightend);
			for(int i=0xB3; i<=0xB6;i++){
			drawBufferPage(bufferpage, i, position, pos, 0);}
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB4, position, pos, 0);
			
			clearbuffer(fulll,fullr);
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB7, position, pos, 0);
		break;}
		
		case 4:{
			
			clearbuffer(fulll,fullr);
			drawBufferPage(bufferpage, 0xB7, position, pos, 0);
			verticalline(leftstart,leftend);
			for(int i=0xB2; i<=0xB4;i++){
			drawBufferPage(bufferpage, i, position, pos, 0);}
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB4, position, pos, 0);
			
			clearbuffer(fulll,fullr);
			verticalline(rightstart,rightend);
			for(int i=0xB5; i<=0xB7;i++){
			drawBufferPage(bufferpage, i, position, pos, 0);}
		break;}
		
		case 5:{
			clearbuffer(fulll,fullr);
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB2, position, pos, 0);

			clearbuffer(fulll,fullr);
			verticalline(leftstart,leftend);
			for(int i=0xB3; i<=0xB4;i++){
			drawBufferPage(bufferpage, i, position, pos, 0);}
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB4, position, pos, 0);
			
			clearbuffer(fulll,fullr);
			verticalline(rightstart,rightend);
			for(int i=0xB5; i<=0xB6;i++){
			drawBufferPage(bufferpage, i, position, pos, 0);}
			
			
			clearbuffer(fulll,fullr);
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB7, position, pos, 0);
		break;}
		
		case 6:{
			
			clearbuffer(fulll,fullr);
			for(int i=0xB2; i<=0xB7;i++){
			drawBufferPage(bufferpage, i,position, pos, 0);}
			
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB2, position, pos, 0);

			clearbuffer(fulll,fullr);
			verticalline(leftstart,leftend);
			for(int i=0xB3; i<=0xB6;i++){
			drawBufferPage(bufferpage, i, position, pos, 0);}
			
			//clearbuffer(0,fullr);
			verticalline(rightstart,rightend);
			for(int i=0xB5; i<=0xB6;i++){
			drawBufferPage(bufferpage, i, position, pos, 0);}
			
			clearbuffer(fulll,fullr);
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB4, position, pos, 0);
			
			clearbuffer(fulll,fullr);
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB7, position, pos, 0);
		break;}
		
		case 7:{
			
			clearbuffer(fulll,fullr);
			drawBufferPage(bufferpage, 0xB7, position, pos, 0);
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB2, position, pos, 0);
			clearbuffer(fulll,fullr);
			verticalline(rightstart,rightend);
			for(int i=0xB3; i<=0xB7;i++){
				drawBufferPage(bufferpage, i, position, pos, 0);
			}
			

		break;	}
		
		case 8:{
			clearbuffer(fulll,fullr);
			
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB2, position, pos, 0);
			clearbuffer(fulll,fullr);
			verticalline(leftstart,leftend);
			verticalline(rightstart,rightend);
			for(int i=0xB3; i<=0xB6;i++){
				drawBufferPage(bufferpage, i, position, pos, 0);
			}
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB4, position, pos, 0);
			clearbuffer(fulll,fullr);
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB7, position, pos, 0);
			break;
		}
		
		case 9:{
			clearbuffer(fulll,fullr);
			for(int i=0xB2; i<=0xB7;i++){
			drawBufferPage(bufferpage, i, position, pos, 0);}
			
			clearbuffer(fulll,fullr);
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB2, position, pos, 0);
			
			clearbuffer(fulll,fullr);
			verticalline(rightstart,rightend);
			for(int i=0xB3; i<=0xB6;i++){
				drawBufferPage(bufferpage, i, position, pos, 0);
			}
			verticalline(leftstart,leftend);
			for(int i=0xB3; i<=0xB4;i++){
				drawBufferPage(bufferpage, i, position, pos, 0);
			}
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB4, position, pos, 0);
			
			clearbuffer(fulll,fullr);
			horizontalline(leftend,rightstart);
			drawBufferPage(bufferpage, 0xB7, position, pos, 0);
			break;
		}
	}
}

void digitdraw(int munitionenergy){
	int hunni = munitionenergy/100;
	int zehner = (munitionenergy-hunni*100)/10;
	int einer = ((munitionenergy-hunni*100)-zehner*10);
	
	if (munitionenergy>99)
	{
		cleardisplay();
		position1(hunni,0,2,29,31,0,64, 0x10);
		position1(zehner,10,12,39,41,0,44, 0x12);
		position1(einer,8,10,37,39,0,41, 0x15);
	}
	if (munitionenergy<100 && munitionenergy>9)
	{
		cleardisplay();
		position1(zehner,10,12,39,41,0,44, 0x12);
		position1(einer,8,10,37,39,0,41, 0x15);
	}
	if (munitionenergy<10)
	{
		cleardisplay();
		position1(einer,8,10,37,39,0,41, 0x15);
	}
}

void playerwriten(){
	
	//p
	clearbuffer(0,44);
	for(int i=0xB2; i<=0xB7;i++){
	drawBufferPage(bufferpage, i, 0x12, 64, 0);}

	clearbuffer(0,44);
	verticalline(10,12);
	for(int i=0xB3; i<=0xB7;i++){
		drawBufferPage(bufferpage, i, 0x12, 64, 0);
	}
	clearbuffer(0,44);
	horizontalline(12,39);
	drawBufferPage(bufferpage, 0xB2,0x12, 64, 0);


	clearbuffer(0,44);
	verticalline(39,41);
	for(int i=0xB3; i<=0xB4;i++){
		drawBufferPage(bufferpage, i, 0x12, 64, 0);
	}

	horizontalline(12,39);
	drawBufferPage(bufferpage, 0xB4, 0x12, 64, 0);


	
}

void out(){
	// o
	clearbuffer(0,64);
	for(int i=0xB2; i<=0xB7;i++){
	drawBufferPage(bufferpage, i, 0x10, 64, 0);}
	

	horizontalline(2,29);
	drawBufferPage(bufferpage,0xB4, 0x10,64, 0);
	clearbuffer(0,64);
	verticalline(0,2);
	verticalline(29,31);
	for(int i=0xB5; i<=0xB7;i++){
	drawBufferPage(bufferpage, i, 0x10, 64, 0);}
	clearbuffer(0,64);
	horizontalline(2,29);
	drawBufferPage(bufferpage,0xB7, 0x10,64, 0);
	
	//u
	
	clearbuffer(0,44);
	for(int i=0xB2; i<=0xB7;i++){
	drawBufferPage(bufferpage, i, 0x12, 44, 0);}
	
	
	clearbuffer(0,44);
	verticalline(10,12);
	verticalline(39,41);
	for(int i=0xB5; i<=0xB7;i++){
	drawBufferPage(bufferpage, i, 0x12, 44, 0);}
	clearbuffer(0,44);
	horizontalline(12,39);
	drawBufferPage(bufferpage,0xB7, 0x12,44, 0);
	
	//t
	clearbuffer(0,41);
	for(int i=0xB2; i<=0xB7;i++){
	drawBufferPage(bufferpage, i, 0x15, 41, 0);}
	
	
	clearbuffer(0,41);
	verticalline(23,25);
	for(int i=0xB2; i<=0xB7;i++){
	drawBufferPage(bufferpage, i, 0x15, 41, 0);}
	clearbuffer(0,41);
	horizontalline(15,36);
	drawBufferPage(bufferpage,0xB4, 0x15,41, 0);
	
}

void heart(int number){
	for(int i = 0; i<40;i++){bufferpagea[i]=0x00;}

	if (number == 1){
		for(int i=0; i<13;i++){ bufferpagea[31+i]=herzoben[i]; bufferpagea[15+i]=bufferherz[i]; bufferpagea[0+i]=bufferherz[i];};
		drawBufferPage(bufferpagea, 0xB0, 0x15, 44, 0);
		for(int i=0; i<13;i++){ bufferpagea[31+i]=herzunten[i]; bufferpagea[15+i]=bufferherz[i]; bufferpagea[0+i]=bufferherz[i];};
		drawBufferPage(bufferpagea, 0xB1, 0x15, 44, 0);
	}
	if (number == 2){
		for(int i=0; i<13;i++){ bufferpagea[31+i]=herzoben[i]; bufferpagea[15+i]=herzoben[i]; bufferpagea[0+i]=bufferherz[i];};
		drawBufferPage(bufferpagea, 0xB0, 0x15, 44, 0);
		for(int i=0; i<13;i++){ bufferpagea[31+i]=herzunten[i];bufferpagea[15+i]=herzunten[i]; bufferpagea[0+i]=bufferherz[i];};
		drawBufferPage(bufferpagea, 0xB1, 0x15, 44, 0);
	}
	if (number == 3){
		for(int i=0; i<13;i++){ bufferpagea[31+i]=herzoben[i]; bufferpagea[15+i]=herzoben[i]; bufferpagea[0+i]=herzoben[i];};
		drawBufferPage(bufferpagea, 0xB0, 0x15, 44, 0);
		for(int i=0; i<13;i++){ bufferpagea[31+i]=herzunten[i];bufferpagea[15+i]=herzunten[i];bufferpagea[0+i]=herzunten[i];};
		drawBufferPage(bufferpagea, 0xB1, 0x15, 44, 0);
	}
}
void admin(){
	//A
	clearbuffer(0,44);
	horizontalline(12,39);
	drawBufferPage(bufferpage, 0xB2, 0x12, 64, 0);
	clearbuffer(0,44);
	verticalline(10,12);
	verticalline(39,41);
	for(int i=0xB3; i<=0xB7;i++){
		drawBufferPage(bufferpage, i, 0x12, 64, 0);
	}
	clearbuffer(0,44);
	horizontalline(12,39);
	drawBufferPage(bufferpage, 0xB4, 0x12, 64, 0);
	
}
void magazin(uint8_t magazinnumberi){
	
	
	for(int i =0; i<64; i++){
		bufferpagea[i]=0x00;
	}
	
	switch (magazinnumberi)
	{
		case 0:
		for(int i = 0; i<64;i++){bufferpagea[i]=0x00;
			drawBufferPage(bufferpagea, 0xB0, 0x10,50,0);
		drawBufferPage(bufferpagea, 0xB0, 0x10, 50,0);}
		break;
		
		case 1:
		
		bufferpagea[1]=0xFF;
		bufferpagea[2]=0xFF;
		drawBufferPage(bufferpagea, 0xB0, 0x10,50,0);
		drawBufferPage(bufferpagea, 0xB1, 0x10,50,0);
		break;				case 2:		bufferpagea[1]=0xFF;
		bufferpagea[2]=0xFF;
				bufferpagea[5]=0xFF;
		bufferpagea[6]=0xFF;
		drawBufferPage(bufferpagea, 0xB0, 0x10,50,0);
		drawBufferPage(bufferpagea, 0xB1, 0x10,50,0);		break;
				case 3:		bufferpagea[1]=0xFF;
		bufferpagea[2]=0xFF;
				bufferpagea[5]=0xFF;
		bufferpagea[6]=0xFF;
				bufferpagea[9]=0xFF;
		bufferpagea[10]=0xFF;
		drawBufferPage(bufferpagea, 0xB0, 0x10,50,0);
		drawBufferPage(bufferpagea, 0xB1, 0x10,50,0);		break;
				case 4:		bufferpagea[1]=0xFF;
		bufferpagea[2]=0xFF;
				bufferpagea[5]=0xFF;
		bufferpagea[6]=0xFF;
				bufferpagea[9]=0xFF;
		bufferpagea[10]=0xFF;
				bufferpagea[13]=0xFF;
		bufferpagea[14]=0xFF;
		drawBufferPage(bufferpagea, 0xB0, 0x10,50,0);
		drawBufferPage(bufferpagea, 0xB1, 0x10,50,0);		break;
				case 5:		bufferpagea[1]=0xFF;
		bufferpagea[2]=0xFF;
				bufferpagea[5]=0xFF;
		bufferpagea[6]=0xFF;
				bufferpagea[9]=0xFF;
		bufferpagea[10]=0xFF;
				bufferpagea[13]=0xFF;
		bufferpagea[14]=0xFF;
				bufferpagea[17]=0xFF;
		bufferpagea[18]=0xFF;
		drawBufferPage(bufferpagea, 0xB0, 0x10,50,0);
		drawBufferPage(bufferpagea, 0xB1, 0x10,50,0);		break;
				case 6:		bufferpagea[1]=0xFF;
		bufferpagea[2]=0xFF;
				bufferpagea[5]=0xFF;
		bufferpagea[6]=0xFF;
				bufferpagea[9]=0xFF;
		bufferpagea[10]=0xFF;
				bufferpagea[13]=0xFF;
		bufferpagea[14]=0xFF;
				bufferpagea[17]=0xFF;
		bufferpagea[18]=0xFF;
		
		bufferpagea[21]=0xFF;
		bufferpagea[22]=0xFF;
		drawBufferPage(bufferpagea, 0xB0, 0x10,50,0);
		drawBufferPage(bufferpagea, 0xB1, 0x10,50,0);		break;

				case 7:		bufferpagea[1]=0xFF;
		bufferpagea[2]=0xFF;
				bufferpagea[5]=0xFF;
		bufferpagea[6]=0xFF;
				bufferpagea[9]=0xFF;
		bufferpagea[10]=0xFF;
				bufferpagea[13]=0xFF;
		bufferpagea[14]=0xFF;
				bufferpagea[17]=0xFF;
		bufferpagea[18]=0xFF;
		
		bufferpagea[21]=0xFF;
		bufferpagea[22]=0xFF;
		
		bufferpagea[25]=0xFF;
		bufferpagea[26]=0xFF;
		drawBufferPage(bufferpagea, 0xB0, 0x10,50,0);
		drawBufferPage(bufferpagea, 0xB1, 0x10,50,0);		break;
				case 8:		bufferpagea[1]=0xFF;
		bufferpagea[2]=0xFF;
				bufferpagea[5]=0xFF;
		bufferpagea[6]=0xFF;
				bufferpagea[9]=0xFF;
		bufferpagea[10]=0xFF;
				bufferpagea[13]=0xFF;
		bufferpagea[14]=0xFF;
				bufferpagea[17]=0xFF;
		bufferpagea[18]=0xFF;
		
		bufferpagea[21]=0xFF;
		bufferpagea[22]=0xFF;
		
		bufferpagea[25]=0xFF;
		bufferpagea[26]=0xFF;
		
		bufferpagea[29]=0xFF;
		bufferpagea[30]=0xFF;
		drawBufferPage(bufferpagea, 0xB0, 0x10,50,0);
		drawBufferPage(bufferpagea, 0xB1, 0x10,50,0);		break;
		
				case 9:		bufferpagea[1]=0xFF;
		bufferpagea[2]=0xFF;
				bufferpagea[5]=0xFF;
		bufferpagea[6]=0xFF;
				bufferpagea[9]=0xFF;
		bufferpagea[10]=0xFF;
				bufferpagea[13]=0xFF;
		bufferpagea[14]=0xFF;
				bufferpagea[17]=0xFF;
		bufferpagea[18]=0xFF;
		
		bufferpagea[21]=0xFF;
		bufferpagea[22]=0xFF;
		
		bufferpagea[25]=0xFF;
		bufferpagea[26]=0xFF;
		
		bufferpagea[29]=0xFF;
		bufferpagea[30]=0xFF;
		
		bufferpagea[33]=0xFF;
		bufferpagea[34]=0xFF;
		drawBufferPage(bufferpagea, 0xB0, 0x10,50,0);
		drawBufferPage(bufferpagea, 0xB1, 0x10,50,0);		break;
		
				case 10:		bufferpagea[1]=0xFF;
		bufferpagea[2]=0xFF;
				bufferpagea[5]=0xFF;
		bufferpagea[6]=0xFF;
				bufferpagea[9]=0xFF;
		bufferpagea[10]=0xFF;
				bufferpagea[13]=0xFF;
		bufferpagea[14]=0xFF;
				bufferpagea[17]=0xFF;
		bufferpagea[18]=0xFF;
		
		bufferpagea[21]=0xFF;
		bufferpagea[22]=0xFF;
		
		bufferpagea[25]=0xFF;
		bufferpagea[26]=0xFF;
		
		bufferpagea[29]=0xFF;
		bufferpagea[30]=0xFF;
		
		bufferpagea[33]=0xFF;
		bufferpagea[34]=0xFF;
		
		bufferpagea[37]=0xFF;
		bufferpagea[38]=0xFF;
		drawBufferPage(bufferpagea, 0xB0, 0x10,50,0);
		drawBufferPage(bufferpagea, 0xB1, 0x10,50,0);		break;
	}

}
void batalert(){
	
	for(int i =0 ; i<64; i++){
		bufferpagea[i]=0x00;
	}
	
	for (int i =20; i<=23; i++){		bufferpagea[i]=0x3F;	}		for (int i =24; i<=37; i++){		bufferpagea[i]=0x30;	}	bufferpagea[38]=0x3F;
	bufferpagea[39]=0x3F;
	bufferpagea[40]=0x3;	bufferpagea[41]=0x3;	
	drawBufferPage(bufferpagea, 0xB1, 0x12,64,0);
	for(int i =20; i<64; i++){
		bufferpagea[i]=0x00;
	}
	for (int i =20; i<=23; i++){		bufferpagea[i]=0xFC;	}		for (int i =24; i<=37; i++){		bufferpagea[i]=0xC;	}	bufferpagea[38]=0xFC;
	bufferpagea[39]=0xFC;
	bufferpagea[40]=0xC0;	bufferpagea[41]=0xC0;
	drawBufferPage(bufferpagea, 0xB0, 0x12,64,0);
	
}

void displayoff(){
	unsigned char ret = i2c_start(DevSSD1306+I2C_WRITE);       // set device address and write mode+I2C_WRITE
	if ( ret ) { i2c_stop();}
	
	else
	{

		setup_i2c();
		i2c_write(0xAE);
		
		uint8_t twbrbackup = TWBR0;
		TWBR0 = 12;
		
	}
	i2c_stop();
	
	
}

void weaponhit()
	{
		cleardisplay();
		clearbuffer(0,44);
		horizontalline(12,39);
		drawBufferPage(bufferpage, 0xB2,0x12, 64, 0);

		
	}
	
void strich()
	{
		clearbuffer(0,64);
		horizontalline(2,29);
		drawBufferPage(bufferpage, 0xB2, 0x10, 64, 0);
		clearbuffer(0,44);
		horizontalline(12,39);
		drawBufferPage(bufferpage, 0xB2, 0x12, 44, 0);
		clearbuffer(0,41);
		horizontalline(12,39);
		drawBufferPage(bufferpage, 0xB2, 0x15, 41, 0);

	}