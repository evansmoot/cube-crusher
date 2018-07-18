/* GBA Game made by Evan Smoot */

#include "mylib.h"

#define REG_DISPCNT *(unsigned short*) 0x4000000
#define MODE_3 3
#define BG2_EN (1 << 10)


#define OFFSET(row, col, rowlen) ((row)*(rowlen)+col)


unsigned short *videoBuffer = (unsigned short *)0x6000000;

void setPixel(int row, int col, unsigned short color) {
	videoBuffer[OFFSET(row, col, 240)] = color;
}


void drawRect(int row, int col, int height, int width, volatile unsigned short color) {
	for (int r=0; r < height; r++) {
		REG_DMA3SAD = (u32)&color;
		REG_DMA3DAD = (u32)(&videoBuffer[OFFSET(row + r, col, 240)]);
		REG_DMA3CNT = width | DMA_ON | DMA_SOURCE_FIXED;
	}	
}

void drawImage3(int row, int col, int width, int height, const u16* image) {
	for (int r = 0; r < height; r++) {
		REG_DMA3SAD = (u32)(&image[OFFSET(r, 0, width)]);
		REG_DMA3DAD = (u32)(&videoBuffer[OFFSET(r + row, col, 240)]);
		REG_DMA3CNT = width | DMA_ON | DMA_SOURCE_INCREMENT;
	}
}
void fillScreen3(volatile unsigned short color) {
	for (int r=0; r < 160; r++) {
		REG_DMA3SAD = (u32)&color;
		REG_DMA3DAD = (u32)(&videoBuffer[OFFSET(r, 0, 240)]);
		REG_DMA3CNT = 240 | DMA_ON | DMA_SOURCE_FIXED;
	}
}

void drawChar(int row, int col, char ch, unsigned short color) {
	for(int r=0; r<8; r++) {
		for(int c=0; c<6; c++) {
			if(fontdata_6x8[OFFSET(r,c,6) + ch*48]) {
				setPixel(row+r, col+c, color);
			}
		}
	}
}

void drawString(int row, int col, char *str, unsigned short color)  {
	while(*str) {
		drawChar(row, col, *str++, color);
		col +=6;
	}
}

void waitForVBlank() {
	while (SCANLINECOUNTER > 160);
	while (SCANLINECOUNTER < 160);
}
