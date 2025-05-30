/**
  ******************************************************************************
  * @file    bsp_key.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   按键应用bsp（扫描模式）
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 F103-指南者 STM32 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
  
#include "./key/bsp_key.h"  
#include "./led/bsp_led.h"
#include "snake.h"
/**
  * @brief  配置按键用到的I/O口
  * @param  无
  * @retval 无
  */
void Key_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/*开启按键端口的时钟*/
	RCC_APB2PeriphClockCmd(KEY1_GPIO_CLK|KEY2_GPIO_CLK,ENABLE);
	
	//选择按键的引脚
	GPIO_InitStructure.GPIO_Pin = KEY1_GPIO_PIN; 
	// 设置按键的引脚为浮空输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	//使用结构体初始化按键
	GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure);
	
	//选择按键的引脚
	GPIO_InitStructure.GPIO_Pin = KEY2_GPIO_PIN; 
	//设置按键的引脚为浮空输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	//使用结构体初始化按键
	GPIO_Init(KEY2_GPIO_PORT, &GPIO_InitStructure);	
}

 /*
 * 函数名：Key_Scan
 * 描述  ：检测是否有按键按下
 * 输入  ：GPIOx：x 可以是 A，B，C，D或者 E
 *		     GPIO_Pin：待读取的端口位 	
 * 输出  ：KEY_OFF(没按下按键)、KEY_ON（按下按键）
 */
uint8_t Key_Scan(enum button b) {
	switch (b) {
		case button_left:
			return GPIO_ReadInputDataBit(KEY1_GPIO_PORT, KEY1_GPIO_PIN);
		case button_right:
			return GPIO_ReadInputDataBit(KEY2_GPIO_PORT, KEY2_GPIO_PIN);
		default:
			return KEY_OFF;
	}
}

typedef enum
{
	BUTTON_STATE_RELEASE  = 0,	//触摸释放
	BUTTON_STATE_WAITING,			//触摸按下
	BUTTON_STATE_PRESSED,			//触摸按下
}enumButtonState	;

/**
  * @brief  按键检测状态机
  * @retval 按键状态
  *   该返回值为以下值之一：
  *     @arg TOUCH_PRESSED : 按键按下
  *     @arg TOUCH_NOT_PRESSED : 无按键按下
  */
uint8_t ButtonDetect(void)
{ 
	static enumButtonState button_state[2] = {BUTTON_STATE_RELEASE, BUTTON_STATE_RELEASE};
	static uint32_t debounce_counter[2] = {0, 0};
	uint8_t detectResult = TOUCH_NOT_PRESSED;
	
	// 按键数组，索引0对应button_left，索引1对应button_right
	enum button buttons[2] = {button_left, button_right};
	int btn_idx;
	// 分别处理两个按键
	for(btn_idx = 0; btn_idx < 2; btn_idx++)
	{
		uint8_t button_pressed = (Key_Scan(buttons[btn_idx]) == KEY_ON);
		
		switch(button_state[btn_idx])
		{
			case BUTTON_STATE_RELEASE:	
				if(button_pressed) //第一次出现按键信号
				{
					button_state[btn_idx] = BUTTON_STATE_WAITING;
				}
				break;
					
			case BUTTON_STATE_WAITING:
				if(button_pressed)
				{
					debounce_counter[btn_idx]++;
					//等待时间大于阈值则认为按键被按下
					//消抖时间 = DURIATION_TIME * 本函数被调用的时间间隔
					//如在定时器中调用，每10ms调用一次，则消抖时间为：DURIATION_TIME*10ms
					if(debounce_counter[btn_idx] > DURIATION_TIME)		
					{
						debounce_counter[btn_idx] = 0;
						button_state[btn_idx] = BUTTON_STATE_PRESSED;
						detectResult = TOUCH_PRESSED;
					}
				}
				else	//等待时间值未达到阈值就为无效电平，当成抖动处理
				{
					debounce_counter[btn_idx] = 0;
					button_state[btn_idx] = BUTTON_STATE_RELEASE; 
				}
				break;
			
			case BUTTON_STATE_PRESSED:	
				if(button_pressed)	//按键持续按下
				{
					button_state[btn_idx] = BUTTON_STATE_PRESSED;
					detectResult = TOUCH_PRESSED;
				}
				else	//按键释放
				{
					button_state[btn_idx] = BUTTON_STATE_RELEASE;
				}
				break;			
			
			default:
				button_state[btn_idx] = BUTTON_STATE_RELEASE;
				break;
		}
	}
	
	return detectResult;
}

/**
  * @brief   按键被按下的时候会调用本函数
  * @param  button_id 被按下的按键
  * @note  请在本函数中编写自己的按键按下处理应用
  * @retval 无
  */
void ButtonDown(enum button button_id)
{
	/***在此处编写自己的按键按下处理应用***/
  
	/*处理按键选择*/
	// Button_Down(button_id);
	
	/***在上面编写自己的按键按下处理应用***/
}

/**
  * @brief   按键释放的时候会调用本函数
  * @param  button_id 被释放的按键
  * @note  请在本函数中编写自己的按键释放处理应用
  * @retval 无
  */
void ButtonUp(enum button button_id) 
{
	/***在此处编写自己的按键释放处理应用***/
  
	/*处理按键选择*/
	// Button_UP(button_id);	
	
	/***在上面编写自己的按键释放处理应用***/
}

/**
  * @brief   检测到按键状态变化时调用的处理函数,通过它调用ButtonDown和ButtonUp汇报按键状态
  * @note    本函数需要在while循环里被调用，也可使用定时器定时调用
  *          例如，可以每隔5ms调用一次，消抖阈值宏DURIATION_TIME可设置为2，这样每秒最多可以检测100个按键事件。
  *          可在ButtonDown及ButtonUp函数中编写自己的按键应用
  * @param   none
  * @retval  none
  */
void ButtonEventHandler(void)
{
	static uint8_t button_was_pressed = 0;
	enum button current_button;
	
	if(ButtonDetect() == TOUCH_PRESSED)
	{
		LED_GREEN;
		
		// 确定哪个按键被按下
		if(Key_Scan(button_left) == KEY_ON)
		{
			current_button = button_left;
		}
		else if(Key_Scan(button_right) == KEY_ON)
		{
			current_button = button_right;
		}
		else
		{
			return; // 没有按键实际被按下（不应该发生）
		}
		
		// 只在第一次检测到按下时调用ButtonDown
		if(!button_was_pressed)
		{
			//调用按键被按下时的处理函数，可在该函数编写自己的按键按下处理过程
			// ButtonDown(current_button);
			Command_Control_Direction(current_button);
			button_was_pressed = 1;
		}
	}
	else
	{
		LED_BLUE;
		
		// 只有在按键之前被按下的情况下才调用ButtonUp
		if(button_was_pressed)
		{
			//调用按键被释放时的处理函数，可在该函数编写自己的按键释放处理过程
			// ButtonUp(last_pressed_button); 
			button_was_pressed = 0;
		}
	}
}


/*********************************************END OF FILE**********************/
