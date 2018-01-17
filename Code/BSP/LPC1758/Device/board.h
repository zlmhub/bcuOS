#ifndef __BOARD_H_
#define __BOARD_H_

#include "lpc17xx_can.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_systick.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_rtc.h"
#include "lpc17xx_wdt.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_gpdma.h"

/*board devices */
#include "gpio.h"
#include "pin.h"
#include "fm24cxx.h"
#include "bxcan.h"
#include "uart.h"
#include "rtc.h"
#include "rs485.h"
#include "timer.h"
#include "adc.h"
#include "mcp2515.h"
#include "pwm.h"

#define OSInLine                   static inline


void OSBoardInit(void);
void OSBoardReBoot( void );
void OSLowPowerCallBack(void *Par);
void OSLedON(void);
void OSLedOFF(void);
#endif
