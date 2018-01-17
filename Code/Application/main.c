/**************************************************************************//**
* @file     main.c
* @brief    Lithium iron battery management system.
* @version  V2.03
* @date     07. October 2017
*
* @note
* Copyright (C) 2015 XY. All rights reserved.
*
* @par
*
******************************************************************************/
/* ------------------------- UCOS includes ----------------------------------*/
#include <ucos_ii.h>
#include <ucos_device.h>
/* -------------------------- BMS includes ----------------------------------*/
#include <app.h>
#include "bmu.h"
#include "bcu.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/*********App threads**********/
void ThreadLCD(void *Par);
void ThreadEntry(void *Par);
void ThreadVCU(void *Par);
void ThreadBCU(void *Par);
void ThreadDEBUG(void *Par);

/**
 * app entry
 */
void ThreadEntry(void *Par)
{
    OS_Device_T PinHf;

    OS_Err_T Ret = OS_ERROR;
    struct os_device_pin_status  PinSta;

    OS_ENTER_CRITICAL();
    OSBoardInit();/* os board init */
    BCUInit();
    OSAppCreate( ThreadLCD, OS_NULL, 128, 2);   /* create a lcd thread >> */
    OSAppCreate( ThreadVCU, OS_NULL, 256, 3);   /* create a vcu thread >> */
    OSAppCreate( ThreadBCU, OS_NULL, 128, 4);   /* create a bcu thread >> */
    OSAppCreate( ThreadDEBUG, OS_NULL, 256, 5); /* creat a debug thread   */

    /* open pin device */
    PinHf = OSDeviceFind("pin");

    if(PinHf != OS_NULL) {
        Ret = OSDeviceOpen( PinHf, OS_DEVICE_FLAG_RDWR );
        if(Ret != OS_EOK) {
            OSPrintf("Open pin device......[ERROR]\n");
        }
    }
    OS_EXIT_CRITICAL();
    while(1) {
        for(INT8U i=0; i<1; i++) {
            OSLedON();
            OSTimeDly(50);
            OSLedOFF();
            OSTimeDly(50);
        }
        if(BCU.LineMsg.LimitVol.MaxVol >3650) {
            BCU.SysSit->IO_CHP = 0;
        }
        if(BCU.LineMsg.LimitVol.MinVol <2700) {
            BCU.SysSit->IO_HVP = 0;
        }
        if(BCU.SysSit->IO_CHP) {
            PinSta.pin = 0;
            PinSta.status = PIN_HIGH;
            OSDeviceWrite( PinHf,0,&PinSta,sizeof(PinSta));
        } else {
            PinSta.pin = 0;
            PinSta.status = PIN_LOW;
            OSDeviceWrite( PinHf,0,&PinSta,sizeof(PinSta));
        }
        if(BCU.SysSit->IO_HVP) {
            PinSta.pin = 1;
            PinSta.status = PIN_HIGH;
            OSDeviceWrite( PinHf,0,&PinSta,sizeof(PinSta));
        } else {
            PinSta.pin = 1;
            PinSta.status = PIN_LOW;
            OSDeviceWrite( PinHf,0,&PinSta,sizeof(PinSta));
        }
    }
}

/**
 * RS485 LCD Dispaly
 */

void ThreadLCD(void *Par)
{
    while(1) {
        OSTimeDly(10);
    }
}

/**
 * BCU
 */
void ThreadBCU(void *Par)
{
    INT32U BatIdleTime=0;
    while(1) {
        CellHighLowAvgCal();
        TempHighLowAvgCal();
        GetTotalCurrent();
        BCUErrorDispatch();
        BatteryModle(0);
        if(BCU.LineMsg.LimitVol.MinVol > 1000) {
            if(BCU.BatSit.CheckSOCFlag) {
                GetSOCAdj(BCU.LineMsg.LimitVol.MinVol);
                BCU.BatSit.CheckSOCFlag = 0;
                BCU.LineMsg.ChargeCurrent++;
            }
        }
        if(BatIdleTime>18000) { /*uint 0.1S 0.5hour*/
            BCU.BatSit.CheckSOCFlag = 1;
            BatIdleTime = 0;
        }
        if(BCU.BatSit.DS_CH_Sta == BAT_IDLE )
            BatIdleTime++;
        else
            BatIdleTime=0;

        BCU.LineMsg.ChargeCode = BatIdleTime;//test data
        BCU.LineMsg.ChargeVol = BCU.BatSit.DS_CH_Sta;
        BCU.SysErrorCode = GetCPPWMSigner();
        if(BCU.SysErrorCode<90){
            BCU.SysSit->IO_CHP=1;
        }else
        {
            BCU.SysSit->IO_CHP=0;
        }
        BatCapacityCal();
        OSTimeDly(10);
    }
}

/**
 * VCU
 */
void ThreadVCU(void *Par)
{
    OS_Device_T RTCHf;
    RTC_TIME_Type  RTC= { /*Initial RTC time*/
        .YEAR = 2018,
        .MONTH = 1,
        .DOY  = 1,
        .HOUR = 1,
        .MIN  = 1,
        .SEC  = 1,
    };
    INT32U TSec=0;
    RTCHf = OSDeviceFind("rtc");
    if(RTCHf != OS_NULL) {
        OSDeviceOpen(RTCHf,OS_DEVICE_FLAG_RDWR);
    }
    /*Two boot intervals. if time > 0.5Hour then CheckSOC*/
    OSDeviceControl(RTCHf,OS_DEVICE_CTRL_RTC_GET_TIME,&BCU.RTC);
    TSec = OSGetRTCSeconds(&BCU.RTC);
    if((TSec >(RTC_ReadGPREG (LPC_RTC, 0)+1800))||(0 == TSec)) {
        BCU.BatSit.CheckSOCFlag = 1;
    }
    while(1) {
        OSDeviceControl(RTCHf,OS_DEVICE_CTRL_RTC_GET_TIME,&BCU.RTC);
        RTC_WriteGPREG (LPC_RTC, 0, TSec);
        /*Year <2017 or year > 2099 Set default time*/
        if((BCU.RTC.YEAR <RTC.YEAR)||(BCU.RTC.YEAR > 2099)) {
            OSDeviceControl(RTCHf,OS_DEVICE_CTRL_RTC_SET_TIME,&RTC);
        }
        OSTimeDly(200);
    }
}

/**
 * Debug thread for Modbus communication.The parameters are modified and the parameters
 * are written to the register.
 */
void ThreadDEBUG(void *Par)
{
    INT8U SetParTime = 0;

    eMBInit(MB_RTU, 0x01, 0, 19200, MB_PAR_NONE);/* establish a MODBUS RTU device */
    eMBEnable();
    ModebusInit();
    while(1) {
        ModebusDebugMain();
        eMBPoll();
        SetParTime++;
        if( SetParTime >100 ) { /*10s test parameters are set */
            ModebusSetPar();
            SetParTime = 0;
        }
        OSTimeDly(20);
    }
}

/**
 * Main Function Port
 */

int main(char arg,char *arv)
{
    OSInit();
    OSAppMEMCreate();
    OSAppCreate( ThreadEntry, OS_NULL, 512, 1 );
    OSStart();
    for(;;) {
        /**/
    }
    return 0;
}
