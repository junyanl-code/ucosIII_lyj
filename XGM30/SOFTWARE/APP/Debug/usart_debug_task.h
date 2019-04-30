/*****************************************************************************/
/* �ļ���:    usart_debug_task.h                                             */
/* ��  ��:    ���ڵ�������ͷ�ļ�                                             */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#ifndef __USART_DEBUG_TASK_H_
#define __USART_DEBUG_TASK_H_
#include "arch/cc.h"
#include <includes.h>


/*-------------------------------*/
/* �궨��                        */
/*-------------------------------*/
#define    USART_DEBUG_TASK_STACK_SIZE   300  /* ��������ջ��С */
#define    USART_DEBUG_THREAD_PRIO       7   /* �����������ȼ� */


/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
extern OS_TCB     gtask_usart_debug_tcb;                                      /* ����������ƿ� */

/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
extern void usart_debug_task_create(void);
extern void net_debug(u8_t *pnet_debug_data, u16_t len);

#endif