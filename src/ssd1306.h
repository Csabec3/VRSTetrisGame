/*
 * ssd1306.h
 *
 *  Created on: Oct 1, 2015
 *      Author: Jozef Rodina
 */

#ifndef SSD1306_H_
#define SSD1306_H_

#include <stdint.h>

#define Start_column	0x00
#define Start_page		0x00
#define	StartLine_set	0x00

void Write_number(uint8_t *n,uint8_t k,uint8_t station_dot);
void Delay1(uint16_t n);
void display_Contrast_level(uint8_t number);
void adj_Contrast(void);
void Delay(uint16_t n);
void Write_Data(unsigned char dat);
void Write_Instruction(unsigned char cmd);
void Set_Page_Address(unsigned char add);
void Set_Column_Address(unsigned char add);
void Set_Contrast_Control_Register(unsigned char mod);
void Display_Chess(unsigned char value);
void Display_Chinese(unsigned char ft[]);
void Display_Chinese_Column(unsigned char ft[]);
void Display_Picture(unsigned char pic[]);

void ssd1306_init(void);

#endif /* SSD1306_H_ */
