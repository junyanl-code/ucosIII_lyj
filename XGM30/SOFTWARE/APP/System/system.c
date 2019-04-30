/*****************************************************************************/
/* �ļ���:    system.c                                                       */
/* ��  ��:    ϵͳ������ش���                                               */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#include <includes.h>
#include "arch/cc.h"
#include "lwip/api.h"
#include "opt.h"
#include "System/system.h"
#include "mtfs30.h"
#include "Debug/mtfs30_debug.h"
#include "BSP/inc/bsp_spi.h"





/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
static system_time_t    g_time;    /* ϵͳʱ�� */


/*-------------------------------*/
/* ȫ�ֱ�������                  */
/*-------------------------------*/


/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/

/*****************************************************************************
 * ��  ��:    system_update_time                                                         
 * ��  ��:    ����ϵͳʱ��                                                                 
 * ��  ��:    ��                         
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_update_time(u16_t year, u8_t mon, u8_t day, u8_t hour, u8_t min, u8_t sec)
{
     g_time.year = year;
     g_time.mon = mon;
     g_time.day = day;
     g_time.hour = hour;
     g_time.min = min;
     g_time.sec = sec;
}

/*****************************************************************************
 * ��  ��:    system_time_get                                                       
 * ��  ��:    ��ȡϵͳʱ��                                                                 
 * ��  ��:    ��                         
 * ��  ��:    p_time: ָ��ϵͳʱ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_time_get(system_time_t *p_time)
{

    if (p_time == NULL)
    {
         return;
    }
    
    
    *p_time = g_time;
}



/*****************************************************************************
 * ��  ��:    system_hardware_version_get                                                         
 * ��  ��:    ��ȡӲ���汾                                                                 
 * ��  ��:    enable: ʹ��/��ʹ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_version_get(system_v_t *p_sysv)
{
    u8_t master_v;
    u8_t sec_v;
    
    
    SPI_FPGA_ByteRead(&master_v, SYSTEM_HARDWARE_VERSION_HIGH_REG_ADDR); 
    SPI_FPGA_ByteRead(&sec_v, SYSTEM_HARDWARE_VERSION_LOW_REG_ADDR);
    
    sprintf((char *)p_sysv->hw_version, "V%02d.%02d", master_v, sec_v);
    
    
    SPI_FPGA_ByteRead(&master_v, SYSTEM_FPGA_VERSION_HIGH_REG_ADDR); 
    SPI_FPGA_ByteRead(&sec_v, SYSTEM_FPGA_VERSION_LOW_REG_ADDR);

    sprintf((char *)p_sysv->fpga_version, "V%02d.%02d", master_v, sec_v);    

}


/*****************************************************************************
 * ��  ��:    system_set_wdienable                                                         
 * ��  ��:    ���Ź�ʹ������                                                                 
 * ��  ��:    enable: ʹ��/��ʹ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_set_wdienable(u8_t wdienable)
{   
     SPI_FPGA_ByteWrite(wdienable, SYSTEM_WDI_ENABLE_REG_ADDR);     
}


/*****************************************************************************
 * ��  ��:    system_set_wdi                                                         
 * ��  ��:    ���Ź�ι��                                                                 
 * ��  ��:    ��                        
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_set_wdi()
{   
     SPI_FPGA_ByteWrite(0, SYSTEM_WDI_REG_ADDR);
     SPI_FPGA_ByteWrite(1, SYSTEM_WDI_REG_ADDR);     
}

/*****************************************************************************
 * ��  ��:    system_get_pll_state                                                         
 * ��  ��:    ��ȡ���໷״̬                                                                 
 * ��  ��:    ��                        
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_get_pll_state(u8_t *p_state)
{   
     
     SPI_FPGA_ByteRead(p_state, SYSTEM_PLL_STATE_REG_ADDR);      
}


///*****************************************************************************
// * ��  ��:    system_set_tod_offset                                                        
// * ��  ��:    �������1pps+tod�źŵ���ʱ����                                                                 
// * ��  ��:    offset: ��ʱ����ֵ                     
// * ��  ��:    ��                                                    
// * ����ֵ:    ��                                                    
// * ��  ��:    2018-07-18 changzehai(DTT)                            
// * ��  ��:    ��                                       
// ****************************************************************************/
//void system_set_tod_offset(u32_t offset)
//{   
//     u8_t tmp_offset = 0;
//     
//     
//     SPI_FPGA_ByteRead(&p_state, SYSTEM_PLLSTATE_REG_ADDR);  
//     
//}


