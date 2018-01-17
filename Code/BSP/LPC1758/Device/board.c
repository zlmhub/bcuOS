#include <board.h>
#include <ucos_ii.h>

void OSBoardPowerLow( void )
{
    NVIC_EnableIRQ( BOD_IRQn );
}

static void SysTickInit( void )
{
    SYSTICK_InternalInit( 1000 / OS_TICKS_PER_SEC );
    SYSTICK_IntCmd( ENABLE );
    SYSTICK_Cmd( ENABLE );
}

void OSLogo(void)
{
    OSPrintf("\n\n>\n");
    OSPrintf("*********************************\n");
    OSPrintf("*      BMS V1.0  ZhuangLiMing   *\n");
    OSPrintf("*********************************\n");
}

void OSLedInit(void)
{
    GPIO_SetDir(1, ( 1 << 29 ), 1);
}

void OSLedON(void)
{
    GPIO_SetValue(1, ( 1 << 29 ));
}

void OSLedOFF(void)
{
    GPIO_ClearValue(1, ( 1 << 29 ));
}

void OSBoardInit(void)
{
    OSLedInit();
    OSLedON();
    SysTickInit();
    LPC17xxHwPinInit();/*pin device*/
    FM24CxxInit();/*fm24c64*/
    LPC17xxBxCanInit();
    MCP2515Init();
    OSHwUartInit();
    LPC17xxHwUsartInit();
	LPC17xxHwRS485Init(19200);
	LPC17xxHwRS485TxEnable(1);
    OSLogo();
    OSBoardPowerLow( );
    LPC17xxHwTime0Init( 100 );/*100ms*/
    LPC17xxHwRTCInit();
    LPC17xxADCInit();
    LPC17xxPWMCaptureInit();
    LPC17xxHwPWMInit();
    OSPrintf("Board Init......[OK]\n");
}

void SysTick_Handler(void)
{
    OS_SysTickHandler();
}


/**
 * board hw reboot
 */
void OSBoardReBoot( void )
{
    asm( "CPSID I" );
    WDT_Init ( WDT_CLKSRC_PCLK, WDT_MODE_RESET );
    WDT_Start( 12 );
    while( 1 );
}

__weak void OSLowPowerCallBack(void *Par)
{

}

void BOD_IRQHandler( void )
{
    OSLowPowerCallBack(0);
}

/**
 * OSPrintf-->printf
 */
#ifndef SIMULATOR
#include <stdio.h>
int fputc(int ch,FILE *f)
{
#if(0)
     LPC17xxHwUsartTransmit(ch);
     if(ch =='\n')
     {
         LPC17xxHwUsartTransmit('\r');
     }
#endif
     return ch;
}

#endif
