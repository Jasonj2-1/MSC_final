//  *
 /*
 *  Created on: 24 June 2025
 *      Author: Jiajun Xu
 * Description: LCD driver serves music player with reference of LEEDS API
*///

#include "LCD_display.h"
#include "DE1SoC_LT24/DE1SoC_LT24.h"
#include "BasicFont/BasicFont.h"
#include <math.h>


#define M_PI     3.14159265358979323846
#define LCD_WIDTH  240
#define LCD_HEIGHT 320

// Specify x and y to draw pixel
HpsErr_t LCD_draw_area(LT24Ctx_t* ctx, unsigned short colour, unsigned int x, unsigned int y, unsigned int x_length, unsigned int y_length) {

	for (unsigned i = 0; i < x_length; i ++) {
		for (unsigned j = 0; j < y_length; j++) {
		  LT24_drawPixel(ctx, colour, x + i, y + j);
		}
	}

	return ERR_SUCCESS;
}

HpsErr_t LCD_draw_char(LT24Ctx_t* ctx, char c, unsigned int x, unsigned int y){
	if (c - ' ' >= 96 + numberOfCustomCharacters) c = '~';
//	for (unsigned int column = 0; column <= 4; column ++){
//		for (unsigned int row = 0; row <= 7; row ++){
	for (unsigned int row = 0; row <= 7; row ++){
		for (unsigned int column = 0; column <= 4; column ++){
//			unsigned short colour = ((BF_fontMap[c - ' '][column] << row) & 0x80) ? LT24_RED : LT24_BLACK;
			unsigned short colour = ((BF_fontMap[c - ' '][column] >> row) & 0x1) ? LT24_RED : LT24_BLACK;
//			LT24_drawPixel(ctx, colour, x + row, y + column);
			LT24_drawPixel(ctx, colour, x + column, y + row);
		}
	}
	return ERR_SUCCESS;
}


HpsErr_t LCD_draw_string(LT24Ctx_t* ctx, char *string, unsigned int x, unsigned int y){
	while (*string){
	LCD_draw_char(ctx, *string, x, y);
	x += 5;
	string ++;
	}
	return ERR_SUCCESS;
}


// This part of code is reference from Terasic example
// Connect two point on the LCD, draw a line: It's a fixed Slope Algorithm
void Drawline(LT24Ctx_t* ctx, int x1, int y1, int x2, int y2, unsigned short color){
	int x, y, t;
    if ((x1 == x2) && (y1 == y2)) {
        LT24_drawPixel(ctx, color, x1, y1);
        return;
    }

    if (abs(y2 - y1) > abs(x2 - x1)) {
        if (y1 > y2) {
            t = y1; y1 = y2; y2 = t;
            t = x1; x1 = x2; x2 = t;
        }
        for (y = y1; y <= y2; y++) {
            x = (int)((y - y1) * (x2 - x1) / (float)(y2 - y1) + x1);
            LT24_drawPixel(ctx, color, x, y);
        }
    } else {
        if (x1 > x2) {
            t = y1; y1 = y2; y2 = t;
            t = x1; x1 = x2; x2 = t;
        }
        for (x = x1; x <= x2; x++) {
            y = (int)((x - x1) * (y2 - y1) / (float)(x2 - x1) + y1);
            LT24_drawPixel(ctx, color, x, y);
        }
    }
}


// To create a sound bar shown on the LCD first
void Soundbar(LT24Ctx_t* ctx, unsigned int x1, unsigned int y1, unsigned int x_length, unsigned int y_length, unsigned short color){
	LCD_draw_area (ctx, color, x1, y1, x_length, y_length);
}


// Increase sound bar
void Soundbar_increase(LT24Ctx_t* ctx, unsigned int x1, unsigned int y1, unsigned int *increase, unsigned int *decrease, short color){
	if ((y1 + *increase * 5 - *decrease * 5 >= 220) && (y1 + *increase * 5 - *decrease * 5 <= 280)){
	LCD_draw_area (ctx, color, x1, y1 + *increase * 5 - *decrease * 5 - 5, 20, 5);
	} else return;
}

// Decrease sound bar
void Soundbar_decrease(LT24Ctx_t* ctx, unsigned int x1, unsigned int y1, unsigned int *increase, unsigned int *decrease){
	if ((y1 + *increase * 5 - *decrease * 5 >= 220) && (y1 + *increase * 5 - *decrease * 5 <= 280)){
	LCD_draw_area (ctx, LT24_BLACK, x1, y1 + *increase * 5 - *decrease * 5 , 20, 5);
	} else return;
}

// Clear Sound bar when reset music player
void Soundbar_clear(LT24Ctx_t* ctx, unsigned int x1, unsigned int y1, unsigned int x_length, unsigned int y_length){
	LCD_draw_area (ctx, LT24_BLACK, x1, y1, x_length, y_length);
}


void Soundbar_word (LT24Ctx_t* ctx, unsigned int x, unsigned int y){
	LCD_draw_string(ctx, "Vol", x, y);
}

// When loading
void Progress_bar (LT24Ctx_t* ctx, unsigned int *progress){
	LCD_draw_area (ctx, LT24_RED, 100 + *progress * 3, 100, 10, 10);
}

// When loading second two
void Progress_bar_second (LT24Ctx_t* ctx, unsigned int *progress){
	LCD_draw_area (ctx, LT24_RED, 20 + *progress, 290, 1, 5);
}

// Custom bar
void Progress_bar_Frame (LT24Ctx_t* ctx){
	LCD_draw_area (ctx, LT24_WHITE, 98, 98, 104, 2);
	LCD_draw_area (ctx, LT24_WHITE, 98, 110, 104, 2);
	LCD_draw_area (ctx, LT24_WHITE, 98, 100, 2, 10);
	LCD_draw_area (ctx, LT24_WHITE, 200, 100, 2, 10);
}

// Second bar
void Progress_bar_second_Recover(LT24Ctx_t* ctx, unsigned int *progress){
	LCD_draw_area (ctx, LT24_RED, 20, 290, 1 + *progress, 5);
}

void Progress_Secondbar_Frame (LT24Ctx_t* ctx){
	LCD_draw_area (ctx, LT24_WHITE, 18, 288, 180, 2);
	LCD_draw_area (ctx, LT24_WHITE, 18, 295, 180, 2);
	LCD_draw_area (ctx, LT24_WHITE, 18, 290, 2, 5);
	LCD_draw_area (ctx, LT24_WHITE, 196, 290, 2, 5);
}
