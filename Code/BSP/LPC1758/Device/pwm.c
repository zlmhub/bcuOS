#include "pwm.h"
uint32_t ccpcont=0;
static char Risflag=0;
uint32_t CcpLowCont=0;
uint32_t CcpHighCont=0;
uint32_t CcpHighAndLowCont=0;
uint32_t DutyTrtion=0;

void LPC17xxHwPWMInit(void)
{
    LPC_GPIOINT->IO0IntEnR=1<<PORT0NUM;
    NVIC_SetPriorityGrouping(4);
    NVIC_SetPriority(EINT3_IRQn,4);
    NVIC_EnableIRQ(EINT3_IRQn);

    TIM_TIMERCFG_Type  TIM_ConfigStruct;
    TIM_MATCHCFG_Type    TIM_MatchConfigStruct;
    TIM_ConfigStruct.PrescaleOption=TIM_PRESCALE_USVAL;
    TIM_ConfigStruct.PrescaleValue=1;
    TIM_MatchConfigStruct.MatchChannel=3;
    TIM_MatchConfigStruct.IntOnMatch=TRUE;
    TIM_MatchConfigStruct.ResetOnMatch=TRUE;
    TIM_MatchConfigStruct.StopOnMatch=FALSE;
    TIM_MatchConfigStruct.ExtMatchOutputType=TIM_EXTMATCH_TOGGLE;
    TIM_MatchConfigStruct.MatchValue=20;/*20us inttruppert*/
    TIM_Init(LPC_TIM3,TIM_TIMER_MODE,&TIM_ConfigStruct);
    TIM_ConfigMatch(LPC_TIM3,&TIM_MatchConfigStruct);
    NVIC_SetPriority(TIMER3_IRQn,1);
    NVIC_EnableIRQ(TIMER3_IRQn);
    TIM_Cmd(LPC_TIM3,ENABLE);
}

/*
*/
void TIMER3_IRQHandler(void)
{
    if(TIM_GetIntStatus(LPC_TIM3,TIM_MR3_INT)) {
        TIM_ClearIntPending(LPC_TIM3,TIM_MR3_INT);
        if(Risflag==1)
            ccpcont++;
    }
}

/*
*/
void EINT3_IRQHandler(void)
{
    static uint32_t riscont=0;
    static uint32_t fallcont=0;
    if(LPC_GPIOINT->IO0IntStatF==(1<<PORT0NUM)) {
        LPC_GPIOINT->IO0IntEnF=0x000000000;
        LPC_GPIOINT->IO0IntEnR=(1<<PORT0NUM);
        LPC_GPIOINT->IO0IntClr|=(1<<PORT0NUM);
        fallcont=ccpcont;
        if(fallcont<riscont) {
            CcpHighCont=fallcont+0xfffffffff-riscont;
        } else {
            CcpHighCont=fallcont-riscont;
        }
        CcpHighAndLowCont=CcpHighCont+CcpLowCont;

        DutyTrtion=(CcpHighCont*100)/CcpHighAndLowCont;
    }
    if(LPC_GPIOINT->IO0IntStatR==(1<<PORT0NUM)) {
        Risflag=1;
        LPC_GPIOINT->IO0IntClr|=(1<<PORT0NUM);
        LPC_GPIOINT->IO0IntEnF=(1<<PORT0NUM);
        LPC_GPIOINT->IO0IntEnR=0x000000000;
        riscont=ccpcont;
        if(riscont<fallcont) {
            CcpLowCont=riscont+0xfffffffff-fallcont;
        } else {
            CcpLowCont=riscont-fallcont;
        }
    }
}

INT8U GetCPPWMSigner(void)
{
    INT8U PwmVol = DutyTrtion;
    DutyTrtion= 0 ;
    return (100-PwmVol);
}