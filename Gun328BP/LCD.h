/*
 * LCD.h
 *
 * Created: 22/07/2022 17:02:55
 *  Author: Katharina Böhm-Klamt
 */ 

#include <inttypes.h>

#ifndef LCD_H_
#define LCD_H_



extern void drawBufferPage(uint16_t *buffp, int page, int position, int ende, int start); // draw one Bufferpage
extern void cleardisplay(); // clear Display
extern void clearbuffer(int x, int y); // clear buffer
extern void horizontalline(int x, int y); //draw horizontalline
extern void verticalline(int x, int y);
extern void position1(int dez, int leftstart, int leftend, int rightstart, int rightend, int fulll, int fullr, int position);
extern void digitdraw(int munitionenergy);
extern void playerwriten();
extern void out();
extern void heart(int number);
extern void admin();
extern void magazin(uint8_t magazinnumberi);
extern void batalert();
extern void displayoff();
extern void weaponhit();


#endif /* LCD_H_ */