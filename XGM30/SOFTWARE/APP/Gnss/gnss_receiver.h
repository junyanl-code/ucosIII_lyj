/*****************************************************************************/
/* �ļ���:    gnss_receiver.h                                                */
/* ��  ��:    GNSS��������ش���ͷ�ļ�                                       */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#ifndef __GNSS_RECEIVER_H_
#define __GNSS_RECEIVER_H_
#include "gnss.h"


/*-------------------------------*/
/* �ṹ����                      */
/*-------------------------------*/


/*-------------------------------*/
/* �궨��                        */
/*-------------------------------*/
#define GNSS_ON       1       /* �� */
#define GNSS_OFF      0       /* �ر� */

/* ������� */
#define GNSS_STMT_ZDA  0       /* GGA���      */
#define GNSS_STMT_GGA  1       /* GGA���      */
#define GNSS_STMT_GSV  2       /* GSV���      */
#define GNSS_STMT_UTC  3       /* ������Ϣ��� */
#define GNSS_STMT_PPS  4       /* 1PPS��Ϣ��� */
#define GNSS_STMT_ANT  5       /* ����״̬��� */
#define GNSS_STMT_VER  6       /* �汾��Ϣ��� */


/* �������� */
#define GNSS_SAT_TYPE_GPS  0       /* GPS����    */
#define GNSS_SAT_TYPE_BDS  1       /* BDS����    */
#define GNSS_SAT_TYPE_GLO  2       /* GLO����    */
#define GNSS_SAT_TYPE_GAL  3       /* GAL����    */  




/* ʹ�õĶ�λ���� */
#define REV_USED_GPS         0x01    /* ʹ��GPS��λ                  */
#define REV_USED_BDS         0x02    /* ʹ��BDS��λ                  */
#define REV_USED_GLO         0x04    /* ʹ��GLO��λ                  */
#define REV_USED_GAL         0x08    /* ʹ��GAL��λ                  */

/* ���ջ�����ģʽ */
#define REV_ACMODE_MIX         0    /* GPS��BDS��GLO��GAL���϶�λ */ 
#define REV_ACMODE_GPS         1    /* ��GPS��λ                  */
#define REV_ACMODE_BDS         2    /* ��BDS��λ                  */
#define REV_ACMODE_GLO         3    /* ��GLO��λ                  */
#define REV_ACMODE_GAL         4    /* ��GAL��λ                  */


 
/* ���ջ����Ͷ��� */
#define    REV_TYPE_UT4B0    0    /* UT4B0���ջ� */

/* ������ */
#define    GNSS_GGA_STMT_ON       0x01 /* ��GGA������  */
#define    GNSS_GSV_STMT_ON       0x02 /* ��GSV������  */
#define    GNSS_UTC_STMT_ON       0x04 /* ��UTC������  */
#define    GNSS_PPS_STMT_ON       0x08 /* ��PPS������  */
#define    GNSS_ANT_STMT_ON       0x10 /* ��ANT������  */
#define    GNSS_ALL_STMT_ON       0X1F /* ��ȫ�������� */
#define    GNSS_ALL_STMT_OFF      0x0  /* �ر�ȫ�������� */




/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
void gnss_receiver_msg_parser(u8_t *pdata);
void gnss_receiver_init(void);
void gnss_receiver_data_clear(void);
u8_t gnss_receiver_acmode_set(u8_t *pmode);
u8_t gnss_receiver_echo_set(u8_t *pparam);
u8_t gnss_receiver_debug_set(u8_t *p_param);
u8_t gnss_receiver_stmt_set(u8_t *p_param);
void gnss_receiver_msg_set(u8_t ac_mode);
void gnss_receiver_reset(u8_t reset_mode);
u8_t gnss_receiver_serial_baud_set(u8_t *pbaud);
u8_t gnss_receiver_get_info(u8_t *p_param, u8_t *p_rtrv);
void gnss_receiver_get_sat_info(u8_t *p_rtrv);
u8_t gnss_receiver_check_primary_mode();
void gnss_receiver_info_clear(void);

#endif
