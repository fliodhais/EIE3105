#include <stm32f10x.h>

void spi2_init() 
{
	// Setup PB13 and PB15
	// PB13 = SPI2_SCK, PB15 = IR LED / MODE
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	RCC_APB1PeriphClockCmd(RCC_APB1ENR_SPI2EN, ENABLE);

	// SPI initialization
        SPI_InitTypeDef   SPI_InitStructure;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
      	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
      	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
      	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
      	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
      	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
      	// 36 MHz / 256 = 140.625 kHz
      	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
        SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
        SPI_Init(SPI2, &SPI_InitStructure);
	// Enable the receive interrupt
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
        // Enable SPI2
        SPI_Cmd(SPI2, ENABLE);
}

// when this function is called, we will get the readings later
void readFloor() 
{
	// Set PB15 to 1
	GPIO_SetBits(GPIOB, GPIO_Pin_15);
	// Initialize the data transmission from the master to the slave
	SPI_I2S_SendData(SPI2, 0);
	// Enable the interrupt to receive data by using the ISR handler
	NVIC_EnableIRQ(SPI2_IRQn);
}

// put the readings to the variable c
void SPI2_IRQHandler() 
{
	// the received character has all the readings
	char c = (char) SPI_I2S_ReceiveData(SPI2) & 0xff;
	// Check PB15. If it is 1, it means the data is ready
	if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_15) == 1) 
	{
	  // Set PB15 to 0 to trigger the shift register
	  GPIO_ResetBits(GPIOB, GPIO_Pin_15);
	  // Go to get the next reading
	  SPI_I2S_SendData(SPI2, 0);
	} 
	else 
	{
	  // disable the interrupt because it is not ready
	  NVIC_DisableIRQ(SPI2_IRQn);
	}
}
