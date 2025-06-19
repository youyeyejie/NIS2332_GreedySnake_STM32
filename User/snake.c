/// Main handlers of greedy snake interface
/// 
/// Copywrite wsm, youyeyejie, etc. 2025.
///
/// 
///  Portions of this code are derived from:
///  - 野火F103指南者emwin综合例程 
///    https://github.com/Embedfire-stm32f103-zhinanzhe/ebf_stm32f103_zhinanzhe_std_code
///    Copyright embedfire
///  
///  - History code by seniors
///    CC BY-SA 4.0

#include "snake.h"
#include "./key/bsp_key.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include <stdlib.h>
#include <stdio.h>
#include "bsp_adc.h"
#include "bsp_TiMbase.h"   

extern __IO uint16_t ADC_ConvertedValue;

snake *head; // head of main snake
snake food1; // food
uint8_t status = 'L';
uint8_t MODE = 1; // 1:running / 0: pause
unsigned score;
unsigned hiscore = 0;
unsigned die = 0; // 1: dead / 0: alive
unsigned speed_factor;
unsigned snake_lives;

static void init_snake(void);
static void create_food(void);
static void judge_alive(void);
static void draw_title(void);

static void Delay(volatile uint32_t nCount) {
  for(; nCount != 0; nCount--);
} 

// init main interface
void init_snake_env(uint8_t LCD_Mode) {
	ADCx_Init();
	ILI9341_GramScan ( LCD_Mode );
	score = 0;
	status = 'L';
	
	// draw gui
	LCD_SetBackColor(CL_WHITE);
	ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	
	
	draw_title();
	init_snake();
	create_food();
	
	// enable timer
	TIM_ITConfig(BASIC_TIM,TIM_IT_Update,ENABLE);
}

static void draw_square_snake(snake* s) {
	ILI9341_DrawRectangle(s->x*10,s->y*10+20,10,10,1);
}

static void draw_square(uint16_t color, int x, int y) {
	LCD_SetTextColor(color);
	ILI9341_DrawRectangle(x*10,y*10+20,10,10,1);
}

// draw eye on head
static void draw_head_eyes() {
	int hx = head->x*10;
	int hy = head->y*10+20;
	int i;
	const char states[4] = {'U','D','L','R'};
	const uint8_t o[4][4] = {
		{2,2,6,3}, {2,5,6,6},
		{3,2,2,6}, {6,2,5,6}
	};
	for(i=0; i<4; i++) 
		if (states[i]==status) {
			LCD_SetColors(CL_BLACK,CL_WHITE);
			ILI9341_DrawRectangle(hx+o[i][0],hy+o[i][1],2,2,1);
			ILI9341_DrawRectangle(hx+o[i][2],hy+o[i][3],2,2,1);
			break;
		}
}

// draw title part of gui
static void draw_title() {
	char disbuff[20];
	// line
	LCD_SetColors(CL_BLUE,CL_WHITE);
	ILI9341_DrawLine(0,20,240,20);
	// text
	LCD_SetFont(&Font8x16);
	LCD_SetTextColor(CL_BLACK);
	sprintf(disbuff,"Score:%d",score);
	ILI9341_DispString_EN_CH(8,4,disbuff);
	sprintf(disbuff,"HP:%d", snake_lives);
	ILI9341_DispString_EN_CH(112,4,disbuff);
	sprintf(disbuff,"HI:%d",hiscore);
	ILI9341_DispString_EN_CH(160,4,disbuff);
}

// init snake
void init_snake(void) {
	int i;
	snake *tail;
	tail = (snake*)malloc(sizeof(snake));
	tail->x = 30;
	tail->y = 10;
	tail->next=NULL;
	speed_factor = 0xfffff;
	snake_lives = 2;
	for(i=1; i<6; i++) {
		head=(snake*)malloc(sizeof(snake));
		head->next=tail;
		head->x = 20-i;
		head->y = 10;
		tail=head;
	} 
	// draw
	while(tail->next!=NULL) {
		draw_square(CL_GREEN, tail->x, tail->y);
		tail = tail->next;
	}
	draw_head_eyes();
}

// cut snake by half
static void snake_half_length() {
	snake* p = head, *pp;
	int count = 0;
	while(p) { count++; p=p->next; }
	count /= 2;
	LCD_SetTextColor(CL_WHITE);
	for(p=head; count>0; count--) {
		draw_square_snake(p);
		p=p->next;
	}
	while(p->next) {
		pp=p->next;
		draw_square_snake(pp);
		p->next=pp->next;
		free(pp);
	}
}

// accelerate by score
static unsigned current_delay() {
	// speedup: (3s+3)/(s+3)
	unsigned sc = speed_factor*score/0xfffff;
	return (0x7ffff/3)*(sc + 3) / (sc + 1);
}

