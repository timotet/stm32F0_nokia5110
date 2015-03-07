/*
 * nokia5110.cpp
 * 6/15/12
 */
//#include "stdlib.h"
#include "spi.hpp"
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include "Delay.h"
#include "nokia5110.hpp"
#include "font8x8.h"
#include "ti.hpp"

#define LCDCOLMAX	84
#define LCDROWMAX	6
#define LCDPIXELROWMAX	48

const unsigned int DMA_Buffer_Size = 504;
// current cursor position
static unsigned char cursor_row = 0; /* 0-5 */
static unsigned char cursor_col = 0; /* 0-83 */

static unsigned char lcd_buffer[LCDROWMAX][LCDCOLMAX];
static unsigned char DMA_buffer[DMA_Buffer_Size] = {0};
static unsigned char * DMA_address;
static unsigned int refresh;
int bufferCounter = 0;



// Default constructor
nokia5110::nokia5110() {
}
/*
 * Name         : init
 * Description  : Main LCD initialization function
 * Argument(s)  : None
 * Return value : None
 */
void nokia5110::init(void) {

	GPIO_SetBits(GPIOC, nokiaVcc); //power to LCD
	GPIO_SetBits(GPIOA, nokiaRst); //set RESET high
	GPIO_ResetBits(GPIOA, nokiaRst); //set RESET low
	Delay(500000);
	GPIO_SetBits(GPIOA, nokiaRst); //set RESET high
	GPIO_SetBits(GPIOC, nokiaSce); //set Sce high
	writeCommand(0x21); // LCD Extended instruction set
	writeCommand(0xF0); // Set LCD Vop (Contrast). //0xE0 - BF  may have to play with
	writeCommand(0x07); // Set Temp coefficient. //0x04 =t0 //0x05=t1 // 0x06=t2 // 0x07=t3
	writeCommand(0x13); // LCD bias mode 1:100 0x10 //1:48 0x13
	writeCommand(0x20); // LCD basic instruction set
	writeCommand(BandW); // LCD  0x0C for black on white //0x0d for inverse
	GPIO_SetBits(GPIOB, nokiaBlight); //power to back light
	clear();
}

/*
 * Name         : writeCommand
 * Description  : Sends command to display controller
 * Argument(s)  : command - The command to be sent
 * Return value : none
 */
void nokia5110::writeCommand(unsigned char command) {

	GPIO_ResetBits(GPIOA, nokiaDc); //set LCD to command mode dc low
	GPIO_ResetBits(GPIOC, nokiaSce); //SCE pin low
	// make sure the transmit buffer is free
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
		;
	SPI_SendData8(SPI2, command); // shift out 8 bits
	// check the TX flag
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
		;
	// check the busy flag
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET)
		;
}

/*
 * Name         : writeData
 * Description  : Sends data to the display controller
 * Argument(s)  : Date - Data to be sent
 * Return value : none
 */
void nokia5110::writeData(unsigned char data) {

	GPIO_SetBits(GPIOA, nokiaDc); //set LCD to data mode dc high
	GPIO_ResetBits(GPIOC, nokiaSce); //SCE pin low
	// make sure the transmit buffer is free
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
		;
	SPI_SendData8(SPI2, data); // shift out 8 bits
	// check the TX flag
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
		;
	// check the busy flag
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET)
		;
}

/*
 * Name         : drawBitmap
 * Description  : Sends a bitmap image stored in ram to the display
 * Argument(s)  : x, y - Position on screen, x 0-83, y 1-6
 *                map - pointer to data
 *                size_x,size_y - Size of the image in pixels,
 *                size_y is multiple of 8
 * Return value : none
 */
void nokia5110::drawBitmap(unsigned char x, unsigned char y,
		const unsigned char * map, unsigned char w, unsigned char h) {
	unsigned int i, n;
	unsigned char row;
	row = (h % 8 == 0) ? h / 8 : h / 8 + 1;
	for (n = 0; n < row; n++) {
		gotoXY(x, y);
		for (i = 0; i < w; i++) {
			writeData(map[i + n * w]);
		}
		y++;
	}
}

/*
 * Name         : clearBitmap
 * Description  : Clear an area of the screen, usually to blank out a
 * 		  previously drawn image or part of image.
 * Argument(s)  : x, y - Position on screen, x 0-83, y 1-6
 *                size_x,size_y - Size of the image in pixels,
 *                size_y is multiple of 8
 * Return value : none
 */
