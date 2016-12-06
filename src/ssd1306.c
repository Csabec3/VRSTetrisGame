/*
 * ssd1306.c
 *
 *  Created on: Oct 1, 2015
 *      Author: Jozef Rodina
 */

#include "ssd1306.h"
#include "spi.h"

uint8_t Contrast_level=0xf0;

uint8_t num[]={0x00,0xF8,0xFC,0x04,0x04,0xFC,0xF8,0x00,0x00,0x03,0x07,0x04,0x04,0x07,0x03,0x00,  	   /*--  ÎÄ×Ö:  0~9  --*/
0x00,0x00,0x08,0xFC,0xFC,0x00,0x00,0x00,0x00,0x00,0x04,0x07,0x07,0x04,0x00,0x00,
0x00,0x18,0x1C,0x84,0xC4,0x7C,0x38,0x00,0x00,0x06,0x07,0x05,0x04,0x04,0x04,0x00,
0x00,0x08,0x0C,0x24,0x24,0xFC,0xD8,0x00,0x00,0x02,0x06,0x04,0x04,0x07,0x03,0x00,
0x80,0xE0,0x70,0x18,0xFC,0xFC,0x00,0x00,0x00,0x01,0x01,0x05,0x07,0x07,0x04,0x00,
0x00,0x7C,0x7C,0x24,0x24,0xE4,0xC4,0x00,0x00,0x03,0x07,0x04,0x04,0x07,0x03,0x00,
0x00,0xF0,0xF8,0x6C,0x24,0xEC,0xCC,0x00,0x00,0x03,0x07,0x04,0x04,0x07,0x03,0x00,
0x00,0x0C,0x0C,0xC4,0xFC,0x3C,0x04,0x00,0x00,0x00,0x00,0x07,0x07,0x00,0x00,0x00,
0x00,0x98,0xFC,0x64,0x64,0xFC,0x98,0x00,0x00,0x03,0x07,0x04,0x04,0x07,0x03,0x00,
0x00,0x78,0xFC,0x84,0xC4,0xFC,0xF8,0x00,0x00,0x06,0x06,0x04,0x06,0x03,0x01,0x00};


void display_Contrast_level(uint8_t number)
{	uint8_t number1,number2,number3;
	number1=number/100;number2=number%100/10;number3=number%100%10;
	Set_Column_Address(Start_column+0*8);
	Set_Page_Address(Start_page);
    Write_number(num,number1,0);
	Set_Column_Address(Start_column+1*8);
	Set_Page_Address(Start_page);
	Write_number(num,number2,1);
	Set_Column_Address(Start_column+2*8);
	Set_Page_Address(Start_page);
	Write_number(num,number3,2);

}

void Write_number(uint8_t *n,uint8_t k,uint8_t station_dot)
{uint8_t i;
			for(i=0;i<8;i++)
				{
				Write_Data(*(n+16*k+i));
				}

	Set_Page_Address(Start_page+1)	;
    Set_Column_Address(Start_column+station_dot*8);
			for(i=8;i<16;i++)
				{
				Write_Data(*(n+16*k+i));
				}
}

void Delay(uint16_t n)
{
	uint32_t nl = n*2;
	while(nl--);


	return;
}



void Write_Data(unsigned char dat)
{
	cd_set();
	device_Select();

	readWriteSPI2(dat);
	device_Unselect();
}




void Write_Instruction(unsigned char cmd)
{
	cd_reset();
	device_Select();

	readWriteSPI2(cmd);
	device_Unselect();
}




// Set page address 0~7
void Set_Page_Address(unsigned char add)
{
    add=0xb0|add;
    Write_Instruction(add);
	return;
}

void Set_Column_Address(unsigned char add)
{
    Write_Instruction((0x10|(add>>4)));
	Write_Instruction((0x0f&add));
	return;
}



void Set_Contrast_Control_Register(unsigned char mod)
{
    Write_Instruction(0x81);
	Write_Instruction(mod);
	return;
}


