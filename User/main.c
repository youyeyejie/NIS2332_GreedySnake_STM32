#include "stm32f10x.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "./lcd/bsp_xpt2046_lcd.h"
#include "./flash/bsp_spi_flash.h"
#include "./led/bsp_led.h" 
#include "./key/bsp_key.h"
#include "snake.h"
#include <string.h>
#include "bsp_TiMbase.h" 

int main(void)
{		
	ILI9341_Init();  
	XPT2046_Init();
	BASIC_TIM_Init();

	// USART_Config();  
	LED_GPIO_Config();
	Key_GPIO_Config();

	// snake
	init_snake_env(LCD_SCAN_MODE);
	while(1) move_snake();
}
