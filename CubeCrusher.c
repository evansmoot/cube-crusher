/* Cube Crusher made by Evan Smoot */

#include "mylib.h"
#include "title_screen.h"
#include "paddle.h"
#include "game_over.h"
#include <stdlib.h>

enum GBAState {
	START,
	START_NODRAW,
	GAME_STATE,
	GAME_STATE_NODRAW,
	WIN_STATE,
	WIN_STATE_NODRAW,
	GAME_OVER,
	GAME_OVER_NODRAW
};

	int paddleWidth;
	int paddleCol;
	int oldPaddleCol;
	int cubeSize;
	int cubeRow;
	int cubeCol;
	int oldCubeRow;
	int oldCubeCol;
	int rd;
	int cd;
	int launched;
	int seed;
	int blockCount;

	struct breakable blockList[88];

int main() {
	u16 colors[] = {RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, PINK, CYAN, MAGENTA, TEAL, MAROON, PINK, GOLD};
	int numColors = sizeof(colors) / sizeof(colors[0]); //numcolors becomes number of colors
	
	enum GBAState state = START;	

	paddleWidth = 34;
	paddleCol = (240 - paddleWidth) / 2;
	oldPaddleCol = paddleCol;
	cubeSize = 3;
	cubeRow = 155 - cubeSize;
	cubeCol = 120;
	oldCubeRow = cubeRow;
	oldCubeCol = cubeCol;
	rd = 0;
	cd = 0;
	launched = 0;
	seed = 0;
	blockCount = 88;

	REG_DISPCNT = MODE_3 | BG2_EN;

	while(TRUE) {
		waitForVBlank();
		if (KEY_DOWN_NOW(BUTTON_SELECT)) {
				state = START;
		}
		switch(state) {
		case START:
			drawImage3(0, 0, TITLE_SCREEN_WIDTH, TITLE_SCREEN_HEIGHT, title_screen);
			drawString(120, 50, "Press A (Z) to continue", YELLOW);
			paddleWidth = 34;
			paddleCol = (240 - paddleWidth) / 2;
			oldPaddleCol = paddleCol;
			cubeSize = 6;
			cubeRow = 155 - cubeSize;
			cubeCol = paddleCol + (paddleWidth / 2) - (cubeSize / 2);
			oldCubeRow = cubeRow;
			oldCubeCol = cubeCol;
			rd = 0;
			cd = 0;
			launched = 0;
			blockCount = 88;
			state = START_NODRAW;
			break; 
		case START_NODRAW:
			if (KEY_DOWN_NOW(BUTTON_A)) {
				state = GAME_STATE;
			}
			seed++;
			break;
		case GAME_STATE:
			fillScreen3(WHITE);
			drawString(80, 55, "Press B to launch cube", RED);
			int colTracker = 10;
			srand(seed);
			for (int i = 0; i < 88; i++) {
				blockList[i].row = 10 + (5 * (i / 11));
				blockList[i].col = colTracker;
				blockList[i].width = 20;
				blockList[i].height = 5;
				blockList[i].color = colors[rand() % numColors];
				blockList[i].visible = 1;
				colTracker = colTracker + 20;
				if (colTracker > 210) {
					colTracker = 10;
				}
			}
			drawBreakables(blockList);
			state = GAME_STATE_NODRAW;
			break;
		case GAME_STATE_NODRAW:
			
			if (KEY_DOWN_NOW(BUTTON_B) && !launched) {
				drawRect(80, 50, 20, 240, WHITE); //DELETES "Press B to launch" message
				rd = 1;
				cd = 0;
				launched = 1;
			}
			if (!launched) {
				if (KEY_DOWN_NOW(BUTTON_RIGHT)) {
					if (paddleCol < 240 - paddleWidth) {
						cubeCol+=3;
				}
			}
				if (KEY_DOWN_NOW(BUTTON_LEFT)) {
					if (paddleCol >= 0) {
						cubeCol-=3;
					}
				}
			}
			cubeRow += rd;
			cubeCol += cd;
			checkHitDetection();
			if (cubeRow < 0) {
				cubeRow = 0;
				rd = -rd;
			}
			if (cubeRow > 159 - cubeSize) {
				state = GAME_OVER;
			}
			if (cubeCol < 0) {
				cubeCol = 0;
				cd = -cd;
			}	
			if (cubeCol > 239 - cubeSize) {
				cubeCol = 239 - cubeSize;
				cd = -cd;
			}
			if (cubeHitsPaddle(cubeRow, cubeCol, cubeSize, paddleCol, paddleWidth)) {
				cubeRow = 155 - cubeSize;
				rd = -rd;
				cd = getCDBounceVector(cubeCol, paddleCol, paddleWidth);
			}
			drawRect(oldCubeRow, oldCubeCol, cubeSize, cubeSize, WHITE);
			drawRect(cubeRow, cubeCol, cubeSize, cubeSize, RED);
			oldCubeRow = cubeRow;
			oldCubeCol = cubeCol;
			if (KEY_DOWN_NOW(BUTTON_RIGHT)) {
				if (paddleCol < 240 - paddleWidth) {
					paddleCol+=3;
				}
			}
			if (KEY_DOWN_NOW(BUTTON_LEFT)) {
				if (paddleCol >= 0) {
					paddleCol-=3;
				}
			}
			drawRect(155, oldPaddleCol, 5, paddleWidth, WHITE);
			drawImage3(155, paddleCol, PADDLE_WIDTH, PADDLE_HEIGHT, paddle);
			oldPaddleCol = paddleCol;
			if (blockCount <= 0) {
				state = WIN_STATE;
			}
			break;	
		case WIN_STATE:
			drawString(80, 100, "YOU WIN!", RED);
			state = WIN_STATE_NODRAW;
			break;
		case WIN_STATE_NODRAW:
			break;
		case GAME_OVER:
			drawImage3(0, 0, GAME_OVER_WIDTH, GAME_OVER_HEIGHT, game_over);
			state = GAME_OVER_NODRAW;
			break;
		case GAME_OVER_NODRAW:
			break;
		}
	}
}

