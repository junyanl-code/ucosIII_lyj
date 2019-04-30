/*****************************************************************************/
/* �ļ���:    fpga_handler.h                                                 */
/* ��  ��:    FPGA��д����ͷ�ļ�                                             */
/* ��  ��:    2018-11-05 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#ifndef __FPGA_HANDLER_H_
#define __FPGA_HANDLER_H_
#include "arch/cc.h"



/*-------------------------------*/
/* �궨��                        */
/*-------------------------------*/
#define    FPGA_RW_MAX_SIZE    256

/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
extern u8_t fpga_write_cmd_handler(int num,...);
extern u8_t fpga_read_cmd_handler(int num,...);
#endif
