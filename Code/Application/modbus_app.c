/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Defines ------------------------------------------*/
#define REG_INPUT_START 	1
#define REG_INPUT_NREGS 	256
#define REG_HOLDING_START 	1
#define REG_HOLDING_NREGS 	256
/* ----------------------- Static variables ---------------------------------*/
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];
static USHORT   usRegHoldingStart = REG_HOLDING_START;
static USHORT   usRegHoldingBuf[REG_HOLDING_NREGS]=
{0x147b,0x3f8e,0x147b,0x400e,0x1eb8,0x4055,0x147b,0x408e};

USHORT  *RegHoldingPoint =usRegHoldingBuf;
USHORT  *RegInputPoint =usRegInputBuf;

/* ----------------------- Defines ------------------------------------------*/
#define REG_COILS_START     1001
#define REG_COILS_SIZE      128

/* ----------------------- Static variables ---------------------------------*/
static unsigned char ucRegCoilsBuf[REG_COILS_SIZE / 8]={0x55,0xA9,0x34};
INT8U *RegCoilsPoint = ucRegCoilsBuf;

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_INPUT_START )
            && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ = ( UCHAR )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ = ( UCHAR )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}


eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;


    if((usAddress >= REG_HOLDING_START)&&\
            ((usAddress+usNRegs) <= (REG_HOLDING_START + REG_HOLDING_NREGS)))
    {
        iRegIndex = (int)(usAddress - usRegHoldingStart);
        switch(eMode)
        {
        case MB_REG_READ:// MB_REG_READ = 0
            while(usNRegs > 0)
            {
                *pucRegBuffer++ = (uint8_t)(usRegHoldingBuf[iRegIndex] >> 8);
                *pucRegBuffer++ = (uint8_t)(usRegHoldingBuf[iRegIndex] & 0xFF);
                iRegIndex++;
                usNRegs--;
            }
            break;
        case MB_REG_WRITE:// MB_REG_WRITE = 0
            while(usNRegs > 0)
            {
                usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}



eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
               eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    short           iNCoils = ( short )usNCoils;
    unsigned short  usBitOffset;

    /* Check if we have registers mapped at this block. */
    if( ( usAddress >= REG_COILS_START ) &&
            ( usAddress + usNCoils <= REG_COILS_START + REG_COILS_SIZE ) )
    {
        usBitOffset = ( unsigned short )( usAddress - REG_COILS_START );
        switch ( eMode )
        {
            /* Read current values and pass to protocol stack. */
        case MB_REG_READ:
            while( iNCoils > 0 )
            {
                *pucRegBuffer++ =
                    xMBUtilGetBits( ucRegCoilsBuf, usBitOffset,
                                    ( unsigned char )( iNCoils >
                                                       8 ? 8 :
                                                       iNCoils ) );
                iNCoils -= 8;
                usBitOffset += 8;
            }
            break;

            /* Update current register values. */
        case MB_REG_WRITE:
            while( iNCoils > 0 )
            {
                xMBUtilSetBits( ucRegCoilsBuf, usBitOffset,
                                ( unsigned char )( iNCoils > 8 ? 8 : iNCoils ),
                                *pucRegBuffer++ );
                iNCoils -= 8;
            }
            break;
        }

    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}


eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    ( void )pucRegBuffer;
    ( void )usAddress;
    ( void )usNDiscrete;
    return MB_ENOREG;
}