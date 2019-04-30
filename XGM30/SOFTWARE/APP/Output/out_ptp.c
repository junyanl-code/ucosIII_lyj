/*****************************************************************************/
/* �ļ���:    out_ptp.c                                                      */
/* ��  ��:    PTP��ش���                                                    */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#include <includes.h>
//#include "arch/cc.h"
//#include "lwip/api.h"
//#include "opt.h"
#include "Output/out_ptp.h"
#include "BSP/inc/bsp_spi.h"
#include "mtfs30.h"
#include "Debug/mtfs30_debug.h"
#include "Util/util_string.h"





/*-------------------------------*/
/* ȫ�ֱ�������                  */
/*-------------------------------*/


/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
static u8_t num_param_check(u8_t *p_param);
static u8_t value2_param_check(u8_t *p_param, u8_t *p_value);
static u8_t domain_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t flag_secu_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t flag_ps2_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t flag_ps1_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t flag_alt_master_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t flag_freq_trac_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t flag_time_trac_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t flag_time_scale_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t flag_cur_utc_offset_valid_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t flag_leap59_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t flag_leap61_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t unimul_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t layer_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t step_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t delay_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t emsc_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t utc_offset_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t gm_prio1_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t gm_clk_qual_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t gm_prio2_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t steps_rm_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t time_source_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t announce_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t sync_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t ssm_parser(u8_t *p_param, ptp_t *p_ptp);
static u8_t set_cmd_param_parser(u8_t *p_msg, ptp_t *p_ptp);
/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
static ptp_parser_t  parser_tbl[30] = {
    domain_parser, 
    flag_secu_parser,
    flag_ps2_parser,
    flag_ps1_parser,
    flag_alt_master_parser,
    flag_freq_trac_parser,
    flag_time_trac_parser,
    flag_time_scale_parser,
    flag_cur_utc_offset_valid_parser,
    flag_leap59_parser,
    flag_leap61_parser,
    unimul_parser,
    layer_parser,
    step_parser,
    delay_parser,
    emsc_parser,
    utc_offset_parser,
    gm_prio1_parser,
    gm_clk_qual_parser,
    gm_prio2_parser,
    steps_rm_parser,
    time_source_parser,
    announce_parser,
    sync_parser,
    ssm_parser,

};


ptp_t    g_ptp;            /* ���PTP������Ϣ */


/*****************************************************************************
 * ��  ��:    ptp_set_handler                                                         
 * ��  ��:    PTP�����������                                                                 
 * ��  ��:    p_msg��������Ϣ                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
u8_t ptp_set_handler(u8_t *p_msg, u8_t *p_rtrv)
{

    ptp_t  ptp;
    
    
    /* �����������е�ÿ������ */
    if (set_cmd_param_parser(p_msg, &ptp))
    {
        return NG;
    }
      
    
    /* ����PTP */
    if (NG == ptp_set(&ptp))
    {
        MTFS30_TIPS("PTP����ʧ��");    
        return NG;
    }
    
    return OK;
}


