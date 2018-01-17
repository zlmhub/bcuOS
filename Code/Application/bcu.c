/**************************************************************************//**
* @file     bcu.c
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
#include "bcu.h"

static void ReadBCUImportentData(void);
static void StorageBCUImportentData( void );

struct bcu_class BCU;
OS_Device_T     FM24C64Hf;

struct bcu_class *BCUOpen(void *par)
{
    return &BCU;
}


INT8U BCUInit(void)
{
    BMUInit();
    BCU.BMU = BMUOpen(0);
    if( 0 == BCU.BMU ) {
        OSPrintf("Open BMU->.....[ERROR]\n");
    }
    FM24C64Hf = OSDeviceFind("fm24cxx");
    if( FM24C64Hf != OS_NULL ) {
        OS_Err_T ret = OSDeviceOpen(FM24C64Hf,OS_DEVICE_FLAG_RDWR);
        if(ret != OS_EOK) {
            OSPrintf("Open FM24C64 device......[ERROR]\n");
            return OS_ERROR;
        }
    }
    /*Read the system setting parameters.*/
    OSDeviceRead( FM24C64Hf,
                  MODBUS_PAR_FLASH_START_ADD,
                  (RegHoldingPoint+MODBUS_PAR_HOLDREG_START_ADD),
                  MODBUS_PAR_FLASH_SIZE);
    BCU.SysPar = (struct par_class *)((RegHoldingPoint+MODBUS_PAR_HOLDREG_START_ADD));
    INT16U crc = Pec15Calc( MODBUS_PAR_FLASH_SIZE-2,(RegHoldingPoint+MODBUS_PAR_HOLDREG_START_ADD) );
    /*CRC proofreading error*/
    if( crc != BCU.SysPar->CRC) {
        OSPrintf("Read system partameter......[ERROR]\n");
        *(BCU.SysPar) = ParDefSet;
    } else {
        OSPrintf("Read system partmater......[OK]\n");
    }

    ReadBCUImportentData();/*read bcu very importen datas*/
    /*Map BCU System sit to Coil(Modbus RTU)*/
    BCU.SysSit = (struct SysStatue *)RegCoilsPoint;
    BCU.SysSit->All = 0;

    /*Read flash partmater and write into BMU*/
    BCU.BMU->CellSeriesNum = BCU.SysPar->CellNum;
    BCU.BMU->CellTempNum = BCU.SysPar->TempNum;
    /*Battery list*/
    BCU.BMU->CellMap[0] = 6;
    BCU.BMU->CellMap[1] = 6;

    OSPrintf("BCU start......[OK]\n");
    return OS_EOK;
}

/**
 * Get battery total voltage,uint mA
 */
void TotalVoltageCal(void)
{
    INT32U _Vol;
    INT16U i;

    _Vol = 0;
    for(i=0; i<BCU.BMU->CellSeriesNum; i++) {
        _Vol += BCU.BMU->CellVol[i];
    }

    BCU.LineMsg.TotalVol = _Vol;
}


void CellHighLowAvgCal(void)
{
    INT16U i;

    INT16U _maxV, _minV, _maxID, _minID;
    INT32U _tolV;

    _maxV = BCU.BMU->CellVol[0];
    _minV = BCU.BMU->CellVol[0];
    _maxID= 1;
    _minID= 1;
    _tolV = 0;

    for(i=0 ; i<BCU.BMU->CellSeriesNum ; i++) {
        if((BCU.BMU->CellVol[i]) > _maxV) {
            _maxV = BCU.BMU->CellVol[i];
            _maxID = i;
        }

        if((BCU.BMU->CellVol[i]) < _minV) {
            _minV = BCU.BMU->CellVol[i];
            _minID = i;
        }

        _tolV += BCU.BMU->CellVol[i];
    }

    BCU.LineMsg.LimitVol.MaxVol = _maxV;
    BCU.LineMsg.LimitVol.MinVol = _minV;
    BCU.LineMsg.LimitVol.MaxVolID = _maxID;
    BCU.LineMsg.LimitVol.MinVolID = _minID;
    BCU.LineMsg.VolAvg = (INT16U)(_tolV/BCU.BMU->CellSeriesNum);
    BCU.LineMsg.TotalVol = _tolV/1000;
}


