/**
 * @file ili9163lcd.c
 * @brief ILI9163 128x128 LCD Driver
 *
 * This code has been ported from the ili9163lcd library for avr made
 * by Simon Inns, to run on a msp430.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 * @author Simon Inns <simon.inns@gmail.com>
 * @author Christopher Vagnetoft (NoccyLabs)
 * @copyright (C) 2012 Simon Inns
 * @copyright parts (C) 2012 NoccyLabs
 */

#include <string.h>

#include "ili9163.h"
#include "font5x8.h"
#include "spi.h"
#include "ssd1306.h"
#include "stm32l1xx.h"
#include <stdio.h>

uint16_t matrix_pom[128][128];
// Low-level LCD driving functions --------------------------------------------------------------------------

// Reset the LCD hardware
void lcdReset(void)
{
	// Reset pin is active low (0 = reset, 1 = ready)
	res_reset();
	Delay(10000);

	res_set();
	Delay(10000);
}

void lcdWriteCommand(uint8_t address)
{
	cd_reset();

	readWriteSPI2(address);
}

void lcdWriteParameter(uint8_t parameter)
{
	cd_set();

	readWriteSPI2(parameter);
}

void lcdWriteData(uint8_t dataByte1, uint8_t dataByte2)
{
	cd_set();

	readWriteSPI2(dataByte1);
	readWriteSPI2(dataByte2);
}

// Initialise the display with the require screen orientation
void lcdInitialise(uint8_t orientation)
{
	// Set up the IO ports for communication with the LCD


	// Hardware reset the LCD
	lcdReset();

    lcdWriteCommand(EXIT_SLEEP_MODE);
    Delay(10000); // Wait for the screen to wake up

    lcdWriteCommand(SET_PIXEL_FORMAT);
    lcdWriteParameter(0x05); // 16 bits per pixel

    lcdWriteCommand(SET_GAMMA_CURVE);
    lcdWriteParameter(0x04); // Select gamma curve 3

    lcdWriteCommand(GAM_R_SEL);
    lcdWriteParameter(0x01); // Gamma adjustment enabled

    lcdWriteCommand(POSITIVE_GAMMA_CORRECT);
    lcdWriteParameter(0x3f); // 1st Parameter
    lcdWriteParameter(0x25); // 2nd Parameter
    lcdWriteParameter(0x1c); // 3rd Parameter
    lcdWriteParameter(0x1e); // 4th Parameter
    lcdWriteParameter(0x20); // 5th Parameter
    lcdWriteParameter(0x12); // 6th Parameter
    lcdWriteParameter(0x2a); // 7th Parameter
    lcdWriteParameter(0x90); // 8th Parameter
    lcdWriteParameter(0x24); // 9th Parameter
    lcdWriteParameter(0x11); // 10th Parameter
    lcdWriteParameter(0x00); // 11th Parameter
    lcdWriteParameter(0x00); // 12th Parameter
    lcdWriteParameter(0x00); // 13th Parameter
    lcdWriteParameter(0x00); // 14th Parameter
    lcdWriteParameter(0x00); // 15th Parameter

    lcdWriteCommand(NEGATIVE_GAMMA_CORRECT);
    lcdWriteParameter(0x20); // 1st Parameter
    lcdWriteParameter(0x20); // 2nd Parameter
    lcdWriteParameter(0x20); // 3rd Parameter
    lcdWriteParameter(0x20); // 4th Parameter
    lcdWriteParameter(0x05); // 5th Parameter
    lcdWriteParameter(0x00); // 6th Parameter
    lcdWriteParameter(0x15); // 7th Parameter
    lcdWriteParameter(0xa7); // 8th Parameter
    lcdWriteParameter(0x3d); // 9th Parameter
    lcdWriteParameter(0x18); // 10th Parameter
    lcdWriteParameter(0x25); // 11th Parameter
    lcdWriteParameter(0x2a); // 12th Parameter
    lcdWriteParameter(0x2b); // 13th Parameter
    lcdWriteParameter(0x2b); // 14th Parameter
    lcdWriteParameter(0x3a); // 15th Parameter

    lcdWriteCommand(FRAME_RATE_CONTROL1);
    lcdWriteParameter(0x08); // DIVA = 8
    lcdWriteParameter(0x08); // VPA = 8

    lcdWriteCommand(DISPLAY_INVERSION);
    lcdWriteParameter(0x07); // NLA = 1, NLB = 1, NLC = 1 (all on Frame Inversion)

    lcdWriteCommand(POWER_CONTROL1);
    lcdWriteParameter(0x0a); // VRH = 10:  GVDD = 4.30
    lcdWriteParameter(0x02); // VC = 2: VCI1 = 2.65

    lcdWriteCommand(POWER_CONTROL2);
    lcdWriteParameter(0x02); // BT = 2: AVDD = 2xVCI1, VCL = -1xVCI1, VGH = 5xVCI1, VGL = -2xVCI1

    lcdWriteCommand(VCOM_CONTROL1);
    lcdWriteParameter(0x50); // VMH = 80: VCOMH voltage = 4.5
    lcdWriteParameter(0x5b); // VML = 91: VCOML voltage = -0.225

    lcdWriteCommand(VCOM_OFFSET_CONTROL);
    lcdWriteParameter(0x40); // nVM = 0, VMF = 64: VCOMH output = VMH, VCOML output = VML

    lcdWriteCommand(SET_COLUMN_ADDRESS);
    lcdWriteParameter(0x20); // XSH
    lcdWriteParameter(0x00); // XSL
    lcdWriteParameter(0x20); // XEH
    lcdWriteParameter(0x7f); // XEL (128 pixels x)

    lcdWriteCommand(SET_PAGE_ADDRESS);
    lcdWriteParameter(0x00);
    lcdWriteParameter(0x20);
    lcdWriteParameter(0x00);
    lcdWriteParameter(0xA0); // 128 pixels y

	// Select display orientation
    lcdWriteCommand(SET_ADDRESS_MODE);
	lcdWriteParameter(orientation);

	// Set the display to on
    lcdWriteCommand(SET_DISPLAY_ON);
    lcdWriteCommand(WRITE_MEMORY_START);
}

// LCD graphics functions -----------------------------------------------------------------------------------

void lcdClearDisplay(uint16_t colour)
{
	uint16_t pixel;

	// Set the column address to 0-127
	lcdWriteCommand(SET_COLUMN_ADDRESS);
	    lcdWriteParameter(0x20); // XSH
	    lcdWriteParameter(0x00); // XSL
	    lcdWriteParameter(0x20); // XEH
	    lcdWriteParameter(0x7f); // XEL (128 pixels x)

	    lcdWriteCommand(SET_PAGE_ADDRESS);
	    lcdWriteParameter(0x00);
	    lcdWriteParameter(0x20);
	    lcdWriteParameter(0x00);
	    lcdWriteParameter(0xA0); // 128 pixels y

	// Plot the pixels
	lcdWriteCommand(WRITE_MEMORY_START);
	for(pixel = 0; pixel < 16385; pixel++) lcdWriteData(colour >> 8, colour);
}

