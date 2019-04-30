/*****************************************************************************/
/* �ļ���:    gnss_handler.h                                                 */
/* ��  ��:    GNSS�������/��ѯ����ͷ�ļ�                                    */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#ifndef __GNSS_HANDLER_H_
#define __GNSS_HANDLER_H_
#include "arch/cc.h"


/*-------------------------------*/
/* �궨��                        */
/*-------------------------------*/
/* �Ĵ�����ַ���� */
#define GNSS_GPS_PPS_OFFSET_REG_ADDR0 0x0027
#define GNSS_GPS_PPS_OFFSET_REG_ADDR1 0x0028
#define GNSS_GPS_PPS_OFFSET_REG_ADDR2 0x0029
#define GNSS_GPS_PPS_OFFSET_REG_ADDR3 0x0030



/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
extern void gnss_global_variable_init(void);
extern void gnss_init(void);
extern u8_t gnss_acmode_set_handler(int num,...);
extern u8_t gnss_serial_baud_set_handler(int num,...);
extern u8_t gnss_echo_set_handler(int num,...);
extern u8_t gnss_debug_set_handler(u8_t *p_param, u8_t *p_rtrv);
extern u8_t gnss_stmt_set_handler(int num,...);
extern u8_t gnss_get_info_handler(u8_t *p_param, u8_t *p_rtrv);
extern u8_t gnss_inquery_set(u8_t *p_param, u8_t *p_rtrv);
extern u8_t gnss_helper(int num,...);
extern void gnss_info_print(void);

void gnss_stmt_echo(u8_t *p_msg);
#endif
