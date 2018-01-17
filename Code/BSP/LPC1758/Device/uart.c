/*
 * File      : board.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-03-08     Bernard      The first version for LPC17xx
 * 2010-05-02     Aozima       update CMSIS to 130
 */

#include <board.h>
#include <ucos_ii.h>
#include <ucos_device.h>
#include <app.h>
#include "LPC17xx.h"

#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

/**
 * @addtogroup LPC11xx
 */



struct OS_UART_LPC
{
	struct OS_Device parent;

	/* buffer for reception */
	INT8U ReadIndex, SaveIndex;
	INT8U RxBuffer[OS_UAOS_RX_BUFFER_SIZE];
}UartDevice;

void UART3_IRQHandler(void)
{
	OS_Ubase_T level, iir;
    struct OS_UART_LPC* uart = &UartDevice;

    /* read IIR and clear it */
	iir = LPC_UART3->IIR;

	iir >>= 1;			    /* skip pending bit in IIR */
	iir &= 0x07;			/* check bit 1~3, interrupt identification */

	if (iir == IIR_RDA)	    /* Receive Data Available */
	{
		/* Receive Data Available */
        uart->RxBuffer[uart->SaveIndex] = LPC_UART3->RBR;

        /*en isr*/
		uart->SaveIndex ++;
        if (uart->SaveIndex >= OS_UAOS_RX_BUFFER_SIZE)
            uart->SaveIndex = 0;
        /*en isr*/

		/* invoke callback */
		if(uart->parent.RxIndicate != OS_NULL)
		{
		    OS_Size_T length;
		    if (uart->ReadIndex > uart->SaveIndex)
                length = OS_UAOS_RX_BUFFER_SIZE - uart->ReadIndex + uart->SaveIndex;
            else
                length = uart->SaveIndex - uart->ReadIndex;

            uart->parent.RxIndicate(&uart->parent, length);
		}
	}

	return;
}

void LPC17xxHwUsartInit(void)
{
    UART_CFG_Type UARTConfigStruct;
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 3;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Pinnum = 28;
    PinCfg.Portnum = 4;
    PINSEL_ConfigPin( &PinCfg );
    PinCfg.Pinnum = 29;
    PINSEL_ConfigPin( &PinCfg );
	
    UARTConfigStruct.Databits = UART_DATABIT_8;
    UARTConfigStruct.Baud_rate = UART_BAUD_RAT;
    UARTConfigStruct.Parity = UART_PARITY_NONE;
    UARTConfigStruct.Stopbits = UART_STOPBIT_1;
	
    UART_Init( ( LPC_UART_TypeDef * )LPC_UART3, &UARTConfigStruct );
    UART_TxCmd( ( LPC_UART_TypeDef * )LPC_UART3, ENABLE );
    UART_IntConfig( ( LPC_UART_TypeDef * )LPC_UART3, UART_INTCFG_RBR, ENABLE );

    NVIC_EnableIRQ( UART3_IRQn );
}

void LPC17xxHwUsartTransmit(INT8U Data)
{
	while ( !(LPC_UART3->LSR & LSR_THRE) );
	/* write data */
	LPC_UART3->THR = Data;
}

static OS_Err_T LPC17xxHwUartInit (OS_Device_T dev)
{
	LPC17xxHwUsartInit( );
	return OS_EOK;
}

static OS_Err_T LPC17xxHwUartOpen(OS_Device_T dev, INT16U oflag)
{

	if (dev->Flag & OS_DEVICE_FLAG_INT_RX)
	{
		/* Enable the UART Interrupt */
		NVIC_EnableIRQ(UART3_IRQn);
	}

	return OS_EOK;
}

static OS_Err_T LPC17xxHwUartClose(OS_Device_T dev)
{

	if (dev->Flag & OS_DEVICE_FLAG_INT_RX)
	{
		/* Disable the UART Interrupt */
		NVIC_DisableIRQ(UART3_IRQn);
	}

	return OS_EOK;
}

static OS_Size_T LPC17xxHwUartRead(OS_Device_T dev, OS_Off_T pos, void* buffer, OS_Size_T size)
{
	INT8U* ptr;
	struct OS_UART_LPC *uart = (struct OS_UART_LPC*)dev;
	//OS_ASSERT(uart != OS_NULL);

	/* point to buffer */
	ptr = (INT8U*) buffer;
	if (dev->Flag & OS_DEVICE_FLAG_INT_RX)
	{
		while (size)
		{
			/* interrupt receive */
			OS_Base_T level;

			/* disable interrupt */
			OS_ENTER_CRITICAL();
			/*en isr*/
			if (uart->ReadIndex != uart->SaveIndex)
			{
				*ptr = uart->RxBuffer[uart->ReadIndex];

				uart->ReadIndex ++;
				if (uart->ReadIndex >= OS_UAOS_RX_BUFFER_SIZE)
					uart->ReadIndex = 0;
			}
			else
			{
				/* no data in rx buffer */

				/* enable interrupt */
				OS_EXIT_CRITICAL();
				///*en isr*/
				break;
			}

			/* enable interrupt */
			OS_EXIT_CRITICAL();

			ptr ++;
			size --;
		}

		return (INT32U)ptr - (INT32U)buffer;
	}

	return 0;
}

static OS_Size_T LPC17xxHwUartWrite(OS_Device_T dev, OS_Off_T pos, const void* buffer, OS_Size_T size)
{
	char *ptr;
	ptr = (char*)buffer;

	if (dev->Flag & OS_DEVICE_FLAG_STREAM)
	{
		/* stream mode */
		while (size)
		{
			if (*ptr == '\n')
			{
				/* THRE status, contain valid data */
				while ( !(LPC_UART3->LSR & LSR_THRE) );
				/* write data */
				LPC_UART3->THR = '\r';
			}

			/* THRE status, contain valid data */
			while ( !(LPC_UART3->LSR & LSR_THRE) );
			/* write data */
			LPC_UART3->THR = *ptr;

			ptr ++;
			size --;
		}
	}
	else
	{
		while ( size != 0 )
		{
			/* THRE status, contain valid data */
			while ( !(LPC_UART3->LSR & LSR_THRE) );

			/* write data */
			LPC_UART3->THR = *ptr;

			ptr++;
			size--;
		}
	}

	return (OS_Size_T) ptr - (OS_Size_T) buffer;
}

void OSHwUartInit(void)
{
	struct OS_UART_LPC* uart;

	/* get uart device */
	uart = &UartDevice;

	/* device initialization */
	uart->parent.Type = OS_Device_Class_Char;
	memset(uart->RxBuffer, 0, sizeof(uart->RxBuffer));
	uart->ReadIndex = uart->SaveIndex = 0;

	/* device interface */
	uart->parent.Init 	    = LPC17xxHwUartInit;
	uart->parent.Open 	    = LPC17xxHwUartOpen;
	uart->parent.Close      = LPC17xxHwUartClose;
	uart->parent.Read 	    = LPC17xxHwUartRead;
	uart->parent.Write      = LPC17xxHwUartWrite;
	uart->parent.Control    = OS_NULL;
	uart->parent.UserData  = OS_NULL;

	OSDeviceRegister(&uart->parent,
		"uart0", OS_DEVICE_FLAG_RDWR | OS_DEVICE_FLAG_STREAM | OS_DEVICE_FLAG_INT_RX);
}


/*@}*/
