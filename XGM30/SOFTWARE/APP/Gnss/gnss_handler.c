/*****************************************************************************/
/* �ļ���:    gnss_handler.c                                                 */
/* ��  ��:    GNSS�������/��ѯ����                                          */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#include "Gnss/gnss_handler.h"
#include "Gnss/gnss_receiver.h"
#include "mtfs30.h"
#include "Debug/user_cmd.h"

#include "Debug/mtfs30_debug.h"
#include "Util/util.h"
#include "BSP/inc/bsp_spi.h"


/*******************************************************************************
/* extern varible define                      
*******************************************************************************/
extern gnss_leap_t             g_leap;                /* ����               */
extern gnss_info_t             g_gnss_info;           /* GNSS��Ϣ           */
extern u8_t                    g_gnss_init_finished_flag; /* ���ǳ�ʼ����ɱ�־ */
extern u8_t                    g_gnss_echo_switch;    /* ���������Կ���   */
extern u8_t                    g_debug_switch;        /* ���ڵ��Կ��ر���   */
extern u8_t                    g_gnss_cmd_send_flag;      /*  ��������ͱ�־ */

/*******************************************************************************
*const char .
********************************************************************************/
 const char *gnss_help = {
"Control and monitor the gnss module\n"\
"Usage:gnss <command>\r\n"                                                                  \
"Command:\r\n"                                                                              \
"  gnss (m)"\
"        (m) config the work mode of satellite receiver,[[MIX]|[GPS]|[BDS]|[GLO]|[GAL]]\r\n" \
"  gnss (m) inquiry about the working mode of satellite receiver\r\n"                       \
"         (p) [priority] in order is the priority of gps,bd,glonass,galileo\r\n"     \
"       (t) inquiry about the utc time of statellite receiver hhmmss.xx,day,month,year\r\n" \
"       (l) inquiry about the locate information of statellite receiver, hhmmss.xx,day,month,year\r\n"\
"         (s):Query the number of satellite positioning stars\r\n"                          \
"         (t):Query the uct time of satellite receiver,hhmmss.xx\r\n"                       \
"         (i):Query the latitude of satellite positioning,xxxx.xxxxxxxxx,N|S\r\n"           \
"         (o):Query the longitude of satellite positioning,xxxx.xxxxxxxxx,E|W\r\n"          \
"         (a):Query the altitude of satellite positioning\r\n"                              \
"       (p) 1pps status\r\n"                                                                \
"       (a) antenna status\r\n"                                                             \
"       (v) the firm version of satellite receiver\r\n"                                     \
"       (o) the leap value of gps\r\n"                                                      \
"         (p) leap second forecasting hhmmss,day,month,year,next leap,now leap,leap mark\r\n"
};

const char *gnss_mode[5] = {"MIX", "GPS", "BD", "GLO", "GAL"};

/*****************************************************************************
 * ��  ��:    gnss_global_variable_init                                                          
 * ��  ��:    ��ʼ��GNSSȫ�ֱ���                                                               
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void gnss_global_variable_init(void)
{
  
    /* ���ǳ�ʼ����ɱ�־ */
    g_gnss_init_finished_flag = 0;
    
    /* �û���������ģ�������־ */
    g_gnss_cmd_send_flag = 0;
    
    /* ���������Կ��� */
    g_gnss_echo_switch = 0;
    
    /* ���� */
    g_leap.now_leap = 18; /* ��ǰ���� */
    g_leap.next_leap = 18; /* �´����� */
    g_leap.sign = 0;      /* 0�� ������Ԥ��; 1:������; 2�������� */
    
    
    /* GNSS����ģʽ */
    strcpy((char *)g_gnss_info.mode.info, "MIX;"); 
