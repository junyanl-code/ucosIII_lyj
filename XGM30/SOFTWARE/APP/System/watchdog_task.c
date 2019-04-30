/*****************************************************************************/
/* �ļ���:    watchdog_task.c                                                */
/* ��  ��:    ���Ź�����                                                     */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#include <includes.h>
#include "arch/cc.h"
#include "System/watchdog_task.h"
#include "bsp_spi.h"
#include "led_interrupt.h"

/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
OS_TCB            g_watchdog_task_tcb;                       /* ���Ź�������ƿ� */
static CPU_STK    g_watchdog_task_stk[WATCHDOG_TASK_STACK_SIZE]; /* ���Ź�����ջ     */


/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/



/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
static void watchdog_task(void *pdata);


/*****************************************************************************
 * ��  ��:    watchdog_task_create                                                         
 * ��  ��:    �������Ź�����                                                                 
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/ 
void watchdog_task_create(void)
{
    OS_ERR err;


    /* �������Ź����� */
    OSTaskCreate( (OS_TCB        *)	&g_watchdog_task_tcb,
				  (CPU_CHAR      *)	"watchdog task",
				  (OS_TASK_PTR    )	watchdog_task,
				  (void          *) 0,
				  (OS_PRIO        )	WATCHDOG_TASK_PRIO,
				  (CPU_STK       *)	&g_watchdog_task_stk[0],
                  (CPU_STK       *) &g_watchdog_task_stk[WATCHDOG_TASK_STACK_SIZE / 10],
				  (OS_STK_SIZE    )	WATCHDOG_TASK_STACK_SIZE,
				  (OS_MSG_QTY     ) 0,
				  (OS_TICK        ) 0,
				  (void          *) 0,
				  (OS_OPT         ) (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				  (OS_ERR        *) &err);		

}


/*****************************************************************************
 * ��  ��:    watchdog_task                                                           
 * ��  ��:    ������Ϣ��������                                                               
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void watchdog_task(void *pdata)
{
     //pdata = pdata;
  
    /*initialize the led and interrupt */
    led_gpio_init();
    /*enable hardware watchdog  */
    SPI_FPGA_ByteWrite(0x0, SYSTEM_WDI_ENABLE_REG_ADDR);
    
    while(1)
    {
 
        SPI_FPGA_ByteWrite(0x0, SYSTEM_WDI_REG_ADDR);
        SPI_FPGA_ByteWrite(0x1, SYSTEM_WDI_REG_ADDR);
        BSP_Sleep(500);    
     }

}



