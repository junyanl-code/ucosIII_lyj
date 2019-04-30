/*****************************************************************************/
/* �ļ���:    gnss_task.c                                                    */
/* ��  ��:    ������Ϣ��������                                               */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#include <includes.h>
#include "arch/cc.h"
#include "lwip/api.h"
#include "Config/config.h"
#include "opt.h"
#include "Gnss/gnss_task.h"
#include "Gnss/gnss_receiver.h"
#include "Gnss/gnss_handler.h"
#include "Debug/mtfs30_debug.h"
#include "Util/util_ringbuffer.h"
#include "BSP/inc/bsp_usart.h"



/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
OS_TCB            g_gnss_task_tcb;                       /* ������Ϣ��ѯ������ƿ� */
static CPU_STK    g_gnss_task_stk[GNSS_TASK_STACK_SIZE]; /* ������Ϣ��ѯ����ջ     */

/*-------------------------------*/
/* ȫ�ֱ�������                  */
/*-------------------------------*/
extern ringbuffer_t g_gnss_usart_rb;           /* ������Ϣ������     */
extern u8_t         g_gnss_init_finished_flag; /* ���ǳ�ʼ����ɱ�־ */
extern OS_SEM       g_gnss_idle_sem;           /* GNSS�����ж��ź��� */
extern gnss_info_t  g_gnss_info;               /* GNSS��Ϣ           */
extern u8_t         g_gnss_cmd_send_flag;      /* ��������ͱ�־   */
extern OS_Q         g_gnss_msgq;               /* ������Ϣ��Ϣ����   */
extern u8_t         g_gnss_echo_switch;        /* ���������Կ���   */
/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
static void gnss_task(void *pdata);


/*****************************************************************************
 * ��  ��:    gnss_task_create                                                         
 * ��  ��:    �������ǲ�ѯ����                                                                 
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/ 
void gnss_task_create(void)
{
    OS_ERR err;


    /* ����������Ϣ��ѯ���� */
    OSTaskCreate( (OS_TCB        *)	&g_gnss_task_tcb,
				  (CPU_CHAR      *)	"gnss task",
				  (OS_TASK_PTR    )	gnss_task,
				  (void          *) 0,
				  (OS_PRIO        )	GNSS_TASK_PRIO,
				  (CPU_STK       *)	&g_gnss_task_stk[0],
                  (CPU_STK       *) &g_gnss_task_stk[GNSS_TASK_STACK_SIZE / 10],
				  (OS_STK_SIZE    )	GNSS_TASK_STACK_SIZE,
				  (OS_MSG_QTY     ) 0,
				  (OS_TICK        ) 0,
				  (void          *) 0,
				  (OS_OPT         ) (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				  (OS_ERR        *) &err);		

}


/*****************************************************************************
 * ��  ��:    gnss_task                                                           
 * ��  ��:    ������Ϣ��������                                                               
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void gnss_task(void *pdata)
{

   u8_t *pmsg = NULL;
   OS_ERR err;
   OS_MSG_SIZE size;
	
   //pdata = pdata;
    
	while(1)
	{
        /* ����Ϣ������ȡ�����������Ϣ */
        pmsg = OSQPend((OS_Q*        )&g_gnss_msgq,   
                       (OS_TICK      )0,
                       (OS_OPT       )OS_OPT_PEND_BLOCKING,
                       (OS_MSG_SIZE* )&size, 
                       (CPU_TS*      )0,
                       (OS_ERR*      )&err);
		
        
        if ((err == OS_ERR_NONE) && (pmsg != NULL))
	{
            
            /* ���������� */
            if (1 == g_gnss_echo_switch)
            {
                //MTFS30_TIPS(pmsg);
                gnss_stmt_echo(pmsg);
            }
			
            /* ���������� */
			if(1 == g_gnss_init_finished_flag)
			{	
				gnss_receiver_msg_parser(pmsg);
			}
		} 
         

	}    
    


}

//static void gnss_task(void *pdata)
//{
//
//    u16_t read_size = 0;                        /* �ӻ��γ������������ݴ�С        */
//    u16_t can_read_size = 0;                    /* ���λ������ɶ������ݴ�С        */
//    u8_t read_buf[RINGBUFFER_READ_MAX_SIZE + 1];/* ��Ŵӻ��λ���������������      */  
//	
//    
//    pdata = pdata;
//    
//    /*-------------------------------------*/
//    /* ����������Ϣ                        */
//    /*-------------------------------------*/
//	while(1)
//	{
// 
//         
//        can_read_size = util_ringbuffer_can_read(&g_gnss_usart_rb);
//        if (can_read_size == 0)
//        {
//            /* ����û�����������ģ�����Ϊ��������������ģ��Ļظ���Ϣ,����˯��ʱ�� */
//            if (g_gnss_cmd_send_flag == 1)
//            {
//                BSP_Sleep(10);
//                continue;
//            } 
//            
//            BSP_Sleep(100);
//            continue;
//        }
//
//
//        /* �ȴ�10ms����ֹ���ݻ�ûд�� */
//        BSP_Sleep(10);
//       
//        /*  
//         * ȡ��ÿ������(ÿ��GNSS�����"\n"����)
//         */        
//        while((read_size = util_ringbuffer_end_ch_read(&g_gnss_usart_rb, read_buf, RINGBUFFER_READ_MAX_SIZE, '\n')) > 0)
//        {
//            
//            read_buf[read_size] = '\0';
//            MTFS30_DEBUG("���ݳ��� = %d, ����: %s", read_size, read_buf);
//
//            /* ����������Ϣ */           
//            gnss_receiver_msg_parser(read_buf);
//
//        }
//        
//        /* ����û�����������ģ�����Ϊ��������������ģ��Ļظ���Ϣ,����˯�� */
//        if (g_gnss_cmd_send_flag == 1)
//        {
//            continue;
//        } 
//
//        BSP_Sleep(1000);
//        
//	}    
//    
//
//
//}



