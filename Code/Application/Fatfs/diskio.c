/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "sd.h"

/* Definitions of physical drive number for each drive */

#define SD_CARD   		        0  		/*SD 卡,卷标为 0 */

/*-----------------------------------------------------------------------
 * Get Drive Status
 *-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    return RES_OK;
}

/*-----------------------------------------------------------------------
 * Inidialize a Drive
 * 返回：0初始化成功，非0初始化失败
 *-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    uint8_t res=0;
    switch(pdrv)
    {
    case SD_CARD:
        res=OSHwSDCardInit();    /* SD卡初始化 */
        break;
    default:
        res=1;
    }
    if(res)
    {
        return STA_NOINIT;
    }
    else
    {
        return 0;
    }
}

/*-----------------------------------------------------------------------*
 * Read Sector(s)                                                        *
 *-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,		/* Physical drive nmuber to identify the drive */
    BYTE *buff,		/* Data buffer to store read data */
    DWORD sector,	/* Start sector in LBA */
    UINT count		/* Number of sectors to read */
)
{
    uint8_t res=0;
    if (!count)
    {
        return RES_PARERR;
    }
    switch(pdrv)
    {
    case SD_CARD:
        res = SD_ReadDisk(buff,sector,count);
        while( res )   /* 读出错  */
        {
            SD_Initialize();
            res=SD_ReadDisk(buff,sector,count);
        }
        break;
    default:
        res=1;
    }
    if(res==0x00)
    {
        return RES_OK;
    }
    else
    {
        return RES_ERROR;
    }
}

/*-----------------------------------------------------------------------*
 * Write Sector(s)                                                       *
 *-----------------------------------------------------------------------*/

DRESULT disk_write (
    BYTE pdrv,			/* Physical drive nmuber to identify the drive */
    const BYTE *buff,	/* Data to be written */
    DWORD sector,		/* Start sector in LBA */
    UINT count			/* Number of sectors to write */
)
{
    uint8_t res=0;
    if (!count)
    {
        return RES_PARERR;
    }
    switch(pdrv)
    {
    case SD_CARD:
        res = SD_WriteDisk((uint8_t*)buff,sector,count);
        while(res)    /* 写出错 */
        {
            SD_Initialize();
            res=SD_WriteDisk((uint8_t*)buff,sector,count);
        }
        break;
    default:
        res=1;
    }
    if(res == 0x00)
    {
        return RES_OK;
    }
    else
    {
        return RES_ERROR;
    }
}

/*-----------------------------------------------------------------------*
 * Miscellaneous Functions                                               *
 *-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,		/* Physical drive nmuber (0..) */
    BYTE cmd,		/* Control code */
    void *buff		/* Buffer to send/receive control data */
)
{
    DRESULT res;
    if( pdrv == SD_CARD )
    {
        switch( cmd )
        {
        case CTRL_SYNC:
            SD_CS0();
            if( SD_WaitReady() == 0 )
            {
                res = RES_OK;
            }
            else
            {
                res = RES_ERROR;
            };
            SD_CS1();
            break;
        case GET_SECTOR_SIZE:
            *(DWORD*)buff = 512;
            res = RES_OK;
            break;
        case GET_BLOCK_SIZE:
            *(WORD*)buff = 8;
            res = RES_OK;
            break;
        case GET_SECTOR_COUNT:
            *(DWORD*)buff = SD_GetSectorCount();
            res = RES_OK;
            break;
        default:
            res = RES_PARERR;
            break;
        }
    }
    else
    {
        res = RES_ERROR;	/* 其他的不支持 */
    }

    return res;
}

/*获得时间
  User defined function to give a current time to fatfs module
  31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31)
  15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2)
 */
DWORD get_fattime (void)
{
    return 0;
}


