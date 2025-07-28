//  *
 /*
 *  Created on: 24 June 2025
 *      Author: Jiajun Xu
 * Description: LCD driver serves music player
*///

#ifndef LCD_display_H_
#define LCD_display_H_

#include "LCD_display.h"
#include "DE1SoC_LT24/DE1SoC_LT24.h"
#include "BasicFont/BasicFont.h"
#include <math.h>


HpsErr_t LCD_draw_area(LT24Ctx_t* ctx, unsigned short colour, unsigned int x, unsigned int y, unsigned int x_length, unsigned int y_length);
HpsErr_t LCD_draw_char(LT24Ctx_t* ctx, char c, unsigned int x, unsigned int y);
HpsErr_t LCD_draw_string(LT24Ctx_t* ctx, char *string, unsigned int x, unsigned int y);
void Drawline(LT24Ctx_t* ctx, int x1, int y1, int x2, int y2, unsigned short color);
void Soundbar(LT24Ctx_t* ctx, unsigned int x1, unsigned int y1, unsigned int x_length, unsigned int y_length, unsigned short color);
void Soundbar_increase (LT24Ctx_t* ctx, unsigned int x1, unsigned int y1, unsigned int *increase, unsigned int *decrease, short color);
void Soundbar_decrease (LT24Ctx_t* ctx, unsigned int x1, unsigned int y1, unsigned int *increase, unsigned int *decrease);
void Soundbar_clear(LT24Ctx_t* ctx, unsigned int x1, unsigned int y1, unsigned int x_length, unsigned int y_length);
void Soundbar_word (LT24Ctx_t* ctx, unsigned int x, unsigned int y);
void Progress_bar (LT24Ctx_t* ctx, unsigned int *progress);
void Progress_bar_Frame (LT24Ctx_t* ctx);
void Progress_bar_second (LT24Ctx_t* ctx, unsigned int *progress);
void Progress_bar_second_Recover(LT24Ctx_t* ctx, unsigned int *progress);
void Progress_Secondbar_Frame (LT24Ctx_t* ctx);

#endif /* LCD_ShowA_H_ */
