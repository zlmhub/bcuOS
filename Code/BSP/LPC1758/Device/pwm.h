#ifndef __PWM_H_
#define __PWM__H_

#include <board.h>
#define PORT0NUM        26

void LPC17xxHwPWMInit(void);
INT8U GetCPPWMSigner(void);
#endif