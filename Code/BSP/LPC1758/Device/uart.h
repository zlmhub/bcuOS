#ifndef __UART_H__
#define __UART_H__

#define  OS_UAOS_RX_BUFFER_SIZE      	(128)
#define  UART_BAUD_RAT					(19200)

void OSHwUartInit(void);
void LPC17xxHwUsartInit(void);
void LPC17xxHwUsartTransmit(INT8U Data);
#endif