void TempHighLowAvgCal(void)
{
    INT16U i;
    INT16U _maxT, _minT, _maxID, _minID;
    INT32U	_tolT = 0;

    _maxT = BCU.BMU->CellTemp[0];
    _minT = BCU.BMU->CellTemp[0];
    _maxID= 1;
    _minID= 1;
    _tolT = 0;

    for(i=0 ; i<BCU.BMU->CellTempNum ; i++) {
        if((BCU.BMU->CellTemp[i]) > _maxT) {
            _maxT = BCU.BMU->CellTemp[i];
            _maxID = i;
        }

        if((BCU.BMU->CellTemp[i]) < _minT) {
            _minT = BCU.BMU->CellTemp[i];
            _minID = i;
        }

        _tolT += BCU.BMU->CellTemp[i];
    }

    BCU.LineMsg.LimitVol.MaxTemp = _maxT;
    BCU.LineMsg.LimitVol.MinTemp = _minT;
    BCU.LineMsg.LimitVol.MaxTempID = _maxID;
    BCU.LineMsg.LimitVol.MinTempID = _minID;
    BCU.LineMsg.TempAvg = (INT16U)(_tolT/BCU.BMU->CellTempNum);
}

INT16U CapacityTfactorCal(INT16S TP)
{
    //_T_Avg = (FP32)(BMS_T_Avg-80)/2;

    // T < -20 C
    INT16U BatCapTFactor = 100;
    if(TP < 40) {
        BatCapTFactor = 52;
    }
    // -20 C <= T < -10 C
    else if(TP < 60) {
        // (1.3 T + 78)% = ( ((T-80)/2) * 1.3 + 78 )
        BatCapTFactor = (INT16U)( (FP32)(TP-80) * 0.65 + 78);
    }
    // -10 C <= T < 0 C
    else if(TP < 80) {
        // (1.5 T + 80)% = ( ((T-80)/2) * 1.5 + 80 )
        BatCapTFactor = (INT16U)( (FP32)(TP-80) * 0.75 + 80);
    }
    // 0 C <= T < 25 C
    else if(TP < 130) {
        // (1.08 T + 80)% = ( ((T-80)/2) * 1.08 + 80 )
        BatCapTFactor = (INT16U)( (FP32)(TP-80) * 0.54 + 80);
    }
    // 25 C <= T < 60 C
    else if(TP < 200) {
        // (0.086 T + 104.86)% = ( ((T-80)/2) * 0.086 + 80 )
        BatCapTFactor = (INT16U)( (FP32)(TP-80) * 0.043 + 104.86);
    } else
        BatCapTFactor = 110;
    return BatCapTFactor;
}

INT32S CapWorstCase(INT32S InCap)
{
    if(InCap < BSP_BAT_Cap_WORST_CASE_L)
        return BSP_BAT_Cap_WORST_CASE_L;
    if(InCap > BSP_BAT_Cap_WORST_CASE_H)
        return BSP_BAT_Cap_WORST_CASE_H;
    else
        return InCap;
}


static FP64 Xk =0.1 ,Kg;
static FP64 Pk = 0.25;
static FP64 R = 0.14,Q = 0.0005;
void Klman(FP64 tmp)
{
    Kg = Pk/(Pk+R);
    Pk = (1-sqrt(Kg))*Pk;
    Xk = Xk+sqrt(Kg)*(tmp-Xk);
    Pk = Pk+Q;
    Xk = Xk;
}

