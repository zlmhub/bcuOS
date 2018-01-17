#include <ucos_device.h>

#define OS_ListEntry(Node, Type, Member) \
    ((Type *)((char *)(Node) - (unsigned long)(&((Type *)0)->Member)))
/**
 * This function registers a Device driver with specified name.
 *
 * @param Dev the pointer of Device driver structure
 * @param name the Device driver's name
 * @param flags the flag of Device
 *
 * @return the error code, OS_EOK on initialization successfully.
 */
OS_Err_T OSDeviceRegister(OS_Device_T Dev,
                            const char *Name,
                           INT16U Flags)
{
    if (Dev == OS_NULL)
        return -OS_ERROR;

    if (OSDeviceFind(Name) != OS_NULL)
        return -OS_ERROR;

    OS_ObjectInit(&(Dev->Parent), OS_Object_Class_Device, Name);
    Dev->Flag = Flags;
    Dev->RefCount = 0;

    return OS_EOK;
}

/**
 * This function finds a Device driver by specified name.
 *
 * @param name the Device driver's name
 *
 * @return the registered Device driver on successful, or OS_NULL on failure.
 */
OS_Device_T OSDeviceFind(const char *Name)
{
    struct OS_Object *Object;
    struct OS_ListNode *Node;
    struct OS_ObjectInformation *Information;

    extern struct OS_ObjectInformation OS_ObjectContainer[];

    /* try to find Device object */
    Information = &OS_ObjectContainer[OS_Object_Class_Device];
    for (Node  = Information->ObjectList.Next;
         Node != &(Information->ObjectList);
         Node  = Node->Next)
    {
        Object = OS_ListEntry(Node, struct OS_Object, List);
        if (OS_StrNCmp(Object->Name, Name, OS_NAME_MAX) == 0)
        {

            return (OS_Device_T)Object;
        }
    }
    /* not found */
    return OS_NULL;
}

/**
 * This function will initialize the specified Device
 *
 * @param Dev the pointer of Device driver structure
 *
 * @return the result
 */
OS_Err_T OSDeviceInit(OS_Device_T Dev)
{
    OS_Err_T Result = OS_EOK;


    /* get Device init handler */
    if (Dev->Init != OS_NULL)
    {
        if (!(Dev->Flag & OS_DEVICE_FLAG_ACTIVATED))
        {
            Result = Dev->Init(Dev);
            if (Result != OS_EOK)
            {
                /*"To initialize Device:%s failed. The error code is %d\n"*/
            }
            else
            {
                Dev->Flag |= OS_DEVICE_FLAG_ACTIVATED;
            }
        }
    }

    return Result;
}


/**
 * This function will open a Device
 *
 * @param Dev the pointer of Device driver structure
 * @param oflag the flags for Device open
 *
 * @return the result
 */
OS_Err_T OSDeviceOpen(OS_Device_T Dev, INT16U Oflag)
{
    OS_Err_T Result = OS_EOK;


    /* if Device is not initialized, initialize it. */
    if (!(Dev->Flag & OS_DEVICE_FLAG_ACTIVATED))
    {
        if (Dev->Init != OS_NULL)
        {
            Result = Dev->Init(Dev);
            if (Result != OS_EOK)
            {

                return Result;
            }
        }

        Dev->Flag |= OS_DEVICE_FLAG_ACTIVATED;
    }

    /* Device is a stand alone Device and opened */
    if ((Dev->Flag & OS_DEVICE_FLAG_STANDALONE) &&
        (Dev->OpenFlag & OS_DEVICE_OFLAG_OPEN))
    {
        return -OS_EBUSY;
    }

    /* call Device open interface */
    if (Dev->Open != OS_NULL)
    {
        Result = Dev->Open(Dev, Oflag);
    }

    /* set open flag */
    if (Result == OS_EOK || Result == -OS_ENOSYS)
    {
        Dev->OpenFlag = Oflag | OS_DEVICE_OFLAG_OPEN;

        Dev->RefCount++;
    }

    return Result;
}

