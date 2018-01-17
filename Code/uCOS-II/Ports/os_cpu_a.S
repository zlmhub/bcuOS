
        IMPORT OSRunning

        IMPORT OSPrioCur
        IMPORT OSPrioHighRdy

        IMPORT OSTCBCur
        IMPORT OSTCBHighRdy

        IMPORT OSIntNesting

        IMPORT OSTaskSwHook

        IMPORT OSRdyGrp
        IMPORT OSRdyTbl
        IMPORT OSPrioHighRdy

        EXPORT  OS_CPU_SR_Save
        EXPORT  OS_CPU_SR_Restore
        EXPORT  OSStartHighRdy
        EXPORT  OSCtxSw
        EXPORT  OSIntCtxSw
        EXPORT  PendSV_Handler
        EXPORT  OS_ISR_Enable
        EXPORT  OS_ISR_Disable


NVIC_INT_CTRL   	EQU     0xE000ED04                              ; Interrupt control state register
NVIC_SYSPRI14    	EQU     0xE000ED22                              ; System priority register (yan).
NVIC_PENDSV_PRI 	EQU           0xFF                              ; PendSV priority value (LOWEST yan).
NVIC_PENDSVSET  	EQU     0x10000000                              ; Value to trigger PendSV exception

	SECTION	.text:CODE(2)
	THUMB
	REQUIRE8
	PRESERVE8

OS_ISR_Enable:
        CPSIE   I
        BX      LR

OS_ISR_Disable:
        CPSID   I
        BX      LR

OS_CPU_SR_Save:
        MRS     R0, PRIMASK
        CPSID   I
        BX      LR

OS_CPU_SR_Restore:
        MSR     PRIMASK, R0
        BX      LR


OSStartHighRdy:
        LDR     R0, =NVIC_SYSPRI14
        LDR     R1, =NVIC_PENDSV_PRI
        STRB    R1, [R0]

        MOVS    R0, #0
        MSR     PSP, R0

        LDR     R0, =OSRunning
        MOVS    R1, #1
        STRB    R1, [R0]

        LDR     R0, =NVIC_INT_CTRL
        LDR     R1, =NVIC_PENDSVSET
        STR     R1, [R0]

        CPSIE   I

OSStartHang:
        B       OSStartHang


OSCtxSw:
        LDR     R0, =NVIC_INT_CTRL
        LDR     R1, =NVIC_PENDSVSET
        STR     R1, [R0]
        BX      LR


OSIntCtxSw:
        LDR     R0, =NVIC_INT_CTRL
        LDR     R1, =NVIC_PENDSVSET
        STR     R1, [R0]
        BX      LR


PendSV_Handler:
        CPSID   I
        MRS     R0, PSP
        CBZ     R0, PendSV_Handler_nosave

        SUBS    R0, R0, #0x20
        STM     R0, {R4-R11}

        LDR     R1, =OSTCBCur
        LDR     R1, [R1]
        STR     R0, [R1]


PendSV_Handler_nosave:
        LDR     R0, =OSPrioCur
        LDR     R1, =OSPrioHighRdy
        LDRB    R2, [R1]
        STRB    R2, [R0]

        LDR     R0, =OSTCBCur
        LDR     R1, =OSTCBHighRdy
        LDR     R2, [R1]
        STR     R2, [R0]

        LDR     R0, [R2]
        LDM     R0, {R4-R11}
        ADDS    R0, R0, #0x20
        MSR     PSP, R0

        ORR     LR, LR, #0x04

        CPSIE   I
        BX      LR

        END
