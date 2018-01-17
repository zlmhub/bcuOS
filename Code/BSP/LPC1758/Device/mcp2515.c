#include "mcp2515.h"
#include <ucos_ii.h>
#include <ucos_device.h>
#include <board.h>
#include "bxcan.h"

struct OS_Device BxCAN3;

#define MCP2515_CS0()  	        GPIO_ClearValue(0, 1U<<16)
#define MCP2515_CS1()           GPIO_SetValue(0, 1<<16)

/********************************************//**
 * \brief �ⲿ�ж�
 *
 * \param void
 * \return void
 *
 ***********************************************/
static void LPC17xxHwEintMCP2515( void )
{
    EXTI_InitTypeDef EXTICfg;
    PINSEL_CFG_Type PINSEL_CFG_Typestruct;
    /**< �ܽ� */
    PINSEL_CFG_Typestruct.Funcnum = PINSEL_FUNC_1;
    PINSEL_CFG_Typestruct.OpenDrain = PINSEL_PINMODE_PULLUP;
    PINSEL_CFG_Typestruct.Pinnum = PINSEL_PIN_10;
    PINSEL_CFG_Typestruct.Portnum = PINSEL_PORT_2;
    PINSEL_CFG_Typestruct.Pinmode = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin( &PINSEL_CFG_Typestruct );

    EXTI_DeInit();

    EXTICfg.EXTI_Line = EXTI_EINT0;
    EXTICfg.EXTI_Mode = EXTI_MODE_LEVEL_SENSITIVE;
    EXTICfg.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
    EXTI_ClearEXTIFlag( EXTI_EINT0 );
    EXTI_Config( &EXTICfg );
    EXTI_Init();
    NVIC_EnableIRQ( EINT0_IRQn );
}

/*------------------------------------------------------------------------------
 * Initialize SPI pin connect
 * P0.15 - SCK;
 * P0.16 - SSEL
 * P0.17 - MISO
 * P0.18 - MOSI
 -----------------------------------------------------------------------------*/
void LPC17xxSPI0Init(INT32U rate)
{
    PINSEL_CFG_Type PinCfg;
    SSP_CFG_Type SSP_CFG_Struct;

    PinCfg.Funcnum = 2;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 3;
    PinCfg.Portnum = 0;

    PinCfg.Pinnum = 15;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 17;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 18;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Funcnum = 0;

    PinCfg.Pinnum = 16;
    PINSEL_ConfigPin(&PinCfg);

    GPIO_SetDir(0, (1 << 16), 1);/*cs*/
    GPIO_SetValue(0, (1 << 16));

    /* initialize SSP configuration */
    SSP_CFG_Struct.CPHA = SSP_CPHA_SECOND;
    SSP_CFG_Struct.CPOL = SSP_CPOL_LO;
    SSP_CFG_Struct.ClockRate = rate;
    SSP_CFG_Struct.Databit = SSP_DATABIT_8;
    SSP_CFG_Struct.Mode = SSP_MASTER_MODE;
    SSP_CFG_Struct.FrameFormat = SSP_FRAME_SPI;
    /* Initialize SSP peripheral with parameter given in structure above */
    SSP_Init(LPC_SSP0, &SSP_CFG_Struct);
    /* Enable SSP peripheral */
    SSP_Cmd(LPC_SSP0, ENABLE);
}

INT8U LPC17xxHwSPI0SendReceive(INT8U data)
{
    SSP_SendData(LPC_SSP0, data);
    while (SSP_GetStatus(LPC_SSP0, SSP_STAT_BUSY) == SET);
    return SSP_ReceiveData(LPC_SSP0);
}

/*------------------------------------------------------------------------------
@\ ������ͨ�������ʱԼnms(��׼ȷ)
@\ ��������
@\ ���أ���
------------------------------------------------------------------------------*/
static void Delay_Nms(INT16U x)
{
    for(INT16U i=0; i<1000; i++)
        for(INT16U j=0; j<x; j++);
}

/*------------------------------------------------------------------------------
@\ ������ͨ��SPI��ȡһ���ֽ�����
@\ ��������
@\ ���أ�rByte(��ȡ����һ���ֽ�����)
------------------------------------------------------------------------------*/
static INT8U SPI_ReadByte(void)
{
    return LPC17xxHwSPI0SendReceive(0x00);
}

