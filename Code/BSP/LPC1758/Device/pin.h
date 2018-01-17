#ifndef __PIN_H_
#define __PIN_H_

#include <ucos_ii.h>
#include <ucos_device.h>
#include <board.h>

#define     PIN_NULL    0x00
#define     PIN_LOW     0x01
#define     PIN_HIGH    0x02

struct os_device_pin_status
{
    INT16U pin;
    INT16U status;
};

OS_Err_T LPC17xxHwPinInit(void);


#endif
