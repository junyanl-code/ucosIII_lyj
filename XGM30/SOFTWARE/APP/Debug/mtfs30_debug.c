/*****************************************************************************/
/* �ļ���:    debug.c                                                        */
/* ��  ��:    ���Դ���                                                       */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#include "mtfs30.h"
#include "Debug/mtfs30_debug.h"
#include "Debug/usart_debug_task.h"
#include <includes.h>
#include "arch/cc.h"
#include "bsp_usart.h"

/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
u8_t                    g_debug_switch;        /* ���ڵ��Կ��ر���   */
u8_t                    g_debug_level = DBG_DISABLE;

/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/


/*****************************************************************************/
/* ��  ��:    debug_task_create                                              */
/* ��  ��:    ������������                                                   */
/* ��  ��:    ��                                                             */
/* ��  ��:    ��                                                             */
/* ����ֵ:    ��                                                             */
/* ��  ��:    2018-05-12 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/*****************************************************************************/
void debug_task_create( void )
{

    
    /* �������ڵ������� */
    usart_debug_task_create();  
}


/*****************************************************************************
 * ��  ��:    debug_msg_format                                                           
 * ��  ��:    ��������Ϣ��ʽ���������                                                                 
 * ��  ��:    ��                       
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void debug_msg_format(u8_t *format, ...)
{
        u16_t len = 0;
        u8_t buf[DEBUG_MSG_MAX_LEN] = {0}; /* ���ڴ�Ÿ�ʽ����ĵ�����Ϣ */
        va_list args; 
        

        /* ��ʽ��������Ϣ */
        va_start(args, format);
        vsnprintf((char *)buf, DEBUG_MSG_MAX_LEN - 3, (const char *)format, args);
        va_end(args);
        
        
        /* ��ʽ��Ϊ������Ϣ��ʽ */
        len = strlen((const char *)buf);
        buf[len] = '\r';
        buf[len + 1] = '\n';
        buf[len + 2] = '\0';
        
        len = len + 2; 
        /* ͨ���������������Ϣ */
        DEBUG_USART_Send((u8_t *)buf, len);

        
#if MTFS30_NET_DEBUG_EN
           
        /* ͨ���������������Ϣ */
        net_debug((u8_t *)buf, len);
#endif
                  
}
void debug_level_format(u8_t *format, ...)
{
        u16_t len = 0;
        u8_t buf[DEBUG_MSG_MAX_LEN] = {0}; /* ���ڴ�Ÿ�ʽ����ĵ�����Ϣ */
        int level = 0;
        va_list args; 
        

        /* ��ʽ��������Ϣ */
        va_start(args, format);
        level = va_arg(args, int);
        if(level <= g_debug_level)
          vsnprintf((char *)buf, DEBUG_MSG_MAX_LEN - 3, (const char *)format, args);
        va_end(args);
        
        if(level <= g_debug_level)
        {
        /* ��ʽ��Ϊ������Ϣ��ʽ */
          len = strlen((const char *)buf);
          buf[len] = '\r';
          buf[len + 1] = '\n';
          buf[len + 2] = '\0';
          
          len = len + 2; 
          /* ͨ���������������Ϣ */
          DEBUG_USART_Send((u8_t *)buf, len);
       }

        
#if MTFS30_NET_DEBUG_EN
           
        /* ͨ���������������Ϣ */
        net_debug((u8_t *)buf, len);
#endif
                  
}


/*****************************************************************************
 * ��  ��:    mtfs30_debug_set_handler                                                           
 * ��  ��:    ��/�رյ�����Ϣ                                                                
 * ��  ��:    p_param: ָ����Կ�����Ϣ     
 * ��  ��:    ��                                                    
 * ����ֵ:    OK: ���óɹ�; NG������ʧ��                                                   
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                   
 ****************************************************************************/
u8_t mtfs30_debug_set_handler(int num,...)
{
    va_list valist;
    u8_t *p_param = NULL;
    u8_t err_flag = OK;
    
    
    /*initalize the varlist */
    va_start(valist, num);
    p_param = va_arg(valist, char *);
    if(p_param != NULL)
    {
      if (0 == strncmp(p_param, "OFF", strlen("OFF")))
      {
          /* �رյ��� */
          g_debug_switch = 0;
      }
      else if (0 == strncmp((char const *)p_param, "ON",strlen("ON")))
      {
          /* �򿪵��� */
          g_debug_switch = 1;
      }
      else
      {
          MTFS30_TIPS("����(%s)����!\n", p_param);  
          err_flag = NG;
      }
    }
    /*clear valist memory*/
    va_end(valist);
 
    return err_flag; 
}

u8_t debug_level_set_handler(int num,...)
{
    u8_t *p_param = NULL;
    u8_t  err_flag = OK;
    int level = 0;
    va_list valist;
    
    va_start(valist, num);
    p_param =(u8_t *) va_arg(valist, char *);
    
    if(p_param != NULL)
    {
      level = atoi(p_param);
      
      if(level >= DBG_DISABLE && level <= DBG_DEBUG)
      {
      
          /* debug level */
          g_debug_level = level;
          MTFS30_TIPS("set debug level = %d", g_debug_level);
      }
      else
      {
          err_flag = NG;
          MTFS30_TIPS("����(%s)����", p_param);      
      }
        
      
    }
    va_end(valist);
    
    return err_flag;
}