/*------------------------------------------------------------------------------
@\ ������SPI����һ���ֽ�����
@\ ������dt:�����͵�����
@\ ���أ���
------------------------------------------------------------------------------*/
static void MCP2515SPISendByte(INT8U dt)
{
    LPC17xxHwSPI0SendReceive(dt);
}

/*------------------------------------------------------------------------------
@\ ������ͨ��SPI��MCP2515ָ����ַ�Ĵ���д1���ֽ�����
@\ ������addr:MCP2515�Ĵ�����ַ,dat:��д�������
@\ ���أ���
------------------------------------------------------------------------------*/
static void MCP2515_WriteByte(INT8U addr,INT8U dat)
{

    SSP_DATA_SETUP_Type dataCfg;
    SSP_TRANSFER_Type xfType;

    INT8U wr[3],rr[3];
    dataCfg.length = 3;
    dataCfg.tx_cnt = 0;
    dataCfg.rx_cnt = 0;
    dataCfg.tx_data = wr;
    dataCfg.rx_data = rr;
    wr[0] = CAN_WRITE;
    wr[1] = addr;
    wr[2] = dat;

    MCP2515_CS0();

    SSP_ReadWrite (LPC_SSP0,&dataCfg,SSP_TRANSFER_POLLING);
    MCP2515_CS1();
}

/*------------------------------------------------------------------------------
@\ ������ͨ��SPI��MCP2515ָ����ַ������1���ֽ�����
@\ ������addr:MCP2515�Ĵ�����ַ
@\ ���أ�rByte:��ȡ���Ĵ�����1���ֽ�����
------------------------------------------------------------------------------*/
static INT8U MCP2515_ReadByte(INT8U addr)
{
    INT8U rByte;
    SSP_DATA_SETUP_Type dataCfg;
    SSP_TRANSFER_Type xfType;

    INT8U wr[3],rr[3];
    dataCfg.length = 3;
    dataCfg.tx_cnt = 0;
    dataCfg.rx_cnt = 0;
    dataCfg.tx_data = wr;
    dataCfg.rx_data = rr;
    wr[0] = CAN_READ;
    wr[1] = addr;
    wr[2] = 0x00;

    MCP2515_CS0();
    SSP_ReadWrite (LPC_SSP0,&dataCfg,SSP_TRANSFER_POLLING);
    MCP2515_CS1();
    return rr[2];

}

/*------------------------------------------------------------------------------
@\ ������MCP2515 ��λ����,����Ӳ����λ���������λ
@\ ��������
@\ ���أ���
------------------------------------------------------------------------------*/
void MCP2515_Reset(void)
{

#if REST_MODE_HAED  > 0
    MCP2515_REST_L();
    Delay_Nms(100);
    MCP2515_REST_H();
    Delay_Nms(100);
#else
    MCP2515_CS0();
    Delay_Nms(10);
    MCP2515SPISendByte(CAN_RESET);
    Delay_Nms(10);
    MCP2515_CS1();
#endif
}

