/**
 * 								bms os v1.1
 * zhuangliming
 */

 /*file */
#include <app.h>

/*cpu sr */
#if OS_CRITICAL_METHOD == 3
OS_CPU_SR cpu_sr  = 0;
#endif

OS_MEM	*MEMPoint;
INT8U	MEM[2][1024]@0x20080000;
INT8U	memErr;

/* create system heaps*/
static OS_STK	OSHeap[OS_HEAP_SIZE]@0x2007C000;
static OS_STK *OSHeapSp = OSHeap;

void OSHeapInit(INT32U StartAdd,INT32U EndAdd)
{
	INT32U index = 0;
	for( index = 0; index < OS_HEAP_SIZE; index++ )
	{
		OSHeap[index] = 0x00;
	}
}

void OSAppMEMCreate(void)
{
	MEMPoint = OSMemCreate(MEM,2,1024,&memErr);
}
/**
 * ucos create a app
 * Task: User Thread(App)
 * Par: input partameter
 */
INT8U OSAppCreate(void (*Task)(void *Par),
				  void *Par,
				  INT16U StkSize,
				  INT8U	Prio)
{
	INT8U	Ret;
	static INT8U	HeapInit_ = 0;
	if( 0 == HeapInit_ )
	{
		HeapInit_ = 1;
		OSHeapInit(0,0);
	}
	if( (OSHeapSp+StkSize) > (OSHeap+OS_HEAP_SIZE) )
	{
		return OS_PRIO_EXIST;
	}
	Ret = OSTaskCreate( Task, Par,(OSHeapSp+StkSize), Prio );
	OSHeapSp += StkSize;
	return Ret;
}
