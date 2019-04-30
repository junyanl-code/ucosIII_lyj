/*****************************************************************************/
/* �ļ���:    watchdog_task.h                                                */
/* ��  ��:    ���Ź�����ͷ�ļ�                                               */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#ifndef __WATCHDOG_TASK_H_
#define __WATCHDOG_TASK_H_
#include <includes.h>


/*-------------------------------*/
/* �궨��                        */
/*-------------------------------*/
#define    WATCHDOG_TASK_STACK_SIZE   128  /* ���Ź�����ջ��С */
#define    WATCHDOG_TASK_PRIO         5    /* ���Ź��������ȼ� */

#define    SYSTEM_WDI_ENABLE_REG_ADDR    0x0005   /* ���Ź�ʹ�ܼĴ�����ַ */
#define    SYSTEM_WDI_REG_ADDR           0x0006   /* ���Ź�ι���Ĵ�����ַ */

/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
extern OS_TCB            g_watchdog_task_tcb;                       /* ���Ź�������ƿ� */

/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
extern void watchdog_task_create(void);

#endif
