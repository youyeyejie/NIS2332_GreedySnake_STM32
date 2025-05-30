#ifndef __KEY_H
#define	__KEY_H


#include "stm32f10x.h"

//  ���Ŷ���
#define    KEY1_GPIO_CLK     RCC_APB2Periph_GPIOA
#define    KEY1_GPIO_PORT    GPIOA			   
#define    KEY1_GPIO_PIN		 GPIO_Pin_0

#define    KEY2_GPIO_CLK     RCC_APB2Periph_GPIOC
#define    KEY2_GPIO_PORT    GPIOC		   
#define    KEY2_GPIO_PIN		  GPIO_Pin_13

/**
  * @brief  ����ö�ٶ���
  */
enum button { 
    button_left, 
    button_right 
};

 /** �������±��ú�
	*  ��������Ϊ�ߵ�ƽ������ KEY_ON=1�� KEY_OFF=0
	*  ����������Ϊ�͵�ƽ���Ѻ����ó�KEY_ON=0 ��KEY_OFF=1 ����
	*/
#define KEY_ON	1
#define KEY_OFF	0


#define TOUCH_PRESSED 				1
#define TOUCH_NOT_PRESSED			0

#define DURIATION_TIME				3

void Key_GPIO_Config(void);
uint8_t Key_Scan(enum button);

uint8_t ButtonDetect(void);
void ButtonEventHandler(void);

#endif /* __KEY_H */

