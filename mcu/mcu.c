#include "mcu.h"

void delay_us(unsigned long us)
{
	int i, tus;
	tus = 9 * us;
	for(i = 0; i < tus; i++) asm("nop");
}

void eeprom_write32(uint32_t address,uint32_t data)
{
	if(IS_FLASH_DATA_ADDRESS(address))
	{
		DATA_EEPROM_Unlock();
//		DATA_EEPROM_EraseWord(address);
		DATA_EEPROM_FastProgramWord(address, data);
		DATA_EEPROM_Lock();
	}
}

/*uint8_t gBacklitDutyCycle = 0;
*/

uint8_t gShimmerFlag = 0;

void backlit_init(/*int8_t intensity*/)
{
	//gBacklitDutyCycle = intensity;

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	unsigned short prescalerValue = (unsigned short) (SystemCoreClock / 1000000) - 1;

	//Structure for timer settings
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// TIM2 clock enable
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

	// Enable the TIM3 gloabal Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_TimeBaseStructure.TIM_Period = 100;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Prescaler = prescalerValue;

	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

	// TIM Interrupts enable
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

	// TIM2 enable counter
	TIM_Cmd(TIM7, ENABLE);
}

void backlit_on(void)
{
	GPIOB->BSRRH = GPIO_Pin_0;
	GPIOA->BSRRH = GPIO_Pin_7;
}

void backlit_off(void)
{
	GPIOB->BSRRL = GPIO_Pin_0;
	GPIOA->BSRRL = GPIO_Pin_7;
}

/*void backlit_setIntensity(int8_t intensity)
{
	if (intensity > 100)
	{
		gBacklitDutyCycle = 100;
	}
	else if (intensity < 0)
	{
		gBacklitDutyCycle = 100;
	}
	else
	{
		gBacklitDutyCycle = intensity;
	}
}*/

static uint64_t backlitTimer = 0;
static uint16_t backlitModulo = 0;
static uint16_t shimmerModulo = 0;
static uint16_t shimmerDutyCycle = SHIMMER_DUTY_CYCLE;

void TIM7_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) == SET)
	{
		backlitTimer++;

		shimmerModulo = backlitTimer % SHIMMER_PERIOD;

		if(shimmerDutyCycle < shimmerModulo)
		{
			gShimmerFlag = 0;
		}
		else
		{
			gShimmerFlag = 1;
		}

		/*if (gBacklitDutyCycle == 0)
		{
			GPIOB->BSRRL = GPIO_Pin_0;
			GPIOA->BSRRL = GPIO_Pin_7;
		}
		else
		{
			backlitModulo = backlitTimer % LED_PERIOD;

			if(gBacklitDutyCycle < backlitModulo)
			{
				GPIOB->BSRRL = GPIO_Pin_0;
				GPIOA->BSRRL = GPIO_Pin_7;
			}
			else
			{
				GPIOB->BSRRH = GPIO_Pin_0;
				GPIOA->BSRRH = GPIO_Pin_7;
			}
		}*/

		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
	}
}

void gpio_init(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
	GPIO_InitTypeDef   GPIO_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource6);

	/* Configure EXTI0 line */
	EXTI_InitStructure.EXTI_Line = EXTI_Line6;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI0 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

uint8_t gpio_getValue(void)
{
	if (GPIOB->IDR&GPIO_Pin_6)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void keypad_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;

	GPIO_InitStructure.GPIO_Pin = BUTTON_UP_PIN;
	GPIO_Init(BUTTON_UP_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = BUTTON_LEFT_PIN|BUTTON_DOWN_PIN;
	GPIO_Init(BUTTON_LEFT_AND_DOWN_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = BUTTON_RIGHT_PIN;
	GPIO_Init(BUTTON_RIGHT_PORT, &GPIO_InitStructure);
}

KEYPAD_KEYS keypad_getStatus(void)
{
	uint8_t buttonState = 0;
	KEYPAD_KEYS retVal = KEY_NONE;

	if (!(BUTTON_UP_PORT->IDR&BUTTON_UP_PIN))
	{
		buttonState |= 1;
	}
	if (!(BUTTON_LEFT_AND_DOWN_PORT->IDR&BUTTON_DOWN_PIN))
	{
		buttonState |= 2;
	}
	if (!(BUTTON_RIGHT_PORT->IDR&BUTTON_RIGHT_PIN))
	{
		buttonState |= 4;
	}
	if (!(BUTTON_LEFT_AND_DOWN_PORT->IDR&BUTTON_LEFT_PIN))
	{
		buttonState |= 8;
	}

	switch(buttonState)
	{
		case 0:
			retVal = KEY_NONE;
			break;
		case 1:
			retVal = KEY_UP;
			break;
		case 2:
			retVal = KEY_DOWN;
			break;
		case 4:
			retVal = KEY_ENTER;
			break;
		case 8:
			retVal = KEY_ESCAPE;
			break;
		case 10:
			retVal = KEY_SHORTCUT1;
			break;
		case 12:
			retVal = KEY_SHORTCUT2;
			break;
		case 15:
			retVal = KEY_SHORTCUT3;
			break;
		case 3:
			retVal = KEY_SHORTCUT4;
			break;
		case 5:
			retVal = KEY_SHORTCUT5;
			break;
		default :
			retVal = KEY_NONE;
			break;
	}
	return retVal;
}