void ssd1306_init(void)
{
	res_set();
	Delay(2000);
	res_reset();
	Delay(2000);
	res_set();

	Delay(2000);

 	Write_Instruction(0xae);//--turn off oled panel

	Write_Instruction(0xd5);//--set display clock divide ratio/oscillator frequency
	Write_Instruction(0x80);//--set divide ratio

	Write_Instruction(0xa8);//--set multiplex ratio(1 to 64)
	Write_Instruction(0x3f);//--1/64 duty

	Write_Instruction(0xd3);//-set display offset
	Write_Instruction(0x00);//-not offset


	Write_Instruction(0x8d);//--set Charge Pump enable/disable
	Write_Instruction(0x14);//--set(0x10) disable


	Write_Instruction(0x40);//--set start line address

	Write_Instruction(0xa6);//--set normal display

	Write_Instruction(0xa4);//Disable Entire Display On

	Write_Instruction(0xa1);//--set segment re-map 128 to 0

	Write_Instruction(0xC8);//--Set COM Output Scan Direction 64 to 0

	Write_Instruction(0xda);//--set com pins hardware configuration
	Write_Instruction(0x12);

	Write_Instruction(0x81);//--set contrast control register
	Write_Instruction(50);


	Write_Instruction(0xd9);//--set pre-charge period
	Write_Instruction(0xf1);

	Write_Instruction(0xdb);//--set vcomh
	Write_Instruction(0x40);



	Write_Instruction(0xaf);//--turn on oled panel
}



void Display_Chess(unsigned char value)
{
    unsigned char i,j,k;
    for(i=0;i<0x08;i++)
	{
		Set_Page_Address(i);

        Set_Column_Address(0x00);

		for(j=0;j<0x10;j++)
		{
		    for(k=0;k<0x04;k++)
		        Write_Data(value);
		    for(k=0;k<0x04;k++)
		        Write_Data(~value);
		}
	}
    return;
}


void Display_Chinese(unsigned char ft[])
{
    unsigned char i,j,k,b,c=0;
	unsigned int	num=0;

for(b=0;b<4;b++)
  {
    for(i=0;i<0x02;i++)
	{	Set_Page_Address(c);
    	Set_Column_Address(0x00);
	    num=i*0x10+b*256;
		for(j=0;j<0x08;j++)
		{
            for(k=0;k<0x10;k++)
			{
		        Write_Data(ft[num+k]);
			}
			num+=0x20;
		}c++;
	}
  }
    return;
}


//Display_Chinese1
void Display_Chinese_Column(unsigned char ft[])
{
    unsigned char i,j,k,num=0x40;
    for(i=0;i<0x08;i++)
	{
		Set_Page_Address(i);
        Set_Column_Address(0x00);
		for(j=0;j<0x08;j++)
		{
            for(k=0;k<0x10;k++)
			{
		        Write_Data(ft[num+k]);
			}
		}
	num+=0x10;
	}
    return;
}


void Display_Picture(unsigned char pic[])
{
    unsigned char i,j,num=0;
	for(i=0;i<0x08;i++)
	{
	Set_Page_Address(i);
    Set_Column_Address(0x00);
        for(j=0;j<0x80;j++)
		{
		    Write_Data(pic[i*0x80+j]);
		}
	}
    return;
}



/*void main(void)
{
	IE=0x81;
	IP=0x01;
	TCON=0x01;
	int0=1;
	Delay(100);

    Initial();

	while(1)
	{
        Display_Picture(pic);
		Delay(65000);

		Write_Instruction(0xa7);//--set Inverse Display
		Delay(65000);

		Write_Instruction(0xa6);//--set normal display
		Display_Picture(pic1);
		Delay(65000);

		Write_Instruction(0xa7);//--set Inverse Display
		Delay(65000);

		Write_Instruction(0xa6);//--set normal display
		Display_Picture(pic2);
		Delay(65000);

		Write_Instruction(0xa7);//--set Inverse Display
		Delay(65000);

		Write_Instruction(0xa6);//--set normal display
		Display_Picture(pic3);
		Delay(65000);

		Write_Instruction(0xa7);//--set Inverse Display
		Delay(65000);

		Write_Instruction(0xa6);//--set normal display
		Display_Picture(pic4);
		Delay(65000);

		Write_Instruction(0xa7);//--set Inverse Display
		Delay(65000);

		Write_Instruction(0xa6);//--set normal display
		Display_Chess(0x0f);
		Delay(65000);

		Write_Instruction(0xa7);//--set Inverse Display
		Delay(65000);

		Write_Instruction(0xa6);//--set normal display
		Display_Chinese(font);
		Delay(65000);

		Write_Instruction(0xa7);//--set Inverse Display
		Display_Chinese(font);
		Delay(65000);

		Write_Instruction(0xa6);//--set normal display
		Display_Chinese_Column(font);
		Delay(65000);

		Write_Instruction(0xa7);//--set Inverse Display
		Display_Chinese_Column(font);
		Delay(65000);

		Write_Instruction(0xa6);//--set normal display
	}
}*/