void nokia5110::clearBitmap(unsigned char x, unsigned char y,
		unsigned char size_x, unsigned char size_y) {
	unsigned int i, n;
	unsigned char row;

	row = (size_y % 8 == 0) ? size_y / 8 : size_y / 8 + 1;
	for (n = 0; n < row; n++) {
		gotoXY(x, y);
		for (i = 0; i < size_x; i++) {
			writeData(0x00);
		}
		y++;
	}
}

/*
 * Name         : writeString
 * Description  : Write a string to the LCD from current position
 * Argument(s)  : x - Column to start at, 0-83
 * 		  y - Row to start, 0-6
 * 		  s - Pointer to start of string
 * 		  mode - Mode of string, reverse or normal
 *
 * Return value : none
 */
void nokia5110::writeString(unsigned char x, unsigned char y, const char *string) {
	gotoXY(x, y);
	while (*string)
		writeChar(*string++);
}

/*
 * Name         : writeChar
 * Description  : Write a single normal font character to screen
 * 		  at current cursor position
 * Argument(s)  : ch - character to display
 *                mode - reverse or normal
 * Return value : none
 */
void nokia5110::writeChar(unsigned char ch) {
	unsigned char j;

	if (cursor_col > LCDCOLMAX - 6)
		cursor_col = LCDCOLMAX - 6; // ensure space is available for the character
	if (cursor_row > LCDROWMAX - 1)
		cursor_row = LCDROWMAX - 1; // ensure space is available for the character
	lcd_buffer[cursor_row][cursor_col] = 0x00;
	for (j = 0; j < 7; j++) {
		lcd_buffer[cursor_row][cursor_col + j] = (font8x8[ch][j]);
	}

	lcd_buffer[cursor_row][cursor_col + 6] = 0x00;

	for (j = 0; j < 7; j++) {
		writeData(lcd_buffer[cursor_row][cursor_col++]);
		if (cursor_col >= LCDCOLMAX) {
			cursor_col = 0;
			cursor_row++;
			if (cursor_row >= LCDROWMAX)
				cursor_row = 0;
		}
	}
}

/*
 * Name         : gotoXY
 * Description  : Move text position cursor to specified position
 * Argument(s)  : x, y - Position, x = 0-83, y = 0-6
 * Return value : none
 */
void nokia5110::gotoXY(unsigned char x, unsigned char y) {
	if (x > LCDCOLMAX - 1)
		x = LCDCOLMAX - 1; // ensure within limits
	if (y > LCDROWMAX - 1)
		y = LCDROWMAX - 1; // ensure within limits

	writeCommand(0x80 | x); //column
	writeCommand(0x40 | y); //row

	cursor_row = y;
	cursor_col = x;
}

/*
 * Name         : clear
 * Description  : Clear the screen and display buffer
 * Argument(s)  : none
 * Return value : none
 */
void nokia5110::clear(void) {
	int i, j;

	gotoXY(0, 0); //start with (0,0) home position

	for (i = 0; i < LCDROWMAX; i++) {
		for (j = 0; j < LCDCOLMAX; j++) {
			writeData(0x00);
			lcd_buffer[i][j] = 0x00;
		}
	}

	gotoXY(0, 0); //bring the XY position back to (0,0)
}

/*
 * Name         : update
 * Description  : Write the screen buffer to the display memory
 * Argument(s)  : none
 * Return value : none
 */
void nokia5110::update(void) {
	int i, j;

	for (i = 0; i < LCDROWMAX; i++) {
		gotoXY(0, i);
		for (j = 0; j < LCDCOLMAX; j++) {
			writeData(lcd_buffer[i][j]);
		}
	}
	gotoXY(0, 0); //bring the XY position back to (0,0)
}

/*
 * Name         : setPixel
 * Description  : Set a single pixel either on or off, update display buffer.
 * Argument(s)  : x,y - position, x = 0-83, y = 0-6
 *                c - colour, either PIXEL_ON, PIXEL_OFF or PIXEL_XOR
 * Return value : none
 */
void nokia5110::setPixel(unsigned char x, unsigned char y, unsigned char c) {
	unsigned char value;
	unsigned char row;

	if (x < 0 || x >= LCDCOLMAX || y < 0 || y >= LCDPIXELROWMAX)
		return;

	row = y / 8;

	value = lcd_buffer[row][x];
	if (c == PIXEL_ON) {
		value |= (1 << (y % 8));
	} else if (c == PIXEL_XOR) {
		value ^= (1 << (y % 8));
	} else {
		value &= ~(1 << (y % 8));
	}

	lcd_buffer[row][x] = value;

	gotoXY(x, row);
	writeData(value);
}

