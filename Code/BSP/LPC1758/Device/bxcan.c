#include "bxcan.h"

struct OS_Device BxCAN1,BxCAN2;
static INT8U can_OpenFlag = 0;

static OS_Err_T LPC17xxCAN1HwInit(OS_Device_T dev)
{
    struct OS_Device *device;
    device = dev;
    /*assert*/
    CAN_DeInit(LPC_CAN1);
    PINSEL_CFG_Type PinCfg;
    /* Pin configuration
     * CAN1: select P0.0 as RD1. P0.1 as TD1
     */
    PinCfg.Funcnum = 1;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Pinnum = 0;
    PinCfg.Portnum = 0;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 1;
    PINSEL_ConfigPin(&PinCfg);
    /* Initialize CAN1 peripheral
     * Note: Self-test mode doesn't require pin selection
     */
    CAN_Init(LPC_CAN1, 250000);
    /* Enable OPERATING mode */
    CAN_ModeConfig(LPC_CAN1, CAN_OPERATING_MODE, ENABLE);

    /* Enable Interrupt */
    CAN_IRQCmd(LPC_CAN1, CANINT_RIE, ENABLE);
    NVIC_EnableIRQ(CAN_IRQn);
    CAN_SetAFMode(LPC_CANAF, CAN_AccBP);

    can_OpenFlag |=(1<<0);
    return OS_EOK;
}


static OS_Err_T LPC17xxCAN2HwInit(OS_Device_T dev)
{
    PINSEL_CFG_Type PinCfg;
    /* Pin configuration
     * CAN2: select P2.7 as RD2, P2.8 as RD2
     */
    PinCfg.Funcnum = 1;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;

    PinCfg.Pinnum = 7;
    PinCfg.Portnum = 2;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 8;
    PINSEL_ConfigPin(&PinCfg);

    /* Initialize CAN1 peripheral
     * Note: Self-test mode doesn't require pin selection
     */
    CAN_Init(LPC_CAN2, 250000);
    /* Enable OPERATING mode */
    CAN_ModeConfig(LPC_CAN2, CAN_OPERATING_MODE, ENABLE);

    /* Enable Interrupt */
    CAN_IRQCmd(LPC_CAN2, CANINT_RIE, ENABLE);
    NVIC_EnableIRQ(CAN_IRQn);
    CAN_SetAFMode(LPC_CANAF, CAN_AccBP);
    can_OpenFlag |=(1<<1);
    return OS_EOK;
}

/*------------------------------------------------------------------------------
 *  说明：CAN发送数据函数
 *  参数：id_id号，len_长度，data_数据
 *  返回：ret_0发送失败，1发送成功
 -----------------------------------------------------------------------------*/
INT8U LPC17xxCANHwTransmit(LPC_CAN_TypeDef* CANx, struct os_bxcan_msg *msg)
{
    INT8U i = 0, ret = 0;
    CAN_MSG_Type TXMsg;

    TXMsg.format = EXT_ID_FORMAT;
    TXMsg.id = msg->Id;
    TXMsg.len = msg->Len;
    TXMsg.type = DATA_FRAME;
    if (msg->Len > 8)
    {
        return ERROR;
    }
    if (msg->Len > 4)
    {
        for (i = 0; i < 4; i++)
        {
            TXMsg.dataA[i] = msg->Data[i];
        }
        for (i = 0; i < (msg->Len - 4); i++)
        {
            TXMsg.dataB[i] = msg->Data[i + 4];
        }
    }
    else
    {
        for (i = 0; i < msg->Len; i++)
        {
            TXMsg.dataA[i] = msg->Data[i];
        }
    }

    ret = CAN_SendMsg(CANx, &TXMsg);
    return ret;
}

static OS_Err_T LPC17xxCAN1HwOpen(struct OS_Device *dev, INT16U oflag)
{
    /*assert*/
    /* get open flags */
    dev->OpenFlag = oflag & 0xff;
    return OS_EOK;
}

static OS_Size_T LPC17xxHwCAN1Write(struct OS_Device *dev,
                                    OS_Off_T          pos,
                                    const void       *buffer,
                                    OS_Size_T         size)
{
    struct os_bxcan_msg *msg;

    /*assert*/
    /*设备已经被打开哦*/
    if ((dev->OpenFlag & OS_DEVICE_OFLAG_OPEN ) && (dev->RefCount > 0))
    {
        msg = (struct os_bxcan_msg*)buffer;
        INT8U ret = 0;
        if((msg->Id >0x1FFFFFFF)||(msg->Len >8))
        {
            OSPrintf("CAN Message Data Error\n");
            return OS_NULL;
        }
        ret = LPC17xxCANHwTransmit(LPC_CAN1,msg);

        if( ERROR != ret )
        {
            return sizeof(msg);
        }
        else
        {
            return OS_NULL;
        }
    }
    return OS_NULL;
}

