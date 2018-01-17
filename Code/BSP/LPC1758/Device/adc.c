#include <board.h>
#include <ucos_ii.h>
#include <ucos_device.h>

void  LPC17xxADCInit( void )
{
    PINSEL_CFG_Type PinCfg;

    PinCfg.Funcnum = 2;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Pinnum = 2;
    PinCfg.Portnum = 0;
    PINSEL_ConfigPin( &PinCfg );                      /**< P0.2 */
    ADC_Init( LPC_ADC, 100000 );                      /**< 采样频率100KHZ */
    ADC_IntConfig( LPC_ADC, ADC_ADINTEN7, DISABLE );  /**< 中断不使能 */
    ADC_ChannelCmd( LPC_ADC, ADC_CHANNEL_7, ENABLE ); /**< 通道7使能 */
}

INT16S LPC17xxHwCurrentAdSamp( INT16U RefV )
{
    INT32S ADCValue = 0;
    ADC_StartCmd( LPC_ADC, ADC_START_NOW );
    /**< Wait conversion complete */
    while ( !( ADC_ChannelGetStatus( LPC_ADC, ADC_CHANNEL_7, ADC_DATA_DONE ) ) );
    ADCValue = ADC_ChannelGetData( LPC_ADC, ADC_CHANNEL_7 );
    ADCValue = ( ADCValue * (INT32S)RefV ) / 4096;
    return ADCValue;
}