/**
* Hall sensor detects current.
*/
#define REFV            (3263UL)/*ADC reference voltage Unit:mV*/
#define HALL_RANGE      (250UL)/*Hall unilateral range. Unit:A*/
#define FACTOR          ((FP64)3.263)
#define ZERO_SUP        ((FP64)2.0)/*Zero suppression Unit:A*/
void GetTotalCurrent(void)
{
    static volatile FP64 current = 0;
    current = LPC17xxHwCurrentAdSamp(REFV);/*MCU ADC Vol Unit:mV*/
    /*-------------
    Current = (ADC*FACTOR)-5000mV    <Hardware decision>
    --------------*/
    OS_ENTER_CRITICAL();
    current = ((current*FACTOR)-5000)/1000;
    Klman( current);
    current = Xk;
    current = (current-2.5)*HALL_RANGE;
    /*Zero suppression*/
    if( ( current < ZERO_SUP ) && ( current > -ZERO_SUP ) ) {
        current = 0;
    }
    BCU.LineMsg.TotalCurrent = current;
    OS_EXIT_CRITICAL();
}

INT32S      Bat_Curr;			  // Discharge +, charger -, unit: 0.1A
INT32S      BMS_BAT_DAs_Acc;	  // Accumlated battery discharge capacity, unit: 0.01As
INT32S      BMS_BAT_As_Acc;		  // Accumlated battery capacity, unit: 0.01As
INT32S      BMS_BAT_Cap;
INT16S      BMS_T_Avg;			  // Average battery temperature, unit: 0.5C
INT32U		BMS_BAT_DAh_Acc;      // Battery accumlative discharge cpapcity, 1Ah
INT32U		BMS_BAT_Ah_Acc;       // Battery accumlative charge cpapcity, 1Ah
INT32S		BMS_BAT_DAs;          // Accumlated battery discharge capacity, unit: 0.01As
INT32S		BMS_BAT_As;           // Accumlated battery capacity, unit: 0.01As
INT16U		BMS_BAT_SoH;          // Battery State of Health, unit: %
INT16U		BMS_BAT_SoC;          // battery State of Charge, unit: %
INT32U		BMS_BAT_Cyc;          // Battery Cycle Count

void BatCapacityCal(void)
{
    Bat_Curr = BCU.LineMsg.TotalCurrent*10;
    if((BCU.SysSit->IO_HVP)||(BCU.SysSit->IO_CHP)) {
        /// Calculate Battery accumulative discharge & charge
        /// Bat_Curr > 0, Discharge
        ///	Bat_Curr < 0, Charge
        if(Bat_Curr>0)
            BMS_BAT_DAs_Acc += Bat_Curr;
        else
            BMS_BAT_As_Acc -= Bat_Curr;

        if(BMS_BAT_DAs_Acc > 360000) {
            BMS_BAT_DAh_Acc++;	// unit: 1Ah
            BMS_BAT_DAs_Acc -= 360000;	// unit: 0.01As
        }

        if(BMS_BAT_As_Acc > 360000) {
            BMS_BAT_Ah_Acc++;	// unit: 1Ah
            BMS_BAT_As_Acc -= 360000;	// unit: 0.01As
        }

        /// Calculate Battery discharge & charge

        BMS_BAT_DAs += Bat_Curr;
        BMS_BAT_As = BMS_BAT_Cap - BMS_BAT_DAs;

        if(BCU.LineMsg.LimitVol.MaxVol >= 3650) {
            if (!BCU.BatSit.FullyFlag) {
                BMS_BAT_Cyc++;
                // Estimated times of fully charged
                BCU.BatSit.FullyFlag = 1;
            }
            BMS_BAT_Cap = CapWorstCase(BMS_BAT_As);
            BMS_BAT_DAs = 0;
        } else if(BCU.LineMsg.LimitVol.MinVol <= 2000) {
            BMS_BAT_Cap = CapWorstCase(BMS_BAT_DAs);
            BMS_BAT_As = 0;
        } else if(BMS_BAT_DAs < 0) {
            BMS_BAT_Cap = CapWorstCase(BMS_BAT_As);
            BMS_BAT_DAs = 0;
        } else if(BMS_BAT_DAs > BMS_BAT_Cap) {
            BMS_BAT_Cap = CapWorstCase(BMS_BAT_DAs);
            BMS_BAT_As = 0;
        } else if(BCU.LineMsg.LimitVol.MaxVol <= 3250) {
            BCU.BatSit.FullyFlag  = 0;
        }
    }

    INT16U BMS_BAT_Cap_Tfactor = CapacityTfactorCal(BMS_T_Avg);

    BMS_BAT_SoH = BMS_BAT_Cap / ((BSP_BAT_Cap_Int/10000) * BMS_BAT_Cap_Tfactor);
    BMS_BAT_SoC = (INT16U)(BMS_BAT_As / (BMS_BAT_Cap/100));

    /*BCU*/
    BCU.LineMsg.SOC = ((FP32)BMS_BAT_SoC)/100;
    BCU.LineMsg.Cyc = BMS_BAT_Cyc;
    StorageBCUImportentData();
}

