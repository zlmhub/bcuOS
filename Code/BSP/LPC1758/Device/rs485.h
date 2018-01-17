#ifndef __RS485_H_
#define __RS485_H_


void LPC17xxHwRS485Init( INT16U Baund );

void LPC17xxHwRS485TxEnable( INT8U cmd);
void LPC17xxHwRS485SendData(INT8U *buf,INT8U len);
void LPC17xxHwRS485TransmitByte(INT8U Data);
#endif