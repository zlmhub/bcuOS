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
 * File: $Id: porttimer.c,v 1.1 2007/04/24 23:15:18 wolti Exp $
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBPortTimersInit( USHORT usTim1Timerout50us )
{
#if(0)
    TIM_TIMERCFG_Type TIM_ConfigStruct;
    TIM_MATCHCFG_Type TIM_MatchConfigStruct;

    TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
    TIM_ConfigStruct.PrescaleValue = 1000;             
    TIM_MatchConfigStruct.MatchChannel = 0;
    TIM_MatchConfigStruct.IntOnMatch = TRUE;
    TIM_MatchConfigStruct.ResetOnMatch = TRUE;
    TIM_MatchConfigStruct.StopOnMatch = FALSE;
    TIM_MatchConfigStruct.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
    TIM_MatchConfigStruct.MatchValue = usTim1Timerout50us;
    TIM_Init( LPC_TIM1, TIM_TIMER_MODE, &TIM_ConfigStruct );
    TIM_ConfigMatch( LPC_TIM1, &TIM_MatchConfigStruct );
    NVIC_SetPriority( TIMER1_IRQn, ( ( 0X01 << 3 ) | 0X01 ) );

    TIM_Cmd( LPC_TIM1, ENABLE );
    NVIC_EnableIRQ( TIMER1_IRQn );
#else
	LPC_SC->PCONP |= (1<<2);
	LPC_TIM1->TCR |= (1<<1);
	LPC_TIM1->PR = 50000;
	LPC_TIM1->MR0 = usTim1Timerout50us;
	LPC_TIM1->IR |= 0xFF;
	LPC_TIM1->MCR |= ((1<<1)|(1<<0));
	LPC_TIM1->TCR = (1<<0);
	NVIC_EnableIRQ( TIMER1_IRQn );
#endif
	return TRUE;
}


void
vMBPortTimersEnable(  )
{
	LPC_TIM1->TC = 0;
	LPC_TIM1->TCR = (1<<0);
    //NVIC_EnableIRQ( TIMER1_IRQn );
}

void vMBPortTimersDisable(  )
{
	LPC_TIM1->TCR |= (1<<1);
    //NVIC_DisableIRQ( TIMER1_IRQn );
}

void TIMERExpiredISR( void )
{
    (void)pxMBPortCBTimerExpired();
}

void TIMER1_IRQHandler( void )
{
	LPC_TIM1->IR |= (1<<0);
	TIMERExpiredISR();
}
