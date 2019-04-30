/*****************************************************************************/
/* �ļ���:    roll_print.c                                                    */
/* ��  ��:    ѭ����ӡ                                                       */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#include <includes.h>
#include "arch/cc.h"
#include "lwip/api.h"
#include "Config/config.h"
#include "opt.h"
#include "Roll/roll_print.h"
#include "mtfs30.h"
#include "Debug/mtfs30_debug.h"
#include "Gnss/gnss_handler.h"
#include "Roll/roll_task.h"
#include "Debug/usart_debug_task.h"
#include "Gnss/gnss_task.h"



/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
u8_t                    g_roll_print_switch;   /* ѭ����ӡ���ر���   */


/*-------------------------------*/
/* ȫ�ֱ�������                  */
/*-------------------------------*/


/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
static void roll_print_gnss_info(void);


/*****************************************************************************
 * ��  ��:    roll_print_info                                                         
 * ��  ��:    ѭ����ӡ��Ϣ                                                                 
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/ 
void roll_print_info(void)
{
    
    
    /* ѭ����ӡGNSS��Ϣ */
    roll_print_gnss_info();	

}


/*****************************************************************************
 * ��  ��:    roll_print_gnss_inf                                                           
 * ��  ��:    ѭ����ӡGNSS��Ϣ                                                               
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void roll_print_gnss_info(void)
{ 
    /* ��ӡGNSS��Ϣ */
    gnss_info_print();
}


/*****************************************************************************
 * ��  ��:    roll_print_set_handler                                                           
 * ��  ��:    ��/�ر�ѭ����ӡ                                                                
 * ��  ��:    p_param: ָ��ѭ����ӡ������Ϣ     
 * ��  ��:    ��                                                    
 * ����ֵ:    OK: ���óɹ�; NG������ʧ��                                                   
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                   
 ****************************************************************************/
u8_t roll_print_set_handler(int num,...)
{
    u8_t *p_param = NULL;
    u8_t  err_flag = OK;
    va_list valist;
    
    va_start(valist, num);
    p_param =(u8_t *) va_arg(valist, char *);
    
    if(p_param != NULL)
    {
      if (0 == strncmp((char const *)p_param, "OFF", strlen("OFF")))
      {
          /* �ر�ѭ����ӡ */
          g_roll_print_switch = 0;
      }
      else if (0 == strncmp((char const *)p_param, "ON", strlen("ON")))
      {
          /* ��ѭ����ӡ */
          g_roll_print_switch = 1;
      }
      else
      {
          MTFS30_TIPS("����(%s)����!\n", p_param);  
          err_flag = NG;
      }
    }
    va_end(valist);
    
    return err_flag;
}


/*****************************************************************************
 * ��  ��:    roll_print_task_info                                                           
 * ��  ��:    ѭ����ӡ����ջ��Ϣ                                                                
 * ��  ��:    ��    
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                   
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                   
 ****************************************************************************/
void roll_print_task_info(void)
{
    OS_STK_SIZE free;
    OS_STK_SIZE used;
    OS_ERR err;
    

    OSTaskStkChk(&gtask_usart_debug_tcb, &free, &used, &err);
    MTFS30_TIPS("����������ջ�� ��ʹ��:%d, δʹ��:%d, ʹ����:%d%%", used, free, ((used * 100)/(free + used)));

    
    OSTaskStkChk(&g_gnss_task_tcb, &free, &used, &err);
    MTFS30_TIPS("����������ջ�� ��ʹ��:%d, δʹ��:%d, ʹ����:%d%%", used, free, ((used * 100)/(free + used)));
    
    
    OSTaskStkChk(&g_roll_task_tcb, &free, &used, &err);
    MTFS30_TIPS("��ѭ������ջ�� ��ʹ��:%d, δʹ��:%d, ʹ����:%d%%", used, free, ((used * 100)/(free + used)));
    
}