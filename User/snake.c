/**
  ******************************************************************************
  * @file    palette.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   触摸画板应用函数
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32  F429 开发板
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */

#include "snake.h"
#include "./key/bsp_key.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include <stdlib.h>
#include <stdio.h>
#include "bsp_adc.h"
#include "bsp_TiMbase.h"   

extern __IO uint16_t ADC_ConvertedValue;
/*按钮结构体数组*/
Touch_Button button[BUTTON_NUM];
snake *head;//头指针
snake food1;//头指针
uint8_t status = 'L';
uint8_t MODE = 1; // MODE == 1说明游戏正在进行
unsigned score;
unsigned hiscore = 0;
unsigned die = 0; //die == 0说明活着
unsigned speed_factor;
unsigned snake_lives;

static void Draw_Mode_Button(void *btn);
static void Draw_Direction_Button(void *btn);

static void Command_Change_Mode(void *btn);  




void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
} 

/**
* @brief  Palette_Init 游戏初始化
* @param  无
* @retval 无
*/
void Palette_Init(uint8_t LCD_Mode)
{
  
  // uint8_t i;
	
	ADCx_Init();
	ILI9341_GramScan ( LCD_Mode );
	score = 0;
	status = 'L';
	
  /* 整屏清为白色 */
	LCD_SetBackColor(CL_WHITE);
  ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	
	
	Touch_Button_Init();  
  Square_Init();
	Snake_Init();
	
  TIM_ITConfig(BASIC_TIM,TIM_IT_Update,ENABLE);
  /* 描绘按钮 */
  // for(i=0;i<BUTTON_NUM;i++)
  // {
  //  button[i].draw_btn(&button[i]);
  // }
	creatFood();
}

static void draw_square_snake(snake* s) {
	ILI9341_DrawRectangle(s->x*10,s->y*10+20,10,10,1);
}
static void draw_square(uint16_t color, int x, int y) {
	LCD_SetTextColor(color);
	ILI9341_DrawRectangle(x*10,y*10+20,10,10,1);
}

static void draw_head_eyes() {
	int hx = head->x*10;
	int hy = head->y*10+20;
	const char states[4] = {'U','D','L','R'};
	const uint8_t o[4][4] = {
		{2,2,6,3}, {2,5,6,6},
		{3,2,2,6}, {6,2,5,6}
	};
	int i;
	for(i=0; i<4; i++) if (states[i]==status) {
		LCD_SetColors(CL_BLACK,CL_WHITE);
		ILI9341_DrawRectangle(hx+o[i][0],hy+o[i][1],2,2,1);
		ILI9341_DrawRectangle(hx+o[i][2],hy+o[i][3],2,2,1);
		break;
	}
}

static void draw_title() {
		char disbuff[20];
		// line
	  LCD_SetColors(CL_BLUE,CL_WHITE);
	  ILI9341_DrawLine(0,20,240,20);
	  // text
		LCD_SetFont(&Font8x16);
		LCD_SetTextColor(CL_BLACK);
		sprintf(disbuff,"Score:%d",score);//打印成绩
		ILI9341_DispString_EN_CH(8,4,disbuff);
		sprintf(disbuff,"HP:%d", snake_lives);//打印成绩
		ILI9341_DispString_EN_CH(112,4,disbuff);
		sprintf(disbuff,"HI:%d",hiscore);//打印成绩
		ILI9341_DispString_EN_CH(160,4,disbuff);
}

/**
* @brief  Snake_Init 蛇身初始化，给定一个长度4
* @param  无
* @retval 无
*/
void Snake_Init(void)//
{
	int i;
	snake *tail;//尾指针
	tail =( snake*)malloc(sizeof(snake));//第一个节点/头结点
	tail->x = 30;//2的倍数，因为方块的长是两个单位
	tail->y = 10;//1个单位
	tail->next=NULL;
	speed_factor = 0xfffff;
	snake_lives = 2;
	for(i=1;i<6;i++)//尾插法
	{
		head=(snake*)malloc(sizeof(snake));//申请一个节点
		head->next=tail;//连接成链
		head->x = 20-i;//下一个节点的位置
		head->y = 10;
		tail=head;
	} 
	//遍历打印出来
	while(tail->next!=NULL)
	{
		draw_square(CL_GREEN, tail->x, tail->y);
		tail = tail->next;
	}
	draw_head_eyes();
}

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

/** 
* @brief accelerate by score
*/
static unsigned current_delay() {
	// speedup: (3s+3)/(s+3)
	unsigned sc = speed_factor*score/0xfffff;
	return (0x7ffff/3)*(sc + 3) / (sc + 1);
}