// LCD text manipulation functions --------------------------------------------------------------------------
#define pgm_read_byte_near(address_short) (uint16_t)(address_short)
// Plot a character at the specified x, y co-ordinates (top left hand corner of character)
void lcdPutCh(unsigned char character, uint8_t x, uint8_t y, uint16_t fgColour, uint16_t bgColour)
{
	uint8_t row, column;

	// To speed up plotting we define a x window of 6 pixels and then
	// write out one row at a time.  This means the LCD will correctly
	// update the memory pointer saving us a good few bytes

	lcdWriteCommand(SET_COLUMN_ADDRESS); // Horizontal Address Start Position
	lcdWriteParameter(0x20);
	lcdWriteParameter(x);
	lcdWriteParameter(0x20);
	lcdWriteParameter(x+5);

	lcdWriteCommand(SET_PAGE_ADDRESS); // Vertical Address end Position
	lcdWriteParameter(0x00);
	lcdWriteParameter(y+32);
	lcdWriteParameter(0x00);
	lcdWriteParameter(0xA0);

	lcdWriteCommand(WRITE_MEMORY_START);

	// Plot the font data
	for (row = 0; row < 8; row++)
	{
		for (column = 0; column < 6; column++)
		{
			if ((font5x8[character][column]) & (1 << row))
				lcdWriteData(fgColour>>8, fgColour);
			else lcdWriteData(bgColour >> 8, bgColour);
		}
	}
}

// Translates a 3 byte RGB value into a 2 byte value for the LCD (values should be 0-31)
uint16_t decodeRgbValue(uint8_t r, uint8_t g, uint8_t b)
{
	return (b << 11) | (g << 6) | (r);
}

// This routine takes a row number from 0 to 20 and
// returns the x coordinate on the screen (0-127) to make
// it easy to place text
uint8_t lcdTextX(uint8_t x) { return x*6; }

// This routine takes a column number from 0 to 20 and
// returns the y coordinate on the screen (0-127) to make
// it easy to place text
uint8_t lcdTextY(uint8_t y) { return y*8; }

// Plot a string of characters to the LCD
void lcdPutS(const char *string, uint8_t x, uint8_t y, uint16_t fgColour, uint16_t bgColour)
{
	uint8_t origin = x;
	uint8_t characterNumber;

	for (characterNumber = 0; characterNumber < strlen(string); characterNumber++)
	{
		// Check if we are out of bounds and move to
		// the next line if we are
		if (x > 121)
		{
			x = origin;
			y += 8;
		}

		// If we move past the bottom of the screen just exit
		if (y > 120) break;

		// Plot the current character
		lcdPutCh(string[characterNumber], x, y, fgColour, bgColour);
		x += 6;
	}
}

void matrixPlot(uint16_t matrix[128][128], int cisloTvaru){

	uint16_t pixels[62*128];
	uint16_t colour;

	int countPix=0;
	for(int i=0;i<128;i++){
		for(int j=56;j<118;j++){
			pixels[countPix]=matrix[j][i];
			countPix++;
		}
	}

	lcdWriteCommand(SET_COLUMN_ADDRESS); // Horizontal Address Start Position
	lcdWriteParameter(0x20);
	lcdWriteParameter(56);
	lcdWriteParameter(0x20);
	lcdWriteParameter(117);

	lcdWriteCommand(SET_PAGE_ADDRESS); // Vertical Address end Position
	lcdWriteParameter(56);
	lcdWriteParameter(0x20);
	lcdWriteParameter(56);
	lcdWriteParameter(160);

	lcdWriteCommand(WRITE_MEMORY_START);

	for (int x = 0; x < (62*128); x++){
		if (pixels[x]==0){
			colour = decodeRgbValue(0, 0, 0);
		}
		else if (pixels[x]==1){
			if (cisloTvaru == 0)
				colour = decodeRgbValue(31, 31, 0);
			if (cisloTvaru == 1 || cisloTvaru == 2)
				colour = decodeRgbValue(0, 31, 31);
			if (cisloTvaru == 3 || cisloTvaru == 4)
				colour = decodeRgbValue(31, 0, 0);
			if (cisloTvaru == 5 || cisloTvaru == 6)
				colour = decodeRgbValue(0, 31, 0);
			if (cisloTvaru == 7 || cisloTvaru == 8 || cisloTvaru == 9 || cisloTvaru == 10)
				colour = decodeRgbValue(0, 0, 31);
			if (cisloTvaru == 11 || cisloTvaru == 12 || cisloTvaru == 13 || cisloTvaru == 14)
				colour = decodeRgbValue(15, 0, 31);
			if (cisloTvaru == 15 || cisloTvaru == 16 || cisloTvaru == 17 || cisloTvaru == 18)
				colour = decodeRgbValue(31, 15, 31);
		}
		else if (pixels[x]==2){
			colour = decodeRgbValue(31, 31, 31);
		}
		else if (pixels[x]==3){
			colour = decodeRgbValue(31, 31, 0);
		}
		else if (pixels[x]==4){
			colour = decodeRgbValue(0, 31, 31);
		}
		else if (pixels[x]==5){
			colour = decodeRgbValue(31, 0, 0);
		}
		else if (pixels[x]==6){
			colour = decodeRgbValue(0, 31, 0);
		}
		else if (pixels[x]==7){
			colour = decodeRgbValue(0, 0, 31);
		}
		else if (pixels[x]==8){
			colour = decodeRgbValue(15, 0, 31);
		}
		else if (pixels[x]==9){
			colour = decodeRgbValue(31, 15, 31);
		}
		lcdWriteData(colour >> 8, colour);;
	}

}

