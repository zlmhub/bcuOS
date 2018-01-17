/*
  * FreeModbus Libary: LPC214X Port
  * Copyright (C) 2007 Tiago Prado Lone <tiago@maxwellbohr.com.br>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial.c,v 1.1 2007/04/24 23:15:18 wolti Exp $
 */

#include "port.h"
#include "board.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
extern volatile UCHAR **pucSndBufferCur_p;
extern volatile USHORT *usSndBufferCount_p;
/* ----------------------- Start implementation -----------------------------*/
void vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    if(TRUE==xRxEnable) {
        LPC17xxHwRS485TxEnable(0);
		    UART_IntConfig( ( LPC_UART_TypeDef * )LPC_UART1, UART_INTCFG_RBR, ENABLE );
	
    } else {
        LPC17xxHwRS485TxEnable(1);
		UART_IntConfig( ( LPC_UART_TypeDef * )LPC_UART1, UART_INTCFG_RBR, DISABLE );
    }

    if(TRUE==xTxEnable) {
		LPC17xxHwRS485TxEnable(1);
		UART_IntConfig( ( LPC_UART_TypeDef * )LPC_UART1, UART_INTCFG_THRE , ENABLE );
		usSndBufferCount_p++;
		LPC_UART1->THR = *(*pucSndBufferCur_p);
		*pucSndBufferCur_p = (*pucSndBufferCur_p+1);
        

    } else {
        LPC17xxHwRS485TxEnable(0);
		UART_IntConfig( ( LPC_UART_TypeDef * )LPC_UART1, UART_INTCFG_THRE , DISABLE );
    }
}

void vMBPortClose( void )
{
	
}

BOOL xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{

	LPC17xxHwRS485Init(ulBaudRate);
	return TRUE;
}



BOOL xMBPortSerialPutByte( CHAR ucByte )
{
	while(!(LPC_UART1->LSR&0X20));
	LPC_UART1->THR=ucByte;
    return TRUE;
}


BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
	*pucByte = UART_ReceiveByte( (LPC_UART_TypeDef *)LPC_UART1);
	
    return TRUE;
}

/*
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call
 * xMBPortSerialPutByte( ) to send the character.
 */
void prvvUARTTxReadyISR(void)
{
    pxMBFrameCBTransmitterEmpty();
}

/*
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR(void)
{
    pxMBFrameCBByteReceived();
}

void UART1_IRQHandler( void )
{
	INT8U IIRValue,LSRValue;
	IIRValue = LPC_UART1->IIR;
	IIRValue >>= 1;
	IIRValue &= 0x07;
	if( IIRValue == 0x02 )
	{
		prvvUARTRxISR();
		
	}
	else if(IIRValue == 0x01)
	{
	    prvvUARTTxReadyISR();
	}
}