//    g_rev_info.ac_mode = REV_ACMODE_MIX;
    
    /* ������Ϣ */
    strcpy((char *)g_gnss_info.leap.info, "235960,31,12,2016,18,18,00;");
    
    /* ʱ����Ϣ */
    strcpy((char *)g_gnss_info.time.info, "000000.00,00,00,0000;");
    
    /* ��λ��Ϣ */
    strcpy((char *)g_gnss_info.loc.info, "00,000000.00,0000.00000000,N,00000.00000000,E,000.0000;");
    
    /* ������Ϣ */
    //memset(&g_gnss_info.sat, 0x00, sizeof(g_gnss_info.sat));
    strcpy((char *)g_gnss_info.sat.info, "GPS,00;BDS,00;GLO,00;GAL,00;");
    g_gnss_info.sat.len = 0;    
    
    /* 1PPS״̬��Ϣ */
    strcpy((char *)g_gnss_info.pps.info, "0;");  
    
    /* ����״̬��Ϣ */
    strcpy((char *)g_gnss_info.ant.info, "ON;");
    
    /* �汾��Ϣ */
    strcpy((char *)g_gnss_info.ver.info, "UT4B0,000000000000000;");  
    
    /* leap �·�*/
    
    system_set_leap(g_leap.now_leap);

}




/*****************************************************************************
 * ��  ��:    gnss_info_print                                                         
 * ��  ��:    ��ӡGNSS��Ϣ                                                               
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void gnss_info_print(void)
{
    u8_t buf[400];
    
    /* ��ӡGNSS����ģʽ��Ϣ */
    gnss_receiver_get_info("MODE", buf);
    MTFS30_TIPS("GNSS����ģʽ: %s", buf);
    
    /* ��ӡʱ����Ϣ */
    gnss_receiver_get_info("TIME", buf);
    MTFS30_TIPS("ʱ����Ϣ: %s", buf);    
    
    /* ��ӡ��λ��Ϣ */
    gnss_receiver_get_info("LOC", buf);
    MTFS30_TIPS("��λ��Ϣ: %s", buf);
    
    /* ��ӡ������Ϣ */
    gnss_receiver_get_info("SAT", buf);
    MTFS30_TIPS("������Ϣ: %s", buf);
    
    /* ��ӡ������Ϣ */
    gnss_receiver_get_info("LEAP", buf);
    MTFS30_TIPS("������Ϣ: %s", buf);
    
    /* ��ӡ1PPS״̬��Ϣ */
    gnss_receiver_get_info("PPS", buf);
    MTFS30_TIPS("1PPS״̬��Ϣ: %s", buf);
    
    /* ��ӡ����״̬��Ϣ */
    gnss_receiver_get_info("ANT", buf);
    MTFS30_TIPS("����״̬��Ϣ: %s", buf);
    
    /* ��ӡGNSS�汾��Ϣ */
    gnss_receiver_get_info("VER", buf);
    MTFS30_TIPS("GNSS�汾��Ϣ: %s", buf);

    MTFS30_TIPS("\n");
}


/*****************************************************************************
 * ��  ��:    gnss_stmt_echo                                                         
 * ��  ��:    ����������                                                               
 * ��  ��:    p_msg: ָ�����������Ϣ                         
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void gnss_stmt_echo(u8_t *p_msg)
{
    u8_t buf[200] = {0};
    u8_t *p = p_msg;
    u8_t *q = NULL;

    
    /* ���ÿһ��������� */
    while(*p != '\0')
    {    
        q = buf;
//        while((*q++ = *p++) != '');
        while(*p != '\n')
        {
            *q++ = *p++;
        }
        *q = '\0';
        p++;
        
        MTFS30_TIPS(buf);

    }
    
}

/*****************************************************************************
 * ��  ��:    gnss_init                                                           
 * ��  ��:    GNSSģ���ʼ��                                                                  
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/ 
void gnss_init(void)
{
    /* GNSS���ջ���ʼ�� */
    gnss_receiver_init();
}



/*****************************************************************************
 * ��  ��:    gnss_acmode_set_handler                                                           
 * ��  ��:    ���ý��ջ�����ģʽ                                                                
 * ��  ��:    p_param  : �������                      
 * ��  ��:    ��                                                    
 * ����ֵ:    OK: ���óɹ��� NG:����ʧ��                                                   
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                   
 ****************************************************************************/  
