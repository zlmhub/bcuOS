#include "bmu.h"
#include <board.h>
#include <app.h>

static OS_Device_T	CANDevBMU;
struct bmu_class	BMU;

#ifdef SIMULATOR_BMU
static void ThreadBMUTest (void *Par);
#endif

static void BMURxCANMsgDispatch(struct os_bxcan_msg *Msg);
static void QueryCellMapQuickly(INT8U  num, INT8U *p1, INT8U  *p2, INT8U  *p3);
/**
* CAN receive message threads assigned by BMU.
*/
OS_Err_T CANRxIndForBMU(OS_Device_T Dev, OS_Size_T Size)
{
    OS_Err_T	Ret = OS_EOK;
    struct os_bxcan_msg	*RxMsg;
    RxMsg = (struct os_bxcan_msg *)Dev->UserData;
    /* dispatch messages*/
    {
        BMURxCANMsgDispatch(RxMsg);
    }
    return Ret;
}

struct bmu_class *BMUOpen(void *par)
{
    return &BMU;
}
/**
 * BMU init
 */
INT8U	BMUInit (void)
{
    OS_Err_T	Ret;
    INT16U 		i;
    CANDevBMU = OSDeviceFind("BxCAN1");
    if(CANDevBMU != OS_NULL)
    {
        Ret = OSDeviceOpen( CANDevBMU, OS_DEVICE_FLAG_RDWR );
        if(Ret != OS_EOK)
        {
            OSPrintf("Open BxCAN1......[ERROR]\n");
            return OS_ERROR;
        }
        OSDeviceSetRxIndicate( CANDevBMU, CANRxIndForBMU );
    }
    else
    {
        OSPrintf("Open BxCAN1 for BMU......[ERROR]\n");
    }
#ifdef SIMULATOR_BMU
    OSAppCreate( ThreadBMUTest, OS_NULL, 128, 30 );
#endif
    for(i=0; i<360; i++)
    {
        BMU.CellVol[i] = 0;
    }
    for(i=0; i<60; i++)
    {
        BMU.CellTemp[i] = 0;
    }
    BMU.CellSeriesNum = 0;
    BMU.CellTempNum = 0;
    OSPrintf("BMU Init......[OK]\n");
    return OS_EOK;
}

static void QueryCellMapQuickly(INT8U  num, INT8U *p1, INT8U  *p2, INT8U  *p3)
{
    if (num > 4)
    {
        *p1 = 4;
        num -= 4;
        if (num > 4)
        {
            *p2 = 4;
            *p3 = num - 4;
        }
        else
        {
            *p2 = num;
            *p3 = 0;
        }
    }
}

/**
* the CAN message is parsed and the data is written to BMU, and BMU.CellSeriesNum 
* is required before use. the number of BMU.CellTempNum.
*/
static void BMURxCANMsgDispatch(struct os_bxcan_msg *Msg)
{
    INT8U   pack = 0;
    INT8U   node = 0;
    static  INT8U  p1, p2, p3;
    INT8U   i = 0;
    INT8U	*data;
    volatile INT32U	id;


    /* we have determined the number of batteries in series and the temperature 
    detection points, which can be prepared to receive data */
    if( (0 != BMU.CellSeriesNum )&&( 0 != BMU.CellTempNum ) )
    {
        id = Msg->Id;
        pack = (INT8U) ((id >> 8) & 0xFF);
        if ((id & 0xFFFF0000) != 0x0AF40000)
        {
            return ;
        }
        node = (INT8U) (id & 0xff) - 1;

        if (node > (BMS_BMU_MAX_NODE - 1))    /**< legal node */
        {
            return ;
        }
        QueryCellMapQuickly(BMU.CellMap[node], &p1, &p2, &p3);
        data = Msg->Data;

        switch (pack)
        {
        case 0x20:
            for (i = 0; i < p1; i++)
            {
                BMU.CellVol[BMU.CellMap[node] + i] = (((INT16U) data[2 * i]) << 8) + (data[2 * i + 1]);
            }
            break;
        case 0x21:
            for (i = 0; i < p2; i++)
            {
                BMU.CellVol[BMU.CellMap[node] + 4 + i] = (((INT16U) data[2 * i]) << 8)+ (data[2 * i + 1]);
            }
            break;
        case 0x22:
            for (i = 0; i < p3; i++)
            {
                BMU.CellVol[BMU.CellMap[node] + 8 + i] = (((INT16U) data[2 * i]) << 8) + (data[2 * i + 1]);
            }
            break;
        case 0x24:
            for (i = 0; i < 2; i++)
            {
                BMU.CellTemp[2 * node + i] = (data[2 * i + 1]);
            }
            break;
        default:
            break;
        }
    }
}


/**
* battery balancing.
*/
void BMUBlanceCells(void *Par)
{

}

#ifdef SIMULATOR_BMU
#include<stdlib.h>
#include<time.h>
struct os_bxcan_msg	BMUSimulatorTxMsg;
INT16U TCellVol[6][12];
const INT8U CellMap[]= {11,10,10,10,10};

static void ThreadBMUTest (void *Par)
{
    OSPrintf("BMU Smulator satrt\n");
    BMU.CellSeriesNum = 30;
    BMU.CellTempNum = 10;
    BMU.CellMap[0] = 0;
    BMU.CellMap[1] = 11;
    BMU.CellMap[2] = 21;
    BMU.CellMap[3] = 31;
    BMU.CellMap[4] = 41;
    BMU.CellMap[5] = 51;
    BMU.CellMap[6] = 0;
    while(1)
    {
        TCellVol[0][1]=3000+rand()%100;
        {
            /*装载CAN报文*/
            BMUSimulatorTxMsg.Id = 0x0AF42001;
            BMUSimulatorTxMsg.Len = 8;
            for(INT8U i=0; i<8; i++)
                BMUSimulatorTxMsg.Data[i] = 10+i;
            /*发送*/
            CANDevBMU->UserData = (void *)(&BMUSimulatorTxMsg);
            CANRxIndForBMU( CANDevBMU, sizeof(BMUSimulatorTxMsg) );
            OSTimeDly(1000);
        }
    }
}
#endif
/*END FILE*/