/**
* @brief snakeMove 蛇移动过程函数，核心部分
* @param  无
* @retval 无
*/
void snakeMove(void)
{
	snake *nexthead;
	snake *p,*q;
	nexthead=(snake*)malloc(sizeof(snake));
	if (die == 0 && MODE==1)
	{
		if(status=='R')//向右走
		{
			nexthead->x = head->x+1;
			nexthead->y = head->y;
		}
		else if(status=='L')//向左走
		{ 
			nexthead->x = head->x-1;
			nexthead->y = head->y;
		}
		else if(status=='U')//向上走
		{
			nexthead->x = head->x;
			nexthead->y = head->y-1;
		}
		else if(status=='D')//向下走
		{
			nexthead->x = head->x;
			nexthead->y = head->y+1;
		}
		if(nexthead->x==food1.x && nexthead->y==food1.y)//吃掉了食物
		{
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
			creatFood();
		}
		else//没有食物
		{
			nexthead->next=head;
			head=nexthead;
			p=head;
			while(p->next->next) p=p->next;
			LCD_SetTextColor(CL_WHITE);
			draw_square_snake(p->next);
			free(p->next);
			p->next=NULL;
		}
		LCD_SetTextColor(CL_GREEN);
		p=head;//p用来从头遍历，打印方块
		while(p->next!=NULL)
		{
			draw_square_snake(p);
			p=p->next;
		}//吃掉了食物得创造
		draw_title();
		draw_head_eyes();
	}

	
	judgeAlive(); //判断存活情况
	if(die == 1)    //如果死了就重启
	{
		LCD_SetColors(CL_RED,CL_WHITE);
		LCD_SetFont(&Font24x32);
		ILI9341_DispString_EN_CH(12,120,"Game over");
		LCD_SetFont(&Font8x16);
		ILI9341_DispString_EN_CH(64,157,"点击任意处开始");
		draw_title();
		TIM_ITConfig(BASIC_TIM,TIM_IT_Update,DISABLE);  //不使能中断，即不是能5个按键
		while(ButtonDetect() != TOUCH_PRESSED);//按任意位置重新初始化
		p = q = head;
		while(p != NULL)   //释放内存
		{
			p = q->next;
			free(q);
			q = p; 
		}
		free(p);	
		Palette_Init(6);  
		die = 0;
	}
	
	Delay(current_delay());//可以调节蛇移动的速度，里面是毫秒，越大速度越慢
}


static void gen_random_pos(int* px, int* py) {
	int x,y;
	snake* p;
redo:
	srand(ADC_ConvertedValue);//随着ADC变化，产生不一样种子，就会得到没规律的食物
	x = rand()%22 + 1;
	y = rand()%28 + 1;
	p = head;//用p来遍历
  while(p->next)//解决食物出现在蛇本身
	{
	    if(x==p->x&&y==p->y) goto redo;
			p=p->next;
	}
	*px = x; *py = y;
}

/**
* @brief judgeAlive 判断蛇的死活
* @param  无
* @retval 无
*/
void judgeAlive(void)
{
	snake *q;
	if(head->x==-1||head->y==-1||head->x==24||head->y==30)//碰到墙
		die=1;
	q=head->next;
	while(q!=NULL)   // 吃到自己
	{
		if(q->x==head->x && head->y==q->y)
			die=1;
		q=q->next;
	};
	if(die == 1 && snake_lives != 1) {
		die = 0; snake_lives --;
		gen_random_pos(&head->x, &head->y);
	}
}

/**
* @brief  Touch_Button_Init 初始化外框游戏区域
* @param  无
* @retval 无
*/
void Square_Init(void)
{
	//绘制显示分数的格子
	// LCD_SetColors(CL_WHITE,CL_WHITE);
  // ILI9341_DrawRectangle(BUTTON_START_X + COLOR_BLOCK_WIDTH*2,
	// 													320-COLOR_BLOCK_HEIGHT*2,
	//													COLOR_BLOCK_WIDTH,
	//													COLOR_BLOCK_HEIGHT,1);
	draw_title();
	// LCD_SetColors(CL_BLUE4,CL_WHITE);
  // ILI9341_DrawRectangle(BUTTON_START_X + COLOR_BLOCK_WIDTH*2,
	// 													320-COLOR_BLOCK_HEIGHT*2,
	// 													COLOR_BLOCK_WIDTH,
	// 													COLOR_BLOCK_HEIGHT,0);
}

