/**
  ******************************************************************************
  * @file    bsp_key.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   ����Ӧ��bsp��ɨ��ģʽ��
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� F103-ָ���� STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
  
#include "./key/bsp_key.h"  
#include "./led/bsp_led.h"
#include "snake.h"
/**
  * @brief  ���ð����õ���I/O��
  * @param  ��
  * @retval ��
  */
void Key_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/*���������˿ڵ�ʱ��*/
	RCC_APB2PeriphClockCmd(KEY1_GPIO_CLK|KEY2_GPIO_CLK,ENABLE);
	
	//ѡ�񰴼�������
	GPIO_InitStructure.GPIO_Pin = KEY1_GPIO_PIN; 
	// ���ð���������Ϊ��������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	//ʹ�ýṹ���ʼ������
	GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure);
	
	//ѡ�񰴼�������
	GPIO_InitStructure.GPIO_Pin = KEY2_GPIO_PIN; 
	//���ð���������Ϊ��������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	//ʹ�ýṹ���ʼ������
	GPIO_Init(KEY2_GPIO_PORT, &GPIO_InitStructure);	
}

 /*
 * ��������Key_Scan
 * ����  ������Ƿ��а�������
 * ����  ��GPIOx��x ������ A��B��C��D���� E
 *		     GPIO_Pin������ȡ�Ķ˿�λ 	
 * ���  ��KEY_OFF(û���°���)��KEY_ON�����°�����
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
	BUTTON_STATE_RELEASE  = 0,	//�����ͷ�
	BUTTON_STATE_WAITING,			//��������
	BUTTON_STATE_PRESSED,			//��������
}enumButtonState	;

/**
  * @brief  �������״̬��
  * @retval ����״̬
  *   �÷���ֵΪ����ֵ֮һ��
  *     @arg TOUCH_PRESSED : ��������
  *     @arg TOUCH_NOT_PRESSED : �ް�������
  */
uint8_t ButtonDetect(void)
{ 
	static enumButtonState button_state[2] = {BUTTON_STATE_RELEASE, BUTTON_STATE_RELEASE};
	static uint32_t debounce_counter[2] = {0, 0};
	uint8_t detectResult = TOUCH_NOT_PRESSED;
	
	// �������飬����0��Ӧbutton_left������1��Ӧbutton_right
	enum button buttons[2] = {button_left, button_right};
	int btn_idx;
	// �ֱ�����������
	for(btn_idx = 0; btn_idx < 2; btn_idx++)
	{
		uint8_t button_pressed = (Key_Scan(buttons[btn_idx]) == KEY_ON);
		
		switch(button_state[btn_idx])
		{
			case BUTTON_STATE_RELEASE:	
				if(button_pressed) //��һ�γ��ְ����ź�
				{
					button_state[btn_idx] = BUTTON_STATE_WAITING;
				}
				break;
					
			case BUTTON_STATE_WAITING:
				if(button_pressed)
				{
					debounce_counter[btn_idx]++;
					//�ȴ�ʱ�������ֵ����Ϊ����������
					//����ʱ�� = DURIATION_TIME * �����������õ�ʱ����
					//���ڶ�ʱ���е��ã�ÿ10ms����һ�Σ�������ʱ��Ϊ��DURIATION_TIME*10ms
					if(debounce_counter[btn_idx] > DURIATION_TIME)		
					{
						debounce_counter[btn_idx] = 0;
						button_state[btn_idx] = BUTTON_STATE_PRESSED;
						detectResult = TOUCH_PRESSED;
					}
				}
				else	//�ȴ�ʱ��ֵδ�ﵽ��ֵ��Ϊ��Ч��ƽ�����ɶ�������
				{
					debounce_counter[btn_idx] = 0;
					button_state[btn_idx] = BUTTON_STATE_RELEASE; 
				}
				break;
			
			case BUTTON_STATE_PRESSED:	
				if(button_pressed)	//������������
				{
					button_state[btn_idx] = BUTTON_STATE_PRESSED;
					detectResult = TOUCH_PRESSED;
				}
				else	//�����ͷ�
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
  * @brief   ���������µ�ʱ�����ñ�����
  * @param  button_id �����µİ���
  * @note  ���ڱ������б�д�Լ��İ������´���Ӧ��
  * @retval ��
  */
void ButtonDown(enum button button_id)
{
	/***�ڴ˴���д�Լ��İ������´���Ӧ��***/
  
	/*������ѡ��*/
	// Button_Down(button_id);
	
	/***�������д�Լ��İ������´���Ӧ��***/
}

/**
  * @brief   �����ͷŵ�ʱ�����ñ�����
  * @param  button_id ���ͷŵİ���
  * @note  ���ڱ������б�д�Լ��İ����ͷŴ���Ӧ��
  * @retval ��
  */
void ButtonUp(enum button button_id) 
{
	/***�ڴ˴���д�Լ��İ����ͷŴ���Ӧ��***/
  
	/*������ѡ��*/
	// Button_UP(button_id);	
	
	/***�������д�Լ��İ����ͷŴ���Ӧ��***/
}

/**
  * @brief   ��⵽����״̬�仯ʱ���õĴ�����,ͨ��������ButtonDown��ButtonUp�㱨����״̬
  * @note    ��������Ҫ��whileѭ���ﱻ���ã�Ҳ��ʹ�ö�ʱ����ʱ����
  *          ���磬����ÿ��5ms����һ�Σ�������ֵ��DURIATION_TIME������Ϊ2������ÿ�������Լ��100�������¼���
  *          ����ButtonDown��ButtonUp�����б�д�Լ��İ���Ӧ��
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
		
		// ȷ���ĸ�����������
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
			return; // û�а���ʵ�ʱ����£���Ӧ�÷�����
		}
		
		// ֻ�ڵ�һ�μ�⵽����ʱ����ButtonDown
		if(!button_was_pressed)
		{
			//���ð���������ʱ�Ĵ����������ڸú�����д�Լ��İ������´������
			// ButtonDown(current_button);
			Command_Control_Direction(current_button);
			button_was_pressed = 1;
		}
	}
	else
	{
		LED_BLUE;
		
		// ֻ���ڰ���֮ǰ�����µ�����²ŵ���ButtonUp
		if(button_was_pressed)
		{
			//���ð������ͷ�ʱ�Ĵ����������ڸú�����д�Լ��İ����ͷŴ������
			// ButtonUp(last_pressed_button); 
			button_was_pressed = 0;
		}
	}
}


/*********************************************END OF FILE**********************/
