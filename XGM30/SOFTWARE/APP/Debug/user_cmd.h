/*****************************************************************************/
/* �ļ���:    user_cmd.h                                                     */
/* ��  ��:    �û���������ͷ�ļ�                                         */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#ifndef __USER_CMD_H_
#define __USER_CMD_H_
#include "arch/cc.h"
#include "Gnss/gnss.h"


/* �û���������ṹ���� */
typedef struct  _USER_CMDHANDLE_T_
{
    u8_t  cmd_type; /* ��������: ��������|��ѯ���� */
	u8_t* p_head;   /* ����ͷ                      */
    /* p_param: ������Ϣ�� p_rtrv����Ų�ѯ��� */
	u8_t (*cmd_fun)(int num,...);
} user_cmdhandle_t;


/*-------------------------------*/
/* �궨��                        */
/*-------------------------------*/
#define USER_CMD_SET      0   /* ��������               */
#define USER_CMD_GET      1   /* ��ѯ����               */
#define RTRV_BUF_MAX_SIZE GNSS_SAT_INFO_MAX_LEN /* ��ѯ�������������С ���������Ϊ������Ϣ�� */
#define USER_CMD_MAX_LEN  128   /* ������󳤶� */

#define MAX_PARAM_NUM     6

#define GNSS_COMMAND_RTRV      3
#define GNSS_COMMAND_SET       4


/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
extern void telnet_cmd_handle(u8_t *pdata);
extern void user_cmd_parser(u8_t *pmsg);
#endif