void createDeleteBlock(uint16_t matrix[128][128], int16_t x0, int16_t y0, int cisloTvaru, int volba){
	// ak objekt je stvorec
	if (cisloTvaru == 0){
		for(int i=0;i<12;i++)
			for(int j=0;j<12;j++)
				if (y0-j>1)
					matrix[x0+i][y0-j]=volba;
	}
	// ak objekt je obdlznik |
	else if (cisloTvaru == 1){
		for(int i=0;i<6;i++)
			for(int j=0;j<24;j++)
				if (y0-j>1)
					matrix[x0+i][y0-j]=volba;
	}
	// ak objekt je obdlznik _
	else if (cisloTvaru == 2){
		for(int i=0;i<24;i++)
			for(int j=0;j<6;j++)
				if (y0-j>1)
					matrix[x0+i][y0-j]=volba;
	}
	// ak objekt je Z
	else if (cisloTvaru == 3){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j<6 && i>5)
						matrix[x0+i][y0-j]=volba;
					if (j>5 && i<12)
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
	// ak objekt je N
	else if (cisloTvaru == 4){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if (j>5 && i>5)
						matrix[x0+i][y0-j]=volba;
					if (j>5 && j<12)
						matrix[x0+i][y0-j]=volba;
					if (j<12 && i<6)
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
	// ak objekt je opaèný Z
	else if (cisloTvaru == 5){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j>5 && i>5)
						matrix[x0+i][y0-j]=volba;
					if (j<6 && i<12)
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
	// ak objekt je opacny N
	else if (cisloTvaru == 6){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if (j>5 && i<6)
						matrix[x0+i][y0-j]=volba;
					if (j>5 && j<12)
						matrix[x0+i][y0-j]=volba;
					if (j<12 && i>5)
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
	// ak objekt je L
	else if (cisloTvaru == 7){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if (j>5 && i<6)
						matrix[x0+i][y0-j]=volba;
					if (j<6)
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
	// ak objekt je _.
	else if (cisloTvaru == 8){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j<6)
						matrix[x0+i][y0-j]=volba;
					if (j>5 && i>11)
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
	// ak objekt je '|
	else if (cisloTvaru == 9){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if (j<12 && i>5)
						matrix[x0+i][y0-j]=volba;
					if (j>11)
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
	// ak objekt je ,..
	else if (cisloTvaru == 10){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j<6 && i<6)
						matrix[x0+i][y0-j]=volba;
					if (j>5)
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
	// ak objekt je _._
	else if (cisloTvaru == 11){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j<6)
						matrix[x0+i][y0-j]=volba;
					if (j>5 && (i>5 && i<12))
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
	// ak objekt je -|
	else if (cisloTvaru == 12){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if ((j>5 && j<12) && (i<6))
						matrix[x0+i][y0-j]=volba;
					if (i>5)
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
	// ak objekt je ..,..
	else if (cisloTvaru == 13){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j<6 && (i>5 && i<12))
						matrix[x0+i][y0-j]=volba;
					if (j>5)
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
	// ak objekt je |-
	else if (cisloTvaru == 14){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if (i<6)
						matrix[x0+i][y0-j]=volba;
					if ((j>5 && j<12) && (i>5))
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
	// ak objekt je opacny L
	else if (cisloTvaru == 15){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if (j<6)
						matrix[x0+i][y0-j]=volba;
					if (j>5 && i>5)
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
	// ak objekt je ..,
	else if (cisloTvaru == 16){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j<6 && i>11)
						matrix[x0+i][y0-j]=volba;
					if (j>5)
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
	// ak objekt je |'
	else if (cisloTvaru == 17){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if (j<12 && i<6)
						matrix[x0+i][y0-j]=volba;
					if (j>11)
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
	// ak objekt je _.
	else if (cisloTvaru == 18){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j<6)
						matrix[x0+i][y0-j]=volba;
					if (j>5 && i<6)
						matrix[x0+i][y0-j]=volba;
				}
			}
	}
}

int checkBlockade(uint16_t matrix[128][128], int16_t x0, int16_t y0, int cisloTvaru){
	int temp = 0;

	// ak objekt je stvorec
	if (cisloTvaru == 0){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0+1]==i || matrix[x0+6][y0+1]==i)
				temp = 1;
	}
	// ak objekt je |
	else if (cisloTvaru == 1){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0+1]==i)
				temp = 1;
	}
	// ak objekt je stvorec _
	else if (cisloTvaru == 2){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0+1]==i || matrix[x0+6][y0+1]==i || matrix[x0+12][y0+1]==i || matrix[x0+18][y0+1]==i)
				temp = 1;
	}
	// ak objekt je Z
	else if (cisloTvaru == 3){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0-5]==i || matrix[x0+6][y0+1]==i || matrix[x0+12][y0+1]==i)
				temp = 1;
	}
	// ak objekt je N
	else if (cisloTvaru == 4){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0+1]==i || matrix[x0+6][y0-5]==i)
				temp = 1;
	}
	// ak objekt je opacny Z
	else if (cisloTvaru == 5){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0+1]==i || matrix[x0+6][y0+1]==i || matrix[x0+12][y0-5]==i)
				temp = 1;
	}
	// ak objekt je opacny N
	else if (cisloTvaru == 6){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0-5]==i || matrix[x0+6][y0+1]==i)
				temp = 1;
	}
	// ak objekt je L
	else if (cisloTvaru == 7){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0+1]==i || matrix[x0+6][y0+1]==i)
				temp = 1;
	}
	// ak objekt je _.
	else if (cisloTvaru == 8){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0+1]==i || matrix[x0+6][y0+1]==i || matrix[x0+12][y0+1]==i)
				temp = 1;
	}
	// ak objekt je '|
	else if (cisloTvaru == 9){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0-11]==i || matrix[x0+6][y0+1]==i)
				temp = 1;
	}
	// ak objekt je ,..
	else if (cisloTvaru == 10){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0+1]==i || matrix[x0+6][y0-5]==i || matrix[x0+12][y0-5]==i)
				temp = 1;
	}
	// ak objekt je .|.
	else if (cisloTvaru == 11){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0+1]==i || matrix[x0+6][y0+1]==i || matrix[x0+12][y0+1]==i)
				temp = 1;
	}
	// ak objekt je -|
	else if (cisloTvaru == 12){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0-5]==i || matrix[x0+6][y0+1]==i)
				temp = 1;
	}
	// ak objekt je .,.
	else if (cisloTvaru == 13){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0-5]==i || matrix[x0+6][y0+1]==i || matrix[x0+12][y0-5]==i)
				temp = 1;
	}
	// ak objekt je |-
	else if (cisloTvaru == 14){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0+1]==i || matrix[x0+6][y0-5]==i)
				temp = 1;
	}
	// ak objekt je opacny L
	else if (cisloTvaru == 15){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0+1]==i || matrix[x0+6][y0+1]==i)
				temp = 1;
	}
	// ak objekt je ..,
	else if (cisloTvaru == 16){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0-5]==i || matrix[x0+6][y0-5]==i || matrix[x0+12][y0+1]==i)
				temp = 1;
	}
	// ak objekt je |'
	else if (cisloTvaru == 17){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0+1]==i || matrix[x0+6][y0-11]==i)
				temp = 1;
	}
	// ak objekt je ._
	else if (cisloTvaru == 18){
		for (int i = 2; i<10; i++)
			if (matrix[x0][y0+1]==i || matrix[x0+6][y0+1]==i || matrix[x0+12][y0+1]==i)
				temp = 1;
	}
	return temp;
}

