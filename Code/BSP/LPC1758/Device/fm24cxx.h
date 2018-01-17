#ifndef __FM24CXX_H_
#define __FM24CXX_H_

#include <ucos_ii.h>
#include <ucos_device.h>

#define FM24C64
#define FM24CXX_ADD	        0xA0

OS_Err_T FM24CxxInit(void);

#endif