/*------------------------------------------------------------------------------
@\ ������MCP2515 CAN��ʼ��
@\ ��������
@\ ���أ���
------------------------------------------------------------------------------*/
INT8U _MCP2515_Init(void)
{
    static INT8U temp = HAED_ERR;
    LPC17xxSPI0Init(500000);
    MCP2515_Reset();        /*��λ*/

    /*���ò�����Ϊ250Kbps
    set CNF1,SJW=00,����Ϊ1TQ,BRP=49,TQ=[2*(BRP+1)]/Fsoc=2*50/8M=12.5us*/
    MCP2515_WriteByte(CNF1,CAN_250Kbps);
    /*set CNF2,SAM=0,�ڲ���������߽���һ�β�����PHSEG1=(2+1)TQ=3TQ,PRSEG=(0+1)TQ=1TQ*/
    MCP2515_WriteByte(CNF2,0x80|PHSEG1_3TQ|PRSEG_1TQ);
    /*set CNF3,PHSEG2=(2+1)TQ=3TQ,ͬʱ��CANCTRL.CLKEN=1ʱ�趨CLKOUT����Ϊʱ�����ʹ��λ*/
    MCP2515_WriteByte(CNF3,PHSEG2_3TQ);

    MCP2515_WriteByte(TXB0SIDH,0xFF);
    MCP2515_WriteByte(TXB0SIDL,0xEB);
    temp=MCP2515_ReadByte(TXB0SIDL);        /*����  temp=0xebΪ�ɹ�*/
    if(  temp != 0xeb  )
    {
        return 1;
    }

    MCP2515_WriteByte(TXB0EID8,0xFF);
    MCP2515_WriteByte(TXB0EID0,0xFF);

    MCP2515_WriteByte(RXB0SIDH,0x00);
    MCP2515_WriteByte(RXB0SIDL,0x00);
    MCP2515_WriteByte(RXB0EID8,0x00);
    MCP2515_WriteByte(RXB0EID0,0x00);
    MCP2515_WriteByte(RXB0CTRL,0x40);
    MCP2515_WriteByte(RXB0DLC,DLC_8);

    MCP2515_WriteByte(RXF0SIDH,0x00);
    MCP2515_WriteByte(RXF0SIDL,0x00);
    MCP2515_WriteByte(RXF0EID8,0x00);
    MCP2515_WriteByte(RXF0EID0,0x00);

    MCP2515_WriteByte(RXM0SIDH,0x00);
    MCP2515_WriteByte(RXM0SIDL,0x00);
    MCP2515_WriteByte(RXM0EID8,0xFF);
    MCP2515_WriteByte(RXM0EID0,0xFF);

    MCP2515_WriteByte(RXB0CTRL,0x60);/*������*/
    /*���CAN�жϱ�־�Ĵ���������λ(������MCU���)*/
    MCP2515_WriteByte(CANINTF,0x00);
    /*����CAN�ж�ʹ�ܼĴ����Ľ��ջ�����0���ж�ʹ��,����λ��ֹ�ж�*/
    MCP2515_WriteByte(CANINTE,0x01);

    MCP2515_WriteByte(CANCTRL,REQOP_NORMAL|CLKOUT_ENABLED);

    temp=MCP2515_ReadByte(CANSTAT);
    if(OPMODE_NORMAL!=(temp&&0xE0))
    {
        MCP2515_WriteByte(CANCTRL,REQOP_NORMAL|CLKOUT_ENABLED);
    }
    return 0;
}

/*
 * ��ȡmcp2515��ϵͳ״̬
 */
INT8U mcp2515_get_statue( void )
{
    return 0;
}

/*------------------------------------------------------------------------------
@\ ������MCP2515 CAN����һ������
@\ ������MCP_CanMsg ���͵���Ϣָ�룬����stm32��Ϣ����
@\ ���أ�0
------------------------------------------------------------------------------*/
INT8U CAN_MCP2515_Transimt(INT32U ID,
                           INT8U len,
                           INT8U *data)
{

    INT8U j,dly,count;
    INT8U id[4]= {0,0,0,0};

    id[0] = ID&0x000000FF;
    id[1] = (ID>>8)&0x000000FF;
    id[2] = ((ID>>16)&0x03)+(((ID>>18)&0x07)<<5);
    id[2] =  id[2] +0x08;
    id[3] = ID>>21;
    count=0;

    while(count<len)
    {
        dly=0;
        while((MCP2515_ReadByte(TXB0CTRL)&0x08) && (dly<50))   /*���ٶ�ĳЩ״ָ̬��,�ȴ�TXREQ��־����*/
        {
            dly++;
        }

        for(j=0; j<8;)
        {
            MCP2515_WriteByte(TXB0D0+j,data[count++]);/*�������͵�����д�뷢�ͻ���Ĵ���*/
            j++;
            if(count>=len) break;
        }
        MCP2515_WriteByte(TXB0DLC,j);
        MCP2515_WriteByte(TXB0SIDH,id[3]);
        MCP2515_WriteByte(TXB0SIDL,id[2]);
        MCP2515_WriteByte(TXB0EID8,id[1]);
        MCP2515_WriteByte(TXB0EID0,id[0] );
        MCP2515_CS0();
        MCP2515_WriteByte(TXB0CTRL,0x08);/*�����ͱ���*/
        MCP2515_CS1();
    }

    LPC17xxHwEintMCP2515();
    return 0;
}



