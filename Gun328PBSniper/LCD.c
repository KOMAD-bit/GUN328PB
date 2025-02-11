/*
 * LCD.c
 *
 * Created: 22/07/2022 17:03:11
 *  Author: Katharina B�hm-Klamt
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

#include <stdint.h>

#define BUFFP_SIZE         256         // Maximum number of elements in buffp
#define DISPLAY_RAM_CMD    0x40        // Command to set display RAM start line
#define I2C_FAST_TWBR      10          // Fast I2C bit rate setting

void drawBufferPage(uint16_t *buffp, int lcdPage, int colPosition, size_t endIndex, size_t startIndex) {
    // Start I2C transmission: set device address and write mode.
    unsigned char ret = i2c_start(DevSSD1306 + I2C_WRITE);
    if (ret) {
        i2c_stop();
        return;
    } else {
        // Configure I2C communication for LCD operation.
        setup_i2c();
        ret = i2c_write(lcdPage);       // Set the active LCD page.
        if (ret) {
            i2c_stop();
            return;
        }
        ret = i2c_write(colPosition);   // Set the starting column (high byte).
        if (ret) {
            i2c_stop();
            return;
        }
        ret = i2c_write(DISPLAY_RAM_CMD);   // Initialize the display RAM command.
        if (ret) {
            i2c_stop();
            return;
        }
        uint8_t twbrbackup = TWBR0;
        TWBR0 = I2C_FAST_TWBR;

        // Combine all I2C writes into a single transmission.
        for (size_t i = startIndex; i < endIndex; i++) {
            // Boundary check to ensure we do not exceed the buffer size.
            if (i >= BUFFP_SIZE) {
                break;
            }
            ret = i2c_write(buffp[i]);
            if (ret) {
                i2c_stop();
                TWBR0 = twbrbackup;
                return;
            }
        }
        i2c_stop();
        TWBR0 = twbrbackup;
    }
}

void cleardisplay(void) {
    // Clear the buffer page by setting all 64 elements to 0x00
    for (int i = 0; i < 64; i++) {
        bufferpage[i] = 0x00;
    }
    
    // Update each of the 8 display pages with two different offsets
    for (int i = 0; i < 8; i++) {
        drawBufferPage(bufferpage, Pages[i], 0x10, 64, 0);
        drawBufferPage(bufferpage, Pages[i], 0x14, 64, 0);
    }
}

void clearbuffer( int x, int y)
{
	for (int i=x ; i<y;i++)
	{
		bufferpage[i]=0x00;
	}
}

/**
 * Draws a horizontal line in the global array bufferpage.
 *
 * Sets elements from index x up to (but not including) y to hex value 0xF0.
 *
 * @param x The starting index of the horizontal line.
 * @param y The ending (exclusive) index of the horizontal line.
 */
void horizontalline(int x, int y) {
    for (int i = x; i < y; i++) {
        bufferpage[i] = 0xF0;
    }
}


void verticalline(int x, int y){
	for(int i = x; i<y; i++){
	bufferpage[i]=0xFF;}
}

void position1(int dez, int leftstart, int leftend, int rightstart, int rightend, int fulll, int fullr, int position) {
    if (fulll < 0 || fullr >= BUFFP_SIZE) return; // Sicherheitsprüfung

    int pos = (position == 0x10 || position == 0x12) ? 64 : 44;

    // Definierte Segmente für jede Ziffer
    bool top = dez != 1 && dez != 4;  
    bool middle = dez != 1 && dez != 7 && dez != 0;  
    bool bottom = dez != 1 && dez != 4 && dez != 7;  
    bool left_top = dez == 4 || dez == 5 || dez == 6 || dez == 8 || dez == 9 || dez == 0;
    bool left_bottom = dez == 2 || dez == 6 || dez == 8 || dez == 0;
    bool right_top = dez != 5 && dez != 6;
    bool right_bottom = dez != 2;

    clearbuffer(fulll, fullr);
    
    if (top) horizontalline(leftend, rightstart), drawBufferPage(bufferpage, 0xB2, position, pos, 0);
    if (middle) horizontalline(leftend, rightstart), drawBufferPage(bufferpage, 0xB4, position, pos, 0);
    if (bottom) horizontalline(leftend, rightstart), drawBufferPage(bufferpage, 0xB7, position, pos, 0);
    
    if (left_top) verticalline(leftstart, leftend), drawBufferPage(bufferpage, 0xB3, position, pos, 0);
    if (left_bottom) verticalline(leftstart, leftend), drawBufferPage(bufferpage, 0xB6, position, pos, 0);
    
    if (right_top) verticalline(rightstart, rightend), drawBufferPage(bufferpage, 0xB5, position, pos, 0);
    if (right_bottom) verticalline(rightstart, rightend), drawBufferPage(bufferpage, 0xB6, position, pos, 0);
}