/*****************************************************************************
 * ��  ��:    ptp_set                                                         
 * ��  ��:    ����PTP                                                                 
 * ��  ��:    p_ptp��ָ��PTP������Ϣ                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
u8_t ptp_set(ptp_t *p_ptp)
{
    u8_t ret = OK;
    u8_t tmp = 0;
    
    
    /* ������� */
    if (NULL == p_ptp)
    {
        MTFS30_ERROR("set_handler()��������");
        return NG;
    }

    
    /* ����DOMAIN */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->domain, PTP_DOMAIN_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
    
    /* ����PTP security */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->flag_secu, PTP_FLAG_SECU_REG_ADDR)))
    {
        goto PTP_SET_ERR; 
    }
    
    /* ����PTP profile Specific 2 */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->flag_ps2, PTP_FLAG_PS2_REG_ADDR)))
    {
        goto PTP_SET_ERR;    
    }
    
    /* ����PTP profile Specific 1 */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->flag_ps1, PTP_FLAG_PS1_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
   
    /* ����alternateMasterFlag */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->flag_alt_master, PTP_FLAG_ALT_MASTER_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }        

    /* ����frequencyTraceable */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->flag_freq_trac, PTP_FLAG_FREQ_TRAC_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
    
    /* ����timeTraceable */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->flag_time_trac, PTP_FLAG_TIME_TRAC_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
    
    /* ����ptpTimescale */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->flag_time_scale, PTP_FLAG_TIME_SCALE_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
    
    /* ����currentUtcOffsetValid */   
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->flag_cur_utc_offset_valid, PTP_FLAG_CUR_UTC_OFFSET_VALID_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }            
        
    /* ����leap59 */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->flag_leap59, PTP_FLAG_LEAP59_REG_ADDR))) 
    {
        goto PTP_SET_ERR;
    }
    
    /* ����leap61 */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->flag_leap61, PTP_FLAG_LEAP61_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
    
    /* ���õ����ಥ���� */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->unimul, PTP_UNIMUL_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
    
    /* ���÷�װ��ʽ */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->layer, PTP_LAYER_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
    
    /* ����ʱ������ģʽ */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->step, PTP_STEP_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
    
    /* ����ʱ������ */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->delay, PTP_DELAY_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
    
    /* ����EMSCʹ�� */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->emsc, PTP_ESMC_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
    
    /* ����currentUtcOffset */
    tmp = (p_ptp->utc_offset & 0xFF00) >> 8;
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->utc_offset, PTP_UTC_OFFSET_REG_ADDR0))) 
    {
        goto PTP_SET_ERR;
    }               
    tmp = p_ptp->utc_offset & 0xFF;
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->utc_offset, PTP_UTC_OFFSET_REG_ADDR1)))
    {
        goto PTP_SET_ERR;
    }  
    
    
    /* ����grandmasterPriority1 */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->gm_prio1, PTP_GM_PRIO1_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
    
    /* ����grandmasterClockQuality */
    tmp = (p_ptp->gm_clk_qual & 0xFF000000) >> 24;
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->gm_clk_qual, PTP_GM_CLK_QUAL_REG_ADDR0)))   
    {
        goto PTP_SET_ERR;
    }               
    tmp = (p_ptp->gm_clk_qual & 0xFF0000) >> 16;
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->gm_clk_qual, PTP_GM_CLK_QUAL_REG_ADDR1))) 
    {
        goto PTP_SET_ERR;
    }               
    tmp = (p_ptp->gm_clk_qual & 0xFF00) >> 8;
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->gm_clk_qual, PTP_GM_CLK_QUAL_REG_ADDR2)))
    {
        goto PTP_SET_ERR;
    }               
    tmp = p_ptp->gm_clk_qual & 0xFF;
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->gm_clk_qual, PTP_GM_CLK_QUAL_REG_ADDR3)))      
    {
        goto PTP_SET_ERR;
    }
    
    /* ����grandmasterPriority2 */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->gm_prio2, PTP_GM_PRIO2_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
    
    /* ����stepsRemoved */
    tmp = (p_ptp->steps_rm & 0xFF00) >> 8;
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->steps_rm, PTP_STEPS_RM_REG_ADDR0)))  
    {
        goto PTP_SET_ERR;
    }               
    tmp = p_ptp->steps_rm & 0xFF;
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->steps_rm, PTP_STEPS_RM_REG_ADDR1))) 
    {
        goto PTP_SET_ERR;
    }               


    /* ����timeSource */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->time_source, PTP_TIME_SOURCE_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
    
    /* ����announce���ķ������ */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->announce, PTP_ANNOUNCE_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
    
    /* ����sync���ķ������ */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->sync, PTP_SYNC_REG_ADDR)))
    {
        goto PTP_SET_ERR;
    }
    
    /* ����ssm */
    if (NG == (ret = SPI_FPGA_ByteWrite(p_ptp->ssm, PTP_SSM_REG_ADDR)))    
    {
        goto PTP_SET_ERR;
    }


PTP_SET_ERR:
    
    if (NG == ret)
    {
        MTFS30_ERROR("ͨ��SPI����PTPʧ��");
        return NG;
    }
        
    return OK;
}