static OS_Err_T MCP2515HwInit(OS_Device_T dev)
{
    struct OS_Device *device;
    device = dev;

    _MCP2515_Init();
    return OS_EOK;
}

static OS_Err_T MCP2515HwOpen(struct OS_Device *dev, INT16U oflag)
{

    /* get open flags */
    dev->OpenFlag = oflag & 0xff;
    return OS_EOK;
}

static OS_Size_T MCP2515HwWrite(struct OS_Device *dev,
                                OS_Off_T          pos,
                                const void       *buffer,
                                OS_Size_T         size)
{
    struct os_bxcan_msg *msg;


    /*�豸�Ѿ�����Ŷ*/
    if ((dev->OpenFlag & OS_DEVICE_OFLAG_OPEN ) && (dev->RefCount > 0))
    {
        msg = (struct os_bxcan_msg*)buffer;
        INT8U ret = 0;
        CAN_MCP2515_Transimt(msg->Id,
                             msg->Len,
                             msg->Data);
        return sizeof(msg);
    }
    return OS_NULL;
}


int MCP2515Init(void)
{
    struct OS_Device *device;
    device = &BxCAN3;

    device->Type  = OS_Device_Class_CAN;/*�豸����*/
    device->Init = MCP2515HwInit;
    device->Open = MCP2515HwOpen;
    device->Write = MCP2515HwWrite;
    device->Read = 0;
    OSDeviceRegister(&BxCAN3, "BxCAN3", OS_DEVICE_FLAG_RDWR);

    return 0;
}


/********************************************//**
 * \brief MCP2515 CAN����һ������
 *
 * \param msg MCP_CanMsg* MCP_CanMsg ���͵���Ϣָ�룬����stm32��Ϣ����
 * \return INT8U �������ݳ���
 *
 ***********************************************/
typedef union
{
    INT8U c[4];
    INT32U f;
} INT32to8Type;
INT8U CAN_MCP2515_Receive(struct os_bxcan_msg *msg)
{
    INT8U i=0,len=0,temp=0,ext[4];
    INT32to8Type extid;
    temp = MCP2515_ReadByte(CANINTF);

    if(temp & 0x01)
    {
        /**< ��ȡ���ջ�����0���յ������ݳ���(0~8���ֽ�) */
        len=MCP2515_ReadByte(RXB0DLC);
        extid.c[0]= MCP2515_ReadByte(RXB0EID0);
        extid.c[1] = MCP2515_ReadByte(RXB0EID8);
        extid.c[2] = MCP2515_ReadByte(RXB0SIDL);
        extid.c[3] = MCP2515_ReadByte(RXB0SIDH);

        //msg->RTR = (extid.c[2] >>5)&0x01;
        extid.c[2] = (extid.c[2]&0x03)+((extid.c[2]>>3)&0x1C)+(extid.c[3]<<5);
        extid.c[3] = extid.c[3]>>3;

        msg->Id = extid.f;
        msg->Len = len;
        while(i<len)
        {
            msg->Data[i]=MCP2515_ReadByte(RXB0D0+i);
            i++;
        }
    }
    /**< ����жϱ�־λ(�жϱ�־�Ĵ���������MCU����) */
    MCP2515_WriteByte(CANINTF,0);
    return len;
}

/********************************************//**
 * \brief MCP2515�жϱ���
 *
 * \param void
 * \return void
 *
 ***********************************************/
void EINT0_IRQHandler( void )
{
    struct os_bxcan_msg msg;
    INT8U Len=0;
    EXTI_ClearEXTIFlag( EXTI_EINT0 );
    Len = CAN_MCP2515_Receive(&msg);
    if((BxCAN3.RxIndicate != OS_NULL )&&(Len>0))
    {
        BxCAN3.UserData = (void *)(&msg);
        BxCAN3.RxIndicate(&BxCAN3,OS_NULL);
    }
}