/**
 * This function will close a Device
 *
 * @param Dev the pointer of Device driver structure
 *
 * @return the result
 */
OS_Err_T OSDeviceClose(OS_Device_T Dev)
{
    OS_Err_T Result = OS_EOK;


    if (Dev->RefCount == 0)
        return -OS_ERROR;

    Dev->RefCount--;

    if (Dev->RefCount != 0)
        return OS_EOK;

    /* call Device close interface */
    if (Dev->Close != OS_NULL)
    {
        Result = Dev->Close(Dev);
    }

    /* set open flag */
    if (Result == OS_EOK || Result == -OS_ENOSYS)
        Dev->OpenFlag = OS_DEVICE_OFLAG_CLOSE;

    return Result;
}


/**
 * This function will Readsome data from a Device.
 *
 * @param Dev the pointer of Device driver structure
 * @param Pos the Position of reading
 * @param Buffer the data Buffer to save Readdata
 * @param Size the Size of Buffer
 *
 * @return the actually ReadSize on successful, otherwise negative returned.
 *
 * @note since 0.4.0, the unit of Size/Pos is a block for block Device.
 */
OS_Size_T OSDeviceRead(OS_Device_T Dev,
                         OS_Off_T    Pos,
                         void       *Buffer,
                         OS_Size_T   Size)
{


    if (Dev->RefCount == 0)
    {
        return 0;
    }

    /* call Device Readinterface */
    if (Dev->Read!= OS_NULL)
    {
        return Dev->Read(Dev, Pos, Buffer, Size);
    }

    /* set error code */


    return 0;
}


/**
 * This function will Write some data to a Device.
 *
 * @param Dev the pointer of Device driver structure
 * @param Pos the Position of written
 * @param Buffer the data Buffer to be written to Device
 * @param Size the Size of Buffer
 *
 * @return the actually written Size on successful, otherwise negative returned.
 *
 * @note since 0.4.0, the unit of Size/Pos is a block for block Device.
 */
OS_Size_T OSDeviceWrite(OS_Device_T Dev,
                          OS_Off_T    Pos,
                          const void *Buffer,
                          OS_Size_T   Size)
{


    if (Dev->RefCount == 0)
    {
        return 0;
    }

    /* call Device Write interface */
    if (Dev->Write != OS_NULL)
    {
        return Dev->Write(Dev, Pos, Buffer, Size);
    }


    return 0;
}

/**
 * This function will perform a variety of Control functions on Devices.
 *
 * @param Dev the pointer of Device driver structure
 * @param Cmd the command sent to Device
 * @param Arg the Argument of command
 *
 * @return the result
 */
OS_Err_T OSDeviceControl(OS_Device_T Dev, INT8U Cmd, void *Arg)
{
    //RT_ASSERT(Dev != OS_NULL);

    /* call Device Write interface */
    if (Dev->Control != OS_NULL)
    {
        return Dev->Control(Dev, Cmd, Arg);
    }

    return OS_EOK;
}


/**
 * This function will set the reception indication callback function. This callback function
 * is invoked when this Device receives data.
 *
 * @param Dev the pointer of Device driver structure
 * @param RxInd the indication callback function
 *
 * @return OS_EOK
 */
OS_Err_T
OSDeviceSetRxIndicate(OS_Device_T Dev,
                          OS_Err_T (*RxInd)(OS_Device_T Dev, OS_Size_T Size))
{
    //RT_ASSERT(Dev != OS_NULL);

    Dev->RxIndicate = RxInd;

    return OS_EOK;
}
//RTM_EXPORT(rt_Device_set_RxIndicate);

/**
 * This function will set the indication callback function when Device has
 * written data to physical hardware.
 *
 * @param Dev the pointer of Device driver structure
 * @param TxDone the indication callback function
 *
 * @return OS_EOK
 */
OS_Err_T
OSDeviceSetTxComplete(OS_Device_T Dev,
                          OS_Err_T (*TxDone)(OS_Device_T Dev, void *Buffer))
{

    Dev->TxComplete = TxDone;

    return OS_EOK;
}
