/*
 * spi.hpp
 * 6/15/12
 */

#ifndef SPI_HPP_
#define SPI_HPP_

#ifdef	__cplusplus
extern "C" {
#endif

//#include "stm32f4xx.h"

 /* SPIx Communication Interface */
#define SPI2_CLK                       RCC_APB1Periph_SPI2
#define SPI2_CLK_CMD                   RCC_APB1PeriphClockCmd
#define SPI2_IRQn                      SPI2_IRQn
#define SPI2_IRQHANDLER                SPI2_IRQHandler

#define SPI2_SCK_PIN                   GPIO_Pin_13
#define SPI2_SCK_GPIO_PORT             GPIOB
#define SPI2_SCK_GPIO_CLK              RCC_AHBPeriph_GPIOB
#define SPI2_SCK_SOURCE                GPIO_PinSource13
#define SPI2_SCK_AF                    GPIO_AF_0

#define SPI2_MOSI_PIN                  GPIO_Pin_15
#define SPI2_MOSI_GPIO_PORT            GPIOB
#define SPI2_MOSI_GPIO_CLK             RCC_AHBPeriph_GPIOB
#define SPI2_MOSI_SOURCE               GPIO_PinSource15
#define SPI2_MOSI_AF                   GPIO_AF_0

/*
#define SPI2_NSS_PIN                  GPIO_Pin_12
#define SPI2_NSS_GPIO_PORT            GPIOB
#define SPI2_NSS_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define SPI2_NSS_SOURCE               GPIO_PinSource12
#define SPI2_NSS_AF                   GPIO_AF_0
*/

/* Definition for DMAx resources **********************************************/
#define SPI_PORT_DR_ADDRESS                SPI_PORT->DR

#define SPI_PORT_DMA                       DMA1
#define SPI_PORT_DMAx_CLK                  RCC_AHB1Periph_DMA1

#define SPI_PORT_TX_DMA_STREAM             DMA1_Channel5
#define SPI_PORT_TX_DMA_FLAG_TEIF          DMA_IT_TE      //transfer error flag
#define SPI_PORT_TX_DMA_FLAG_HTIF          DMA_IT_HT      //half transfer flag
#define SPI_PORT_TX_DMA_FLAG_TCIF          DMA_IT_TC      //transfer complete flag

#define SPI_PORT_DMA_TX_IRQn               DMA1_Channel4_5_IRQn
#define SPI_PORT_DMA_TX_IRQHandler         DMA1_Channel4_5_IRQHandler

//////////function prototypes//////////
void SPI_Config(void);
void spiPutBufferDMA(unsigned char * buffer, unsigned int length);


#ifdef	__cplusplus
}
#endif


#endif /* SPI_HPP_ */
