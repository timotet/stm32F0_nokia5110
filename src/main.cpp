/*
 * main.cpp
 * 6/15/12
 *
 */
#include "stm32f0_discovery.h"
#include "spi.hpp"
#include "stm32f0xx.h"
#include "nokia5110.hpp"
#include "Delay.h"
#include "ti.hpp"
#include "ST.hpp"

/*Private defines---------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure;

/* function prototypes -----------------------------------------------*/
static void GPIO_Config(void);
static void SysTickConfig(void);
//for Mandelbrot
double Pmax, Pmin, Qmax, Qmin; //image boundaries

////////////////////////////////////////////main//////////////////////////////////////////
int main(void) {

	nokia5110 lcd = nokia5110();
    SysTickConfig();
    GPIO_Config();
	SPI_Config();
	lcd.init();

	while (1) {
		//lcd.clear();
		lcd.writeString(8, 4, "Nokia 5110");
		lcd.writeString(30, 6, "LCD");

		Delay(10000000);
		char j = 60;
		char k = 20;
		lcd.drawLine(0,0,15,6*2,PIXEL_ON);
		lcd.drawLine(6,0,21,6*2,PIXEL_ON);
		lcd.drawLine(12,0,24,6*2,PIXEL_ON);
		lcd.drawLine(18,0,27,6*2,PIXEL_ON);
		lcd.drawLine(24,0,30,6*2,PIXEL_ON);
		for (int i = 0; i < 20; i++) {
			lcd.drawLine(j++, 2, k++, 6 * 5, PIXEL_ON);
			Delay(100000);
		}
		Delay(10000000);
		lcd.clear();
		lcd.drawRectangle(63, 0, 56, 6 * 7, PIXEL_ON);
		Delay(200000);
		lcd.drawRectangle(0, 1, 32, 6 * 3, PIXEL_ON);
		Delay(200000);
		lcd.drawRectangle(20, 20, 83, 3 * 6, PIXEL_ON);
		Delay(200000);
		lcd.drawRectangle(50, 3, 30, 3 * 6, PIXEL_ON);
		Delay(200000);
		lcd.drawRectangle(30, 4, 60, 6 * 2, PIXEL_ON);
		Delay(200000);
		lcd.drawRectangle(5, 5 * 6, 14, 4 * 6, PIXEL_ON);
		Delay(200000);
		lcd.drawRectangle(57, 6 * 4, 22, 5 * 6, PIXEL_ON);
		Delay(200000);
		lcd.drawRectangle(4, 6 * 7, 70, 4 * 6, PIXEL_ON);
		Delay(200000);
		lcd.drawRectangle(4, 18, 36, 3 * 6, PIXEL_ON);
		int n = 41;
		int o = 6 * 3;
		int p = 6 * 3;
		int q = 41;
		for (int m = 0; m < 15; m++) {
			lcd.drawRectangle(n--, o--, q++, p++, PIXEL_ON);
			Delay(100000);
		}
		lcd.drawFilledRectangle(65,4*6,83,6*6,PIXEL_ON);
		Delay(5000000);
		lcd.clear();
		lcd.drawCircle(10, 10, 5, PIXEL_ON);
		Delay(200000);
		lcd.drawCircle(35, 40, 10, PIXEL_ON);
		Delay(200000);
		lcd.drawCircle(60, 23, 8, PIXEL_ON);
		Delay(200000);
		lcd.drawCircle(50, 30, 13, PIXEL_ON);
		Delay(200000);
		lcd.drawCircle(40, 20, 20, PIXEL_ON);
		Delay(200000);
		lcd.drawCircle(70, 40, 5, PIXEL_ON);
		Delay(200000);
		lcd.drawCircle(62, 31, 10, PIXEL_ON);
		Delay(200000);
		lcd.drawCircle(43, 13, 8, PIXEL_ON);
		Delay(200000);
		lcd.drawCircle(22, 17, 13, PIXEL_ON);
		Delay(200000);
		lcd.drawCircle(67, 35, 17, PIXEL_ON);
		Delay(200000);
		lcd.drawCircle(58, 28, 20, PIXEL_ON);
		for (int l = 0; l < 15; l++) {
			lcd.drawCircle(58, 28, l, PIXEL_ON);
			Delay(100000);
		}
		Delay(10000000);
		lcd.clear();
		lcd.drawBitmap(0, 0, ST, 84, 48);
		Delay(10000000);
		lcd.clear();
		lcd.drawBitmap(0, 0, ti, 48, 48);
		Delay(1000000);
		for (int i = 0; i < 38; i++) {
			lcd.drawBitmap(i, 0, ti, 48, 48);
			Delay(100000);
		}
		Delay(10000000);
		lcd.clear();

		//Mandelbrot set

		Pmax = 9.49930897284705;
		Pmin = -13.4977220670337;
		Qmax = 7.66528639334017;
		Qmin = -7.66606763324697; //initial coordinates
		while ((Pmax - Pmin) > 0.1181641) //zoom in
		{
			lcd.panLeftRight(0.005);
			lcd.mandel();
			lcd.zoom(-0.2); //zoom in 20 percent
		}
		while (Pmax < -1.740722663272) //pan right
		{
			lcd.panLeftRight(0.1); //pan right 20 percent
			lcd.mandel();
		}

        lcd.clearBufferDMA();
		//lcd.clear();
		//lcd.fillBufferTest();
		//Delay(10000000);

        lcd.clear();


		lcd.printV(15, 1, 3, "well");
	    lcd.printV(30, 0, 5, "  it  ");
		lcd.printV(45, 1, 4, "works");
		lcd.printV(60, 1, 4, "neat!");
		Delay(10000000);
		lcd.clear();

	}

}
/////////////////////////Functions/////////////////////////////

static void GPIO_Config(void) {

    /* GPIOB Periph clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	/* Configure PB14 in outpu mode */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	/* GPIOC Periph clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	/* Configure PC7,PC6 in output mode */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	/* GPIOA Periph clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	/* Configure PA8,PA9 in output mode */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    GPIO_InitStructure.GPIO_Pin = nokiaBlight;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = nokiaSce | nokiaVcc;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = nokiaRst | nokiaDc;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/*initialize led 3 on discovery board for systick*/
	STM_EVAL_LEDInit(LED3);
	STM_EVAL_LEDInit(LED4);

}
/**
 * @brief  Configure a SysTick Base time to 10 ms.
 * @param  None
 * @retval None
 */
static void SysTickConfig(void) {
	/* Setup SysTick Timer for 10ms interrupts  */
	if (SysTick_Config(SystemCoreClock / 100)) {
		/* Capture error */
		while (1)
			;
	}
	/* Configure the SysTick handler priority */
	NVIC_SetPriority(SysTick_IRQn, 0x0);
}



