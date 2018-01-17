#ifndef __UCOS_DEVICE_H_
#define __UCOS_DEVICE_H_

#include 	<UCOS_II.H>

#define		OS_NAME_MAX		              12

/* 32bit CPU */
typedef INT32S                          OS_Base_T;      /**< Nbit CPU related date type */
typedef INT32U                  		OS_Ubase_T;     /**< Nbit unsigned CPU related data type */

typedef OS_Base_T                       OS_Err_T;       /**< Type for error number */
typedef OS_Ubase_T                      OS_Size_T;      /**< Type for size number */
typedef OS_Base_T                       OS_Off_T;       /**< Type for offset */

/* OS-Thread error code definitions */
#define OS_EOK                          0               /**< There is no error */
#define OS_ERROR                        1               /**< A generic error happens */
#define OS_ETIMEOUT                     2               /**< Timed out */
#define OS_EFULL                        3               /**< The resource is full */
#define OS_EEMPTY                       4               /**< The resource is empty */
#define OS_ENOMEM                       5               /**< No memory */
#define OS_ENOSYS                       6               /**< No system */
#define OS_EBUSY                        7               /**< Busy */
#define OS_EIO                          8               /**< IO error */

/**
 * @ingroup BasicDef
 *
 * @def OS_NULL
 * Similar as the \c NULL in C library.
 */
#define OS_NULL                         (0)

struct OS_ListNode
{
    struct OS_ListNode *Next;                          /**< point to next node. */
    struct OS_ListNode *Prev;                          /**< point to prev node. */
};
typedef struct OS_ListNode OS_List_T;                  /**< Type for lists. */

/**
 * Base structure of Kernel object
 */
struct OS_Object
{
    char       Name[OS_NAME_MAX];                       /**< name of kernel object */
    INT8U Type;                                    /**< type of kernel object */
    INT8U Flag;                                    /**< flag of kernel object */

#ifdef OS_USING_MODULE
    void      *ModuleId;                               /**< id of application module */
#endif
    OS_List_T  List;                                    /**< list node of kernel object */
};
typedef struct OS_Object *OS_Object_T;                  /**< Type for kernel objects. */

/*@{*/

/**
 * device (I/O) class type
 */
enum OS_DeviceClassType
{
    OS_Device_Class_Char = 0,                           /**< character device */
    OS_Device_Class_Block,                              /**< block device */
    OS_Device_Class_NetIf,                              /**< net interface */
    OS_Device_Class_MTD,                                /**< memory device */
    OS_Device_Class_CAN,                                /**< CAN device */
    OS_Device_Class_OSC,                                /**< OSC device */
    OS_Device_Class_Sound,                              /**< Sound device */
    OS_Device_Class_Graphic,                            /**< Graphic device */
    OS_Device_Class_I2CBUS,                             /**< I2C bus device */
    OS_Device_Class_USBDevice,                          /**< USB slave device */
    OS_Device_Class_USBHost,                            /**< USB host bus */
    OS_Device_Class_SPIBUS,                             /**< SPI bus device */
    OS_Device_Class_SPIDevice,                          /**< SPI device */
    OS_Device_Class_SDIO,                               /**< SDIO bus device */
    OS_Device_Class_PM,                                 /**< PM pseudo device */
    OS_Device_Class_Pipe,                               /**< Pipe device */
    OS_Device_Class_Portal,                             /**< Portal device */
    OS_Device_Class_Miscellaneous,                      /**< Miscellaneous device */
    OS_Device_Class_RTC,
    OS_Device_Class_Unknown                             /**< unknown device */
};

enum OS_ObjectClassType
{
  	OS_Object_Class_Device = 0,
    OS_Object_Class_Unknown,                            /**< The object is unknown. */
    OS_Object_Class_Static = 0x80                       /**< The object is a static object. */
};


/**
 * The information of the kernel object
 */
struct OS_ObjectInformation
{
    enum OS_ObjectClassType Type;                     /**< object class type */
    OS_List_T                 ObjectList;              /**< object list */
    OS_Size_T                 ObjectSize;              /**< object size */
};


/**
 * device flags defitions
 */
#define OS_DEVICE_FLAG_DEACTIVATE       0x000           /**< device is not not initialized */

#define OS_DEVICE_FLAG_RDONLY           0x001           /**< read only */
#define OS_DEVICE_FLAG_WRONLY           0x002           /**< write only */
#define OS_DEVICE_FLAG_RDWR             0x003           /**< read and write */

#define OS_DEVICE_FLAG_REMOVABLE        0x004           /**< removable device */
#define OS_DEVICE_FLAG_STANDALONE       0x008           /**< standalone device */
#define OS_DEVICE_FLAG_ACTIVATED        0x010           /**< device is activated */
#define OS_DEVICE_FLAG_SUSPENDED        0x020           /**< device is suspended */
#define OS_DEVICE_FLAG_STREAM           0x040           /**< stream mode */

