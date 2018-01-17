#include <board.h>
#include <ucos_ii.h>
#include <ucos_device.h>

void LPC17xxHwRS485Init( INT16U Baund )
{
    UART_CFG_Type UARTConfigStruct;
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 2;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Pinnum = 0;
    PinCfg.Portnum = 2;
    PINSEL_ConfigPin( &PinCfg );
    PinCfg.Pinnum = 1;
    PINSEL_ConfigPin( &PinCfg );
    PinCfg.Funcnum = 0;
    PinCfg.Pinnum = 5;
    PINSEL_ConfigPin( &PinCfg );
    /*p2.4*/
    PinCfg.Funcnum = 0;
    PinCfg.Pinnum = 4;
    PINSEL_ConfigPin( &PinCfg );

    GPIO_SetDir( 2, ( 1 << 5 )|(1<<4), 1 );
    UARTConfigStruct.Databits = UART_DATABIT_8;
    UARTConfigStruct.Baud_rate = Baund;
    UARTConfigStruct.Parity = UART_PARITY_NONE;
    UARTConfigStruct.Stopbits = UART_STOPBIT_1;
    UART_Init( ( LPC_UART_TypeDef * )LPC_UART1, &UARTConfigStruct );
    UART_TxCmd( ( LPC_UART_TypeDef * )LPC_UART1, ENABLE );
    UART_IntConfig( ( LPC_UART_TypeDef * )LPC_UART1, UART_INTCFG_RBR, ENABLE );
	//UART_IntConfig( ( LPC_UART_TypeDef * )LPC_UART1, UART_INTCFG_THRE , ENABLE );
    NVIC_EnableIRQ( UART1_IRQn );
}

void LPC17xxHwRS485TxEnable( INT8U cmd)
{
	switch(cmd)
	{
	case 1:
		GPIO_SetValue(2, ( 1 << 5 )|(1<<4));
		break;
	case 0:
		GPIO_ClearValue(2, ( 1 << 5 )|(1<<4));
		break;
	default:break;
	}
}

/*485·¢ËÍº¯Êý*/
void LPC17xxHwRS485SendData(INT8U *buf,INT8U len)
{
	INT8U t;
	for(t=0; t<len; t++)
	{
		while(!(LPC_UART1->LSR&0X20));
        LPC_UART1->THR=buf[t];
	}
	while(!(LPC_UART1->LSR&0X20));
}

void LPC17xxHwRS485TransmitByte(INT8U Data)
{
	while ( !(LPC_UART1->LSR & 0X20) );
	/* write data */
	LPC_UART1->THR = Data;
}