//changes cube's vector and removes rectangle if rectangle is hit by cube
void checkHitDetection() {
	int x1;
	int y1;
	int x2;
	int y2;
	for (int i=0; i < 88; i++) {
		if (blockList[i].visible) {
			x1 = MAX(cubeRow, blockList[i].row);
			y1 = MAX(cubeCol, blockList[i].col);
			x2 = MIN(cubeRow + cubeSize, blockList[i].row + blockList[i].height);
			y2 = MIN(cubeCol + cubeSize, blockList[i].col + blockList[i].width);
			if (x1 < x2 && y1 < y2) { //rectangles intersect
				if (y2 - y1 >= x2 - x1) { //rectangles collide vertically
					rd = -rd;
				} else if (x2 - x1 >= y2 - y1) { //collide horizontally
					cd = -cd;
				} else { //collide at perfect square
					rd = -rd;
					cd = -cd;
				}
				drawRect(blockList[i].row, blockList[i].col, blockList[i].height, blockList[i].width, WHITE);
				blockList[i].visible = 0;
				blockCount--;
			}
		}
		
	}
}

//draws 88 breakable blocks to screen
void drawBreakables(struct breakable blockList[]) {
	for (int i = 0; i < 88; i++) {
		drawRect(blockList[i].row, blockList[i].col, blockList[i].height, blockList[i].width, blockList[i].color);  
	}
}

//returns 1 if cube hits the paddle, 0 if it does not hit the paddle
int cubeHitsPaddle(int cubeRow, int cubeCol, int cubeSize, int paddleCol, int paddleWidth) {
	if ((cubeRow > 160 - 5 - cubeSize) && (cubeCol > paddleCol) && (cubeCol < paddleCol + paddleWidth)) {
		return 1;
	} else if ((cubeRow > 160 - 5 - cubeSize) && (cubeCol + cubeSize > paddleCol) && (cubeCol < paddleCol + paddleWidth)) {
		return 1;
	} else {
		return 0;
	}
}

//returns slope that ball is projected off of paddle
int getCDBounceVector(int cubeCol, int paddleCol, int paddleWidth) {
	return (cubeCol - (paddleCol + (paddleWidth / 2))) / 4;
}