int checkLeftSide(uint16_t matrix[128][128], int16_t x0, int16_t y0, int cisloTvaru){
	int temp = 0;
	// ak objekt je stvorec
	if (cisloTvaru == 0){
		for (int i = 2; i<10; i++)
			if (matrix[x0-1][y0]==i || matrix[x0-1][y0-6]==i)
				temp = 1;
	}
	// ak objekt je |
	else if (cisloTvaru == 1){
		for (int i = 2; i<10; i++)
			if (matrix[x0-1][y0]==i || matrix[x0-1][y0-6]==i || matrix[x0-1][y0-12]==i || matrix[x0-1][y0-18]==i)
				temp = 1;
	}
	// ak objekt je stvorec _
	else if (cisloTvaru == 2){
		for (int i = 2; i<10; i++)
			if (matrix[x0-1][y0]==i)
				temp = 1;
	}
	// ak objekt je Z
	else if (cisloTvaru == 3){
		if (matrix[x0+5][y0]==2 || matrix[x0+5][y0]==3 || matrix[x0-1][y0-6]==2 || matrix[x0-1][y0-6]==3)
			temp = 1;
	}
	// ak objekt je N
	else if (cisloTvaru == 4){
		for (int i = 2; i<10; i++)
			if (matrix[x0-1][y0]==i || matrix[x0-1][y0-6]==i || matrix[x0+5][y0-12]==i)
				temp = 1;
	}
	// ak objekt je opacny Z
	else if (cisloTvaru == 5){
		for (int i = 2; i<10; i++)
			if (matrix[x0-1][y0]==i || matrix[x0+5][y0-6]==i)
				temp = 1;
	}
	// ak objekt je opacny N
	else if (cisloTvaru == 6){
		for (int i = 2; i<10; i++)
			if (matrix[x0+5][y0]==i || matrix[x0-1][y0-6]==i || matrix[x0-1][y0-12]==i)
				temp = 1;
	}
	// ak objekt je L
	else if (cisloTvaru == 7){
		for (int i = 2; i<10; i++)
			if (matrix[x0-1][y0]==i || matrix[x0-1][y0-6]==i || matrix[x0-1][y0-12]==i)
				temp = 1;
	}
	// ak objekt je _.
	else if (cisloTvaru == 8){
		for (int i = 2; i<10; i++)
			if (matrix[x0-1][y0]==i || matrix[x0+11][y0-6]==i)
				temp = 1;
	}
	// ak objekt je '|
	else if (cisloTvaru == 9){
		for (int i = 2; i<10; i++)
			if (matrix[x0+5][y0]==i || matrix[x0+5][y0-6]==i || matrix[x0-1][y0-12]==i)
				temp = 1;
	}
	// ak objekt je ,..
	else if (cisloTvaru == 10){
		for (int i = 2; i<10; i++)
			if (matrix[x0-1][y0]==i || matrix[x0-1][y0-6]==i)
				temp = 1;
	}
	// ak objekt je .|.
	else if (cisloTvaru == 11){
		for (int i = 2; i<10; i++)
			if (matrix[x0-1][y0]==i || matrix[x0+5][y0-6]==i)
				temp = 1;
	}
	// ak objekt je -|
	else if (cisloTvaru == 12){
		for (int i = 2; i<10; i++)
			if (matrix[x0+5][y0]==i || matrix[x0-1][y0-6]==i || matrix[x0+5][y0-12]==i)
				temp = 1;
	}
	// ak objekt je .,.
	else if (cisloTvaru == 13){
		for (int i = 2; i<10; i++)
			if (matrix[x0+5][y0]==i || matrix[x0-1][y0-6]==i)
				temp = 1;
	}
	// ak objekt je |-
	else if (cisloTvaru == 14){
		for (int i = 2; i<10; i++)
			if (matrix[x0-1][y0]==i || matrix[x0-1][y0-6]==i || matrix[x0-1][y0-12]==i)
				temp = 1;
	}
	// ak objekt je opacny L
	else if (cisloTvaru == 15){
		for (int i = 2; i<10; i++)
			if (matrix[x0-1][y0]==i || matrix[x0+5][y0-6]==i || matrix[x0+5][y0-12]==i)
				temp = 1;
	}
	// ak objekt je ..,
	else if (cisloTvaru == 16){
		for (int i = 2; i<10; i++)
			if (matrix[x0+11][y0]==i || matrix[x0-1][y0-6]==i)
				temp = 1;
	}
	// ak objekt je |'
	else if (cisloTvaru == 17){
		for (int i = 2; i<10; i++)
			if (matrix[x0-1][y0]==i || matrix[x0-1][y0-6]==i || matrix[x0-1][y0-12]==i)
				temp = 1;
	}
	// ak objekt je ._
	else if (cisloTvaru == 18){
		for (int i = 2; i<10; i++)
			if (matrix[x0-1][y0]==i || matrix[x0-1][y0-6]==i)
				temp = 1;
	}
	return temp;
}

int checkRightSide(uint16_t matrix[128][128], int16_t x0, int16_t y0, int cisloTvaru){
	int temp = 0;
	// ak objekt je stvorec
	if (cisloTvaru == 0){
		for (int i = 2; i<10; i++)
			if (matrix[x0+12][y0]==i || matrix[x0+12][y0-6]==i)
				temp = 1;
	}
	// ak objekt je |
	else if (cisloTvaru == 1){
		for (int i = 2; i<10; i++)
			if (matrix[x0+6][y0]==i || matrix[x0+6][y0-6]==i || matrix[x0+6][y0-12]==i || matrix[x0+6][y0-18]==i)
				temp = 1;
	}
	// ak objekt je stvorec _
	else if (cisloTvaru == 2){
		for (int i = 2; i<10; i++)
			if (matrix[x0+12][y0]==i)
				temp = 1;
	}
	// ak objekt je Z
	else if (cisloTvaru == 3){
		for (int i = 2; i<10; i++)
			if (matrix[x0+18][y0]==i || matrix[x0+12][y0-6]==i)
				temp = 1;
	}
	// ak objekt je N
	else if (cisloTvaru == 4){
		for (int i = 2; i<10; i++)
			if (matrix[x0+6][y0]==i || matrix[x0+12][y0-6]==i || matrix[x0+12][y0-12]==i)
				temp = 1;
	}
	// ak objekt je opacny Z
	else if (cisloTvaru == 5){
		for (int i = 2; i<10; i++)
			if (matrix[x0+12][y0]==i || matrix[x0+18][y0-6]==i)
				temp = 1;
	}
	// ak objekt je opacny N
	else if (cisloTvaru == 6){
		for (int i = 2; i<10; i++)
			if (matrix[x0+12][y0]==i || matrix[x0+12][y0-6]==i || matrix[x0+6][y0-12]==i)
				temp = 1;
	}
	// ak objekt je L
	else if (cisloTvaru == 7){
		for (int i = 2; i<10; i++)
			if (matrix[x0+12][y0]==i || matrix[x0+6][y0-6]==i || matrix[x0+6][y0-12]==i)
				temp = 1;
	}
	// ak objekt je _.
	else if (cisloTvaru == 8){
		for (int i = 2; i<10; i++)
			if (matrix[x0+18][y0]==i || matrix[x0+18][y0-6]==i)
				temp = 1;
	}
	// ak objekt je '|
	else if (cisloTvaru == 9){
		for (int i = 2; i<10; i++)
			if (matrix[x0+12][y0]==i || matrix[x0+12][y0-6]==i || matrix[x0+12][y0-12]==i)
				temp = 1;
	}
	// ak objekt je ,..
	else if (cisloTvaru == 10){
		for (int i = 2; i<10; i++)
			if (matrix[x0+6][y0]==i || matrix[x0+18][y0-6]==i)
				temp = 1;
	}
	// ak objekt je .|.
	else if (cisloTvaru == 11){
		for (int i = 2; i<10; i++)
			if (matrix[x0+18][y0]==i || matrix[x0+12][y0-6]==i)
				temp = 1;
	}
	// ak objekt je -|
	else if (cisloTvaru == 12){
		for (int i = 2; i<10; i++)
			if (matrix[x0+12][y0]==i || matrix[x0+12][y0-6]==i || matrix[x0+12][y0-12]==i)
				temp = 1;
	}
	// ak objekt je .,.
	else if (cisloTvaru == 13){
		for (int i = 2; i<10; i++)
			if (matrix[x0+12][y0]==i || matrix[x0+18][y0-6]==i)
				temp = 1;
	}
	// ak objekt je |-
	else if (cisloTvaru == 14){
		for (int i = 2; i<10; i++)
			if (matrix[x0+6][y0]==i || matrix[x0+12][y0-6]==i || matrix[x0+6][y0-12]==i)
				temp = 1;
	}
	// ak objekt je opacny L
	else if (cisloTvaru == 15){
		for (int i = 2; i<10; i++)
			if (matrix[x0+12][y0]==i || matrix[x0+12][y0-6]==i || matrix[x0+12][y0-12]==i)
				temp = 1;
	}
	// ak objekt je ..,
	else if (cisloTvaru == 16){
		for (int i = 2; i<10; i++)
			if (matrix[x0+18][y0]==i || matrix[x0+18][y0-6]==i)
				temp = 1;
	}
	// ak objekt je |'
	else if (cisloTvaru == 17){
		for (int i = 2; i<10; i++)
			if (matrix[x0+6][y0]==i || matrix[x0+6][y0-6]==i || matrix[x0+12][y0-12]==i)
				temp = 1;
	}
	// ak objekt je ._
	else if (cisloTvaru == 18){
		for (int i = 2; i<10; i++)
			if (matrix[x0+18][y0]==i || matrix[x0+6][y0-6]==i)
				temp = 1;
	}
	return temp;
}