// core part to move snake
void move_snake(void) {
	snake *nexthead;
	snake *p,*q;
	int nx=head->x, ny=head->y;
	nexthead=(snake*)malloc(sizeof(snake));
	if (die == 0 && MODE == 1) {
		// next head pos
		switch(status) {
			case 'R': nx++; break;
			case 'L': nx--; break;
			case 'U': ny--; break;
			case 'D': ny++; break;
		}
		nexthead->x = nx;
		nexthead->y = ny;
		// judge food
		if(nexthead->x == food1.x && nexthead->y == food1.y) {
			nexthead->next=head;
			head=nexthead;
			switch (food1.food_type) {
				case food_normal: score += 1; break;
				case food_double_score: score = score==0 ? 1 : score*2; break;
				case food_half_length: snake_half_length(); break;
				case food_speed_low: speed_factor = speed_factor*4/5; break;
				case food_undead: snake_lives += 1;
			}
			if(score > hiscore) hiscore = score;
			create_food();
		}
		else {
			nexthead->next=head;
			head=nexthead;
			p=head;
			while(p->next->next) p=p->next;
			LCD_SetTextColor(CL_WHITE);
			draw_square_snake(p->next);
			free(p->next);
			p->next=NULL;
		}
		// draw snake
		LCD_SetTextColor(CL_GREEN);
		p=head;
		while(p->next!=NULL) {
			draw_square_snake(p);
			p=p->next;
		}
		draw_title();
		draw_head_eyes();
	}

	// judge alive
	judge_alive(); //判断存活情况
	if(die == 1)    //如果死了就重启
	{
		LCD_SetColors(CL_RED,CL_WHITE);
		LCD_SetFont(&Font24x32);
		ILI9341_DispString_EN_CH(12,120,"Game over");
		LCD_SetFont(&Font8x16);
		ILI9341_DispString_EN_CH(64,157,"点击任意处开始");
		draw_title();
		TIM_ITConfig(BASIC_TIM,TIM_IT_Update,DISABLE);  // disable timer
		while(ButtonDetect() != TOUCH_PRESSED);
		p = q = head;
		while(p != NULL) { // free memory
			p = q->next;
			free(q);
			q = p; 
		}
		free(p);
		init_snake_env(6);  
		die = 0;
	}
	
	Delay(current_delay());
}

static void gen_random_pos(int* px, int* py) {
	int x,y;
	snake* p;
redo:
	// 利用 ADC
	srand(ADC_ConvertedValue);
	x = rand()%22 + 1;
	y = rand()%28 + 1;
	p = head;
	while(p->next) {
		if(x==p->x&&y==p->y) goto redo;
		p=p->next;
	}
	*px = x; *py = y;
}

// 判断死活
void judge_alive(void) {
	snake *q;
	// boundary
	if(head->x == -1 || head->y == -1 || head->x == 24 || head->y == 30)
		die=1;
	// hit self
	q = head->next;
	while(q != NULL) {
		if(q->x==head->x && head->y==q->y)
			die=1;
		q=q->next;
	};
	// revive on dead
	if(die == 1 && snake_lives != 1) {
		die = 0; snake_lives --;
		gen_random_pos(&head->x, &head->y);
	}
}

// generate random food
void create_food(void) {
	uint16_t food_color;
	gen_random_pos(&food1.x, &food1.y);
	switch (rand()%10) {
		case 0: food1.food_type = food_double_score; food_color = CL_ORANGE; break;
		case 1: food1.food_type = food_undead; food_color = CL_GREEN2; break;
		case 2: food1.food_type = food_half_length; food_color = CL_BLUE; break;
		case 3: food1.food_type = food_speed_low; food_color = CL_CYAN; break;
		default: food1.food_type = food_normal; food_color = CL_RED; break;
	}
	draw_square(food_color, food1.x, food1.y);
}

/**
* @brief  Touch_Button_Down 按键被按下时调用的函数，由触摸屏调用
* @param  x 触摸位置的x坐标
* @param  y 触摸位置的y坐标
* @retval 无
*/
void Touch_Button_Down(uint16_t x,uint16_t y) {}

/**
* @brief  Touch_Button_Up 按键被释放时调用的函数，由触摸屏调用
* @param  x 触摸最后释放时的x坐标
* @param  y 触摸最后释放时的y坐标
* @retval 无
*/
void Touch_Button_Up(uint16_t x,uint16_t y) { MODE = !MODE; }

/**
* @brief  Command_Control_Direction 运动方向功能执行函数
* @param  btn Touch_Button 类型的按键参数
* @retval 无
*/
void Command_Control_Direction(enum button b) {
	const char rotate_list[5] = {'D', 'R', 'U', 'L', 'D'};
	int i;
	switch (b) {
	case button_left: // rotate left
		for(i=0; i<4; i++) {
			if (rotate_list[i] == status) {
				status = rotate_list[i+1];
				break;
			}
		}
		break;
	case button_right: // rotate right
		for(i=4; i>0; i--) {
			if (rotate_list[i] == status) {
				status = rotate_list[i-1];
				return;
			}
		}
		break;
	}
}
