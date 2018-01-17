#include "modbus_debug.h"


struct bcu_class  *BCU_M;

OS_Device_T     ModebusFM24C64Hf;


INT8U ModebusInit(void)
{
    INT8U Len = sizeof(struct par_class);
    ModebusFM24C64Hf = OSDeviceFind("fm24cxx");
    if( ModebusFM24C64Hf != OS_NULL ) {
        OS_Err_T ret = OSDeviceOpen(ModebusFM24C64Hf,OS_DEVICE_FLAG_RDWR);
        if(ret != OS_EOK) {
            OSPrintf("Open FM24C64 device......[ERROR]\n");
            return OS_ERROR;
        }
    }
    OSDeviceRead( ModebusFM24C64Hf, 512,(void *) RegHoldingPoint, Len );
}

/**
 * Transfer the data of BCU to Modbus register.
 */
void ModebusDebugMain(void)
{
    struct main_class	*P;
    BCU_M  = BCUOpen(0);
    P = (struct main_class *)RegInputPoint;
    /*The main information*/
    P->TotalVol = (INT16U)BCU_M->LineMsg.TotalVol;
    P->TotalCurrent = (INT16S)BCU_M->LineMsg.TotalCurrent;
    P->SOC = BCU_M->LineMsg.SOC*100;
    P->SOH = BCU_M->LineMsg.SOH;
    P->Cyc = BCU_M->LineMsg.Cyc;
    P->CellVolAvg = BCU_M->LineMsg.VolAvg;
    P->CellTempAvg = ((INT16S)BCU_M->LineMsg.TempAvg)-40;
    /*Basic information of single battery.*/
    P->CellMaxVol = BCU_M->LineMsg.LimitVol.MaxVol;
    P->CellMaxVolId = BCU_M->LineMsg.LimitVol.MaxVolID;
    P->CellMinVol = BCU_M->LineMsg.LimitVol.MinVol;
    P->CellMinVolId = BCU_M->LineMsg.LimitVol.MinVolID;
    P->CellMaxTemp = (INT16S)BCU_M->LineMsg.LimitVol.MaxTemp-40;
    P->CellMaxTempId = BCU_M->LineMsg.LimitVol.MaxTempID;
    P->CellMinTemp = (INT16S)BCU_M->LineMsg.LimitVol.MinTemp-40;
    P->CellMinTempId = BCU_M->LineMsg.LimitVol.MinTempID;
    /*The charging information*/
    P->ChargeVol = (INT16U)BCU_M->LineMsg.ChargeVol;
    P->ChargeCurrent = (INT16U)BCU_M->LineMsg.ChargeCurrent;
    P->ChargeCode = (INT16U)BCU_M->LineMsg.ChargeCode;

    RegInputPoint[15] = BCU_M->SysErrorCode;
    /*The alarm information*/
    INT32U *ErrP;
    ErrP = (INT32U *)(RegCoilsPoint+2);
    *ErrP = BCU.Error[0][0];
    ErrP = (INT32U *)(RegCoilsPoint+6);
    *ErrP = BCU.Error[1][0];
    ErrP = (INT32U *)(RegCoilsPoint+10);
    *ErrP = BCU.Error[2][0];
    /*BCU RTC*/
    struct RTC{
        INT16U Year;
        INT16U Month;
        INT16U Day;
        INT16U Hour;
        INT16U Min;
    };
    struct RTC *_rtc;
    _rtc = (struct RTC *)(RegInputPoint+32);
    _rtc->Year = BCU.RTC.YEAR;
    _rtc->Day = BCU.RTC.DOY;
    _rtc->Month = BCU.RTC.MONTH;
    _rtc->Hour = BCU.RTC.HOUR;
    _rtc->Min = BCU.RTC.MIN;
}



void ModebusSetPar(void)
{
    struct par_class *ParP;
    INT16U CRC;
    ParP = (struct par_class *)RegHoldingPoint;

    CRC = Pec15Calc( MODBUS_PAR_FLASH_SIZE-2,RegHoldingPoint );
    if(CRC != ParP->CRC) { /*参数有设置过，写入flash中*/
        OS_ENTER_CRITICAL();
        ParP->CRC = Pec15Calc( MODBUS_PAR_FLASH_SIZE-2,RegHoldingPoint );
        OSDeviceWrite( ModebusFM24C64Hf,
                       MODBUS_PAR_FLASH_START_ADD,
                       (void *) RegHoldingPoint,
                       MODBUS_PAR_FLASH_SIZE );
        OS_EXIT_CRITICAL();
    }
}

const struct par_class ParDefSet= {
    101,//INT16U  Version;
    1,//INT16U  ModleId;
    1,//INT16U  ID;
    1,//INT16U  BoxNum;
    36,//INT16U  CellNum;
    6,//INT16U  TempNum;
    10,//INT16U  BatCap;
    1,//INT16U  HallCoef;
    0xBBB0,//INT16U  CellArea1;
    0x0000,//INT16U  CellArea2;
    0x0000,//INT16U  CellArea3;
    0x0000,//INT16U  CellArea4;
    0x0000,//INT16U  CellArea5;
    0x0000,//INT16U  RESV1;
    0x0000,//INT16U  RESV2;
    0x0000,//INT16U  RESV3;
    0x0000,//INT16U  RESV4;
    {3500,3600,3680},//INT16U  CellMaxWar[3];
    {3500,3600,3680},//INT16U  CellMinWar[3];
    {50,55,60},//INT16S  TempMaxWar[3];
    {10,5,0},//INT16S  TempMinWar[3];
    {100,150,200},//INT16U  CellVolDiffMax[3];
    {100,105,110},//INT16U  TotalVolMax[3];
    {90,85,80},//INT16U  TotalVolMin[3];
    {10,20,30},//INT16U  DchCurrMax[3];
    {10,20,30},//INT16U  ChCurrMax[3];
    {100,105,110},//INT16U  SOCMax[3];
    {20,15,10},//INT16U  SOCMin[3];
    {20,10,1},//INT16U  InsulaMin[3];
    {0x0000}//INT16U  CRC;
};