void digitdraw(int munitionenergy) {
    if (munitionenergy < 0) return;  // Sicherheitsprüfung

    int hunni = munitionenergy / 100;
    int zehner = (munitionenergy / 10) % 10;
    int einer = munitionenergy % 10;

    cleardisplay(); // Vorheriger Inhalt wird gelöscht

    if (munitionenergy >= 100) {  
        position1(hunni,  0,  2, 29, 31,  0, 64, 0x10);
    }
    if (munitionenergy >= 10) {  
        position1(zehner, 10, 12, 39, 41,  0, 44, 0x12);
    }
    position1(einer, 8, 10, 37, 39, 0, 41, 0x15); // Einer wird immer gezeichnet
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

void out() {
    // Helper macro to draw buffer pages from startPage to endPage.
    #define DRAW_PAGES(startPage, endPage, cp, height) \
        for (int i = (startPage); i <= (endPage); i++) { \
            drawBufferPage(bufferpage, i, (cp), (height), 0); \
        }

    // Draw letter 'o'
    clearbuffer(0, 64);
    DRAW_PAGES(0xB2, 0xB7, 0x10, 64);
    
    horizontalline(2, 29);
    drawBufferPage(bufferpage, 0xB4, 0x10, 64, 0);
    
    clearbuffer(0, 64);
    verticalline(0, 2);
    verticalline(29, 31);
    DRAW_PAGES(0xB5, 0xB7, 0x10, 64);
    
    clearbuffer(0, 64);
    horizontalline(2, 29);
    drawBufferPage(bufferpage, 0xB7, 0x10, 64, 0);

    // Draw letter 'u'
    clearbuffer(0, 44);
    DRAW_PAGES(0xB2, 0xB7, 0x12, 44);
    
    clearbuffer(0, 44);
    verticalline(10, 12);
    verticalline(39, 41);
    DRAW_PAGES(0xB5, 0xB7, 0x12, 44);
    
    clearbuffer(0, 44);
    horizontalline(12, 39);
    drawBufferPage(bufferpage, 0xB7, 0x12, 44, 0);

    // Draw letter 't'
    clearbuffer(0, 41);
    DRAW_PAGES(0xB2, 0xB7, 0x15, 41);
    
    clearbuffer(0, 41);
    verticalline(23, 25);
    DRAW_PAGES(0xB2, 0xB7, 0x15, 41);
    
    clearbuffer(0, 41);
    horizontalline(15, 36);
    drawBufferPage(bufferpage, 0xB4, 0x15, 41, 0);

    #undef DRAW_PAGES
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
void magazin(uint8_t magazinnumberi) {
    memset(bufferpagea, 0x00, sizeof(bufferpagea));
    
    if (magazinnumberi == 0) {
        drawBufferPage(bufferpagea, 0xB0, 0x10, 50, 0);
        drawBufferPage(bufferpagea, 0xB1, 0x10, 50, 0);
        return;
    }
    
    for (int i = 1, count = 0; count < magazinnumberi; i += 4, count++) {
        bufferpagea[i] = 0xFF;
        bufferpagea[i + 1] = 0xFF;
    }
    
    drawBufferPage(bufferpagea, 0xB0, 0x10, 50, 0);
    drawBufferPage(bufferpagea, 0xB1, 0x10, 50, 0);
}

void batalert(){
	
	for(int i =0 ; i<64; i++){
		bufferpagea[i]=0x00;
	}
	
	for (int i =20; i<=23; i++){
		bufferpagea[i]=0x3F;
	}
	
	for (int i =24; i<=37; i++){
		bufferpagea[i]=0x30;
	}
	bufferpagea[38]=0x3F;
	bufferpagea[39]=0x3F;
	bufferpagea[40]=0x3;
	bufferpagea[41]=0x3;
	
	drawBufferPage(bufferpagea, 0xB1, 0x12,64,0);
	for(int i =20; i<64; i++){
		bufferpagea[i]=0x00;
	}
	for (int i =20; i<=23; i++){
		bufferpagea[i]=0xFC;
	}
	
	for (int i =24; i<=37; i++){
		bufferpagea[i]=0xC;
	}
	bufferpagea[38]=0xFC;
	bufferpagea[39]=0xFC;
	bufferpagea[40]=0xC0;
	bufferpagea[41]=0xC0;
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