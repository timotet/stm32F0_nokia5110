/*
 * spi.cpp
 * this is for stm32f0 discovery connected to a nokia5110 lcd
 * 6/15/12
 */

#include "spi.hpp"
#include "stm32f0_discovery.h"
#include "nokia5110.hpp"

/* Private typedef -----------------------------------------------------------*/
SPI_InitTypeDef SPI_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
DMA_InitTypeDef DMA_InitStructure;
extern GPIO_InitTypeDef GPIO_InitStructure;


// Set up SPI2 peripheral
void SPI_Config(void) {
	// enable SPI peripheral clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	// enable the peripheral GPIO port clocks
	RCC_AHBPeriphClockCmd(SPI2_SCK_GPIO_CLK | SPI2_MOSI_GPIO_CLK, ENABLE);

	// Connect GPIO pins for SPI as alternate function
	GPIO_PinAFConfig(SPI2_SCK_GPIO_PORT, SPI2_SCK_SOURCE, SPI2_SCK_AF);
	GPIO_PinAFConfig(SPI2_MOSI_GPIO_PORT, SPI2_MOSI_SOURCE, SPI2_MOSI_AF);
	//GPIO_PinAFConfig(SPI2_NSS_GPIO_PORT, SPI2_NSS_SOURCE, SPI2_NSS_AF);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	// SPI SCK pin configuration
	GPIO_InitStructure.GPIO_Pin = SPI2_SCK_PIN;
	GPIO_Init(SPI2_SCK_GPIO_PORT, &GPIO_InitStructure);

	// SPI  MOSI pin configuration
	GPIO_InitStructure.GPIO_Pin = SPI2_MOSI_PIN;
	GPIO_Init(SPI2_MOSI_GPIO_PORT, &GPIO_InitStructure);

	// set up the SPI peripheral
	SPI_I2S_DeInit(SPI2);
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_Init(SPI2, &SPI_InitStructure);

	//Enable the SPI port
	SPI_Cmd(SPI2, ENABLE);

	//DMA setup

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	// reset DMA configuration
	DMA_DeInit(SPI_PORT_TX_DMA_STREAM);
	DMA_StructInit(&DMA_InitStructure);
	//configure DMA
	DMA_InitStructure.DMA_MemoryBaseAddr = 0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(SPI2->DR);
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	//DMA_Init(SPI_PORT_TX_DMA_STREAM, &DMA_InitStructure);

	// Enable the interrupt in the NVIC
	NVIC_InitStructure.NVIC_IRQChannel = SPI_PORT_DMA_TX_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    // Enable the DMA transfer complete interrupt
	DMA_ITConfig(SPI_PORT_TX_DMA_STREAM, DMA_IT_TC, ENABLE);
	// Enable SPI dma tx request.
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

    DMA_Init(SPI_PORT_TX_DMA_STREAM, &DMA_InitStructure);
}

//DMA buffer
void spiPutBufferDMA(unsigned char * buffer, unsigned int length) {
	STM_EVAL_LEDOn(LED4);
    GPIO_SetBits(GPIOA, nokiaDc); //set LCD to data mode dc high
	GPIO_ResetBits(GPIOC, nokiaSce); //SCE pin low
	SPI_PORT_TX_DMA_STREAM->CNDTR = length;
	SPI_PORT_TX_DMA_STREAM->CMAR = (uint32_t) buffer;
	DMA_Cmd(SPI_PORT_TX_DMA_STREAM, ENABLE);//put data on the bus

}

extern"C"
{
///DMA interrupt routine
void SPI_PORT_DMA_TX_IRQHandler() {

	// Test if DMA Stream Transfer Complete
	if (DMA_GetITStatus(DMA1_IT_TC5)) {
		DMA_ClearITPendingBit(DMA1_IT_TC5);
        // check the TX flag to make sure transaction is complete this is good
		while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) {
		};
		//check the busy flag
		while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET) {
		};
	}

	GPIO_SetBits(GPIOC, nokiaSce); //SCE pin high
	STM_EVAL_LEDOff(LED4);
	DMA_Cmd(SPI_PORT_TX_DMA_STREAM, DISABLE);
 }
}

