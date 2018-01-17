#ifndef __BMU_H_
#define __BMU_H_

#include <ucos_ii.h>
#include <ucos_device.h>

//#define SIMULATOR_BMU

#define  BMS_BMU_MAX_NODE		10

struct bmu_class{
	INT8U	CellMap[30];	/*电池排列*/
	INT16U	CellSeriesNum;	/*电池串联数*/
	INT8U	CellTempNum;	/*温度采集点*/
	INT16U	CellVol[360];
	INT8U	CellTemp[60];
};

INT8U	BMUInit (void);
struct bmu_class *BMUOpen(void *par);
#endif