const INT16U    Bat25DCV[]= {3650,3339,3332,3330,3329,3327,3308,3294,3291,3289,3288,3287,3286,3283,3275,3261,3249,3232,3211,3198,3170,2984,2500};
const INT16U    Bat25DCSoc[]= {1000,954,908,862,816,770,724,678,632,586,540,494,448,402,356,310,264,218,172,126,80,34,0};


/**
* SOC correction
* CellV: minimum battery voltage.
*/
void GetSOCAdj(INT16U CellV)
{
    INT16U i;
    INT16U	SOC;
    INT8U	BatMapLen = sizeof(Bat25DCV);
    for(i=0; i<(BatMapLen-2); i++) {
        if(CellV == Bat25DCV[i]) {
            SOC = Bat25DCSoc[i];
        } else if( (CellV>Bat25DCV[i+1]) && (CellV<Bat25DCV[i]) ) {
            SOC = Bat25DCSoc[i+1]+ (CellV-Bat25DCV[i+1])*(Bat25DCSoc[i]-Bat25DCSoc[i+1])/(Bat25DCV[i]-Bat25DCV[i+1]);
        }
    }
    if(CellV == Bat25DCV[BatMapLen-1]) {
        SOC = Bat25DCSoc[BatMapLen-1];
    }
    if(CellV >Bat25DCV[0]) {
        SOC = Bat25DCSoc[0];
    }
    BMS_BAT_SoC = SOC/10;
    BMS_BAT_Cap = BSP_BAT_Cap_Int;
    FP32 Isr=(1000.0-(FP32)SOC)/1000.0;
    BMS_BAT_DAs = BMS_BAT_Cap*Isr;
    BMS_BAT_As = BMS_BAT_Cap - BMS_BAT_DAs;

    /*BCU*/
    BCU.LineMsg.SOC = ((FP32)BMS_BAT_SoC)/100;


}


/**
 *  Important data.RTC registers
 *  -------------------------------------------
 *      Address(RTC)|   Class(BCU)
 *  -------------------------------------------
 *          0       |   Run time
 *  -------------------------------------------
 *          1       |   SOC+(Cyc<<16)
 *  -------------------------------------------
 */
static void StorageBCUImportentData( void )
{
#if (0)
#define	IMPORTENT_DATA_SIZE		64
    INT8U	Buffer[IMPORTENT_DATA_SIZE];
    INT8U	*pBuff;
    pBuff = Buffer;
    *(INT32U*)pBuff = BCU.RunTime;
    pBuff += 4;
    *(INT32S*)pBuff = BMS_BAT_Cap;
    pBuff += 4;
    *(INT32S*)pBuff = BMS_BAT_DAs;
    pBuff += 4;
    *(INT32S*)pBuff = BMS_BAT_As;
    pBuff += 4;
    *(INT32S*)pBuff = BMS_BAT_DAs_Acc;
    pBuff += 4;
    *(INT32S*)pBuff = BMS_BAT_As_Acc;
    pBuff += 4;
    *(INT32U*)pBuff = BMS_BAT_DAh_Acc;
    pBuff += 4;
    *(INT32U*)pBuff = BMS_BAT_Ah_Acc;
    pBuff += 4;
    *(INT16U*)pBuff = BMS_BAT_SoC;
    pBuff += 2;
    *(INT16U*)pBuff = BMS_BAT_SoH;
    pBuff += 2;
    *(INT32U*)pBuff = BMS_BAT_Cyc;
    pBuff += 4;
    OSDeviceWrite(FM24C64Hf,0,Buffer,IMPORTENT_DATA_SIZE);
#else
    INT32U Vol=0;
    Vol = (((INT32U)BMS_BAT_Cyc)<<16)+(INT16U)BMS_BAT_SoC;
    RTC_WriteGPREG (LPC_RTC, 1, Vol);
#endif
}