/*****************************************************************************
 * ��  ��:    system_get_phase                                                        
 * ��  ��:    ��ȡϵͳ����ֵ                                                                 
 * ��  ��:    p_phase: ָ���ȡ���ļ���ֵ                     
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_get_phase(s32_t *p_phase)
{   
     u8_t tmp = 0;
     u32_t phase = 0;
     s8_t sign = 1;
     
     
     if (p_phase == NULL)
     {
         MTFS30_ERROR("system_get_phase()��������");
         return;
     }
     
     
     SPI_FPGA_ByteRead(&tmp, SYSTEM_PHASE_REG_ADDR0);
     
     
     
     if ((tmp & (1 << 7)))
     {
         sign = -1;
     }
     
     phase |= (tmp & 0x7F) << 16;
     
     
     SPI_FPGA_ByteRead(&tmp, SYSTEM_PHASE_REG_ADDR1);
     phase |= (tmp & 0xFF) << 8;
     
     
     SPI_FPGA_ByteRead(&tmp, SYSTEM_PHASE_REG_ADDR2);
     phase |= tmp & 0xFF;
     
     
     
     *p_phase = sign * phase;
         
}



/*****************************************************************************
 * ��  ��:    system_vlan_enable                                                        
 * ��  ��:    VLANʹ������                                                                 
 * ��  ��:    enable: ʹ��/��ʹ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_vlan_enable(u8_t enable)
{
    
    SPI_FPGA_ByteWrite((enable&0x1), SYSTEM_VLAN_ENABLE_REG_ADDR);    
}






/*****************************************************************************
 * ��  ��:    system_vlan_proi                                                        
 * ��  ��:    VLAN���ȼ�����                                                                 
 * ��  ��:    proi: ���ȼ�                         
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_vlan_proi(u8_t proi)
{
    
    SPI_FPGA_ByteWrite((proi&0x7), SYSTEM_VLAN_PROI_REG_ADDR);    
}







/*****************************************************************************
 * ��  ��:    system_vlan_cfi                                                        
 * ��  ��:    VLAN��ʽ����                                                                 
 * ��  ��:    cfi: ��ʽ                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_vlan_cfi(u8_t cfi)
{
    
    SPI_FPGA_ByteWrite(cfi, SYSTEM_VLAN_CFI_REG_ADDR);    
}

/*****************************************************************************
 * ��  ��:    system_vlan_id                                                       
 * ��  ��:    VLAN ID����                                                                 
 * ��  ��:    id: ID                         
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_vlan_id(u8_t id)
{
    u8_t tmp = 0;
    
    tmp = (id &0xFF00) >> 8;
    SPI_FPGA_ByteWrite(tmp, SYSTEM_VLAN_ID_REG_ADDR0);   
    
    tmp = id &0xFF;
    SPI_FPGA_ByteWrite(tmp, SYSTEM_VLAN_ID_REG_ADDR1);      
}

/*****************************************************************************
 * ��  ��:    system_set_mac                                                         
 * ��  ��:    MAC��ַ����                                                                 
 * ��  ��:    mac: mac��ַ                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_set_mac(u8_t mac[])
{
    
    u8_t tmp_mac = 0;
    
    
    tmp_mac = mac[0] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_mac, SYSTEM_MAC_REG_ADDR0);
    
    tmp_mac = mac[1] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_mac, SYSTEM_MAC_REG_ADDR1);  
    
    tmp_mac = mac[2] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_mac, SYSTEM_MAC_REG_ADDR2);
    
    tmp_mac = mac[3] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_mac, SYSTEM_MAC_REG_ADDR3);
    
    tmp_mac = mac[4] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_mac, SYSTEM_MAC_REG_ADDR4);
    
    tmp_mac = mac[5] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_mac, SYSTEM_MAC_REG_ADDR5);
}


/*****************************************************************************
 * ��  ��:    system_set_ip                                                         
 * ��  ��:    MAC��ַ����                                                                 
 * ��  ��:    ip: ip��ַ                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_set_ip(u8_t ip[])
{
    
    u8_t tmp_ip = 0;
    
    
    tmp_ip = ip[0] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_ip, SYSTEM_IP_REG_ADDR0);
    
    tmp_ip = ip[1] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_ip, SYSTEM_IP_REG_ADDR1);
    
    tmp_ip = ip[2] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_ip, SYSTEM_IP_REG_ADDR2);

    tmp_ip = ip[3] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_ip, SYSTEM_IP_REG_ADDR3);    
}


/*****************************************************************************
 * ��  ��:    system_set_mask                                                         
 * ��  ��:    MASK����                                                                 
 * ��  ��:    mask: MASK                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_set_mask(u8_t mask[])
{
    
    u8_t tmp_mask = 0;
    
    
    tmp_mask = mask[0] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_mask, SYSTEM_MASK_REG_ADDR0);
    
    tmp_mask = mask[1] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_mask, SYSTEM_MASK_REG_ADDR1);
    
    tmp_mask = mask[2] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_mask, SYSTEM_MASK_REG_ADDR2);

    tmp_mask = mask[3] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_mask, SYSTEM_MASK_REG_ADDR3);
}



/*****************************************************************************
 * ��  ��:    system_set_gateway                                                        
 * ��  ��:    GATEWAY����                                                                 
 * ��  ��:    gateway: GATEWAY                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_set_gateway(u8_t gateway[])
{
    
    u8_t tmp_gateway = 0;
    
    
    tmp_gateway = gateway[0] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_gateway, SYSTEM_GATEWAY_REG_ADDR0);
    
    tmp_gateway = gateway[1] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_gateway, SYSTEM_GATEWAY_REG_ADDR1);
    
    tmp_gateway = gateway[2] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_gateway, SYSTEM_GATEWAY_REG_ADDR2);

    tmp_gateway = gateway[3] & 0xFF;
    SPI_FPGA_ByteWrite(tmp_gateway, SYSTEM_GATEWAY_REG_ADDR3);    
}


/*****************************************************************************
 * ��  ��:    system_get_refactory_signal                                                       
 * ��  ��:    ��ȡ�ָ����������ź�                                                                 
 * ��  ��:    ��                         
 * ��  ��:    ��                                                   
 * ����ֵ:    OK����ȡ���ָ����������ź�; NG���޻ָ����������ź�                                                   
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
u8_t system_get_refactory_signal()
{
   
    u8_t tmp = 0;
    
    SPI_FPGA_ByteRead(&tmp, SYSTEM_REFACTORY_SIGNAL_REG_ADDR);
    
    if (tmp != 0xAA)
    {
        return NG;
    }
    else
    {
        return OK;   
    }
}


/*****************************************************************************
 * ��  ��:    system_set_leap                                                       
 * ��  ��:    ��������                                                                 
 * ��  ��:    leap������ֵ                         
 * ��  ��:    ��                                                   
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_set_leap(u8_t leap)
{
   
    SPI_FPGA_ByteWrite(leap, SYSTEM_LEAP_SEC_REG_ADDR);
}


/*****************************************************************************
 * ��  ��:    system_set_time_skew                                                      
 * ��  ��:    ����UTCʱ��ƫ��                                                                 
 * ��  ��:    skew��ʱ��ƫ��ֵ                         
 * ��  ��:    ��                                                   
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_set_time_skew(u8_t skew)
{
   
    SPI_FPGA_ByteWrite((skew&0x1F), SYSTEM_TIME_SKEW_REG_ADDR);
}


/*****************************************************************************
 * ��  ��:    system_xo_check                                                      
 * ��  ��:    �����Ӽ��                                                                 
 * ��  ��:    ��                         
 * ��  ��:    ��                                                   
 * ����ֵ:    OK������������; NG: ������LOS                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
u8_t system_xo_check()
{
   static u8_t value = 0;
   u8_t tmp = 0;
   u8_t ret = OK;
   
   SPI_FPGA_ByteWrite(value, SYSTEM_TIME_SKEW_REG_ADDR);
   SPI_FPGA_ByteRead(&tmp, SYSTEM_TIME_SKEW_REG_ADDR);
   
   if (value == ~tmp)
   {
       ret = OK;
   }
   else
   {
       ret = NG;
   }
   
   value++;

   return ret;     
}


/*****************************************************************************
 * ��  ��:    system_set_alarm_led                                                      
 * ��  ��:    ���Ƹ澯��                                                                 
 * ��  ��:    level: �澯����                        
 * ��  ��:    ��                                                   
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_set_alarm_led(u8_t level)
{
   
    SPI_FPGA_ByteWrite(level, SYSTEM_ALARM_LED_CTL_REG_ADDR);
}

/*****************************************************************************
 * ��  ��:    system_set_gnss_led                                                      
 * ��  ��:    ��������ģ������״̬��                                                                 
 * ��  ��:    state: ����ģ������״̬                       
 * ��  ��:    ��                                                   
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_set_gnss_led(u8_t state)
{
   
    SPI_FPGA_ByteWrite(state, SYSTEM_GNSS_LED_CTL_REG_ADDR);
}

/*****************************************************************************
 * ��  ��:    system_set_out_signal_type                                                      
 * ��  ��:    ��������ź�����                                                                 
 * ��  ��:    type: ����ź�����                       
 * ��  ��:    ��                                                   
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_set_out_signal_type(u8_t type)
{
   
    SPI_FPGA_ByteWrite(type, SYSTEM_OUT_SIGNAL_TYPE_REG_ADDR);
}



/*****************************************************************************
 * ��  ��:    system_set_ref_source_alarm                                                      
 * ��  ��:    ���òο�Դ�澯                                                                 
 * ��  ��:    state: ��/�޸澯                       
 * ��  ��:    ��                                                   
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void system_set_ref_source_alarm(u8_t state)
{
   
    SPI_FPGA_ByteWrite((state&0x1), SYSTEM_REF_SOURCE_ALARM_REG_ADDR);
}







