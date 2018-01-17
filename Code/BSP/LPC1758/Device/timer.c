#include <board.h>
#include <ucos_ii.h>
#include <ucos_device.h>

void LPC17xxHwTime0Init( INT16U Nms )
{
    TIM_TIMERCFG_Type TIM_ConfigStruct;
    TIM_MATCHCFG_Type TIM_MatchConfigStruct;

    TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
    TIM_ConfigStruct.PrescaleValue = 1000;             /**< 该值确定是多少US */
    TIM_MatchConfigStruct.MatchChannel = 0;
    TIM_MatchConfigStruct.IntOnMatch = TRUE;
    TIM_MatchConfigStruct.ResetOnMatch = TRUE;
    TIM_MatchConfigStruct.StopOnMatch = FALSE;
    TIM_MatchConfigStruct.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
    TIM_MatchConfigStruct.MatchValue = Nms;
    TIM_Init( LPC_TIM0, TIM_TIMER_MODE, &TIM_ConfigStruct );
    TIM_ConfigMatch( LPC_TIM0, &TIM_MatchConfigStruct );
    NVIC_SetPriority( TIMER0_IRQn, ( ( 0X01 << 3 ) | 0X01 ) );

    TIM_Cmd( LPC_TIM0, ENABLE );
    NVIC_EnableIRQ( TIMER0_IRQn );
}
#if(1)
void TIMER0_IRQHandler( void )
{
    if ( TIM_GetIntStatus( LPC_TIM0, 0 ) )
    {
        TIM_ClearIntPending( LPC_TIM0, 0 );
    }
}
#endif