static void ReadBCUImportentData(void)
{
    OS_Device_T RTCHf;
    RTCHf = OSDeviceFind("rtc");
    if(OS_NULL != RTCHf) {
        OSDeviceControl(RTCHf,OS_DEVICE_CTRL_RTC_GET_TIME,&BCU.RTC);
    } else {
        OSPrintf("OS RTC not find......[ERROR]\n");
    }
    INT32U Vol=0;
    Vol = RTC_ReadGPREG (LPC_RTC, 1);
    BMS_BAT_Cyc = Vol >>16;
    BMS_BAT_SoC = Vol&0x0000FFFF;
    if(BMS_BAT_SoC >100) {
        BCU.BatSit.CheckSOCFlag = 1;
    } else {
        BMS_BAT_Cap = BSP_BAT_Cap_Int;
        FP32 Isr=(100.0-(FP32)BMS_BAT_SoC)/100.0;
        BMS_BAT_DAs = BMS_BAT_Cap*Isr;
        BMS_BAT_As = BMS_BAT_Cap - BMS_BAT_DAs;

        /*BCU*/
        BCU.LineMsg.SOC = ((FP32)BMS_BAT_SoC)/100;
    }
}

/**
 * low soc
 */
void OSLowPowerCallBack(void *Par)
{
    StorageBCUImportentData( );
}

/**
* misjudgment
* par 1: par is the data to be judged.
* par 2: epar1 is a level 1 alarm threshold.
* parameter 3: epar2 is a level 2 alarm threshold.
* par 4: epar3 is a level 3 alarm threshold.
* parameter id: for alarm address.
* parameter mod: >0 is used to judge data less than threshold alarm.< 0 and vice
*/
static void ErrorCheck(INT32U par,
                       INT32U epar1,INT32U epar2,INT32U epar3,
                       INT8U id,
                       INT8U mod)
{

    INT8U pak,num;
    if(id>4*32)
        return;
    pak = id/32;
    num = id%32;

    if(mod ==1) {
        if(par > epar1)
            BCU.Error[0][pak] |=  1<<num;
        else
            BCU.Error[0][pak] &=  ~(1<<num);
        if(par > epar2)
            BCU.Error[1][pak] |=  1<<num;
        else
            BCU.Error[1][pak] &=  ~(1<<num);
        if(par > epar3)
            BCU.Error[2][pak] |=  1<<num;
        else
            BCU.Error[2][pak] &=  ~(1<<num);
    } else {
        if(par < epar1)
            BCU.Error[0][pak] |=  1<<num;
        else
            BCU.Error[0][pak] &=  ~(1<<num);
        if(par < epar2)
            BCU.Error[1][pak] |=  1<<num;
        else
            BCU.Error[1][pak] &=  ~(1<<num);
        if(par < epar3)
            BCU.Error[2][pak] |=  1<<num;
        else
            BCU.Error[2][pak] &=  ~(1<<num);
    }
}

/*------------------------------------------------------------------------------
 * The system error checking is consistent with the basic error information detection
 * requirement of QTC897.
 -----------------------------------------------------------------------------*/