int checkLineFilled(uint16_t matrix[128][128]){
	int cRow[127];
	int temp = 0;
	int count=0;
	for (int x =0; x<127;x++){
		cRow[x]=0;
	}
	for(int i=0;i<127;i++){
		for(int j=57;j<117;j++){
			for (int z = 2; z<10; z++)
				if (matrix[j][i]==z)
					cRow[i] += 1;
				}
		if (cRow[i]==60){
			count++;
			for(int j=57;j<117;j++){
				matrix[j][i]=0;
			}
		}
	}
	for(int i=0;i<127;i++){
		for(int j=57;j<117;j++){
			if (cRow[i]==60){
				int pom = i;
				while(pom!=0){
					matrix[j][pom] = matrix[j][pom-1];
					pom--;
				}
			}
		}
	}
	if (count/6 == 1){
		temp = 100;
	}
	else if (count/6 == 2){
		temp = 200;
	}
	else if (count/6 == 3){
		temp = 300;
	}
	else if (count/6 == 4){
		temp = 800;
	}
	return temp;
}

int checkGameOver(uint16_t matrix[128][128], int16_t x0, int16_t y0, int cisloTvaru){
	int temp = 0;
	for (int i=0; i<19; i++)
		if (i!=2)
			for (int j=2; j<10; j++)
			if ((cisloTvaru == i) && (y0-6)==0 && (matrix[x0][y0+1]==j))
				temp=1;
	return temp;
}

void createFrame(uint16_t matrix[128][128]){
	for(int i=56;i<118;i++){
		for(int j=0;j<128;j++){
			if(i==56)
				matrix[i][j]=2;
			else if(i==117)
				matrix[i][j]=2;
			else if(j==127)
				matrix[i][j]=2;
			else
				matrix[i][j]=0;
		}
	}
}

