#ifndef _PALETTE_H
#define _PALETTE_H
#pragma anon_unions
#include "stm32f10x.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "./key/bsp_key.h"
 

#define COLOR_BLOCK_WIDTH   80
#define COLOR_BLOCK_HEIGHT  39

#define BUTTON_NUM 5
#define PALETTE_START_Y   0
#define PALETTE_END_Y     LCD_Y_LENGTH

extern uint8_t MODE;

static const int LCD_X = 240;
static const int LCD_Y = 320;
static const int SQUARE_SIZE = 10;
static const int PLAYGROUND_MINY = 20;
static const int PLAYGROUND_MAXY = LCD_Y;
static const int PLAYGROUND_X = LCD_X/SQUARE_SIZE;
static const int PLAYGROUND_Y = (PLAYGROUND_MAXY-PLAYGROUND_MINY)/SQUARE_SIZE;

#if LCD_RGB_888
/*RGB888颜色转换*/
#define RGB(R,G,B)	( (R<< 16) | (G << 8) | (B))	/* 将8位R,G,B转化为 24位RGB888格式 */

#else 
/*RGB565 颜色转换*/
#define RGB(R,G,B)	(((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3))	/* 将8位R,G,B转化为 16位RGB565格式 */
#define RGB565_R(x)  ((x >> 8) & 0xF8)
#define RGB565_G(x)  ((x >> 3) & 0xFC)
#define RGB565_B(x)  ((x << 3) & 0xF8)

#endif

enum
{
	CL_WHITE    = RGB(255,255,255),	/* 白色 */
	CL_BLACK    = RGB(  0,  0,  0),	/* 黑色 */
	CL_RED      = RGB(255,	0,  0),	/* 红色 */
	CL_GREEN    = RGB(  0,255,  0),	/* 绿色 */
	CL_BLUE     = RGB(  0,	0,255),	/* 蓝色 */
	CL_YELLOW   = RGB(255,255,  0),	/* 黄色 */
	CL_ORANGE   = RGB(255,150,  0),	/* 黄色 */

	CL_GREY    = RGB( 98, 98, 98), 	/* 深灰色 */
	CL_GREY1		= RGB( 150, 150, 150), 	/* 浅灰色 */
	CL_GREY2		= RGB( 180, 180, 180), 	/* 浅灰色 */
	CL_GREY3		= RGB( 200, 200, 200), 	/* 最浅灰色 */
	CL_GREY4		= RGB( 230, 230, 230), 	/* 最浅灰色 */

	CL_BUTTON_GREY	= RGB( 195, 195, 195), /* WINDOWS 按钮表面灰色 */

	CL_MAGENTA      = RGB(255, 0, 255),	/* 红紫色，洋红色 */
	CL_CYAN         = RGB( 0, 255, 255),	/* 蓝绿色，青色 */
	CL_GREEN2    = RGB(  12, 109,  55),	/* 绿色 */

	CL_BLUE1        = RGB(  0,  0, 240),		/* 深蓝色 */
	CL_BLUE2        = RGB(  0,  0, 128),		/* 深蓝色 */
	CL_BLUE3        = RGB(  128, 128, 255),		/* 浅蓝色1 */
	CL_BLUE4        = RGB(  0, 64, 128),		/* 浅蓝色1 */

	/* UI 界面 Windows控件常用色 */
	CL_BTN_FACE		  = RGB(236, 233, 216),	/* 按钮表面颜色(灰) */
	CL_BOX_BORDER1	= RGB(172, 168,153),	/* 分组框主线颜色 */
	CL_BOX_BORDER2	= RGB(255, 255,255),	/* 分组框阴影线颜色 */
                                                                                                                                                                                                              
	CL_MASK			    = 0x7FFF	/* RGB565颜色掩码，用于文字背景透明 */
};

enum food_type {
	food_normal, // 60%
	food_double_score, // 10%
	food_undead, // 10%
	food_half_length, // 10%
	food_speed_low, // 10%
};

typedef struct Snake//相当于蛇一个节点
{
	int x;//横坐标
	int y;//纵坐标
	union {
    struct Snake *next;
		enum food_type food_type;
	};
} snake;

void init_snake_env(uint8_t LCD_Mode);
void move_snake(void);
void Touch_Button_Down(uint16_t x,uint16_t y);
void Touch_Button_Up(uint16_t x,uint16_t y);
void Command_Control_Direction(enum button b);

#endif //_PALETTE_H