/**
* @brief  creatFood 随机生成一个坐标并绘制食物
* @param  无
* @retval 无
*/
void creatFood(void)
{
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
* @brief  Touch_Button_Init 初始化按钮参数
* @param  无
* @retval 无
*/
void Touch_Button_Init(void)
{
  /*方向键*/
  button[0].start_x = BUTTON_START_X;
  button[0].start_y = 320-COLOR_BLOCK_HEIGHT;
  button[0].end_x = BUTTON_START_X + COLOR_BLOCK_WIDTH ;
  button[0].end_y = 320;
  button[0].para = LEFT;
  button[0].touch_flag = 0;  
  button[0].draw_btn = Draw_Direction_Button;   
  button[0].btn_command = Command_Change_Mode ;  
  
  button[1].start_x = BUTTON_START_X + COLOR_BLOCK_WIDTH ;
  button[1].start_y = 320-COLOR_BLOCK_HEIGHT;
  button[1].end_x = BUTTON_START_X + COLOR_BLOCK_WIDTH*2 ;
  button[1].end_y = 320;
  button[1].para = DOWN;
  button[1].touch_flag = 0;  
  button[1].draw_btn = Draw_Direction_Button;
  button[1].btn_command = Command_Change_Mode ;
  
  button[2].start_x = BUTTON_START_X + COLOR_BLOCK_WIDTH*2;
  button[2].start_y = 320-COLOR_BLOCK_HEIGHT;
  button[2].end_x = BUTTON_START_X + COLOR_BLOCK_WIDTH*3;
  button[2].end_y = 320;
  button[2].para = RIGHT;
  button[2].touch_flag = 0;  
  button[2].draw_btn = Draw_Direction_Button;
  button[2].btn_command = Command_Change_Mode ;
 
  button[3].start_x = BUTTON_START_X + COLOR_BLOCK_WIDTH;
  button[3].start_y = 320-COLOR_BLOCK_HEIGHT*2;
  button[3].end_x = BUTTON_START_X+COLOR_BLOCK_WIDTH*2;
  button[3].end_y = 320-COLOR_BLOCK_HEIGHT;
  button[3].para = UP;
  button[3].touch_flag = 0;  
  button[3].draw_btn = Draw_Direction_Button;
  button[3].btn_command = Command_Change_Mode ;
//下面是开始或暂停键  
  button[4].start_x = 0;
  button[4].start_y = 0;
  button[4].end_x = 240;
  button[4].end_y = 320;
  button[4].para = 0;
  button[4].touch_flag = 0;  
  button[4].draw_btn = Draw_Mode_Button ;
  button[4].btn_command = Command_Change_Mode;

}

/**
* @brief  Touch_Button_Down 按键被按下时调用的函数，由触摸屏调用
* @param  x 触摸位置的x坐标
* @param  y 触摸位置的y坐标
* @retval 无
*/
void Touch_Button_Down(uint16_t x,uint16_t y)
{
  uint8_t i;
  for(i=4;i<BUTTON_NUM;i++)
  {
    /* 触摸到了按钮 */
    if(x<=button[i].end_x && y<=button[i].end_y && y>=button[i].start_y && x>=button[i].start_x )
    {
      if(button[i].touch_flag == 0)     /*原本的状态为没有按下，则更新状态*/
      {
      button[i].touch_flag = 1;         /* 记录按下标志 */
      
      button[i].draw_btn(&button[i]);  /*重绘按钮*/
      }        
      
    }
    else if(button[i].touch_flag == 1) /* 触摸移出了按键的范围且之前有按下按钮 */
    {
      button[i].touch_flag = 0;         /* 清除按下标志，判断为误操作*/
      
      button[i].draw_btn(&button[i]);   /*重绘按钮*/
    }

  }

}

/**
* @brief  Touch_Button_Up 按键被释放时调用的函数，由触摸屏调用
* @param  x 触摸最后释放时的x坐标
* @param  y 触摸最后释放时的y坐标
* @retval 无
*/
void Touch_Button_Up(uint16_t x,uint16_t y)
{
  // uint8_t i; 
	Command_Change_Mode(NULL);
	button[4].draw_btn(&button[4]);
	return;
	
	/*
   for(i=4;i<BUTTON_NUM;i++)
   {
     // 触笔在按钮区域释放 
      if((x<button[i].end_x && x>button[i].start_x && y<button[i].end_y && y>button[i].start_y))
      {        
        button[i].touch_flag = 0;       //释放触摸标志
        
        button[i].btn_command(&button[i]);  //执行按键的功能命令
				
        button[i].draw_btn(&button[i]); //重绘按钮
        break;
      }
    }  
	*/
}

/**
* @brief  Draw_Direction_Button 方向键绘制
* @param  &button[i]
* @retval 无
*/
static void Draw_Direction_Button(void *btn)
{
  Touch_Button *ptr = (Touch_Button *)btn;
  uint16_t RGB;

	if(ptr->touch_flag == 0)
  {
	
		LCD_SetColors(CL_BUTTON_GREY,CL_WHITE);
    ILI9341_DrawRectangle(	ptr->start_x,
															ptr->start_y,
															ptr->end_x - ptr->start_x,
															ptr->end_y - ptr->start_y,1);
    RGB = CL_BUTTON_GREY;
  }
  else  /*按键按下*/
  {
		
		LCD_SetColors(CL_WHITE,CL_WHITE);
    ILI9341_DrawRectangle(ptr->start_x,
														ptr->start_y,
														ptr->end_x - ptr->start_x,
														ptr->end_y - ptr->start_y,1);
    
		RGB = CL_WHITE;

  } 
  LCD_SetColors(CL_BLUE4,CL_WHITE);
  ILI9341_DrawRectangle(ptr->start_x,
														ptr->start_y,
														ptr->end_x - ptr->start_x,
														ptr->end_y - ptr->start_y,0);
     
  LCD_SetColors(CL_BLACK,RGB);
	LCD_SetFont(&Font8x16);

  /*根据按键方向写字*/
	switch(ptr->para)
  {
    		
		case LEFT:      
				LCD_SetColors(CL_BLACK,RGB);
        ILI9341_DispString_EN_CH( ptr->start_x + (ptr->end_x - ptr->start_x - 16)/2,
//																ptr->start_y+15,
																ptr->start_y+ ((ptr->end_y - ptr->start_y-16)/2),	
																"左");
      
      break;   
      
    case DOWN:
        LCD_SetColors(CL_BLACK,RGB);
        ILI9341_DispString_EN_CH( ptr->start_x + (ptr->end_x - ptr->start_x - 16)/2,
//																ptr->start_y+15,
																ptr->start_y+ ((ptr->end_y - ptr->start_y-16)/2),	
																"下");
    
      break;
        
    case RIGHT:
				LCD_SetColors(CL_BLACK,RGB);
        ILI9341_DispString_EN_CH( ptr->start_x + (ptr->end_x - ptr->start_x - 16)/2,
//																ptr->start_y+15,
																ptr->start_y+ ((ptr->end_y - ptr->start_y-16)/2),	
																"右");
 
      break;
    
		case UP:
        LCD_SetColors(CL_BLACK,RGB);
        ILI9341_DispString_EN_CH( ptr->start_x + (ptr->end_x - ptr->start_x - 16)/2,
//																ptr->start_y+15,
																ptr->start_y+ ((ptr->end_y - ptr->start_y-16)/2),	
																"上");
    
      break;
		
  }
  
}

/**
* @brief  Draw_Mode_Button 模式键绘制
* @param  &button[i]
* @retval 无
*/
static void Draw_Mode_Button(void *btn)
{
  Touch_Button *ptr = (Touch_Button *)btn;
  // uint16_t RGB;

	if(ptr->touch_flag == 0)
  {
	
		LCD_SetColors(CL_BUTTON_GREY,CL_WHITE);
    // ILI9341_DrawRectangle(	ptr->start_x,
		//													ptr->start_y,
		//													ptr->end_x - ptr->start_x,
		//													ptr->end_y - ptr->start_y,1);
    // RGB = CL_BUTTON_GREY;
  }
  else  /*按键按下*/
  {
		
		LCD_SetColors(CL_WHITE,CL_WHITE);
    // ILI9341_DrawRectangle(ptr->start_x,
		//												ptr->start_y,
		//												ptr->end_x - ptr->start_x,
		//												ptr->end_y - ptr->start_y,1);
    
		// RGB = CL_WHITE;

  } 
	
	/*
  LCD_SetColors(CL_BLUE4,CL_WHITE);
  // ILI9341_DrawRectangle(ptr->start_x,
	//													ptr->start_y,
	//													ptr->end_x - ptr->start_x,
	//													ptr->end_y - ptr->start_y,0);
     
  LCD_SetColors(CL_BLACK,RGB);
	LCD_SetFont(&Font8x16);
	if(!MODE)
	{
		ILI9341_DispString_EN_CH( ptr->start_x + (ptr->end_x - ptr->start_x - 16*2)/2,
																ptr->start_y+ ((ptr->end_y - ptr->start_y-16)/2),	
																"暂停");
	}
	else
	{
		ILI9341_DispString_EN_CH( ptr->start_x + (ptr->end_x - ptr->start_x - 16*2)/2,
																ptr->start_y+ ((ptr->end_y - ptr->start_y-16)/2),	
																"继续");
	}
	*/
}

/**
* @brief  Command_Change_Mode 控制游戏的暂停与继续
* @param  btn Touch_Button 类型的按键参数
* @retval 无
*/
static void Command_Change_Mode(void *btn)
{
	MODE = !MODE;
}

/**
* @brief  Command_Control_Direction 运动方向功能执行函数
* @param  btn Touch_Button 类型的按键参数
* @retval 无
*/
void Command_Control_Direction(enum button b)
{
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



#define ABS(X)  ((X) > 0 ? (X) : -(X))





/* ------------------------------------------end of file---------------------------------------- */