void createText(char alias[7]){
	lcdPutS("Player:", lcdTextX(1), lcdTextY(1), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
	for (int i=0; i<7; i++)
		lcdPutCh(alias[i], lcdTextX(i+1), lcdTextY(2), decodeRgbValue(255, 255, 255), decodeRgbValue(0, 0, 0));
	lcdPutS("Lines:", lcdTextX(1), lcdTextY(4), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
	lcdPutS("Score:", lcdTextX(1), lcdTextY(7), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
	lcdPutS("Time:", lcdTextX(1), lcdTextY(10), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
	lcdPutS("Apm:", lcdTextX(1), lcdTextY(13), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
}

int rotateObject(int cisloTvaru){
	if (cisloTvaru == 1)
		cisloTvaru+=1;
	else if (cisloTvaru == 2)
	  cisloTvaru-=1;
	else if (cisloTvaru == 3)
	  cisloTvaru+=1;
	else if (cisloTvaru == 4)
	  cisloTvaru-=1;
	else if (cisloTvaru == 5)
	  cisloTvaru+=1;
	else if (cisloTvaru == 6)
	  cisloTvaru-=1;
	else if (cisloTvaru == 7)
	  cisloTvaru+=1;
	else if (cisloTvaru == 8)
	  cisloTvaru+=1;
	else if (cisloTvaru == 9)
	  cisloTvaru+=1;
	else if (cisloTvaru == 10)
	  cisloTvaru-=3;
	else if (cisloTvaru == 11)
	  cisloTvaru+=1;
	else if (cisloTvaru == 12)
	  cisloTvaru+=1;
	else if (cisloTvaru == 13)
	  cisloTvaru+=1;
	else if (cisloTvaru == 14)
	  cisloTvaru-=3;
	else if (cisloTvaru == 15)
	  cisloTvaru+=1;
	else if (cisloTvaru == 16)
	  cisloTvaru+=1;
	else if (cisloTvaru == 17)
	  cisloTvaru+=1;
	else if (cisloTvaru == 18)
	  cisloTvaru-=3;
	return cisloTvaru;
}

int checkRotation(uint16_t matrix[128][128], int16_t x0, int16_t y0, int cisloTvaru){
	int temp = 0;
	if (cisloTvaru == 1){
		for(int i=0;i<24;i++)
			for(int j=0;j<6;j++)
				for (int z = 2; z<10; z++)
					if (matrix[x0+i][y0-j]==z)
						temp = 1;
	}
	else if (cisloTvaru == 2){
		for(int i=0;i<6;i++)
			for(int j=0;j<24;j++)
				for (int z = 2; z<10; z++)
					if (matrix[x0+i][y0-j]==z)
						temp = 1;
	}
	else if (cisloTvaru == 3){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				for (int z = 2; z<10; z++){
					if (j>5 && i>5)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j>5 && j<12)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j<12 && i<6)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}
	else if (cisloTvaru == 4){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				for (int z = 2; z<10; z++){
					if (j<6 && i>5)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j>5 && i<12)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}
	else if (cisloTvaru == 5){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				for (int z = 2; z<10; z++){
					if (j>5 && i<6)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j>5 && j<12)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j<12 && i>5)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}
	else if (cisloTvaru == 6){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				for (int z = 2; z<10; z++){
					if (j>5 && i>5)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j<6 && i<12)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}
	else if (cisloTvaru == 7){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				for (int z = 2; z<10; z++){
					if (j<6)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j>5 && i>11)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}
	else if (cisloTvaru == 8){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				for (int z = 2; z<10; z++){
					if (j<12 && i>5)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j>11)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}
	else if (cisloTvaru == 9){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				for (int z = 2; z<10; z++){
					if (j<6 && i<6)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j>5)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}
	else if (cisloTvaru == 10){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				for (int z = 2; z<10; z++){
					if (j>5 && i<6)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j<6)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}
	else if (cisloTvaru == 11){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				for (int z = 2; z<10; z++){
					if ((j>5 && j<12) && (i<6))
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (i>5)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}
	else if (cisloTvaru == 12){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				for (int z = 2; z<10; z++){
					if (j<6 && (i>5 && i<12))
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j>5)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}
	else if (cisloTvaru == 13){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				for (int z = 2; z<10; z++){
					if (i<6)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if ((j>5 && j<12) && (i>5))
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}
	else if (cisloTvaru == 14){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				for (int z = 2; z<10; z++){
					if (j<6)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j>5 && (i>5 && i<12))
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}
	else if (cisloTvaru == 15){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				for (int z = 2; z<10; z++){
					if (j<6 && i>11)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j>5)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}
	else if (cisloTvaru == 16){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				for (int z = 2; z<10; z++){
					if (j<12 && i<6)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j>11)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}
	else if (cisloTvaru == 17){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				for (int z = 2; z<10; z++){
					if (j<6)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j>5 && i<6)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}
	else if (cisloTvaru == 18){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				for (int z = 2; z<10; z++){
					if (j<6)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
					if (j>5 && i>5)
						if (matrix[x0+i][y0-j]==z)
							temp = 1;
				}
			}
	}

	return temp;
}

void placeDownBlock(uint16_t matrix[128][128], int16_t x0, int16_t y0, int cisloTvaru){
	// ak objekt je stvorec
	if (cisloTvaru == 0){
		for(int i=0;i<12;i++)
			for(int j=0;j<12;j++)
				if (y0-j>1)
					matrix[x0+i][y0-j]=3;
	}
	// ak objekt je obdlznik |
	else if (cisloTvaru == 1){
		for(int i=0;i<6;i++)
			for(int j=0;j<24;j++)
				if (y0-j>1)
					matrix[x0+i][y0-j]=4;
	}
	// ak objekt je obdlznik _
	else if (cisloTvaru == 2){
		for(int i=0;i<24;i++)
			for(int j=0;j<6;j++)
				if (y0-j>1)
					matrix[x0+i][y0-j]=4;
	}
	// ak objekt je Z
	else if (cisloTvaru == 3){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j<6 && i>5)
						matrix[x0+i][y0-j]=5;
					if (j>5 && i<12)
						matrix[x0+i][y0-j]=5;

				}
			}
	}
	// ak objekt je N
	else if (cisloTvaru == 4){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if (j>5 && i>5)
						matrix[x0+i][y0-j]=5;
					if (j>5 && j<12)
						matrix[x0+i][y0-j]=5;
					if (j<12 && i<6)
						matrix[x0+i][y0-j]=5;
				}
			}
	}
	// ak objekt je opaèný Z
	else if (cisloTvaru == 5){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j>5 && i>5)
						matrix[x0+i][y0-j]=6;
					if (j<6 && i<12)
						matrix[x0+i][y0-j]=6;
				}
			}
	}
	// ak objekt je opacny N
	else if (cisloTvaru == 6){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if (j>5 && i<6)
						matrix[x0+i][y0-j]=6;
					if (j>5 && j<12)
						matrix[x0+i][y0-j]=6;
					if (j<12 && i>5)
						matrix[x0+i][y0-j]=6;
				}
			}
	}
	// ak objekt je L
	else if (cisloTvaru == 7){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if (j>5 && i<6)
						matrix[x0+i][y0-j]=7;
					if (j<6)
						matrix[x0+i][y0-j]=7;
				}
			}
	}
	// ak objekt je _.
	else if (cisloTvaru == 8){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j<6)
						matrix[x0+i][y0-j]=7;
					if (j>5 && i>11)
						matrix[x0+i][y0-j]=7;
				}
			}
	}
	// ak objekt je '|
	else if (cisloTvaru == 9){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if (j<12 && i>5)
						matrix[x0+i][y0-j]=7;
					if (j>11)
						matrix[x0+i][y0-j]=7;
				}
			}
	}
	// ak objekt je ,..
	else if (cisloTvaru == 10){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j<6 && i<6)
						matrix[x0+i][y0-j]=7;
					if (j>5)
						matrix[x0+i][y0-j]=7;
				}
			}
	}
	// ak objekt je _._
	else if (cisloTvaru == 11){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j<6)
						matrix[x0+i][y0-j]=8;
					if (j>5 && (i>5 && i<12))
						matrix[x0+i][y0-j]=8;
				}
			}
	}
	// ak objekt je -|
	else if (cisloTvaru == 12){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if ((j>5 && j<12) && (i<6))
						matrix[x0+i][y0-j]=8;
					if (i>5)
						matrix[x0+i][y0-j]=8;
				}
			}
	}
	// ak objekt je ..,..
	else if (cisloTvaru == 13){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j<6 && (i>5 && i<12))
						matrix[x0+i][y0-j]=8;
					if (j>5)
						matrix[x0+i][y0-j]=8;
				}
			}
	}
	// ak objekt je |-
	else if (cisloTvaru == 14){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if (i<6)
						matrix[x0+i][y0-j]=8;
					if ((j>5 && j<12) && (i>5))
						matrix[x0+i][y0-j]=8;
				}
			}
	}
	// ak objekt je opacny L
	else if (cisloTvaru == 15){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if (j<6)
						matrix[x0+i][y0-j]=9;
					if (j>5 && i>5)
						matrix[x0+i][y0-j]=9;
				}
			}
	}
	// ak objekt je ..,
	else if (cisloTvaru == 16){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j<6 && i>11)
						matrix[x0+i][y0-j]=9;
					if (j>5)
						matrix[x0+i][y0-j]=9;
				}
			}
	}
	// ak objekt je |'
	else if (cisloTvaru == 17){
		for(int i=0;i<12;i++)
			for(int j=0;j<18;j++){
				if (y0-j>1){
					if (j<12 && i<6)
						matrix[x0+i][y0-j]=9;
					if (j>11)
						matrix[x0+i][y0-j]=9;
				}
			}
	}
	// ak objekt je _.
	else if (cisloTvaru == 18){
		for(int i=0;i<18;i++)
			for(int j=0;j<12;j++){
				if (y0-j>1){
					if (j<6)
						matrix[x0+i][y0-j]=9;
					if (j>5 && i<6)
						matrix[x0+i][y0-j]=9;
				}
			}
	}
}

int generateNumber(volatile int AD_value){
	int cislo = AD_value%7;
	int cisloTvaru = 0;
	int temp = 0;

	if (cislo == 0)
		cisloTvaru = 0;
	else if (cislo == 1){
		temp = AD_value%2;
		if (temp == 0)
			cisloTvaru = 1;
		else if (temp == 1)
			cisloTvaru = 2;
	}
	else if (cislo == 2){
		temp = AD_value%2;
		if (temp == 0)
			cisloTvaru = 3;
		else if (temp == 1)
			cisloTvaru = 4;
	}
	else if (cislo == 3){
		temp = AD_value%2;
		if (temp == 0)
			cisloTvaru = 5;
		else if (temp == 1)
			cisloTvaru = 6;
	}
	else if (cislo == 4){
		temp = AD_value%4;
		if (temp == 0)
			cisloTvaru = 7;
		else if (temp == 1)
			cisloTvaru = 8;
		else if (temp == 2)
			cisloTvaru = 9;
		else if (temp == 3)
			cisloTvaru = 10;
	}
	else if (cislo == 5){
		temp = AD_value%4;
		if (temp == 0)
			cisloTvaru = 11;
		else if (temp == 1)
			cisloTvaru = 12;
		else if (temp == 2)
			cisloTvaru = 13;
		else if (temp == 3)
			cisloTvaru = 14;
	}
	else if (cislo == 6){
		temp = AD_value%4;
		if (temp == 0)
			cisloTvaru = 15;
		else if (temp == 1)
			cisloTvaru = 16;
		else if (temp == 2)
			cisloTvaru = 17;
		else if (temp == 3)
			cisloTvaru = 18;
	}

	return cisloTvaru;
}

