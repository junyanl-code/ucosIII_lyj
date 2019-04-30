/*****************************************************************************/
/* �ļ���:    usart_debug_task.c                                             */
/* ��  ��:    ���ڵ�������                                                   */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#include <includes.h>
#include "arch/cc.h"
#include "lwip/api.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "opt.h"
#include "Debug/usart_debug_task.h"
//#include "Cmd/user_cmd.h"
#include "Debug/mtfs30_debug.h"






OS_TCB     gtask_usart_debug_tcb;                                      /* ����������ƿ� */
static CPU_STK    gtask_usart_debug_stk[USART_DEBUG_TASK_STACK_SIZE];  /* ��������ջ     */



/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
extern u8      g_debug_usart_rx_buf[]; /* ���Դ��ڽ��ջ����� */
extern OS_SEM  g_usart_debug_sem;      /* ���ڵ������ź���   */


/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
static void usart_debug_task(void *pdata);


/*****************************************************************************
 * ��  ��:    usart_debug_task                                                           
 * ��  ��:    �������ڵ�������                                                                 
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/ 
void usart_debug_task_create(void)
{
    OS_ERR err;


    /* ����������Ϣ��ѯ���� */
    OSTaskCreate( (OS_TCB        *)	&gtask_usart_debug_tcb,
				  (CPU_CHAR      *)	"usart debug task",
				  (OS_TASK_PTR    )	usart_debug_task,
				  (void          *) 0,
				  (OS_PRIO        )	USART_DEBUG_THREAD_PRIO,
				  (CPU_STK       *)	&gtask_usart_debug_stk[0],
                  (CPU_STK       *) &gtask_usart_debug_stk[USART_DEBUG_TASK_STACK_SIZE / 10],
				  (OS_STK_SIZE    )	USART_DEBUG_TASK_STACK_SIZE,
				  (OS_MSG_QTY     ) 0,
				  (OS_TICK        ) 0,
				  (void          *) 0,
				  (OS_OPT         ) (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				  (OS_ERR        *) &err);		

}


/*****************************************************************************
 * ��  ��:    usart_debug_task                                                          
 * ��  ��:    ���ڵ�������                                                               
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void usart_debug_task(void *pdata)
{
    OS_ERR err;
    //pdata = pdata;
  
    while(1)
    {
    
       /* block the serial if not receive command (serial) */
       OSSemPend ((OS_SEM *) &g_usart_debug_sem,
                  (OS_TICK ) 0,
                  (OS_OPT  ) OS_OPT_PEND_BLOCKING,
                  (CPU_TS *) NULL,
                  (OS_ERR *) &err);
       
       
       
       /* parser the command (serial) */
       //MTFS30_DEBUG("#######g_debug_usart_rx_buf: %s\n", g_debug_usart_rx_buf);
       user_cmd_parser((u8_t *)g_debug_usart_rx_buf);
        
    }
   
}