void BCUErrorDispatch(void)
{
#define MAXMODE     1
#define MINMODE     0
    /*单体CELL*/
    ErrorCheck(BCU.LineMsg.LimitVol.MaxVol,
               BCU.SysPar->CellMaxWar[0],
               BCU.SysPar->CellMaxWar[1],
               BCU.SysPar->CellMaxWar[2],
               0,
               MAXMODE);
    ErrorCheck(BCU.LineMsg.LimitVol.MinVol,
               BCU.SysPar->CellMinWar[0],
               BCU.SysPar->CellMinWar[1],
               BCU.SysPar->CellMinWar[2],
               1,
               MINMODE);
    /*温度*/
    ErrorCheck(BCU.LineMsg.LimitVol.MaxTemp,
               BCU.SysPar->TempMaxWar[0],
               BCU.SysPar->TempMaxWar[1],
               BCU.SysPar->TempMaxWar[2],
               2,
               MAXMODE);
    ErrorCheck(BCU.LineMsg.LimitVol.MinTemp,
               BCU.SysPar->TempMinWar[0],
               BCU.SysPar->TempMinWar[1],
               BCU.SysPar->TempMinWar[2],
               3,
               MINMODE);
    /*压差*/
    ErrorCheck(BCU.LineMsg.LimitVol.MaxVol-BCU.LineMsg.LimitVol.MinVol,
               BCU.SysPar->CellVolDiffMax[0],
               BCU.SysPar->CellVolDiffMax[1],
               BCU.SysPar->CellVolDiffMax[2],
               4,
               MAXMODE);
    /*总电压*/
    ErrorCheck(BCU.LineMsg.TotalVol,
               BCU.SysPar->TotalVolMax[0],
               BCU.SysPar->TotalVolMax[1],
               BCU.SysPar->TotalVolMax[2],
               5,
               MAXMODE);
    ErrorCheck(BCU.LineMsg.TotalVol,
               BCU.SysPar->TotalVolMin[0],
               BCU.SysPar->TotalVolMin[1],
               BCU.SysPar->TotalVolMin[2],
               6,
               MINMODE);
    /*电流*/
    ErrorCheck(BCU.LineMsg.TotalCurrent,
               BCU.SysPar->DchCurrMax[0],
               BCU.SysPar->DchCurrMax[1],
               BCU.SysPar->DchCurrMax[2],
               7,
               MAXMODE);
    ErrorCheck(BCU.LineMsg.TotalCurrent,
               BCU.SysPar->ChCurrMax[0],
               BCU.SysPar->ChCurrMax[1],
               BCU.SysPar->ChCurrMax[2],
               8,
               MAXMODE);
    /*SOC*/
    ErrorCheck((INT32U)(BCU.LineMsg.SOC*100),
               BCU.SysPar->SOCMax[0],
               BCU.SysPar->SOCMax[1],
               BCU.SysPar->SOCMax[2],
               9,
               MAXMODE);
    ErrorCheck((INT32U)(BCU.LineMsg.SOC*100),
               BCU.SysPar->SOCMin[0],
               BCU.SysPar->SOCMin[1],
               BCU.SysPar->SOCMin[2],
               10,
               MINMODE);
    /*绝缘*/
    ErrorCheck(BCU.LineMsg.InsuR,
               BCU.SysPar->InsulaMin[0],
               BCU.SysPar->InsulaMin[1],
               BCU.SysPar->InsulaMin[2],
               11,
               MINMODE);
}

void BatteryModle(INT16U Tick)
{
    static INT16U TimeCHF  = 0;
    static INT16U TimeDSF  = 0;
    static INT16U TimeIDLE = 0;

    if(BCU.LineMsg.TotalCurrent >2) { /*>2A*/
        TimeDSF++;
        TimeCHF = 0;
        if(TimeDSF  > 20) { /*>2S*/
            BCU.BatSit.DS_CH_Sta = BAT_DSING;/*Dschargeing Modle*/
            TimeDSF = 0;
        }
    } else if (BCU.LineMsg.TotalCurrent < -2) {
        TimeCHF++;
        TimeDSF = 0;
        if( TimeCHF >20) {
            BCU.BatSit.DS_CH_Sta = BAT_CHING;/*Charging Modle*/
            TimeCHF = 0;
        }
    } else if((BCU.LineMsg.TotalCurrent > -2) &&(BCU.LineMsg.TotalCurrent < 2)) {
        TimeCHF = 0;
        TimeDSF = 0;
        BCU.BatSit.DS_CH_Sta = BAT_IDLE;
    }
}
/**END**/