/*****************************************************************************
 * ��  ��:    domain_parser                                                         
 * ��  ��:    ����DOMAIN                                                                 
 * ��  ��:    p_param�����õ�domian��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                  
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t domain_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;
    
    
    if (NG == num_param_check(p_param))
    {
        MTFS30_TIPS("DOMIAN�趨ֵ(%s)����ȷ,��ȷֵΪ0~255", p_param);
        return NG;
    }
    
    p_ptp->domain = value;     
    return OK;
}

/*****************************************************************************
 * ��  ��:    flag_secu_parser                                                         
 * ��  ��:    ����PTP security                                                                
 * ��  ��:    p_param�����õ�PTP security��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t flag_secu_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;

    
    if (NG == value2_param_check(p_param, &value))
    {
        MTFS30_TIPS("PTP security�趨ֵ(%s)����ȷ,��ȷֵΪ0����1", p_param);
        return NG;
    }
    
    p_ptp->flag_secu = value&0x1; 
    return OK;
}


/*****************************************************************************
 * ��  ��:    flag_ps2_parser                                                         
 * ��  ��:    ����PTP profile Specific 2                                                                
 * ��  ��:    p_param�����õ�PTP profile Specific 2 ��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t flag_ps2_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;

    
    if (NG == value2_param_check(p_param, &value))
    {
        MTFS30_TIPS("PTP profile Specific 2 �趨ֵ(%s)����ȷ,��ȷֵΪ0����1", p_param);
        return NG;
    }
    
    p_ptp->flag_ps2 = value&0x1;    
    return OK;
}


/*****************************************************************************
 * ��  ��:    flag_ps1_parser                                                         
 * ��  ��:    ����PTP profile Specific 1                                                                
 * ��  ��:    p_param�����õ�PTP profile Specific 1 ��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t flag_ps1_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;

    
    if (NG == value2_param_check(p_param, &value))
    {
        MTFS30_TIPS("PTP profile Specific 1 �趨ֵ(%s)����ȷ,��ȷֵΪ0����1", p_param);
        return NG;
    }
    
    p_ptp->flag_ps1 = value&0x1;        
    return OK;
}

/*****************************************************************************
 * ��  ��:    flag_alt_master_parser                                                         
 * ��  ��:    ����alternateMasterFlag                                                                
 * ��  ��:    p_param�����õ�alternateMasterFlag ��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t flag_alt_master_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;

    
    if (NG == value2_param_check(p_param, &value))
    {
        MTFS30_TIPS("alternateMasterFlag �趨ֵ(%s)����ȷ,��ȷֵΪ0����1", p_param);
        return NG;
    }
    
    p_ptp->flag_alt_master = value&0x1;     
    return OK;
}

/*****************************************************************************
 * ��  ��:    flag_freq_trac_parser                                                         
 * ��  ��:    ����frequencyTraceable                                                          
 * ��  ��:    p_param�����õ�frequencyTraceable ��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t flag_freq_trac_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;

    
    if (NG == value2_param_check(p_param, &value))
    {
        MTFS30_TIPS("frequencyTraceable �趨ֵ(%s)����ȷ,��ȷֵΪ0����1", p_param);
        return NG;
    }
    
    p_ptp->flag_freq_trac = value&0x1;       
    return OK;
}

/*****************************************************************************
 * ��  ��:    flag_time_trac_parser                                                         
 * ��  ��:    ����timeTraceable                                                          
 * ��  ��:    p_param�����õ�timeTraceable ��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t flag_time_trac_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;

    
    if (NG == value2_param_check(p_param, &value))
    {
        MTFS30_TIPS("timeTraceable �趨ֵ(%s)����ȷ,��ȷֵΪ0����1", p_param);
        return NG;
    }
    
    p_ptp->flag_time_trac = value&0x1;      
    return OK;
}

/*****************************************************************************
 * ��  ��:    flag_time_scale_parser                                                         
 * ��  ��:    ����ptpTimescale                                                          
 * ��  ��:    p_param�����õ�ptpTimescale ��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t flag_time_scale_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;

    
    if (NG == value2_param_check(p_param, &value))
    {
        MTFS30_TIPS("ptpTimescale �趨ֵ(%s)����ȷ,��ȷֵΪ0����1", p_param);
        return NG;
    }
    
    p_ptp->flag_time_scale = value&0x1;   
    return OK;
}


/*****************************************************************************
 * ��  ��:    flag_cur_utc_offset_valid_parser                                                         
 * ��  ��:    ����currentUtcOffsetValid                                                         
 * ��  ��:    p_param�����õ�currentUtcOffsetValid ��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t flag_cur_utc_offset_valid_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;

    
    if (NG == value2_param_check(p_param, &value))
    {
        MTFS30_TIPS("currentUtcOffsetValid �趨ֵ(%s)����ȷ,��ȷֵΪ0����1", p_param);
        return NG;
    }
    
    p_ptp->flag_cur_utc_offset_valid = value&0x1;   
    return OK;
}

/*****************************************************************************
 * ��  ��:    flag_leap59_parser                                                         
 * ��  ��:    ����leap59                                                        
 * ��  ��:    p_param�����õ�leap59��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t flag_leap59_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;


    if (NG == value2_param_check(p_param, &value))
    {
        MTFS30_TIPS("leap59 �趨ֵ(%s)����ȷ,��ȷֵΪ0����1", p_param);
        return NG;
    }
	
    p_ptp->flag_leap59 = value&0x1;    
    return OK;
}

/*****************************************************************************
 * ��  ��:    flag_leap61_parser                                                         
 * ��  ��:    ����leap61                                                        
 * ��  ��:    p_param�����õ�leap61��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t flag_leap61_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;


    if (NG == value2_param_check(p_param, &value))
    {
        MTFS30_TIPS("leap61 �趨ֵ(%s)����ȷ,��ȷֵΪ0����1", p_param);
        return NG;
    }
	
    p_ptp->flag_leap61 = value&0x1;  
    return OK;
}

/*****************************************************************************
 * ��  ��:    unimul_parser                                                         
 * ��  ��:    ���������ಥ����������Ϣ                                                        
 * ��  ��:    p_param�����õĲ��ಥ������Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t unimul_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;


    if (NG == value2_param_check(p_param, &value))
    {
        MTFS30_TIPS("�����ಥ �趨ֵ(%s)����ȷ,��ȷֵΪ0����1", p_param);
        return NG;
    }
	
    p_ptp->unimul = value&0x1;  
    return OK;
}

/*****************************************************************************
 * ��  ��:    layer_parser                                                         
 * ��  ��:    ������װ��ʽ������Ϣ                                                        
 * ��  ��:    p_param�����õķ�װ��ʽ��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t layer_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;


    if (NG == value2_param_check(p_param, &value))
    {
        MTFS30_TIPS("��װ��ʽ(����/����) �趨ֵ(%s)����ȷ,��ȷֵΪ0����1", p_param);
        return NG;
    }
	
    p_ptp->layer = value&0x1;  
    return OK;
}

/*****************************************************************************
 * ��  ��:    step_parser                                                         
 * ��  ��:    ����ʱ������ģʽ������Ϣ                                                        
 * ��  ��:    p_param�����õ�ʱ������ģʽ��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t step_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;


    if (NG == value2_param_check(p_param, &value))
    {
        MTFS30_TIPS("ʱ������ģʽ(onestep/twostep) �趨ֵ(%s)����ȷ,��ȷֵΪ0����1", p_param);
        return NG;
    }
    p_ptp->step = value&0x1;    
    return OK;
}

/*****************************************************************************
 * ��  ��:    delay_parser                                                         
 * ��  ��:    ����ʱ������������Ϣ                                                        
 * ��  ��:    p_param�����õ�ʱ��������Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t delay_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;


    if (NG == value2_param_check(p_param, &value))
    {
        MTFS30_TIPS("ʱ������(P2P/E2E) �趨ֵ(%s)����ȷ,��ȷֵΪ0����1", p_param);
        return NG;
    }
    p_ptp->delay = value&0x1;  
    return OK;
}

/*****************************************************************************
 * ��  ��:    emsc_parser                                                         
 * ��  ��:    ����EMSCʹ��������Ϣ                                                        
 * ��  ��:    p_param�����õ�EMSCʹ����Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t emsc_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;


    if (NG == value2_param_check(p_param, &value))
    {
        MTFS30_TIPS("EMSCʹ�� �趨ֵ(%s)����ȷ,��ȷֵΪ0����1", p_param);
        return NG;
    }
    
    p_ptp->emsc = value&0x1;                    
    return OK;
}

/*****************************************************************************
 * ��  ��:    utc_offset_parser                                                         
 * ��  ��:    ����currentUtcOffset������Ϣ                                                        
 * ��  ��:    p_param�����õ�currentUtcOffset��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t utc_offset_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u16_t value = 0;



    if (NG == num_param_check(p_param))
    {
        MTFS30_TIPS("currentUtcOffset �趨ֵ(%s)����ȷ,����������ֵ", p_param);
        return NG;
    }
    
    value = (u16_t)atoi((char const *)p_param);
    
    p_ptp->utc_offset = value;                    
    return OK;
}


/*****************************************************************************
 * ��  ��:    gm_prio1_parser                                                         
 * ��  ��:    ����grandmasterPriority1������Ϣ                                                        
 * ��  ��:    p_param�����õ�grandmasterPriority1��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t gm_prio1_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;



    if (NG == num_param_check(p_param))
    {
        MTFS30_TIPS("grandmasterPriority1 �趨ֵ(%s)����ȷ,����������ֵ", p_param);
        return NG;
    }
    
    value = (u8_t)atoi((char const *)p_param);
    
    p_ptp->gm_prio1 = value;                    
    return OK;
}

/*****************************************************************************
 * ��  ��:    gm_clk_qual_parser                                                         
 * ��  ��:    ����grandmasterClockQuality������Ϣ                                                        
 * ��  ��:    p_param�����õ�grandmasterClockQuality��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t gm_clk_qual_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u32_t value = 0;



    if (NG == num_param_check(p_param))
    {
        MTFS30_TIPS("grandmasterClockQuality �趨ֵ(%s)����ȷ,����������ֵ", p_param);
        return NG;
    }
    
    value = (u32_t)atoi((char const *)p_param);
    
    p_ptp->gm_clk_qual = value;                    
    return OK;
}


/*****************************************************************************
 * ��  ��:    gm_prio2_parser                                                         
 * ��  ��:    ����grandmasterPriority2������Ϣ                                                        
 * ��  ��:    p_param�����õ�grandmasterPriority2��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t gm_prio2_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;



    if (NG == num_param_check(p_param))
    {
        MTFS30_TIPS("grandmasterPriority2 �趨ֵ(%s)����ȷ,����������ֵ", p_param);
        return NG;
    }
    
    value = (u8_t)atoi((char const *)p_param);
    
    p_ptp->gm_prio2 = value;                    
    return OK;
}


/*****************************************************************************
 * ��  ��:    steps_rm_parser                                                         
 * ��  ��:    ����stepsRemoved������Ϣ                                                        
 * ��  ��:    p_param�����õ�stepsRemoved��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t steps_rm_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u16_t value = 0;



    if (NG == num_param_check(p_param))
    {
        MTFS30_TIPS("stepsRemoved �趨ֵ(%s)����ȷ,����������ֵ", p_param);
        return NG;
    }
    
    value = (u16_t)atoi((char const *)p_param);
    
    p_ptp->steps_rm = value;                    
    return OK;
}


/*****************************************************************************
 * ��  ��:    time_source_parser                                                         
 * ��  ��:    ����timeSource������Ϣ                                                        
 * ��  ��:    p_param�����õ�timeSource��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t time_source_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;



    if (NG == num_param_check(p_param))
    {
        MTFS30_TIPS("timeSource �趨ֵ(%s)����ȷ,����������ֵ", p_param);
        return NG;
    }
    
    value = (u8_t)atoi((char const *)p_param);
    
    p_ptp->time_source = value;                    
    return OK;
}


/*****************************************************************************
 * ��  ��:    announce_parser                                                         
 * ��  ��:    ����announce���ķ������������Ϣ                                                        
 * ��  ��:    p_param�����õ�announce���ķ��������Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t announce_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;



    if (NG == num_param_check(p_param))
    {
        MTFS30_TIPS("announce���ķ������ �趨ֵ(%s)����ȷ,����������ֵ", p_param);
        return NG;
    }
    
    value = (u8_t)atoi((char const *)p_param);
    
    p_ptp->announce = value;                    
    return OK;
}


/*****************************************************************************
 * ��  ��:    sync_parser                                                         
 * ��  ��:    ����sync���ķ������������Ϣ                                                        
 * ��  ��:    p_param�����õ�sync���ķ��������Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t sync_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;



    if (NG == num_param_check(p_param))
    {
        MTFS30_TIPS("sync���ķ������ �趨ֵ(%s)����ȷ,����������ֵ", p_param);
        return NG;
    }
    
    value = (u8_t)atoi((char const *)p_param);
    
    p_ptp->sync = value;                    
    return OK;
}

/*****************************************************************************
 * ��  ��:    ssm_parser                                                         
 * ��  ��:    ����ssm������Ϣ                                                        
 * ��  ��:    p_param�����õ�ssm��Ϣ  
 * ��  ��:    p_ptp: ��Ž������PTP������Ϣ                                                 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t ssm_parser(u8_t *p_param, ptp_t *p_ptp)
{
    u8_t value = 0;



    if (NG == num_param_check(p_param))
    {
        MTFS30_TIPS("ssm �趨ֵ(%s)����ȷ,����������ֵ", p_param);
        return NG;
    }
    
    value = (u8_t)atoi((char const *)p_param);
    
    p_ptp->ssm = value;                    
    return OK;
}

/*****************************************************************************
 * ��  ��:    set_cmd_param_parser                                                         
 * ��  ��:    PTP���������������                                                                 
 * ��  ��:    p_msg������������Ϣ    
 *            p_ptp: ָ�������Ĳ���
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t set_cmd_param_parser(u8_t *p_msg, ptp_t *p_ptp)
{
    u8_t cnt = 0;
    u8_t *p = p_msg;
    u8_t *p_param = NULL;

    
    
    /* ������ÿ������ */
    while(*p)
    {
        /* ȡ��ÿ������ */
        if (NULL != (p_param = (u8_t *)strtok((char *)p, ",")))
        {
            
            
            /* ����ÿ��������Ӧ�Ľ������� */
            if (NG == parser_tbl[cnt].cmd_fun(p_param, p_ptp))
            {
                
                return NG; 
            }
            
            p += strlen((const char *)p_param);
            cnt++;
        
        }
        else
        {
            MTFS30_TIPS("PTP���������к��пղ���������������");
            return NG;            
        }
                                  
        p++;
    }
    
    return OK;

}






