#ifndef __BXCAN_H_
#define __BXCAN_H_

#include <ucos_ii.h>
#include <ucos_device.h>
#include <board.h>

struct os_bxcan_msg{
    INT32U Id;
    INT8U Len;
    INT8U Data[8];
};

OS_Err_T LPC17xxBxCanInit(void);

#endif
