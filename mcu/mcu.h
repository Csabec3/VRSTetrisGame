/**
  ******************************************************************************
  * @file    firmware/src/mcu/mcu.h
  * @author  Jozef Rodina
  * @version V1.0.0
  * @date    20-January-2012
  * @brief   MCU file - Some kind of HAL.
  ******************************************************************************
  *
  * <h2><center>&copy; COPYRIGHT 2012 Jozef Rodina</center></h2>
  ******************************************************************************
  */
#ifndef __MCU_H
#define __MCU_H

#include "stm32l1xx.h"
#include "stm32l1xx_conf.h"
#include "stm32l1xx_i2c.h"
#include "stm32l1xx_pwr.h"
#include "stm32l1xx_spi.h"
#include "stm32l1xx_gpio.h"
#include "stm32l1xx_syscfg.h"

#define BUTTON_UP_PORT	GPIOB
#define BUTTON_UP_PIN	GPIO_Pin_1

#define BUTTON_DOWN_PIN		GPIO_Pin_15

#define BUTTON_LEFT_AND_DOWN_PORT	GPIOC
#define BUTTON_LEFT_PIN		GPIO_Pin_13

#define BUTTON_RIGHT_PORT	GPIOA
#define BUTTON_RIGHT_PIN	GPIO_Pin_15

#define LED_PERIOD	100;
#define SHIMMER_PERIOD 10000;
#define SHIMMER_DUTY_CYCLE 5000;

typedef enum
{
    KEY_NONE = 0,
    KEY_UP,
    KEY_DOWN,
    KEY_ESCAPE,
    KEY_ENTER,
    KEY_SHORTCUT1,
    KEY_SHORTCUT2,
    KEY_SHORTCUT3,
    KEY_SHORTCUT4,
    KEY_SHORTCUT5
}KEYPAD_KEYS;

typedef enum
{
    PWM_GO_DOWN = 0,
    PWM_GO_UP
}SW_PWM_STATES;

#define nop()	asm("nop")

void delay_us(unsigned long us);

#define eeprom_read32(address) *((uint32_t*)(address))
void eeprom_write32(uint32_t address,uint32_t data);

void backlit_init(void/*int8_t intensity*/);
//void backlit_setIntensity(int8_t intensity);
void backlit_on(void);
void backlit_off(void);

void gpio_init(void);
uint8_t gpio_getValue(void);

void keypad_init(void);
KEYPAD_KEYS keypad_getStatus(void);

#endif