/*****************************************************************************
 * ��  ��:    value2_param_check                                                        
 * ��  ��:    2ֵ(0/1)����ֵ���                                                                 
 * ��  ��:    p_param: ָ���趨ֵ 
 *            p_value: ����ֵ
 * ��  ��:    ��                                                    
 * ����ֵ:    OK: ����ֵ��ȷ�� NG������ֵ����                                                   
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t value2_param_check(u8_t *p_param, u8_t *p_value)
{
    u8_t value = 0;
    
    *p_value = 0;
    
    value = *(u8_t *)p_param - '0';
    if (value != 0 && value != 1)
    {       
        return NG;
    }

    *p_value = value;
    return OK;
} 

/*****************************************************************************
 * ��  ��:    num_param_check                                                         
 * ��  ��:    ���ֲ���ֵ���                                                               
 * ��  ��:    p_param: ָ���趨ֵ                          
 * ��  ��:    ��                                                 
 * ����ֵ:    OK: ������ȷ; NG:��������ȷ                                                      
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t num_param_check(u8_t *p_param)
{
    u8_t i = 0;
    
    
    /* ����Ƿ���0~9֮��������ַ� */
    while(p_param[i])
    {
        if (p_param[i] < '0' || p_param[i] > '9')
        {
            if (i == 0 && p_param[i] == '-') /* ���� */
            {
                continue;
            }
            
            return NG;
        }
    
        i++;
    }
    
    
    return OK;
}







