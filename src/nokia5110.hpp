/*
 * nokia5110.hpp
 * 6/15/12
 */

#ifndef NOKIA5110_H_
#define NOKIA5110_H_

#ifdef	__cplusplus
extern "C" {
#endif

#define nokiaVcc GPIO_Pin_6   //connect to pin  PC 6 on discovery board
#define nokiaSce GPIO_Pin_7   //connect to pin  PC 7 on discovery board
#define nokiaRst GPIO_Pin_8    //connect to pin  PA 8 on discovery board
#define nokiaDc  GPIO_Pin_9    //connect to pin  PA 9 on discovery board
#define nokiaBlight GPIO_Pin_14  //connect to pin  PB 14 on discovery board
#define inverse 0x0d //display control inverse mode
#define BandW 0x0c //display control black on white
#define vAddress  0x22
#define hAddress  0x20

#define PIXEL_OFF 0
#define PIXEL_ON  1
#define PIXEL_XOR 2


class nokia5110 {

private:
	void writeCommand(unsigned char);
	void writeData(unsigned char);

public:

	nokia5110();

	// Init/Clear/position functions
	void init(void);
	void clear(void);
	void gotoXY(unsigned char x, unsigned char y);
	void update(void);

	// String and character functions
	void writeString(unsigned char x, unsigned char y, const char *s);
	void writeChar(unsigned char ch);
	void printV(unsigned char x, unsigned char y, unsigned char length,
			const char *characters);

	// Bitmap functions
	void drawBitmap(unsigned char x, unsigned char y, const unsigned char *map,
			unsigned char w, unsigned char h);
	void clearBitmap(unsigned char x, unsigned char y, unsigned char size_x,
			unsigned char size_y);

	// Graphic functions
	void setPixel(unsigned char x, unsigned char y, unsigned char c);
	void drawLine(unsigned char x1, unsigned char y1, unsigned char x2,
			unsigned char y2, unsigned char c);
	void drawRectangle(unsigned char x1, unsigned char y1, unsigned char x2,
			unsigned char y2, unsigned char c);
	void drawFilledRectangle(unsigned char x1, unsigned char y1,
			unsigned char x2, unsigned char y2, unsigned char c);
	void drawCircle(unsigned char xc, unsigned char yc, unsigned char r,
			unsigned char c);

	// buffer functions
	void drawPointDMA(unsigned char col, unsigned char row, unsigned char c);
	void fillBufferTest();
	void loadBufferDMA(unsigned char *buf, unsigned int length);
	void clearBufferDMA();

    //Mandelbrot functions
	void mandel();
	void zoom(float multiplier);
	void panLeftRight(float multiplier);

};


#ifdef	__cplusplus
}
#endif

#endif /* NOKIA5110_H_ */
