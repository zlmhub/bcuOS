#include <board.h>
#include "gpio.h"
// * 							- 0: Rising edge
// * 							- 1: Falling edge
void LPC17xxPWMCaptureInit(void)
{
    GPIO_SetDir(0, (1<<26), 0);
    GPIO_IntCmd(0, (1<<26), 0);
}