/*
 * Name         : drawLine
 * Description  : Draws a line between two points on the display.
 * Argument(s)  : x1, y1 - Absolute pixel coordinates for line origin.
 *                x2, y2 - Absolute pixel coordinates for line end.
 *                c - either PIXEL_ON, PIXEL_OFF or PIXEL_XOR
 * Return value : none
 */
void nokia5110::drawLine(unsigned char x1, unsigned char y1, unsigned char x2,
		unsigned char y2, unsigned char c) {
	int dx, dy, stepx, stepy, fraction;

	/* Calculate differential form */
	/* dy   y2 - y1 */
	/* -- = ------- */
	/* dx   x2 - x1 */

	/* Take differences */
	dy = y2 - y1;
	dx = x2 - x1;

	/* dy is negative */
	if (dy < 0) {
		dy = -dy;
		stepy = -1;
	} else {
		stepy = 1;
	}

	/* dx is negative */
	if (dx < 0) {
		dx = -dx;
		stepx = -1;
	} else {
		stepx = 1;
	}

	dx <<= 1;
	dy <<= 1;

	/* Draw initial position */
	setPixel(x1, y1, c);

	/* Draw next positions until end */
	if (dx > dy) {
		/* Take fraction */
		fraction = dy - (dx >> 1);
		while (x1 != x2) {
			if (fraction >= 0) {
				y1 += stepy;
				fraction -= dx;
			}
			x1 += stepx;
			fraction += dy;

			/* Draw calculated point */
			setPixel(x1, y1, c);
		}
	} else {
		/* Take fraction */
		fraction = dx - (dy >> 1);
		while (y1 != y2) {
			if (fraction >= 0) {
				x1 += stepx;
				fraction -= dy;
			}
			y1 += stepy;
			fraction += dx;

			/* Draw calculated point */
			setPixel(x1, y1, c);
		}
	}
}

/*
 * Name         : drawRectangle
 * Description  : Draw a rectangle given the top left and bottom right points
 * Argument(s)  : x1, y1 - Absolute pixel coordinates for top left corner
 *                x2, y2 - Absolute pixel coordinates for bottom right corner
 *                c - either PIXEL_ON, PIXEL_OFF or PIXEL_XOR
 * Return value : none
 */
void nokia5110::drawRectangle(unsigned char x1, unsigned char y1,
		unsigned char x2, unsigned char y2, unsigned char c) {
	drawLine(x1, y1, x2, y1, c);
	drawLine(x1, y1, x1, y2, c);
	drawLine(x1, y2, x2, y2, c);
	drawLine(x2, y1, x2, y2, c);
}

/*
 * Name         : drawFilledRectangle
 * Description  : Draw a filled rectangle given to top left and bottom right points
 * 		  just simply draws horizontal lines where the rectangle would be
 * Argument(s)  : x1, y1 - Absolute pixel coordinates for top left corner
 *                x2, y2 - Absolute pixel coordinates for bottom right corner
 *                c - either PIXEL_ON, PIXEL_OFF or PIXEL_XOR
 * Return value : none
 */
void nokia5110::drawFilledRectangle(unsigned char x1, unsigned char y1,
		unsigned char x2, unsigned char y2, unsigned char c) {
	for (int i = y1; i <= y2; i++) {
		drawLine(x1, i, x2, i, c);
	}
}

/*
 * Name         : drawCircle
 * Description  : Draw a circle using Bresenham's algorithm.
 * 		  Some small circles will look like squares!!
 * Argument(s)  : xc, yc - Center of circle
 * 		  r - Radius
 * 		  c - either PIXEL_ON, PIXEL_OFF or PIXEL_XOR
 * Return value : None
 */
void nokia5110::drawCircle(unsigned char xc, unsigned char yc, unsigned char r,
		unsigned char c) {

	int ddF_x = 1;
	int ddF_y = -2 * r;
	int f = 1 - r;
	int x = 0;
	int y = r;

	setPixel(xc, yc + r, c);
	setPixel(xc, yc - r, c);
	setPixel(xc + r, yc, c);
	setPixel(xc - r, yc, c);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		setPixel((xc + x), (yc + y), c);
		setPixel((xc - x), (yc + y), c);
		setPixel((xc + x), (yc - y), c);
		setPixel((xc - x), (yc - y), c);

		setPixel((xc + y), (yc + x), c);
		setPixel((xc - y), (yc + x), c);
		setPixel((xc + y), (yc - x), c);
		setPixel((xc - y), (yc - x), c);
	}

}