u8_t gnss_acmode_set_handler(int num,...)
{
    u8_t *p_param = NULL;
    u8_t  err_flag = OK;
    va_list valist;
    
    va_start(valist, num);
    p_param = (u8_t *)va_arg(valist, char *);
    
    if(p_param != NULL)
      err_flag = gnss_receiver_acmode_set(p_param);
    
    va_end(valist);
    
    return err_flag;
   // return gnss_receiver_acmode_set(p_param);
}

/*****************************************************************************
 * ��  ��:    gnss_serial baud_set_handler                                                           
 * ��  ��:    �������Ǵ��ڲ�����                                                                
 * ��  ��:    p_param  : �������
 *            presult: ���ý����־(�ɹ�|ʧ��)                         
 * ��  ��:    ��                                                    
 * ����ֵ:    OK: ���óɹ�; NG: ����ʧ��                                                
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                   
 ****************************************************************************/
u8_t gnss_serial_baud_set_handler(int num,...)
{
    u8_t *p_param = NULL;
    u8_t  err_flag = OK;
    va_list valist;
    
    va_start(valist, num);
    p_param = (u8_t *)va_arg(valist, char *);
    
    if(p_param != NULL)
      err_flag = gnss_receiver_serial_baud_set(p_param);
    va_end(valist);
    
    return err_flag;
    //return gnss_receiver_serial_baud_set(p_param);
}

u8_t gnss_helper(int num,...)
{
    u8_t *p_param[MAX_PARAM_NUM] = NULL;
    u8_t  err_flag = OK;
    u8_t  inx;
    va_list valist;
    
    va_start(valist, num);
    for(inx = 0; inx < num; inx++)
    {
        p_param[inx] = (u8_t *)va_arg(valist, char *);
        if(p_param[inx] == NULL)
          err_flag = NG;
    }
    va_end(valist);
    
    if(err_flag == OK)
    if(0== strncmp((char const*)p_param[0], "help", strlen("help")))
    {
      //strncpy(p_param[1], gnss_help, strlen(gnss_help) );
      DEBUG_USART_Send((u8_t *)gnss_help, strlen(gnss_help));
    }
    
    return err_flag;
    //return gnss_receiver_serial_baud_set(p_param);
}


u8_t gnss_command_execute(int num,...)
{
    u8_t *p_param[MAX_PARAM_NUM] = NULL;
    u8_t  err_flag = OK;
    u8_t  inx;
    
    va_list valist;
    
    va_start(valist, num);
    for(inx = 0; inx < num; inx++)
    {
        p_param[inx] = (u8_t *)va_arg(valist, char *);
        if(p_param[inx] == NULL)
          err_flag = NG;
    }
    va_end(valist);
    
    if(err_flag == OK)
    {
       if(num <= GNSS_COMMAND_RTRV)
       {
          switch(*p_param[0])
          {
               case 'm':
                 {
                     if(*p_param[1] == 'm')
                     { 
                       inx = g_sParameters.GnssParameters.mode;
                       sprintf(p_param[num -1], "%s", gnss_mode[inx-1]); 
                       MTFS30_TIPS("gnss work mode:%s\n", p_param[num -1]);
                     }
                     else if(*p_param[1] == 'p')
                     {
                       sprintf(p_param[num -1], "%s", g_sParameters.GnssParameters.priority); 
                       MTFS30_TIPS("gnss work mode priority:%s\n", p_param[num -1]);
                     }
                     else
                       MTFS30_TIPS("����(%s)����!\n", p_param);
                     break;
                 }
               case 't':
                 {
                     sprintf(p_param[num -1], "%s", g_sParameters.GnssParameters.priority); 
                     MTFS30_TIPS("gnss work mode priority:%s\n", p_param[num -1]);
                     
                     break;
                 }
       }
    }
  }
//    if(0== strncmp((char const*)p_param[0], "m", strlen("help")))
//    {
//      //strncpy(p_param[1], gnss_help, strlen(gnss_help) );
//      DEBUG_USART_Send((u8_t *)gnss_help, strlen(gnss_help));
//    }
    
    return err_flag;
    //return gnss_receiver_serial_baud_set(p_param);
}

