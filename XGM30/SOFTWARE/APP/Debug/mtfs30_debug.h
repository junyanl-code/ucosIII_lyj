/*****************************************************************************/
/* �ļ���:    debug.h                                                        */
/* ��  ��:    ���Դ���ͷ�ļ�                                                 */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#ifndef __MTFS30_DEBUG_H_
#define __MTFS30_DEBUG_H_
#include <includes.h>
#include "arch/cc.h"
#include "lwip/api.h"
#include "Config/config.h"
#include "opt.h"


/*-------------------------------*/
/* �궨��                        */
/*-------------------------------*/
#define  DEBUG_MSG_MAX_LEN        256     /* ÿ��������Ϣ��󳤶ȣ���������"\r\n\0" */

/* ���Կ��� */
#define  MTFS30_DEBUG_EN             1       /* ʹ�ܵ�����Ϣ���     */
#define  MTFS30_ERROR_EN             0       /* ʹ�ܴ�����Ϣ���     */
//#define  MTFS30_NET_DEBUG_EN         0       /* ʹ���������         */


/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
extern u8_t                    g_debug_switch;        /* ���ڵ��Կ��ر���   */

/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
extern void debug_init( void );
extern void debug_task_create( void );
extern void debug_msg_format(u8_t *format, ...);
extern u8_t mtfs30_debug_set_handler(int num,...);


/*-------------------------------*/
/* �꺯������                    */
/*-------------------------------*/
#define  MTFS30_ERROR(fmt, args...)       debug_msg_format("[ERROR][%s:%d] "fmt, __FUNCTION__, __LINE__, ##args) /* ���������Ϣ */
#define  MTFS30_DEBUG(fmt, args...)       if (g_debug_switch) {debug_msg_format("[DEBUG][%s:%d] "fmt, __FUNCTION__, __LINE__, ##args);} /* ���������Ϣ */
#define  MTFS30_TIPS(fmt, args...)        debug_msg_format(fmt, ##args)                                          /* �����ʾ��Ϣ */

#endif