static OS_Size_T LPC17xxHwCAN2Write(struct OS_Device *dev,
                                    OS_Off_T          pos,
                                    const void       *buffer,
                                    OS_Size_T         size)
{
    struct os_bxcan_msg *msg;

    /*assert*/
    /*设备已经被打开哦*/
    if ((dev->OpenFlag & OS_DEVICE_OFLAG_OPEN ) && (dev->RefCount > 0))
    {
        msg = (struct os_bxcan_msg*)buffer;
        INT8U ret = 0;
        ret = LPC17xxCANHwTransmit(LPC_CAN2,msg);
        if( ERROR != ret )
        {
            return sizeof(msg);
        }
        else
        {
            return OS_NULL;
        }
    }
    return OS_NULL;
}

static OS_Size_T LPC17xxCAN1Read(struct OS_Device *dev,
                                 OS_Off_T          pos,
                                 void             *buffer,
                                 OS_Size_T         size)
{
    struct rt_can_device *can;

    /*assert*/
    return 0;
}

static OS_Err_T LPC17xxCAN2Open(struct OS_Device *dev, INT16U oflag)
{
    /*assert*/
    /* get open flags */
    dev->OpenFlag = oflag & 0xff;
    return OS_EOK;
}

/*
 * 注册BXCAN设备
 */
OS_Err_T LPC17xxBxCanInit(void)
{
    struct OS_Device *device;
    device = &BxCAN1;

    device->Type        = OS_Device_Class_CAN;/*设备类型*/
    device->Init = LPC17xxCAN1HwInit;
    device->Open = LPC17xxCAN1HwOpen;
    device->Write = LPC17xxHwCAN1Write;
    device->Read = LPC17xxCAN1Read;

    OSDeviceRegister(&BxCAN1, "BxCAN1", OS_DEVICE_FLAG_RDWR);

    device = &BxCAN2;
    device->Type = OS_Device_Class_CAN;/*设备类型*/
    device->Init = LPC17xxCAN2HwInit;
    device->Open = LPC17xxCAN2Open;
    device->Write = LPC17xxHwCAN2Write;

    OSDeviceRegister(&BxCAN2, "BxCAN2", OS_DEVICE_FLAG_RDWR);
    return OS_EOK;
}



INT8U LPC17xxCANReceive( LPC_CAN_TypeDef* CANx,struct os_bxcan_msg *rx_msg)
{

    INT8U IntStatus;
    CAN_MSG_Type RXMsg;
    /* Get CAN status */
    IntStatus = CAN_GetCTRLStatus(CANx, CANCTRL_STS);
    if ((IntStatus >> 0) & 0x01)
    {
        CAN_ReceiveMsg(CANx, &RXMsg);
    }

    /* check receive buffer status */
    if (!((IntStatus >> 0) & 0x01))
    {
        return ERROR;
    }
    /*有数据存入*/
    rx_msg->Id = RXMsg.id;
    rx_msg->Len = RXMsg.len;
    if (rx_msg->Len > 4)
    {
        for (INT8U i = 0; i < 4; i++)
            rx_msg->Data[i] = RXMsg.dataA[i];
        for (INT8U j = 0; j < (rx_msg->Len - 4); j++)
            rx_msg->Data[j + 4] = RXMsg.dataB[j];
    }
    else
    {

        for (INT8U i = 0; i < rx_msg->Len; i++)
            rx_msg->Data[i] = RXMsg.dataA[i];
    }

    return SUCCESS;
}



void CAN_IRQHandler(void)
{
    struct os_bxcan_msg buffer;

    if(can_OpenFlag &0x01)
    {
        if ( SUCCESS == LPC17xxCANReceive( LPC_CAN1,&buffer ) )
        {
            if(BxCAN1.RxIndicate != OS_NULL )
            {
                BxCAN1.UserData = (void *)(&buffer);
                BxCAN1.RxIndicate(&BxCAN1,OS_NULL);
            }
        }
    }
    if((can_OpenFlag >>1)&0x01)
    {
        if ( SUCCESS == LPC17xxCANReceive( LPC_CAN2,&buffer ) )
        {
            if(BxCAN2.RxIndicate != OS_NULL )
            {
                BxCAN2.UserData = (void *)(&buffer);
                BxCAN2.RxIndicate(&BxCAN2,OS_NULL);
            }
        }
    }
}