/*****************************************************************************
 * ��  ��:    gnss_echo_set_handler                                                           
 * ��  ��:    ��/�ر�ָ�����Ļ��Թ���                                                                
 * ��  ��:    p_param  : �������                 
 * ��  ��:    ��                                                    
 * ����ֵ:    OK: ���óɹ�; NG: ����ʧ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                   
 ****************************************************************************/
u8_t gnss_echo_set_handler(int num,...)
{  
    u8_t *p_param = NULL;
    u8_t  err_flag = OK;
    va_list valist;
    
    va_start(valist, num);
    p_param = (u8_t *)va_arg(valist, char *);
    
    if(p_param != NULL)
    {
      if (0 == strncmp((char const *)p_param, "OFF", strlen("OFF")))
      {
          /* �ر�ѭ����ӡ */
          g_gnss_echo_switch = 0;
      }
      else if (0 == strncmp((char const *)p_param, "ON", strlen("ON")))
      {
          /* ��ѭ����ӡ */
          g_gnss_echo_switch = 1;
      }
      else
      {
          MTFS30_TIPS("����(%s)����!\n", p_param);  
          err_flag = NG;
      }
    }
    va_end(valist);
    
    return err_flag;
    //return gnss_receiver_echo_set(p_param);
}


/*****************************************************************************
 * ��  ��:    gnss_stmt_set_handler                                                           
 * ��  ��:    ��/�ر�GNSS�忨���ָ�����                                                               
 * ��  ��:    p_param  : �������
 * ��  ��:    ��                                                    
 * ����ֵ:    OK: ���óɹ�; NG: ����ʧ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                   
 ****************************************************************************/
u8_t gnss_stmt_set_handler(int num,...)
{
    u8_t *p_param = NULL;
    u8_t  err_flag = OK;
    va_list valist;
    
    va_start(valist, num);
    p_param = va_arg(valist, char *);
    
    if(p_param != NULL)
      err_flag = gnss_receiver_stmt_set(p_param);
    va_end(valist);
    
    return err_flag;
    //return gnss_receiver_stmt_set(p_param);
}



/*****************************************************************************
 * ��  ��:    gnss_get_handler                                                           
 * ��  ��:    ��ѯGNSSģ��ָ����Ϣ                                                               
 * ��  ��:    p_param : �������           
 * ��  ��:    p_rtrv: ��ѯ���                                           
 * ����ֵ:    OK: ��ѯ�ɹ�; NG: ��ѯʧ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                   
 ****************************************************************************/
u8_t gnss_get_info_handler(u8_t *p_param, u8_t *p_rtrv)
{
    return gnss_receiver_get_info(p_param, p_rtrv);
}

/*****************************************************************************
 * funtion:       gnss_inquery_set                                                        
 * description:   description gnss help information  and gnss receiver information                                                              
 * parameters:    command1,command2, tmp_buffer                          
 * return :       inquery result                                                                                          
 ****************************************************************************/
u8_t gnss_inquery_set(u8_t *p_param, u8_t *p_rtrv)
{
    return gnss_receiver_get_info(p_param, p_rtrv);
    
}
/*****************************************************************************
 * ��  ��:    gnss_set_gps_pps_offset                                                     
 * ��  ��:    ��������gps�źŵ���ʱ����                                                             
 * ��  ��:    offset: ��ʱ����                     
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void gnss_set_gps_pps_offset(u32_t offset)
{   
     u8_t tmp = 0;
     
     tmp = (offset & 0x0F000000) >> 24;
     SPI_FPGA_ByteRead(&tmp, GNSS_GPS_PPS_OFFSET_REG_ADDR0);
     
     tmp = (offset & 0xFF0000) >> 16;
     SPI_FPGA_ByteRead(&tmp, GNSS_GPS_PPS_OFFSET_REG_ADDR1); 
     
     
     tmp = (offset & 0xFF00) >> 8;
     SPI_FPGA_ByteRead(&tmp, GNSS_GPS_PPS_OFFSET_REG_ADDR2);
     
     
     tmp = offset & 0xFF;
     SPI_FPGA_ByteRead(&tmp, GNSS_GPS_PPS_OFFSET_REG_ADDR3);     
        
}