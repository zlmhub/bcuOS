/*
********************************************************************************
*                             BMS  uC/OS-II
*                         The Real-Time Kernel
*                                             CORE FUNCTIONS
*
*               (c) Copyright 2015-2017, XinYun
*                                           All Rights Reserved
*
* File    : APP.H
* By      : ZhuangLiMing
* Version : V1.06
*
********************************************************************************
*/
#ifndef __APP_H_
#define __APP_H_

#include <ucos_ii.h>
#include <board.h>

#define		OS_HEAP_SIZE		(4*1024UL)

#if OS_CRITICAL_METHOD == 3
extern OS_CPU_SR cpu_sr;
#endif

extern OS_MEM	*MEMPoint;
void OSAppMEMCreate(void);

INT8U OSAppCreate(void (*Task)(void *Par),
				  void *Par,
				  INT16U StkSize,
				  INT8U	Prio);
#endif
/*END FILE*/