void nokia5110::printV(unsigned char x, unsigned char y, unsigned char length,
		const char *characters) {
	for (length = 0; length < 5; length++) {
		gotoXY(x, y);
		y++;
		writeChar(*characters++);
	}
}

 void nokia5110::drawPointDMA(unsigned char col, unsigned char row, unsigned char c){

  if (col < 0 || col >= LCDCOLMAX || row < 0 || row >= LCDPIXELROWMAX)
  return;

  unsigned int pages = row / 8;
  unsigned int index = pages * LCDCOLMAX + col;
  unsigned char byteOffset = row % 8;
  unsigned char mask;


  if(c % 2 == 0){
	  mask = 1 << byteOffset;
	  DMA_buffer[index] |= mask;
  }
  else{
	  mask = ~(1 << byteOffset);
      DMA_buffer[index] &= mask;
  }

}

void nokia5110::fillBufferTest() {
	for (unsigned int i = 0; i < DMA_Buffer_Size; i++) {
			DMA_buffer[i] = 0xFF;
	}

	loadBufferDMA(DMA_buffer,DMA_Buffer_Size);
	spiPutBufferDMA(DMA_buffer,DMA_Buffer_Size);

}

void nokia5110::clearBufferDMA() {
	for (unsigned int i = 0; i < DMA_Buffer_Size; i++) {
			DMA_buffer[i] = 0x00;
	}

	//loadBufferDMA(DMA_buffer,DMA_Buffer_Size);
	spiPutBufferDMA(DMA_buffer,DMA_Buffer_Size);

}

void nokia5110::loadBufferDMA(unsigned char * buf, unsigned int length) {

	DMA_address = buf;
	refresh = length;
}

//Mandelbrot related functions

extern double Pmax, Pmin, Qmax, Qmin; //image boundaries
void nokia5110::mandel() //calculate the Mandelbrot set
{
	int col;
	int row;
	short color; //number of iterations for a point (P, Q)
	double P, Q, deltaP, deltaQ, //P: real, Q: imaginary
			Xlast, Ylast, Xcur, Ycur, //temporary Z components used during iterations
			modulus; //squared modulus of the complex number

	deltaP = (Pmax - Pmin) / (LCDCOLMAX - 1); //real axis increment between points
	deltaQ = (Qmax - Qmin) / (LCDPIXELROWMAX - 1); //imaginary axis increment
	//clear();

	for (row = 0; row < LCDPIXELROWMAX; row++) { //look at each point on the screen
		for (col = 0; col < LCDCOLMAX; col++) {
			P = Pmin + col * deltaP; //find coordinates of current point
			Q = Qmin + row * deltaQ;
			Xlast = Ylast = modulus = 0.0; //initialize variables
			color = 0;
			while ((modulus < 4) && (color < 50)) { //iterate until it blows up, or iteration limit reached
				Xcur = Xlast * Xlast - Ylast * Ylast + P; //find current Z
				Ycur = 2 * Xlast * Ylast + Q;
				color++; //increment loop counter (used to set point color)
				Xlast = Xcur; //update variables for next loop
				Ylast = Ycur;
				modulus = Xcur * Xcur + Ycur * Ycur; //find size squared
			}
			    //if(color % 2 == 0)
				//setPixel(col, row, PIXEL_ON); //color this point
				drawPointDMA(col, row, color); //color this point in buffer

		   }

	    }

	 spiPutBufferDMA(DMA_buffer,DMA_Buffer_Size);
	}


void nokia5110::zoom(float multiplier) {
	float deltaP, deltaQ;

	multiplier *= 0.5; //use half since it's applied to both parameters
	deltaP = multiplier * (Pmax - Pmin);
	deltaQ = multiplier * (Qmax - Qmin);
	Pmin -= deltaP;
	Pmax += deltaP;
	Qmin -= deltaQ;
	Qmax += deltaQ;
}

void nokia5110::panLeftRight(float multiplier) {
	float delta;

	delta = multiplier * (Pmax - Pmin);
	Pmin += delta;
	Pmax += delta;
}

