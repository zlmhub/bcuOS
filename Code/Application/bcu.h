#ifndef __BCU_H_
#define __BCU_H_

#include <UCOS_II.H>
#include <UCOS_DEVICE.H>
#include <BOARD.H>
#include <APP.H>
#include <math.h>
#include "bmu.h"
#include "modbus_debug.h"

// Initial battery total capacity, unit: 0.01As
// 80Ah = 80 * 3600 * (1/0.01) = 28800000 As
#define BSP_BAT_Cap_Int			((INT32S)(25*360000))
// Battery Cap worst case, unit: 0.01As
// 80Ah * 20% = BSP_BAT_Cap_Int/4
#define BSP_BAT_Cap_WORST_CASE_L	(BSP_BAT_Cap_Int>>3)
// Battery Cap worst case, unit: 0.01As
// 80Ah * 150% = BSP_BAT_Cap_Int * 1.25
#define BSP_BAT_Cap_WORST_CASE_H	(BSP_BAT_Cap_Int + (BSP_BAT_Cap_Int>>2))


struct limit
{
    INT16U      MaxVol;
    INT16U      MinVol;
    INT8U       MaxTemp;
    INT8U       MinTemp;
    INT16U      MaxVolID;
    INT16U      MinVolID;
    INT16U      MaxTempID;
    INT16U      MinTempID;
};

struct BatteryMsg
{
    struct  limit   LimitVol;
    INT16U  VolAvg;
    INT8U	TempAvg;
    FP32    SOC;
    FP32    SOH;
    INT16U  Cyc;
    FP32    TotalVol;
    FP32    TotalCurrent;
    FP32    CapAged;
    FP32    CapRated;
    FP32    ChargeVol;
    FP32    ChargeCurrent;
    INT16U  ChargeCode;
    INT16U  InsuR;/*绝缘*/
};

/*系统状态*/
struct SysStatue
{
    union
    {
        struct
        {
            INT32U ERR_HW:1;    /* 硬件故障*/
            INT32U ERR_CAN1:1;  /* CAN1通讯故障*/
            INT32U ERR_CAN2:1;  /* CAN2通讯故障*/
            INT32U IO_HVP:1;    /*高压继电器正极（+）*/
            INT32U IO_HVN:1;    /*高压继电器负极（-）*/
            INT32U IO_PreCH:1;  /*预充继电器*/
            INT32U IO_CHP:1;    /*充电继电器正极*/
            INT32U Resv1:1;     /*保留*/
            INT32U OpenF:1;
        };
        INT32U All;
    };
};

#define		HV_ON			(1)
#define		HV_OFF			(0)

#define		BAT_IDLE		(0)             /**/
#define     BAT_DSING		(1)
#define     BAT_CHING       (2)

struct BatStatue
{
    union
    {
        struct
        {
            INT32U  FullyFlag:1;/* 1 ON HV*/
            INT32U	DS_CH_Sta:2;/*0 空闲，1放电 2充电*/
            INT32U  CheckSOCFlag:1;/*校对SOC*/
        };
        INT32U All;
    };
};


struct bcu_class
{
    struct bmu_class 	*BMU;
    struct par_class 	*SysPar;
    struct BatteryMsg 	LineMsg;
    struct SysStatue    *SysSit;
    struct BatStatue    BatSit;
    
    INT16U SysErrorCode;/*系统错误代码*/
    
    INT32U Error[3][4];
    INT32U RunTime;
    RTC_TIME_Type   RTC;
};


extern struct 		bcu_class BCU;
INT8U BCUInit(void);
struct bcu_class *BCUOpen(void *par);
void BatCapacityCal(void);

void GetSOCAdj(INT16U CellV);
void GetTotalCurrent(void);
void BCUErrorDispatch(void);
void BatteryModle(INT16U Tick);

extern INT32S       Bat_Curr;			  // Discharge +, charger -, unit: 0.1A
extern INT32S       BMS_BAT_DAs_Acc;	  // Accumlated battery discharge capacity, unit: 0.01As
extern INT32S       BMS_BAT_As_Acc;		  // Accumlated battery capacity, unit: 0.01As
extern INT32S       BMS_BAT_Cap;
extern INT16S       BMS_T_Avg;			  // Average battery temperature, unit: 0.5C
extern INT32U		BMS_BAT_DAh_Acc;      // Battery accumlative discharge cpapcity, 1Ah
extern INT32U		BMS_BAT_Ah_Acc;       // Battery accumlative charge cpapcity, 1Ah
extern INT32S		BMS_BAT_DAs;          // Accumlated battery discharge capacity, unit: 0.01As
extern INT32S		BMS_BAT_As;           // Accumlated battery capacity, unit: 0.01As
extern INT16U		BMS_BAT_SoH;          // Battery State of Health, unit: %
extern INT16U		BMS_BAT_SoC;          // battery State of Charge, unit: %
extern INT32U		BMS_BAT_Cyc;          // Battery Cycle Coun

#endif
