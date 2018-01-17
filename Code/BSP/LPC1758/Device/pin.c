#include "pin.h"

static struct OS_Device	PinDev;

#define ITEM_NUM(items) sizeof(items)/sizeof(items[0])

/* LPC17XX GPIO driver */
struct pin_index
{
    int Index;
    INT32U Gpio;
    INT32U Pin;
};

/*GPIO -- index*/
static const struct pin_index Pins[] =
{
    /*id ,pin, port*/
    {0,1,(1<<18)},
    {1,1,(1<<19)},
    {2,1,(1<<20)},
    {3,1,(1<<22)},
    {4,1,(1<<23)},
};

const struct pin_index *GetPin(INT8U pin)
{
    const struct pin_index *index;

    if (pin < ITEM_NUM(Pins))
    {
        index = &Pins[pin];
        if (index->Index == -1)
            index = OS_NULL;
    }
    else
    {
        index = OS_NULL;
    }
    return index;
};


void LPC17xxPinWrite(OS_Device_T dev, OS_Base_T pin, OS_Base_T value)
{
    const struct pin_index *index;

    index = GetPin(pin);
    if (index == OS_NULL)
    {
        return ;
    }
    if (value == PIN_LOW)
    {
        GPIO_ClearValue(index->Gpio, index->Pin);
    }
    else
    {
        GPIO_SetValue(index->Gpio, index->Pin);
    }
}


int LPC17xxPinRead(OS_Device_T dev, OS_Base_T pin)
{
    const struct pin_index *index;

    index = GetPin(pin);
    if (index == OS_NULL)
    {
        return PIN_NULL;
    }
    if (GPIO_ReadValue( index->Pin))
    {
        return PIN_HIGH;
    }
    else
    {
        return PIN_LOW;
    }
    return PIN_NULL;
}

static OS_Err_T LPC17xxGpioOpen(struct OS_Device *dev, INT16U oflag)
{
    /* get open flags */
    dev->OpenFlag = oflag & 0xff;
    return OS_EOK;
}

static OS_Size_T LPC17xxGpioWrite(struct OS_Device *dev,
                                  OS_Off_T          pos,
                                  const void       *buffer,
                                  OS_Size_T         size)
{
    struct os_device_pin_status *msg;

    /*设备已经被打开哦*/
    if ((dev->OpenFlag & OS_DEVICE_OFLAG_OPEN ) && (dev->RefCount > 0))
    {
        msg = (struct os_device_pin_status*)buffer;
        LPC17xxPinWrite(dev, msg->pin, msg->status);
        return sizeof(msg);
    }
    return OS_NULL;
}

/**
 * read lpc17xx gpio value
 */
static OS_Size_T LPC17xxGpioRead(struct OS_Device *dev,
                                 OS_Off_T          pos,
                                 void             *buffer,
                                 OS_Size_T         size)
{
    struct os_device_pin_status *msg;

    /*设备已经被打开哦*/
    if ((dev->OpenFlag & OS_DEVICE_OFLAG_OPEN ) && (dev->RefCount > 0))
    {
        msg = (struct os_device_pin_status*)buffer;
        msg->status = LPC17xxPinRead(dev, msg->pin);
        return sizeof(msg);
    }
    return OS_NULL;
}


/*
 * lpc17xx gpio init
 */
static OS_Err_T LPC17xxHwGpioInit(OS_Device_T dev)
{
    for(int i=0; i<ITEM_NUM(Pins); i++)
        GPIO_SetDir(Pins[i].Gpio, Pins[i].Pin, 1);
    return OS_EOK;
}


/*
 * hw pin init
 */
OS_Err_T LPC17xxHwPinInit(void)
{
    int result = OS_EOK;
    struct OS_Device *device;
    device = &PinDev;

    device->Type = OS_Device_Class_Miscellaneous;/*设备类型*/
    device->Init = LPC17xxHwGpioInit;
    device->Open = LPC17xxGpioOpen;
    device->Write = LPC17xxGpioWrite;
    device->Read = LPC17xxGpioRead;

    OSDeviceRegister(device, "pin", OS_DEVICE_FLAG_RDWR);
    return result;
}
