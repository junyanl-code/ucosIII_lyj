/*****************************************************************************/
/* �ļ���:    pps_tod.c                                                      */
/* ��  ��:    1PPS+TOD��ش���                                               */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#include <includes.h>
#include "arch/cc.h"
#include "lwip/api.h"
#include "opt.h"
#include "out_pps_tod.h"
#include "bsp_spi.h"





/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/



/*-------------------------------*/
/* ȫ�ֱ�������                  */
/*-------------------------------*/


/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/


/*****************************************************************************
 * ��  ��:    out_set_tod_offset                                                        
 * ��  ��:    �������1pps+tod�źŵ���ʱ����                                                                 
 * ��  ��:    offset: ��ʱ����ֵ                     
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void out_set_tod_offset(u32_t offset)
{   
     u8_t tmp = 0;
     
     tmp = (offset & 0x0F000000) >> 24;
     SPI_FPGA_ByteRead(&tmp, OUT_TOD_OFFSET_REG_ADDR0);
     
     tmp = (offset & 0xFF0000) >> 16;
     SPI_FPGA_ByteRead(&tmp, OUT_TOD_OFFSET_REG_ADDR1); 
     
     
     tmp = (offset & 0xFF00) >> 8;
     SPI_FPGA_ByteRead(&tmp, OUT_TOD_OFFSET_REG_ADDR2);
     
     
     tmp = offset & 0xFF00;
     SPI_FPGA_ByteRead(&tmp, OUT_TOD_OFFSET_REG_ADDR3);     
        
}





/*****************************************************************************
 * ��  ��:    out_set_tod_type                                                       
 * ��  ��:    �������1pps+tod�źŵ�����                                                                 
 * ��  ��:    type: ��ʱ����ֵ                     
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void out_set_tod_type(u8_t type)
{   

     SPI_FPGA_ByteWrite((type&0x1), OUT_TOD_TYPE_REG_ADDR);     
        
}

/*****************************************************************************
 * ��  ��:    out_set_pps_state                                                       
 * ��  ��:    �������1pps+tod�źŵ�������״̬                                                                
 * ��  ��:    state: ������״̬                     
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void pps_tod_set_pps_state(u8_t state)
{   

     SPI_FPGA_ByteWrite(state, OUT_TOD_PPS_STATE_REG_ADDR);     
        
}

/*****************************************************************************
 * ��  ��:    pps_tod_set_tacc                                                       
 * ��  ��:    �������1pps+tod�źŵĶ�������                                                              
 * ��  ��:    tacc: ��������                     
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void pps_tod_set_tacc(u8_t tacc)
{   

     SPI_FPGA_ByteWrite(tacc, OUT_TOD_TACC_REG_ADDR);     
        
}

/*****************************************************************************
 * ��  ��:    out_set_tod_clk_type                                                      
 * ��  ��:    �������1pps+tod�źŵ�ʱ��Դ����                                                             
 * ��  ��:    type: ʱ��Դ����                     
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void out_set_tod_clk_type(u8_t type)
{   

     SPI_FPGA_ByteWrite(type, OUT_TOD_CLK_TYPE_REG_ADDR);          
}

/*****************************************************************************
 * ��  ��:    out_set_tod_clk_state                                                     
 * ��  ��:    �������1pps+tod�źŵ�ʱ��Դ����״̬                                                             
 * ��  ��:    state: ʱ��Դ����״̬                     
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void out_set_tod_clk_state(u16_t state)
{   

     u8_t tmp = 0;
     
     tmp = (state & 0xFF00) >> 8;
     SPI_FPGA_ByteWrite(tmp, OUT_TOD_CLK_STATE_REG_ADDR0);  
     
     tmp = state & 0xFF;
     SPI_FPGA_ByteWrite(tmp, OUT_TOD_CLK_STATE_REG_ADDR1);
}


/*****************************************************************************
 * ��  ��:    out_set_tod_monitor_alarm                                                     
 * ��  ��:    �������1pps+tod�źŵļ�ظ澯                                                             
 * ��  ��:    alarm: ��ظ澯                     
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void out_set_tod_monitor_alarm(u16_t alarm)
{   

     u8_t tmp = 0;
     
     tmp = (alarm & 0xFF00) >> 8;
     SPI_FPGA_ByteWrite(tmp, OUT_TOD_MONITOR_ALARM_REG_ADDR0);  
     
     tmp = alarm & 0xFF;
     SPI_FPGA_ByteWrite(tmp, OUT_TOD_MONITOR_ALARM_REG_ADDR1);
}


/*****************************************************************************
 * ��  ��:    out_set_pps_offset                                                     
 * ��  ��:    �������1pps�źŵ���ʱ����                                                             
 * ��  ��:    offset: ��ʱ����                     
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void out_set_pps_offset(u32_t offset)
{   
     u8_t tmp = 0;
     
     tmp = (offset & 0x0F000000) >> 24;
     SPI_FPGA_ByteRead(&tmp, OUT_PPS_OFFSET_REG_ADDR0);
     
     tmp = (offset & 0xFF0000) >> 16;
     SPI_FPGA_ByteRead(&tmp, OUT_PPS_OFFSET_REG_ADDR1); 
     
     
     tmp = (offset & 0xFF00) >> 8;
     SPI_FPGA_ByteRead(&tmp, OUT_PPS_OFFSET_REG_ADDR2);
     
     
     tmp = offset & 0xFF00;
     SPI_FPGA_ByteRead(&tmp, OUT_PPS_OFFSET_REG_ADDR3);     
        
}


