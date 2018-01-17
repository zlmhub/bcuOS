#ifndef __MODBUS_DEBUG_H_
#define __MODBUS_DEBUG_H_

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include <ucos_ii.h>
#include "bcu.h"

/*参数写入flash起始地址和写入的大小*/
#define MODBUS_PAR_HOLDREG_START_ADD        0
#define MODBUS_PAR_FLASH_START_ADD          512
#define MODBUS_PAR_FLASH_SIZE     (sizeof(struct par_class))

extern USHORT  *RegHoldingPoint;
extern USHORT  *RegInputPoint;
extern INT8U   *RegCoilsPoint;

/*系统基本信息(20个)*/
struct main_class
{
    INT16U	TotalVol;
    INT16U	TotalCurrent;
    INT16U	SOC;
    INT16U	SOH;
    INT16U	Cyc;
    INT16U  CellVolAvg;
    INT16U  CellTempAvg;
    INT16U	CellMaxVol;
    INT16U	CellMaxVolId;
    INT16U	CellMinVol;
    INT16U	CellMinVolId;
    INT16U	CellMaxTemp;
    INT16U	CellMaxTempId;
    INT16U	CellMinTemp;
    INT16U	CellMinTempId;
    INT16U	SysWarningCode;
    INT16U	SysErrorCode;
    INT16U	ChargeVol;
    INT16U	ChargeCurrent;
    INT16U	ChargeCode;
};


/*系统参数设置(54个)*/
struct par_class
{
    INT16U  Version;
    INT16U  ModleId;
    INT16U  ID;
    INT16U  BoxNum;
    INT16U  CellNum;
    INT16U  TempNum;
    INT16U  BatCap;
    INT16U  HallCoef;
    INT16U  CellArea1;
    INT16U  CellArea2;
    INT16U  CellArea3;
    INT16U  CellArea4;
    INT16U  CellArea5;
    INT16U  ChargeMaxVol;
    INT16U  ChargeMaxCurrent;
    INT16U  RESV1;
    INT16U  RESV2;
    INT16U  CellMaxWar[3];
    INT16U  CellMinWar[3];
    INT16S  TempMaxWar[3];
    INT16S  TempMinWar[3];
    INT16U  CellVolDiffMax[3];
    INT16U  TotalVolMax[3];
    INT16U  TotalVolMin[3];
    INT16U  DchCurrMax[3];
    INT16U  ChCurrMax[3];
    INT16U  SOCMax[3];
    INT16U  SOCMin[3];
    INT16U  InsulaMin[3];
    INT16U  CRC;
};

/*系统报警信息*/
struct err_class
{
  INT32U Error[3][4];
};

extern const struct par_class ParDefSet;

INT8U ModebusInit(void);
void ModebusSetPar(void);
#endif