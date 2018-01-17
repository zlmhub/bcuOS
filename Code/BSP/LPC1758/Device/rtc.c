#include <board.h>
#include <ucos_ii.h>
#include <ucos_device.h>
#include "rtc.h"

static void _LPC17xxHwRTCInit( void )
{
    RTC_Init ( LPC_RTC );
    RTC_ResetClockTickCounter( LPC_RTC );
    RTC_Cmd ( LPC_RTC, ENABLE );
}

static struct OS_Device RTCDev;
static OS_Err_T OS_LPC17xxHwRTCOpen(OS_Device_T dev, INT16U oflag)
{
    if (dev->RxIndicate != OS_NULL)
    {
        /* Open Interrupt */
    }
    return OS_EOK;
}

//计算从1970年到（year-1）年间一共有多少天
INT8U Is_Leap_Year(INT16U year)
{			  
	if(year%4==0)
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0) return 1;   
			else return 0;   
		}else return 1;   
	}else return 0;	
}

//计算从1970年到（year-1）年间一共有多少天
INT32U Days_Of_Year(INT16U year)
{			  
	INT16U leap_year,comm_year;
	INT32U num;
	
	leap_year = (((year-1-1972)/4+1)-((year-1-2000)/100+1)+((year-1-2000)/400+1));
	comm_year = ((year-1-1970)-leap_year);
	num = leap_year*366+comm_year*365;
	
	return num;
}

INT32U OSGetRTCSeconds(RTC_TIME_Type *RTC)
{
	INT8U i = 1;
	INT32U sec = 0,day = 0;
	const INT8U map[]={31,28,31,30,31,30,31,31,30,31,30};
	
	RTC_GetFullTime(LPC_RTC,RTC);
	
	if (((RTC->MONTH)>2) && (Is_Leap_Year(RTC->YEAR))) {
		while (i<(RTC->MONTH)) {
			day += map[i-1];
			i++;
		}
		day += 1;
	}else {
		while (i<(RTC->MONTH)) {
			day += map[i-1];
			i++;
		}
	}
	day = day + (RTC->DOM) + Days_Of_Year(RTC->YEAR);
	sec = day*24*3600;
	
	return sec;
}

static OS_Err_T OS_LPC17xxHwRTCControl(OS_Device_T dev, INT8U cmd, void *args)
{
    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        RTC_GetFullTime (LPC_RTC, (RTC_TIME_Type *)args);
        break;

    case OS_DEVICE_CTRL_RTC_SET_TIME:
        RTC_SetFullTime (LPC_RTC, (RTC_TIME_Type *)args);
        break;
    case OS_DEVICE_CTRL_RTC_GET_SECOND:
        OSGetRTCSeconds((RTC_TIME_Type *)args);
      break;
    default:break;
    }
    return OS_EOK;
}

void LPC17xxHwRTCInit(void)
{
    OS_Device_T   rtc = &RTCDev;

    rtc->Type	= OS_Device_Class_RTC;

    /* register rtc device */
    rtc->Init 	= OS_NULL;
    rtc->Open 	= OS_LPC17xxHwRTCOpen;
    rtc->Close	= OS_NULL;
    rtc->Read 	= OS_NULL;
    rtc->Write	= OS_NULL;
    rtc->Control = OS_LPC17xxHwRTCControl;

    /* no private */
    rtc->UserData = OS_NULL;
    _LPC17xxHwRTCInit( );
    OSDeviceRegister(rtc, "rtc", OS_DEVICE_FLAG_RDWR);

    return;
}