#define OS_DEVICE_FLAG_INT_RX           0x100           /**< INT mode on Rx */
#define OS_DEVICE_FLAG_DMA_RX           0x200           /**< DMA mode on Rx */
#define OS_DEVICE_FLAG_INT_TX           0x400           /**< INT mode on Tx */
#define OS_DEVICE_FLAG_DMA_TX           0x800           /**< DMA mode on Tx */

#define OS_DEVICE_OFLAG_CLOSE           0x000           /**< device is closed */
#define OS_DEVICE_OFLAG_RDONLY          0x001           /**< read only access */
#define OS_DEVICE_OFLAG_WRONLY          0x002           /**< write only access */
#define OS_DEVICE_OFLAG_RDWR            0x003           /**< read and write */
#define OS_DEVICE_OFLAG_OPEN            0x008           /**< device is opened */

/**
 * general device commands
 */
#define OS_DEVICE_CTRL_RESUME           0x01            /**< resume device */
#define OS_DEVICE_CTRL_SUSPEND          0x02            /**< suspend device */

/**
 * special device commands
 */
#define OS_DEVICE_CTRL_CHAR_STREAM      0x10            /**< stream mode on char device */
#define OS_DEVICE_CTRL_BLK_GETGEOME     0x10            /**< get geometry information   */
#define OS_DEVICE_CTRL_BLK_SYNC         0x11            /**< flush data to block device */
#define OS_DEVICE_CTRL_BLK_ERASE        0x12            /**< erase block on block device */
#define OS_DEVICE_CTRL_NETIF_GETMAC     0x10            /**< get mac address */
#define OS_DEVICE_CTRL_MTD_FORMAT       0x10            /**< format a MTD device */
#define OS_DEVICE_CTRL_OSC_GET_TIME     0x10            /**< get time */
#define OS_DEVICE_CTRL_OSC_SET_TIME     0x11            /**< set time */
#define OS_DEVICE_CTRL_OSC_GET_ALARM    0x12            /**< get alarm */
#define OS_DEVICE_CTRL_OSC_SET_ALARM    0x13            /**< set alarm */

typedef struct OS_Device *OS_Device_T;
/**
 * Device structure
 */
struct OS_Device
{
    struct OS_Object          Parent;                   /**< inherit from rt_object */

    enum OS_DeviceClassType Type;                     /**< device type */
    INT16U               Flag;                     /**< device flag */
    INT16U               OpenFlag;                /**< device open flag */

    INT8U                RefCount;                /**< reference count */
    INT8U                DeviceId;                /**< 0 - 255 */

    /* device call back */
    OS_Err_T (*RxIndicate)(OS_Device_T Dev, OS_Size_T Size);
    OS_Err_T (*TxComplete)(OS_Device_T Dev, void *Buffer);

    /* common device interface */
    OS_Err_T  (*Init)   (OS_Device_T Dev);
    OS_Err_T  (*Open)   (OS_Device_T Dev, INT16U Oflag);
    OS_Err_T  (*Close)  (OS_Device_T Dev);
    OS_Size_T (*Read)   (OS_Device_T Dev, OS_Off_T Pos, void *Buffer, OS_Size_T size);
    OS_Size_T (*Write)  (OS_Device_T Dev, OS_Off_T Pos, const void *Buffer, OS_Size_T size);
    OS_Err_T  (*Control)(OS_Device_T Dev, INT8U Cmd, void *Args);

    void                     *UserData;                /**< device private data */
};


OS_Device_T OSDeviceFind(const char *Name);
OS_Err_T OSDeviceRegister(OS_Device_T Dev,const char *Name,INT16U Flags);
OS_Err_T OSDeviceInit(OS_Device_T Dev);
OS_Err_T OSDeviceOpen(OS_Device_T Dev, INT16U Oflag);
OS_Err_T OSDeviceClose(OS_Device_T Dev);
OS_Size_T OSDeviceRead(OS_Device_T Dev,
                         OS_Off_T    Pos,
                         void       *Buffer,
                         OS_Size_T   Size);
OS_Size_T OSDeviceWrite(OS_Device_T Dev,
                          OS_Off_T    Pos,
                          const void *Buffer,
                          OS_Size_T   Size);
OS_Err_T OSDeviceControl(OS_Device_T Dev, INT8U Cmd, void *Arg);
OS_Err_T
OSDeviceSetRxIndicate(OS_Device_T Dev,
                          OS_Err_T (*RxInd)(OS_Device_T Dev, OS_Size_T Size));
OS_Err_T
OSDeviceSetTxComplete(OS_Device_T Dev,
                          OS_Err_T (*TxDone)(OS_Device_T Dev, void *Buffer));

#endif