int returnLines(int tempScore, int score){
	int lines = 0;
	int temp = score - tempScore;

	switch(temp){
		case 0:
			lines = 0;
			break;
		case 100:
			lines = 1;
			break;
		case 200:
			lines = 2;
			break;
		case 300:
			lines = 3;
			break;
		case 800:
			lines = 4;
			break;
	}
	return lines;
}

// Funkcia vykresli startovaciu obrazovku a riadi pohyb medzi volbami
void drawMenu(volatile int AD_value, int volba, char* menuVolba[]){
	lcdPutS(".TETRIS.", lcdTextX(7), lcdTextY(2), decodeRgbValue(10, 31, 10), decodeRgbValue(0, 0, 0));
	lcdPutS("THE STM32 GAME", lcdTextX(4), lcdTextY(4), decodeRgbValue(15, 31, 0), decodeRgbValue(0, 0, 0));
	int x = 0;
	int j = 1;

	for (int i = 0; i < 3; i++){
		if (i == 0)
			x = 7;
		else if (i == 1)
			x = 4;
		else if (i == 2)
			x = 6;
		if(i == volba)
			lcdPutS(menuVolba[i], lcdTextX(x), lcdTextY(i+7+j), decodeRgbValue(255, 255, 255), decodeRgbValue(31, 0, 0));
		else
			lcdPutS(menuVolba[i], lcdTextX(x), lcdTextY(i+7+j), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
		j++;
	}
}

// Funkcia vrati hodnotu vybranej volby
int returnVolba(volatile int AD_value, int volba){
	if ((AD_value > 1700) && (AD_value < 2300)){
		volba += 1;
		if (volba > 2)
			volba = 0;
	}
	else if ((AD_value > 2500) && (AD_value < 3100)){
		volba -= 1;
		if (volba < 0)
			volba = 2;
	}
	return volba;
}

// Funkcia vrati hodnotu dalsieho vybranej okna
int returnRun(volatile int AD_value, int volba, int run){
	if (((AD_value > 3300) && (AD_value < 3650)) && volba == 0){
		lcdClearDisplay(decodeRgbValue(0, 0, 0));
		run = 1;
	}
	else if (((AD_value > 3300) && (AD_value < 3650)) && volba == 1){
		lcdClearDisplay(decodeRgbValue(0, 0, 0));
		run = 2;
	}
	else if (((AD_value > 3300) && (AD_value < 3650)) && volba == 2){
		lcdClearDisplay(decodeRgbValue(0, 0, 0));
		run = 3;
	}
	return run;
}

// Funkcia vykresli poradie, nazvy a score najlepsich hracov
void showHighscore(int highscore[], char* names[]){
	char hScore[6];
	int j = 5;

	for (int i = 0; i < 7; i++){
		lcdPutCh(names[0][i], lcdTextX(i+6), lcdTextY(5), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
		lcdPutCh(names[1][i], lcdTextX(i+6), lcdTextY(7), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
		lcdPutCh(names[2][i], lcdTextX(i+6), lcdTextY(9), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
		lcdPutCh(names[3][i], lcdTextX(i+6), lcdTextY(11), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
		lcdPutCh(names[4][i], lcdTextX(i+6), lcdTextY(13), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
		if (i < 5){
			sprintf(hScore, "%d", highscore[i]);
			lcdPutS(hScore, lcdTextX(15), lcdTextY(j), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
			lcdPutS(":", lcdTextX(14), lcdTextY(j), decodeRgbValue(255, 255, 255), decodeRgbValue(0, 0, 0));
			j = j + 2;
		}
	}
	lcdPutS("HIGHSCORE", lcdTextX(6), lcdTextY(2), decodeRgbValue(10, 31, 10), decodeRgbValue(0, 0, 0));
	lcdPutS("1.", lcdTextX(3), lcdTextY(5), decodeRgbValue(255, 255, 255), decodeRgbValue(0, 0, 0));
	lcdPutS("2.", lcdTextX(3), lcdTextY(7), decodeRgbValue(255, 255, 255), decodeRgbValue(0, 0, 0));
	lcdPutS("3.", lcdTextX(3), lcdTextY(9), decodeRgbValue(255, 255, 255), decodeRgbValue(0, 0, 0));
	lcdPutS("4.", lcdTextX(3), lcdTextY(11), decodeRgbValue(255, 255, 255), decodeRgbValue(0, 0, 0));
	lcdPutS("5.", lcdTextX(3), lcdTextY(13), decodeRgbValue(255, 255, 255), decodeRgbValue(0, 0, 0));
	lcdPutS("BACK", lcdTextX(1), lcdTextY(15), decodeRgbValue(255, 255, 255), decodeRgbValue(31, 0, 0));
}

// Funkcia prepne hodnotu run na 0 ak je stlacene hociktore tlacidlo
int goBack(volatile int AD_value, int run){
	if ((AD_value > 1700) && (AD_value < 3650)){
		lcdClearDisplay(decodeRgbValue(0, 0, 0));
		run = 0;
	}
	return run;
}

// Funkcia vypise na obrazovku ABC a umoznuje prepnut medzi nimi
void drawABC(int abcVolba, char* abc[]){
	int k = 2;
	int y = 4;
	lcdPutS("PICK CHARACTERS", lcdTextX(3), lcdTextY(1), decodeRgbValue(10, 31, 10), decodeRgbValue(0, 0, 0));
	for(int i = 0; i < 29; i++){
		if (k < 17 && y < 10){
			k = k + 2;
		}
		else if (k > 16 && y < 10){
			y = y + 2;
			k = 2;
		}
		else if (k < 17 && y > 9){
			k = k + 7;
		}
		if (i == abcVolba)
			if (y > 9)
				lcdPutS(abc[i], lcdTextX(k), lcdTextY(y), decodeRgbValue(31, 0, 0), decodeRgbValue(10, 31, 10));
			else
				lcdPutS(abc[i], lcdTextX(k), lcdTextY(y), decodeRgbValue(255, 255, 255), decodeRgbValue(31, 0, 0));
		else
			if (y > 9)
				lcdPutS(abc[i], lcdTextX(k), lcdTextY(y), decodeRgbValue(10, 31, 10), decodeRgbValue(0, 0, 0));
			else
				lcdPutS(abc[i], lcdTextX(k), lcdTextY(y), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
	}
}

// Funkcia vrati hodnotu ABC volby
int returnAbcVolba(volatile int AD_value, int abcVolba){
	if ((AD_value > 1700) && (AD_value < 2300)){
		abcVolba += 1;
		if (abcVolba > 28)
			abcVolba = 0;
	}
	else if ((AD_value > 2500) && (AD_value < 3100)){
		abcVolba -= 1;
		if (abcVolba < 0)
			abcVolba = 28;
	}
	return abcVolba;
}

// Funkcia vymeni meno hraca alebo vrati naspat do menu
void changeName(volatile int AD_value, int abcVolba, int *index, char newAlias[7], char abc2[], int *run, char alias[7]){
	if ((AD_value > 3300) && (AD_value < 3650)){
		if (abcVolba < 26 && *index < 7){
			newAlias[*index] = abc2[abcVolba];
			lcdPutCh(newAlias[*index], lcdTextX(*index + 7), lcdTextY(12), decodeRgbValue(31, 31, 0), decodeRgbValue(0, 0, 0));
			*index = *index + 1;
		}
		else if (abcVolba < 26 && *index > 6){
			lcdPutS("MAX 7 CHARS!!!", lcdTextX(4), lcdTextY(14), decodeRgbValue(255, 255, 255), decodeRgbValue(0, 0, 0));
		}
		else if (abcVolba == 26){
			*index = 0;
			lcdClearDisplay(decodeRgbValue(0, 0, 0));
			*run = 0;
		}
		else if (abcVolba == 27 && *index > 0){
			*index = *index - 1;
			newAlias[*index] = ' ';
			lcdPutCh(newAlias[*index], lcdTextX(*index + 7), lcdTextY(12), decodeRgbValue(255, 255, 255), decodeRgbValue(0, 0, 0));
			lcdPutS("MAX 7 CHARS!!!", lcdTextX(4), lcdTextY(14), decodeRgbValue(0, 0, 0), decodeRgbValue(0, 0, 0));
		}
		else if (abcVolba == 28){
			for (int i = 0; i < 7; i++)
				if (newAlias[i] == ' ')
					alias[i] = ' ';
				else
					alias[i] = newAlias[i];
			lcdClearDisplay(decodeRgbValue(0, 0, 0));
			*index = 0;
			*run = 0;
		}
	}
}

// Funkcia vykresli text na obraze game over
void drawGameOver(char scoree[7], int score, int highscore[], char* names[], char alias[7], char time[7], char pm[7]){
	if (score >= highscore[0]){
		highscore[0] = score;
		names[0] = alias;
	}
	else if (score < highscore[0]  && score >= highscore[1]){
		highscore[1] = score;
		names[1] = alias;
	}
	else if (score < highscore[1]  && score >= highscore[2]){
		highscore[2] = score;
		names[2] = alias;
	}
	else if (score < highscore[2]  && score >= highscore[3]){
		highscore[3] = score;
		names[3] = alias;
	}
	else if (score < highscore[3]  && score >= highscore[4]){
		highscore[4] = score;
		names[4] = alias;
	}
	lcdPutS("Game Over!", lcdTextX(6), lcdTextY(2), decodeRgbValue(10, 31, 10), decodeRgbValue(0, 0, 0));
	lcdPutS("Chin up", lcdTextX(4), lcdTextY(6), decodeRgbValue(31, 31, 31), decodeRgbValue(0, 0, 0));
	lcdPutS(alias, lcdTextX(12), lcdTextY(6), decodeRgbValue(31, 31, 0), decodeRgbValue(0, 0, 0));
	lcdPutS("Your score :", lcdTextX(2), lcdTextY(8), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
	lcdPutS(scoree, lcdTextX(15), lcdTextY(8), decodeRgbValue(31, 31, 31), decodeRgbValue(0, 0, 0));
	lcdPutS("Your time  :", lcdTextX(2), lcdTextY(10), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
	lcdPutS(time, lcdTextX(15), lcdTextY(10), decodeRgbValue(31, 31, 31), decodeRgbValue(0, 0, 0));
	lcdPutS("Your p/m   :", lcdTextX(2), lcdTextY(12), decodeRgbValue(31, 0, 0), decodeRgbValue(0, 0, 0));
	lcdPutS(pm, lcdTextX(15), lcdTextY(12), decodeRgbValue(31, 31, 31), decodeRgbValue(0, 0, 0));
	lcdPutS("BACK", lcdTextX(1), lcdTextY(15), decodeRgbValue(255, 255, 255), decodeRgbValue(31, 0, 0));
}

// Funkcia resetuje parametre pre novu hru
void clearData(volatile int AD_value, int *score, float *t, int *lines, int *apm, int *run, uint8_t blockX[1000], uint8_t blockY[1000], uint8_t xDir[1000], uint8_t yDir[1000], int *count, uint16_t matrix[128][128]){
	if ((AD_value > 1700) && (AD_value < 3650)){
		lcdClearDisplay(decodeRgbValue(0, 0, 0));
		*score = 0;
		*t = 0;
		*lines = 0;
		*apm = 0;
		*run = 0;
		for (int i = 0; i < 1000; i++){
			blockX[i] = 81;
			blockY[i] = 0;
			xDir[i] = 6;
			yDir[i] = 6;
		}
		*count = 0;
		createFrame(matrix);
	}
}

// Funkcia riadi jednotlive klavesy
void buttonPressed(volatile int AD_value, uint8_t *xDir, uint16_t matrix[128][128], uint8_t *blockX, uint8_t *blockY, int *cisloTvaru, int *cc){
	// ked gombiky su stlacene, tak posuva objekt dolava alebo doprava
	if ((AD_value > 1700) && (AD_value < 2300)){
		*xDir = 6;
		// v kazdom kroku checkuje ci sa nenachadza nieco na lavej strane objektu
		if (checkLeftSide(matrix, *blockX, *blockY, *cisloTvaru))
		  *xDir = 0;
		else
		  *blockX -= *xDir; // dolava
	}
	else if ((AD_value > 2500) && (AD_value < 3100)){
		*xDir = 6;
		// v kazdom kroku checkuje ci sa nenachadza nieco na pravej strane objektu
		if (checkRightSide(matrix, *blockX, *blockY, *cisloTvaru))
			*xDir = 0;
		else
			*blockX += *xDir; // doprava
	}
	// ak stlacime stvrte tlacidlo, otoci sa objekt
	else if ((AD_value > 3520) && (AD_value < 3650) && *cc == 0){
		if (!checkRotation(matrix, *blockX, *blockY, *cisloTvaru)){
			*cisloTvaru = rotateObject(*cisloTvaru);
			*cc=1;
		}
	}
	// ak stlacime tretie tlacidlo, tak posunutie dole je zrychlene
	else if ((AD_value > 3300) && (AD_value < 3450)){
		if (checkBlockade(matrix, *blockX, *blockY+12, *cisloTvaru))
			*blockY += 0;
		else
			*blockY += 6;
	}
}

// Funkcia checkuje ci sa nenachadza objekt pred danym tvarom a checkuje ci sa nenastane koniec hry
void checkObstacleAndGameOver(uint16_t matrix[128][128], uint8_t *blockX, uint8_t *blockY, int *cisloTvaru, uint8_t *yDir, int *run, int *count, volatile int AD_value){
	// v kazdom kroku checkuje, ci sa nenachadza dalsi objekt alebo ramec pred objektom
	if (checkBlockade(matrix, *blockX, *blockY, *cisloTvaru))
	{
	  // zastavi sa objekt
	  *yDir = 0;
	  // necha objekt na konecnom mieste
	  placeDownBlock(matrix, *blockX, *blockY, *cisloTvaru);
	  // GAME OVER
	  if(checkGameOver(matrix, *blockX, *blockY, *cisloTvaru)){
		  matrixPlot(matrix, *cisloTvaru);
		  lcdClearDisplay(decodeRgbValue(0, 0, 0));
		  *run = 4;
	  }
	  // vygenerujeme dalsi objekt
	  *count = *count + 1;
	  if (*count > 999)
		  *run = 4;
	  *cisloTvaru = generateNumber(AD_value);
	}
}

//

