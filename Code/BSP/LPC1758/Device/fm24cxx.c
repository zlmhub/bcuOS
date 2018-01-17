#include "fm24cxx.h"
#include <board.h>
#include <ucos_ii.h>
#include <ucos_device.h>
struct OS_Device FM24CxxDev;

static void LPC17xxHwIIcConfig(void)
{

    PINSEL_CFG_Type PinCfg;
    /*
     * Initialize IIC pin connect
     * P0.10 - SDA;
     * P0.11 - CLK
     */
    PinCfg.Funcnum = 2;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Portnum = 0;
    PinCfg.Pinnum = 10;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 11;
    PINSEL_ConfigPin(&PinCfg);

    I2C_DeInit(LPC_I2C2);
    I2C_Init(LPC_I2C2, 200000); /* 200kbit/s */
    I2C_IntCmd(LPC_I2C2, DISABLE);
    I2C_Cmd(LPC_I2C2, I2C_MASTER_MODE, ENABLE);
}

/**-----------------------------------------------------------------------------
 * \brief fm24c64写入一定长度的数据
 *
 * \param address 为写入的地址1~65535
 * \param *buff为指向写入的数据
 * \param length为写入的长度
 * \return void
 *
 -----------------------------------------------------------------------------*/
void FM24C64WriteBuffer(INT16U address,INT8U *buff,INT16U length)
{

    I2C_M_SETUP_Type M_Data;
    INT8U wbuf[512];
    M_Data.tx_data = wbuf;
    M_Data.sl_addr7bit = FM24CXX_ADD>>1;

#ifdef      FM24C02
    M_Data.tx_length = length+1;
    M_Data.tx_data[0] = (INT8U)(address&0xff);
    memcpy(M_Data.tx_data+1,buff,length);
#endif
#ifdef      FM24C64
    M_Data.tx_length = length+2;
    M_Data.tx_data[0] = (INT8U)(address>>8);
    M_Data.tx_data[1] = (INT8U)(address&0x00ff);
    memcpy(M_Data.tx_data+2,buff,length);
#endif

    M_Data.rx_length = 0;
    M_Data.rx_data = 0;

    I2C_MasterTransferData(LPC_I2C2,&M_Data, I2C_TRANSFER_POLLING);

}

/*------------------------------------------------------------------------------
 * \brief fm24c64读出一定长度的数据
 *
 * \param address 为读出的地址1~65535
 * \param *buff为指向读出的数据
 * \param length为读出数据长度
 * \return void
 *
 -----------------------------------------------------------------------------*/
void FM24C64ReadBuffer(INT16U address,INT8U *buff,INT16U length)
{

    I2C_M_SETUP_Type M_Data;
    INT32U ret = 0;
    INT16U ad;

    M_Data.tx_data = (INT8U*)(&ad);
    M_Data.sl_addr7bit = FM24CXX_ADD>>1;
#ifdef      FM24C02

    M_Data.tx_data[0] = (INT8U)(address&0xff);
    M_Data.tx_length = 1;
#endif

#ifdef      FM24C64
    M_Data.tx_data[0] = address>>8;
    M_Data.tx_data[1] = (INT8U)(address&0xff);
    M_Data.tx_length = 2;
#endif

    M_Data.rx_length = 0;

    ret = I2C_MasterTransferData(LPC_I2C2,&M_Data, I2C_TRANSFER_POLLING);
    if(ERROR == ret)
    {
        return;
    }

    M_Data.tx_length = 0;
    M_Data.rx_length = length;
    M_Data.rx_data = buff;
    ret = I2C_MasterTransferData(LPC_I2C2,&M_Data, I2C_TRANSFER_POLLING);
}

static OS_Err_T FM24CxxHwInit(OS_Device_T dev)
{
    LPC17xxHwIIcConfig();
    return OS_EOK;
}

static OS_Err_T FM24CxxHwOpen(struct OS_Device *dev, INT16U oflag)
{
    /* get open flags */
    dev->OpenFlag = oflag & 0xff;
    return OS_EOK;
}

static OS_Size_T FM24CxxHwWrite(struct OS_Device *dev,
                                OS_Off_T          pos,
                                const void       *buffer,
                                OS_Size_T         size)
{

    /*设备已经被打开哦*/
    if ((dev->OpenFlag & OS_DEVICE_OFLAG_OPEN ) && (dev->RefCount > 0))
    {
        FM24C64WriteBuffer(pos,(INT8U *)buffer,size);
        return size;
    }
    return OS_NULL;
}

static OS_Size_T FM24CxxHwRead(struct OS_Device *dev,
                               OS_Off_T          pos,
                               void             *buffer,
                               OS_Size_T         size)
{

    if ((dev->OpenFlag & OS_DEVICE_OFLAG_OPEN ) && (dev->RefCount > 0))
    {
        FM24C64ReadBuffer(pos,(INT8U*) buffer,size);
        return size;
    }
    return OS_NULL;
}

/*
 * 注册FM24CXX设备
 */
OS_Err_T FM24CxxInit(void)
{
    struct OS_Device *device;
    device = &FM24CxxDev;

    device->Type = OS_Device_Class_Char;/*设备类型*/
    device->Init = FM24CxxHwInit;
    device->Open = FM24CxxHwOpen;
    device->Write = FM24CxxHwWrite;
    device->Read = FM24CxxHwRead;
    device->Control = OS_NULL;

    OSDeviceRegister(device, "fm24cxx", OS_DEVICE_FLAG_RDWR);
    return OS_EOK;
}
