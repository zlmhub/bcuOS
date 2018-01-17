#include <board.h>                              /* LPC17xx definitions    */
#include "spi.h"

/* bit definitions for register SSPCR0. */
#define SSPCR0_DSS      0
#define SSPCR0_CPOL     6
#define SSPCR0_CPHA     7
#define SSPCR0_SCR      8
/* bit definitions for register SSPCR1. */
#define SSPCR1_SSE      1
/* bit definitions for register SSPSR. */
#define SSPSR_TFE       0
#define SSPSR_TNF       1
#define SSPSR_RNE       2
#define SSPSR_RFF       3
#define SSPSR_BSY       4

/* Local functions */
static uint8_t LPC17xx_SPI_SendRecvByte (uint8_t byte_s);

/* Initialize the SSP0, SSP0_PCLK=CCLK=72MHz */
void LPC17xx_SPI_Init (void)
{
    PINSEL_CFG_Type PinCfg;
    SSP_CFG_Type SSP_CFG_Struct;

    PinCfg.Funcnum = 2;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 3;
    PinCfg.Portnum = 0;

    PinCfg.Pinnum = 7;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 8;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 9;
    PINSEL_ConfigPin(&PinCfg);

    PinCfg.Funcnum = 0;
    PinCfg.Pinnum = 6;
    PINSEL_ConfigPin(&PinCfg);

    GPIO_SetDir(0, (1 << 6), 1);
    GPIO_SetValue(0, (1 << 6));

    /* initialize SSP configuration */
    SSP_CFG_Struct.CPHA = SSP_CPHA_SECOND;
    SSP_CFG_Struct.CPOL = SSP_CPOL_LO;
    SSP_CFG_Struct.ClockRate = 1000;
    SSP_CFG_Struct.Databit = SSP_DATABIT_8;
    SSP_CFG_Struct.Mode = SSP_MASTER_MODE;
    SSP_CFG_Struct.FrameFormat = SSP_FRAME_SPI;

    /* Initialize SSP peripheral with parameter given in structure above */
    SSP_Init(LPC_SSP1, &SSP_CFG_Struct);
    /* Enable SSP peripheral */
    SSP_Cmd(LPC_SSP1, ENABLE);
}

/* Close SSP0 */
void LPC17xx_SPI_DeInit( void )
{
	// disable SPI
    SSP_DeInit(LPC_SSP1);
}

/* Set a SSP0 clock speed to desired value. */
void LPC17xx_SPI_SetSpeed (uint8_t speed)
{
	speed &= 0xFE;
	if ( speed < 2  ) {
		speed = 2 ;
	}
	LPC_SSP1->CPSR = speed;
}

/* SSEL: low */
void LPC17xx_SPI_Select ()
{
    GPIO_ClearValue(0, 1<<6);
}

/* SSEL: high */
void LPC17xx_SPI_DeSelect ()
{
    GPIO_SetValue(0, 1<<6);
}

/* Send one byte then recv one byte of response. */
static uint8_t LPC17xx_SPI_SendRecvByte (uint8_t byte_s)
{
    SSP_SendData(LPC_SSP1, byte_s);
    while (SSP_GetStatus(LPC_SSP1, SSP_STAT_BUSY) == SET);
    return SSP_ReceiveData(LPC_SSP1);
}

/* Send one byte */
void LPC17xx_SPI_SendByte (uint8_t data)
{
	LPC17xx_SPI_SendRecvByte (data);
}

/* Recv one byte */
uint8_t LPC17xx_SPI_RecvByte ()
{
	return LPC17xx_SPI_SendRecvByte (0xFF);
}

/* Release SSP0 */
void LPC17xx_SPI_Release (void)
{
	LPC17xx_SPI_DeSelect ();
	LPC17xx_SPI_RecvByte ();
}


#if USE_FIFO
/* on LPC17xx the FIFOs have 8 elements which each can hold up to 16 bits */
#define FIFO_ELEM 8

/* Receive btr (must be multiple of 4) bytes of data and store in buff. */
void LPC17xx_SPI_RecvBlock_FIFO (uint8_t *buff,	uint32_t btr)
{
	uint32_t hwtr, startcnt, i, rec;

	hwtr = btr/2;  /* byte number in unit of short */
	if ( btr < FIFO_ELEM ) {
		startcnt = hwtr;
	} else {
		startcnt = FIFO_ELEM;
	}

	LPC_SSP1 -> CR0 |= 0x0f;  /* DSS to 16 bit */

	for ( i = startcnt; i; i-- ) {
		LPC_SSP1 -> DR = 0xffff;  /* fill TX FIFO, prepare clk for receive */
	}

	do {
		while ( !(LPC_SSP1->SR & ( 1 << SSPSR_RNE ) ) ) {
			// wait for data in RX FIFO (RNE set)
		}
		rec = LPC_SSP1->DR;
		if ( i < ( hwtr - startcnt ) ) {
			LPC_SSP1->DR = 0xffff;	/* fill TX FIFO, prepare clk for receive */
		}
		*buff++ = (uint8_t)(rec>>8);
		*buff++ = (uint8_t)(rec);
		i++;
	} while ( i < hwtr );

	LPC_SSP1->CR0 &= ~0x08;  /* DSS to 8 bit */
}

/* Send 512 bytes of data block (stored in buff). */
void LPC17xx_SPI_SendBlock_FIFO (const uint8_t *buff)
{
	uint32_t cnt;
	uint16_t data;

	LPC_SSP1->CR0 |= 0x0f;  /* DSS to 16 bit */

	/* fill the FIFO unless it is full */
	for ( cnt = 0; cnt < ( 512 / 2 ); cnt++ )
	{
		/* wait for TX FIFO not full (TNF) */
		while ( !( LPC_SSP1->SR & ( 1 << SSPSR_TNF ) ) );

		data  = (*buff++) << 8;
		data |= *buff++;
		LPC_SSP1->DR = data;
	}

	/* wait for BSY gone */
	while ( LPC_SSP1->SR & ( 1 << SSPSR_BSY ) );

	/* drain receive FIFO */
	while ( LPC_SSP1->SR & ( 1 << SSPSR_RNE ) ) {
		data = LPC_SSP1->DR;
	}

	LPC_SSP1->CR0 &= ~0x08;  /* DSS to 8 bit */
}
#endif /* USE_FIFO */
