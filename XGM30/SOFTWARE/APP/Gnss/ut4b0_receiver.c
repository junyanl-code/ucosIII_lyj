/*****************************************************************************/
/* �ļ���:    ut4b0_receiver.c                                               */
/* ��  ��:    UT4B0���ջ���ش���                                            */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#include "Gnss/ut4b0_receiver.h"
#include "Gnss/gnss_receiver.h"
#include "Util/util_check.h"
#include "BSP/inc/bsp_usart.h"
#include "Debug/mtfs30_debug.h"
#include "string.h"
#include "arch/cc.h"
#include "System/system.h"

#include "os_type.h"
#include "cpu.h"

/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
//u8_t   gut4b0_config_cmd[UT4B0_CONFIG_CMD_MAX_LEN] = {0};      /* ���ڱ��淢�͸�UT4B0���������� */
static ut4b0_combcmd_t   g_save_cmd;   /* ���ڱ��淢�͸�UT4B0���������� */

/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/

extern u8_t                    g_gnss_echo_switch;  /* �����Կ���       */
extern OS_SEM                  g_ut4b0_reply_sem;   /* ����ظ��ź���     */
extern gnss_info_t             g_gnss_info;         /* GNSS��Ϣ           */
extern gnss_recviver_info_t    g_rev_info;          /* GNSS���ջ���Ϣ     */ 
extern gnss_leap_t             g_leap;              /* ����               */
extern u8_t                    g_gnss_echo_switch;  /* ���������Կ���   */
extern OS_SEM                  g_gnss_idle_sem;     /* GNSS�����ж��ź��� */
extern u8_t                    g_gnss_cmd_send_flag;/*  ��������ͱ�־  */


/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
static void ut4b0_check_cmdhandler(u8_t *pdata);
static void ut4b0_zda_msg_parser(u8_t *pdata);
static void ut4b0_gga_msg_parser(u8_t *pdata);
static void ut4b0_gsv_msg_parser(u8_t *pdata, u8_t type);
static void ut4b0_gpgsv_msg_parser(u8_t *pdata);
static void ut4b0_bdgsv_msg_parser(u8_t *pdata);
static void ut4b0_glgsv_msg_parser(u8_t *pdata);
static void ut4b0_gagsv_msg_parser(u8_t *pdata);
static void ut4b0_gpsbds_utc_msg_parser(u8_t *pdata);
static void ut4b0_gal_utc_msg_parser(u8_t *pdata);
//static void ut4b0_leap_forecast(u8_t sat_type, u32_t wn, u8_t dn, u8_t now_leap,u8_t next_leap, u8_t sign);
static void ut4b0_leap_process(u8_t sat_type, u32_t wn, u8_t dn, u8_t now_leap, u8_t next_leap);
static void ut4b0_get_leap_time(u8_t sat_type, u32_t wn, u8_t dn, u8_t sign, gnss_time_t *p_time);
static u8_t ut4b0_checkcode_compare(u8_t *pmsg, u8_t *pcode, u8_t check_type);
static void ut4b0_1pps_msg_parser(u8_t *pdata);
static void ut4b0_cmd_reply_msg_parser(u8_t *pdata);
static u8_t ut4b0_receiver_configuration(u8_t *pcmd);
static void ut4b0_antenna_msg_parser(u8_t *pdata);
static void ut4b0_cmd_version_msg_parser(u8_t *pdata);
static u8_t ut4b0_receiver_utc_statement_set(void);
static void ut4b0_receiver_cmd_save(u8_t *p_cmd);


/* ut4b0���ջ��������ָ���� */
static gnss_stmt_handle_t ut4b0_stmt_handle[] =
{
	/* UTC��Ϣ */
	{(u8_t *)"#GPSUTC",     CHECK_TYPE_CRC32,    ut4b0_gpsbds_utc_msg_parser},
	{(u8_t *)"#BDSUTC",     CHECK_TYPE_CRC32,    ut4b0_gpsbds_utc_msg_parser},
	{(u8_t *)"#GALUTC",     CHECK_TYPE_CRC32,    ut4b0_gal_utc_msg_parser},
    
    /* �汾��Ϣ */
       {(u8_t *)"#VERSIONA",    CHECK_TYPE_CRC32,    ut4b0_cmd_version_msg_parser},

    /* ����״̬ */
       {(u8_t *)"#ANTENNAA",    CHECK_TYPE_CRC32,    ut4b0_antenna_msg_parser},
    
	/* GGA��Ϣ */
	{(u8_t *)"$GNGGA",      CHECK_TYPE_XOR,      ut4b0_gga_msg_parser},
	{(u8_t *)"$GPSGGA",     CHECK_TYPE_XOR,      ut4b0_gga_msg_parser},
        {(u8_t *)"$GPGGA",      CHECK_TYPE_XOR,      ut4b0_gga_msg_parser},
        {(u8_t *)"$BDGGA",      CHECK_TYPE_XOR,      ut4b0_gga_msg_parser},    
	{(u8_t *)"$BDSGGA",     CHECK_TYPE_XOR,      ut4b0_gga_msg_parser},
	{(u8_t *)"$GLOGGA",     CHECK_TYPE_XOR,      ut4b0_gga_msg_parser},
	{(u8_t *)"$GALGGA",     CHECK_TYPE_XOR,      ut4b0_gga_msg_parser}, 
    
    /* ʱ����Ϣ */
        {(u8_t *)"$GNZDA",      CHECK_TYPE_XOR,      ut4b0_zda_msg_parser},    
        {(u8_t *)"$GPZDA",      CHECK_TYPE_XOR,      ut4b0_zda_msg_parser},
        {(u8_t *)"$BDZDA",      CHECK_TYPE_XOR,      ut4b0_zda_msg_parser},    

	/* GSV��Ϣ */
	{(u8_t *)"$GPGSV",      CHECK_TYPE_XOR,      ut4b0_gpgsv_msg_parser},
	{(u8_t *)"$BDGSV",      CHECK_TYPE_XOR,      ut4b0_bdgsv_msg_parser},
	{(u8_t *)"$GLGSV",      CHECK_TYPE_XOR,      ut4b0_glgsv_msg_parser},
	{(u8_t *)"$GAGSV",      CHECK_TYPE_XOR,      ut4b0_gagsv_msg_parser},	

   
    /* 1PPS */
        {(u8_t *)"#BBPPSMSGAA", CHECK_TYPE_CRC32,    ut4b0_1pps_msg_parser},
            
    /* ����ظ� */
        {(u8_t *)"$command",    CHECK_TYPE_XOR,      ut4b0_cmd_reply_msg_parser},
    
};

/*****************************************************************************
 * ��  ��:    ut4b0_receiver_init                                                           
 * ��  ��:    ut4b0���ջ���ʼ��                                                                
 * ��  ��:     *pdata: ��������                         
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/  
void ut4b0_receiver_init(void)
{
    
     /* ��ʼ��ut4b0���ջ����ڲ����� */
     GNSS_USART_Send("CONFIG COM1 115200"); /* ʹ������ģ�鴮�ڷ������� */
     BSP_Sleep(10);
     
     /* PPS�������� */
     GNSS_USART_Send("CONFIG PPS ENABLE GPS POSITIVE 500000 1000 0 0");
 
     /* ��ʼ��ut4b0���ջ�����ģʽ */
     GNSS_USART_Send("MASK -");
     

    
    /* ��ʼ��ut4b0���ջ������������� */
    GNSS_USART_Send("UNLOG"); /* ��ֹ���������� */
    GNSS_USART_Send("GPSUTCA 1\r\nBDSUTCA 1\r\nGALUTCA 1\r\nVERSIONA 1\r\nANTENNAA 1\r\nGNGGA 1\r\nGPZDA 1\r\nGPGSV 1\r\nBBPPSMSGA ONCHANGED\r\n");
       
}


/*****************************************************************************
 * ��  ��:    ut4b0_receiver_msg_parser                                                           
 * ��  ��:    ����ut4b0���ջ��������������                                                                 
 * ��  ��:     *pdata: ��������                         
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void ut4b0_receiver_msg_parser(u8_t *pdata)
{
	u8_t *p = pdata;
	u8_t *p_msg  = NULL;
	
	while(*p != '\0')
	{	
		//������׼���
		if(*p == '$' || *p == '#')
		{	
			p_msg  = (u8_t*)strtok((char*)p, "\n");
			p += strlen((char*)p_msg); 		  			//������\0��
			
			if (NULL != strchr((const char *)p_msg, '*') &&
				NULL != strchr((const char *)p_msg, '\r')) 
			{
				
                /* ���Ҳ�ִ�и�������ָ��Ľ������� */
       		    ut4b0_check_cmdhandler(p_msg);  
			}
		}			
		p++;
	}		

}


/*****************************************************************************
 * ��  ��:    ut4b0_checkcode_compare                                                           
 * ��  ��:    ��������ָ�����ݲ��ֵ�У���벢�������ָ���е�У������бȽ�                                                              
 * ��  ��:    *pmsg      : ����ָ�����ݲ���   
 *            *pcode     : ����ָ���е�У����
 *            check_type : У�鷽ʽ
 *                         CHECK_TYPE_XOR  : ���У��
 *                         CHECK_TYPE_CRC32: crc32У��
 * ��  ��:    ��                                                    
 * ����ֵ:    CHECKCODE_EQ: У������ͬ; CHECKCODE_NOEQ: У���벻ͬ                                                   
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t ut4b0_checkcode_compare(u8_t *pmsg, u8_t *pcode, u8_t check_type)
{
    
    u8_t ret = CHECKCODE_NOEQ;
   
    /* �ж�У�鷽ʽ */
    switch(check_type)
    {
    case CHECK_TYPE_XOR:    /* ���У�� */
        ret = util_xor_check(pmsg, strlen((char *)pmsg), pcode);
        break;
        
    case CHECK_TYPE_CRC32: /* crc32У�� */
        ret = util_check_crc32(pmsg, strlen((char *)pmsg), pcode);
        break;
    
    default:
	MTFS30_ERROR("У�鷽ʽ(%d)����\n", check_type);   
        break;
    }


    return ret;
}

/*****************************************************************************
 * ��  ��:    ut4b0_check_cmdhandler                                                           
 * ��  ��:    ����UT4B0���ջ�����������Ľ�������������                                                              
 * ��  ��:     *pdata: ��������                         
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/  
static void ut4b0_check_cmdhandler(u8_t *pdata)
{
    u8_t i = 0;
    u8_t cnt = 0;        /* ���������������   */
    u8_t *pmsg = NULL;       /* "*"ǰ������        */
    u8_t *pcode = NULL;      /* ָ��У����         */
    u8_t *pcheck_msg = NULL; /* ָ��μ�У������� */


	/* �������� */
	cnt = sizeof(ut4b0_stmt_handle) / sizeof(ut4b0_stmt_handle[0]);

	/* ����ָ����� */
	for (i = 0; i < cnt; i++)
	{
        /* �ҵ������ */
	   if(strncmp((char *)ut4b0_stmt_handle[i].p_head, (char*)pdata, strlen((char *)ut4b0_stmt_handle[i].p_head)) == 0)
	   {
            /* ���������GPSUTC���Ϊ��ͷ������GPSUTC�������µ�һ��Ŀ�ʼ, �ڴ������һ�뱣���GNSS���� */ 
            if (0 == i) 
            {
                gnss_receiver_info_clear();   
            }
            
             /* ��ȡ��"*"��ǰ������ */           			
            pmsg = (u8_t*)strtok((char *)pdata, "*");
        
            /* ��ȡ��У���� */
            pcode =(u8_t*)strtok(NULL, "\r");
            
            if (pmsg == NULL || pcode == NULL)
            {
                return;
            }
            
       
            /* "command"��������"$"����"#"�����μ�У�� */
            pcheck_msg = pmsg;
            if (0 != strncmp((char const *)pmsg, "$command", 8))
            {
             
                pcheck_msg = pmsg + 1;
            }
            
            /* ����У�� */
            if (CHECKCODE_EQ == ut4b0_checkcode_compare(pcheck_msg, pcode, ut4b0_stmt_handle[i].check_type)) /* У����һ�� */
            {
                pmsg++;
			    /* ���ý������� */
			    ut4b0_stmt_handle[i].parse_func(pmsg);
			    
            }
            else /* У���벻һ�� */
            {
               MTFS30_ERROR("(%s)У���벻һ��", pdata);
            }
            
            return;
		}
	} 


    /* û���ҵ������ */
	u8_t *pcmd = pdata;
	pcmd = (u8_t *)strtok((char *)pdata, ",");
	MTFS30_ERROR("������(%s)����", pcmd);
	

}

/*****************************************************************************
 * ��  ��:    ut4b0_zda_msg_parser                                                           
 * ��  ��:    ����UT4B0���ջ����ZDA���                                                                
 * ��  ��:    *pdata: ZDA�����Ϣ;                       
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void ut4b0_zda_msg_parser(u8_t *pdata)
{
    
    u8_t i = 0;
//    u8_t *p = NULL;
    u8_t comma_cnt = 0;   /* ���Ÿ���             */
    u8_t index[10] ={0};  /* ��������             */   
    u8_t len = 0;         /* �������Ϣ����       */    

    
    /* ��ȡ��ÿ������������ */    
    /* index: 0         1  2  3         
     * $GPZDA,060014.00,28,01,2019,,*64 
     */
    for(i = 0, comma_cnt = 0; comma_cnt < 5; i++)
    {
        if (',' == pdata[i])
        {
            index[comma_cnt] = i + 1;
            comma_cnt++;
        }
    }
    
    /* ���ZDA��Ϣ�Ƿ���Ч */
    /* ��Ч���ݵ���������֮��û������,��: $GPZDA,,,,,,*48 */
    if (',' == pdata[index[0]])
    {
        strcpy((char *)g_gnss_info.time.info, "000000.00,00,00,0000;");
        return;
    }
    
    /* ����ʱ����Ϣ */
    for(i = 0; i < 4; i++)
    {
        memcpy(&g_gnss_info.time.info[len], &pdata[index[i]], index[i+1]-index[i]);
        len += index[i+1]-index[i];
    
    }
    
    /* �����Ķ��Ż��ɷֺ� */
    g_gnss_info.time.info[len - 1] = ';';
    g_gnss_info.time.info[len] = '\0';
    
    
    MTFS30_DEBUG("ʱ����Ϣ: %s", g_gnss_info.time.info);
    
       
}

/*****************************************************************************
 * ��  ��:    ut4b0_gga_msg_parser                                                           
 * ��  ��:    ����UT4B0���ջ����GGA��Ϣ                                                                
 * ��  ��:    *pdata: XXXGGA��Ϣ                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/  
static void ut4b0_gga_msg_parser(u8_t *pdata)
{

    
    u8_t i = 0; 
    u8_t comma_cnt = 0;   /* ���Ÿ���             */
    u8_t index[10] ={0};  /* ��������             */   
    u8_t len = 0;         /* �������Ϣ����       */

    /* ��ȡ��ÿ��GGA���������� */    
    /* index: 0         1             2 3              4 5 6  7   8        9 10       11
     * $GNGGA,044426.00,3032.49556656,N,10403.16548096,E,1,15,0.9,529.1063,M,-42.4491,M,,*5D 
     */
    for(i = 0, comma_cnt = 0; comma_cnt < 10; i++)
    {
        if (',' == pdata[i])
        {
            index[comma_cnt] = i + 1;
            comma_cnt++;
        }
    }
    
    /*--------------------------------*/
    /* ���涨λ��Ϣ                   */
    /*--------------------------------*/     
    /* ʹ���е������� */
    memcpy(&g_gnss_info.loc.info[len], &pdata[index[6]], index[7]-index[6]);
    len += index[7]-index[6];
    
    g_rev_info.post_sats = (pdata[index[6]] - '0') * 10 + (pdata[index[6]+1] - '0');
    MTFS30_DEBUG("��λ����:%d", g_rev_info.post_sats);
    if (0 == g_rev_info.post_sats)
    {
        /* ��λ����Ϊ0����������������Ϣ */
        strcpy((char *)g_gnss_info.sat.info, "GPS,00;BDS,00;GLO,00;GAL,00;");
        g_gnss_info.sat.len = 0;

        return;        
    }

          
    /* UTCʱ�� */
    memcpy(&g_gnss_info.loc.info[len], &pdata[index[0]], index[1]-index[0]);
    len += index[1]-index[0];
    
    /* γ�� */
    memcpy(&g_gnss_info.loc.info[len], &pdata[index[1]], index[2]-index[1]);
    len += index[2]-index[1];
    
    /* γ�ȷ��� */
    memcpy(&g_gnss_info.loc.info[len], &pdata[index[2]], index[3]-index[2]);
    len += index[3]-index[2];
    
    /* ���� */
    memcpy(&g_gnss_info.loc.info[len], &pdata[index[3]], index[4]-index[3]);
    len += index[4]-index[3];

    /* ���ȷ��� */
    memcpy(&g_gnss_info.loc.info[len], &pdata[index[4]], index[5]-index[4]);
    len += index[5]-index[4];    
    
    /* ���� */
    memcpy(&g_gnss_info.loc.info[len], &pdata[index[8]], index[9]-index[8]); 
    len += index[9]-index[8];
    
    
    /* �����Ķ��ű�Ϊ�ֺ� */
    g_gnss_info.loc.info[len - 1] = ';';
    g_gnss_info.loc.info[len] = '\0';   
    

    
    MTFS30_DEBUG("��λ��Ϣ:%s", g_gnss_info.loc.info);

}

/*****************************************************************************
 * ��  ��:    ut4b0_gpgsv_msg_parser                                                           
 * ��  ��:    ����UT4B0���ջ����GPGSV��Ϣ                                                                 
 * ��  ��:    *pdata: GPGSV��Ϣ;                       
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/  
static void ut4b0_gpgsv_msg_parser(u8_t *pmsg)
{
    ut4b0_gsv_msg_parser(pmsg, GNSS_SAT_TYPE_GPS);
}

/*****************************************************************************
 * ��  ��:    ut4b0_bdgsv_msg_parser                                                           
 * ��  ��:    ����UT4B0���ջ����BDGSV��Ϣ                                                                 
 * ��  ��:    *pdata: BDGSV��Ϣ;                       
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/ 
static void ut4b0_bdgsv_msg_parser(u8_t *pmsg)
{
    ut4b0_gsv_msg_parser(pmsg, GNSS_SAT_TYPE_BDS);
}

/*****************************************************************************
 * ��  ��:    ut4b0_glgsv_msg_parser                                                           
 * ��  ��:    ����UT4B0���ջ����GLGSV��Ϣ                                                                 
 * ��  ��:    *pdata: GLGSV��Ϣ;                       
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/ 
static void ut4b0_glgsv_msg_parser(u8_t *pmsg)
{
    ut4b0_gsv_msg_parser(pmsg, GNSS_SAT_TYPE_GLO);
}

/*****************************************************************************
 * ��  ��:    ut4b0_gagsv_msg_parser                                                           
 * ��  ��:    ����UT4B0���ջ����GAGSV��Ϣ                                                                 
 * ��  ��:    *pdata: GAGSV��Ϣ;                       
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/ 
static void ut4b0_gagsv_msg_parser(u8_t *pmsg)
{
    ut4b0_gsv_msg_parser(pmsg, GNSS_SAT_TYPE_GAL);
}
/*****************************************************************************
 * ��  ��:    ut4b0_gsv_msg_parser                                                           
 * ��  ��:    ����UT4B0���ջ����GSV��Ϣ                                                                 
 * ��  ��:    *pdata: GSV��Ϣ;   
 *             type: ��������
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/ 
static void ut4b0_gsv_msg_parser(u8_t *pdata, u8_t type)
{
	u8_t  i = 0;
	u8_t  index[20] ={0};    /* ��������            */
    u8_t  comma_cnt = 0;     /* ���Ÿ���            */
    u8_t  msg_total = 0;     /* ��Ϣ����            */
	u8_t  msg_cur = 0;       /* ��ǰ��Ϣ��          */
	u8_t  amount = 0;        /* �ɼ�����������      */
    u8_t data[GNSS_GSV_MSG_MAX_LEN] = {0}; /* ���GSV���� */
    u16_t len = 0;

    
    


    /* ��������һ��','��Ϊ��ȡ��','ǰ������� */
    len = strlen((const char *)pdata);
	if (len > GNSS_GSV_MSG_MAX_LEN - 2)
	{
	    len = GNSS_GSV_MSG_MAX_LEN - 2;
	}
    strncpy((char *)data, (const char *)pdata, len);
    data[len++] = ',';

    
    /* ȡ��ÿ������������ */
    /* index: 0 1 2  3  4  5   6  7  8  9   10 11 12 13  14 15 16 17  18 19 
     * $GPGSV,2,1,07,25,44,074,36,31,55,256,37,12,19,045,36,14,53,335,36,\0
     */    
    while(data[i])
    {
        
        if((',' == data[i]))
        {
            index[comma_cnt] = i + 1;
            comma_cnt++;
        }
        
        i++;
    }
    
    
 
    /* �ɼ����������� */
    amount = (data[index[2]] - '0') * 10 + (data[index[2]+1] - '0');
    if (amount == 0)
    {
        strcpy((char *)g_gnss_info.sat.info, "GPS,00;BDS,00;GLO,00;GAL,00;");
        return;
    }
    

	/* ��Ϣ���� */
    msg_total = data[index[0]];
	

	/* ��ǰ��Ϣ�� */
	msg_cur = data[index[1]]; 
    if (msg_cur == '1')
    {
    
        switch(type)
        {
        case GNSS_SAT_TYPE_GPS:
            sprintf((char *)&g_gnss_info.sat.info[g_gnss_info.sat.len], "GPS,%02d,", amount);
            
            break;
        case GNSS_SAT_TYPE_BDS:
            sprintf((char *)&g_gnss_info.sat.info[g_gnss_info.sat.len], "BDS,%02d,", amount);
            break;
        case GNSS_SAT_TYPE_GLO:
            sprintf((char *)&g_gnss_info.sat.info[g_gnss_info.sat.len], "GLO,%02d,", amount);
            break;
        case GNSS_SAT_TYPE_GAL:
            sprintf((char *)&g_gnss_info.sat.info[g_gnss_info.sat.len], "GAL,%02d,", amount);
            break;
            
        default:
            break;   
        }
        
        g_gnss_info.sat.len += 7;
    }
    

	
    /* ȡ��ÿһ�����ǵ���Ϣ */
    for(i = 3; i < comma_cnt - 1; i++)
    {
        /* �汱��λ�ǣ������� */
        if (i == 5 || i == 9 || i == 13 || i == 17)
        {
            continue;
        }
    
        /* ��������Ƿ���� */
        if (i == 6 || i == 10 || i == 14 || i == 18) 
        {
            /* �����Ϊ0�����ǲ����� */
            if (data[index[i]] == '0' && data[index[i]+1] == '0')  
            {
                //g_gnss_info.sat.info[g_gnss_info.sat.len++] = '0';   
                strcpy((char *)&g_gnss_info.sat.info[g_gnss_info.sat.len], "0,");
            }
            else
            {
                strcpy((char *)&g_gnss_info.sat.info[g_gnss_info.sat.len], "1,"); 
            }
            
            g_gnss_info.sat.len += 2;
        }
        
        /* ���Ǳ�š����� */

        memcpy(&g_gnss_info.sat.info[g_gnss_info.sat.len], &data[index[i]], index[i+1]-index[i]);
        g_gnss_info.sat.len += index[i+1]-index[i];
            
    }
    
    /* ��ǰ��Ϣ��������Ϣ����������һ�����͵�������Ϣ��ȡ��� */
    if (msg_cur == msg_total)
    {
        strcpy((char *)&g_gnss_info.sat.info[g_gnss_info.sat.len-1], ";");
//      MTFS30_DEBUG("#####������Ϣ[%s]     ����[%d]", g_gnss_info.sat.info, g_gnss_info.sat.len);
    }
    
   
     
}   


/*****************************************************************************
 * ��  ��:    ut4b0_gpsbds_utc_msg_parser                                                           
 * ��  ��:    ����UT4B0���ջ������UTC��Ϣ����������Ԥ��(GPS/BDS����)                                                                 
 * ��  ��:    *pdata: UTC��Ϣ;                       
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void ut4b0_gpsbds_utc_msg_parser(u8_t *pdata)
{


    u8_t i = 0; 
    u8_t comma_cnt = 0;     /* ���Ÿ���                     */
    u8_t param_len = 0;     /* ����ÿ�������ĳ���           */
    u8_t param_buf[20]= {0};/* ����ÿ������                 */
    u32_t wn = 0;           /* �µ�������Ч���ܼ���         */
    u8_t  dn = 0;           /* �µ�������Ч�������ռ���     */
    u8_t now_leap = 0;      /* ��ǰ����                     */
    u8_t next_leap = 0;     /* �µ�����                     */
    u8_t sate_type = 0;     /* ��������                     */


  
    /* �Թ�Logͷ */
	while(pdata[i++] != ';');
      
    /* ����ÿ������ */
    while(pdata[i])
    {
        if (pdata[i] == ',')
        {
            /* ���Ÿ�����1 */
            comma_cnt++; 
            
            param_buf[param_len]='\0';
            switch(comma_cnt)
            {
            case 5:    /* �µ�������Ч���ܼ���     */
                wn = (u32_t)atoi((char const *)param_buf);
                break;
            case 6:    /* �µ�������Ч�������ռ��� */
                dn = (u8_t)atoi((char const *)param_buf);
                break;
            case 7:    /* ��ǰ����                */
                now_leap = (u8_t)atoi((char const *)param_buf);
                break;
            case 8:    /* �µ�����                */
                next_leap = (u8_t)atoi((char const *)param_buf);
                goto LEAP;
                break;
            }    
            
            param_len = 0;
        }
        else 
        {
            param_buf[param_len++] = pdata[i];
        }
        
        i++;
    } 
    

    
LEAP:

//            MTFS30_DEBUG("�µ�������Ч���ܼ����� %d\n�µ�������Ч�������ռ����� %d\n��ǰ���룺 %d\n�µ����룺 %d\n", 
//                         wn, dn, now_leap,next_leap);

	if (strncmp((char *)pdata, "GPSUTC", 6) == 0)   /* GPS���� */
	{
        
        sate_type = GNSS_SAT_TYPE_GPS;
	}
	else if (strncmp((char *)pdata, "BDSUTC", 6) == 0)    /* BDS���� */
	{
        
        sate_type = GNSS_SAT_TYPE_BDS;
        
        /* BDS���⴦�� */
        now_leap = now_leap + 14;  /* ��ǰ���룬��GPSʱ��Ϊ׼  */
        next_leap = next_leap + 14;/*  �µ����룬��GPSʱ��Ϊ׼ */        
	
	}
    
    
    /* ���봦�� */
    ut4b0_leap_process(sate_type, wn, dn, now_leap, next_leap);
    
               
    MTFS30_DEBUG("GPS|BDS����Ԥ��:%s", g_gnss_info.leap.info); 
//    MTFS30_DEBUG("GPS|BDS����Ԥ��:%x %x", g_gnss_info.leap.info[0],g_gnss_info.leap.info[1]); 
}

/*****************************************************************************
 * ��  ��:    ut4b0_gal_utc_msg_parser                                                           
 * ��  ��:    ����UT4B0���ջ������UTC��Ϣ����������Ԥ��(GAL����)                                                                 
 * ��  ��:    *pdata: UTC��Ϣ;                       
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void ut4b0_gal_utc_msg_parser(u8_t *pdata)
{


    u8_t i = 0; 
    u8_t comma_cnt = 0;     /* ���Ÿ���                     */
    u8_t param_len = 0;     /* ����ÿ�������ĳ���           */
    u8_t param_buf[20]= {0};/* ����ÿ������                 */
	u32_t wn = 0;           /* �µ�������Ч���ܼ���         */
	u8_t  dn = 0;           /* �µ�������Ч�������ռ���     */
	u8_t now_leap = 0;      /* ��ǰ����                     */
	u8_t next_leap = 0;     /* �µ�����                     */

    
    /* �Թ�Logͷ */
	while(pdata[i++] != ';');
      
    /* ����ÿ������ */
    while(pdata[i])
    {
        if (pdata[i] == ',')
        {
            comma_cnt++;
            
            param_buf[param_len]='\0';
            switch(comma_cnt)
            {
            case 3:    /* ��ǰ����                */
                now_leap = (u8_t)atoi((char const *)param_buf);
                break;
            case 6:   /* �µ�������Ч���ܼ���     */
                wn = (u32_t)atoi((char const *)param_buf);
                break;
            case 7:   /* �µ�������Ч�������ռ��� */
                dn = (u8_t)atoi((char const *)param_buf);
                break;
            case 8:   /* �µ�����                 */
                next_leap = (u8_t)atoi((char const *)param_buf);
                goto LEAP;
                break;
            }    
            
            param_len = 0;
        }
        else 
        {
            param_buf[param_len++] = pdata[i];
        }
        
        i++;
    } 
    
    
LEAP:
    
//            MTFS30_DEBUG("GAL �µ�������Ч���ܼ����� %d\n�µ�������Ч�������ռ����� %d\n��ǰ���룺 %d\n�µ����룺 %d\n", 
//                         wn, dn, now_leap,next_leap);

    /* ���봦�� */
    ut4b0_leap_process(GNSS_SAT_TYPE_GAL, wn, dn, now_leap, next_leap);
    

    MTFS30_DEBUG("GAL����Ԥ��:%s", g_gnss_info.leap.info);    
}



/*****************************************************************************
 * ��  ��:    ut4b0_leap_process                                                          
 * ��  ��:    ���봦��                                                                 
 * ��  ��:    sat_type  : ��������
 *            wn        ���µ�������Ч���ܼ��� 
 *            dn        ���µ�������Ч�������ռ���
 *            now_leap  : UTC����а����ĵ�ǰ����       
 *            next_leap : UTC����а������µ����� 
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void ut4b0_leap_process(u8_t sat_type, u32_t wn, u8_t dn, u8_t now_leap, u8_t next_leap)
{
       
    if ((g_leap.sign == 0) && (now_leap != next_leap) && (now_leap == g_leap.now_leap)) /* ��������Ԥ�� */
    {
        g_leap.next_leap = next_leap;
        
        if ((g_leap.next_leap - g_leap.now_leap) > 0)
        {
            g_leap.sign = 1; /* ������ */    
        }
        else
        {
            g_leap.sign = 2; /* ������ */
            
        }
        
        /* ��ȡ�µ����뷢��ʱ�� */
        ut4b0_get_leap_time(sat_type, wn, dn, g_leap.sign, &g_leap.leap_time);
        
        /* ��������Ԥ����Ϣ */
        sprintf((char *)g_gnss_info.leap.info, "%6s,%2d,%2d,%4d,%d,%d,%d;", 
                g_leap.leap_time.hms, g_leap.leap_time.day, g_leap.leap_time.mon, g_leap.leap_time.year, 
                g_leap.next_leap, g_leap.now_leap, g_leap.sign);
        MTFS30_DEBUG("��������Ԥ��: %s", g_gnss_info.leap.info);

    }
    else if ((g_leap.sign != 0) && (now_leap == next_leap) && (next_leap == g_leap.next_leap)) /* ��������Ԥ�� */
    {
        g_leap.now_leap =  now_leap;
        g_leap.sign = 0;

        /* ��������Ԥ��������Ϣ */
        sprintf((char *)g_gnss_info.leap.info, "%6s,%2d,%2d,%4d,%d,%d,%d;", 
                g_leap.leap_time.hms, g_leap.leap_time.day, g_leap.leap_time.mon, g_leap.leap_time.year, 
                g_leap.next_leap, g_leap.now_leap, g_leap.sign);  
        MTFS30_DEBUG("��������Ԥ��: %s", g_gnss_info.leap.info);
    } 


}


/*****************************************************************************
 * ��  ��:    ut4b0_get_leap_time                                                           
 * ��  ��:    ��ȡ���뷢��ʱ��                                                                 
 * ��  ��:    sat_type  : ��������
 *            wn        ���µ�������Ч���ܼ��� 
 *            dn        ���µ�������Ч�������ռ���
 *            sign      ����/������
 * ��  ��:    p_time    : ָ�����뷢��ʱ��                                                   
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void ut4b0_get_leap_time(u8_t sat_type, u32_t wn, u8_t dn, u8_t sign, gnss_time_t *p_time)
{
	u16_t year = 0;   /* �µ����뷢���� */ 
	u8_t  mon = 1;    /* �µ����뷢���� */
	u8_t  day = 0;    /* �µ����뷢���� */
	u32_t days = 0;   /* �µ����뷢���վ�����ʼ��Ԫ�������� */
	u8_t  mon_days[12] ={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; /* ÿ������ */


    

    /* ������� */
    if (NULL == p_time)
    {
        MTFS30_ERROR("ut4b0_get_leap_time() p_time is NULL");
        return ;
    }
    
    
	switch(sat_type)
	{
		case GNSS_SAT_TYPE_GPS:    /* GPS���� */
			year = 1980; /* GPSʱ����ʼ�� */
			mon = 1;     /* GPSʱ����ʼ�� */
			day = 6;     /* GPSʱ����ʼ�� */
            dn = dn - 1;
			break;

		case GNSS_SAT_TYPE_BDS:    /* BDS���� */
			year = 2006; /* BDSʱ����ʼ�� */      
			mon = 1;     /* BDSʱ����ʼ�� */    
			day = 1;     /* BDSʱ����ʼ�� */
			break;
            
        case GNSS_SAT_TYPE_GAL:     /* GAL���� */
			year = 1999; /* GALʱ����ʼ�� */      
			mon = 8;     /* GALʱ����ʼ�� */    
			day = 22;    /* GALʱ����ʼ�� */ 
            dn = dn - 1;
            break;
            
        default:            
            break;
	}


	/*-------------------------------*/
	/* �����µ����뷢����������      */
	/*-------------------------------*/	 

	/* �µ����뷢���վ�����ʼ��Ԫ�������� */
	days = wn * 7 + dn + day;

	while(days > mon_days[mon-1])
	{

		/* �ж����껹��ƽ�� */
		if ((year % 400 == 0) || (year % 100 != 0 && year % 4 == 0))
		{
			mon_days[1] = 29; /* �����2��Ϊ29�� */
		}
		else 
		{
			mon_days[1] = 28; /* ƽ���2��Ϊ28�� */
		}     
        
		/* �����������µ����������������һ���� */
		days -= mon_days[mon-1];
		mon++;

		/* ����12�½�����һ�� */
		if (mon > 12)
		{
			year++;
			mon = 1; /* �µ�һ�꣬�·����´�1�¿�ʼ */
		}


	}

	/* �µ����뷢���� */
	day = days; 
    
    

    if (1 == sign) /* ������ʱ��Ϊ��23:59:60 */
    {
        strncpy((char *)p_time->hms, "235960", GNSS_LEAP_TIME_HMS_LEN);
    }
    else /* ������ʱ��Ϊ��23:59:58 */
    {
        strncpy((char *)p_time->hms, "235958", GNSS_LEAP_TIME_HMS_LEN);
    }
    
    p_time->day = day;
    p_time->mon = mon;
    p_time->year = year;
   
}



/*****************************************************************************
 * ��  ��:    ut4b0_1pps_msg_parser                                                           
 * ��  ��:    ����UT4B0���ջ������1PPS״̬��Ϣ                                                                
 * ��  ��:    *pdata: 1PPS״̬��Ϣ;                       
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void ut4b0_1pps_msg_parser(u8_t *pdata)
{
    u8_t pps_id = 0;
    u8_t pps_status = 0;    
    u8_t *p = NULL; 
  
    
    
    /* LOGͷ */
    p = (u8_t *)strtok((char *)pdata, ";");
      
    /* 1PPS ID */
    p = (u8_t *)strtok(NULL, ",");
    pps_id = (u8_t)atoi((char const *)p);

    
    /* 1PPS STATUS */
    p = (u8_t *)strtok(NULL, ",");
    pps_status = (u8_t)atoi((char const *)p);
    
    
    /* ��λ����С��4ʱΪ��Ч��λ��1PPS������ */
    if (g_rev_info.post_sats < 4) 
    {
        strcpy((char *)g_gnss_info.pps.info, "0;");   
        MTFS30_DEBUG("1PPS״̬��Ϣ: %s", g_gnss_info.pps.info);   
    }
    else
    {
        
        if (0 == pps_id)
        {
            snprintf((char *)g_gnss_info.pps.info, GNSS_1PPS_INFO_LEN, "%d;", pps_status);
            MTFS30_DEBUG("1PPS״̬��Ϣ: %s", g_gnss_info.pps.info);        
        }
    }
    
}


/*****************************************************************************
 * ��  ��:    ut4b0_antenna_msg_parser                                                           
 * ��  ��:    ����UT4B0���ջ��������״̬��Ϣ                                                                
 * ��  ��:    *pdata: ����״̬��Ϣ;                       
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void ut4b0_antenna_msg_parser(u8_t *pdata)
{
    u8_t *p = NULL;
 
    
    /* LOGͷ */
    p = (u8_t *)strtok((char *)pdata, ";");
    
    /* Pָ������״̬��Ϣ��ͷ */
    p = (u8_t *)strtok(NULL, ",");
    

    snprintf((char *)g_gnss_info.ant.info, GNSS_ANTENNA_INFO_LEN, "%s;", p);
    MTFS30_DEBUG("����״̬��Ϣ: %s", g_gnss_info.ant.info);
    
}

/*****************************************************************************
 * ��  ��:    ut4b0_cmd_version_msg_parser                                                           
 * ��  ��:    ����UT4B0���ջ�����İ汾��Ϣ                                                                
 * ��  ��:    *pdata: �汾��Ϣ;                       
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void ut4b0_cmd_version_msg_parser(u8_t *pdata)
{
    u8_t i = 0;
    u8_t *p = NULL;
	u8_t *q = NULL;
    u8_t buf[GNSS_VERSION_INFO_LEN] = {0};
    u8_t comma_cnt = 0;     /* ���Ÿ���             */
    u8_t len = 0;
    

    
    /* LOGͷ */
    p = (u8_t *)strtok((char *)pdata, ";");
    
    /* Pָ��汾��Ϣ */
    p = (u8_t *)strtok(NULL, "");
    
    while(p[i])
    {
    
        if (p[i] == ',')
        {
            comma_cnt++;
            /* �汾��Ϣ��ȫ��ȡ�� */
            if (comma_cnt == 2)
            {
                strncpy((char *)buf, (char *)p, len); 
                buf[len] = ';';
                buf[len+1] = '\0';

                break;
            }
        }
        
        len++;
        i++;
    }
       
    /* ȥ�����ջ��汾����Ϣǰ���˫���ţ���"R1.00Build19876" -> R1.00Build19876 */	
    q = g_gnss_info.ver.info;
	i = 0;
	while(buf[i])
	{
		if (buf[i] != '\"')
		{
	        *q++ = buf[i];
		}
		i++;
	}
	*q = '\0';	
    MTFS30_DEBUG("�汾��Ϣ: %s", g_gnss_info.ver.info);
}


/*****************************************************************************
 * ��  ��:    ut4b0_cmd_reply_msg_parser                                                           
 * ��  ��:    ����UT4B0����ظ���Ϣ                                                                
 * ��  ��:    *pdata: ����ظ���Ϣ;                       
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void ut4b0_cmd_reply_msg_parser(u8_t *pdata)
{

    OS_ERR err;
    u8_t *p_reply_cmd;      /* �ظ���Ϣ�а��������� */
    u8_t *p_reply_result;   /* �ظ���Ϣ�еĻظ���� */
    u8_t *p = NULL;
    static u8_t cmd_cnt = 0;/* ͳ���յ��Ļظ�������� */
   

    /* $commandͷ */
    p = (u8_t *)strtok((char *)pdata, ",");
    
    /* �ظ���Ϣ�а������û���������� */
    p_reply_cmd = (u8_t *)strtok(NULL, ",");
    
    /* ȡ���ظ���� */
    p = (u8_t *)strtok(NULL, "*");
    p_reply_result = p + 10; 
 
    /* �ظ���Ϣ�а����������뷢������һ�£��һظ����ΪOK */
    if ((0 == strncmp((char *)&g_save_cmd.buf[g_save_cmd.index[cmd_cnt]], (char *)p_reply_cmd, strlen((const char *)p_reply_cmd))) &&
        (0 == strcmp("OK", (char *)p_reply_result)))
    {

        //MTFS30_DEBUG("���������: %s, �ظ���Ϣ: %s,�ظ����:%s", gut4b0_config_cmd, p_reply_cmd, p_reply_result);        
       
        cmd_cnt++;
        if (cmd_cnt == g_save_cmd.num) /* ���з��͵�����յ��˻ظ� */
        {
        
            /* post����ظ��ź��� */
            OSSemPost ((OS_SEM* ) &g_ut4b0_reply_sem,
                       (OS_OPT  ) OS_OPT_POST_1,
                       (OS_ERR *) &err); 
            
            cmd_cnt = 0;
        }        
    }
    else
    {
        cmd_cnt = 0;
    }

    
}


/*****************************************************************************
 * ��  ��:    ut4b0_receiver_acmode_set                                                           
 * ��  ��:    ����ut4b0���ջ�����ģʽ                                                              
 * ��  ��:    acmode: ���ջ�����ģʽ                      
 * ��  ��:    ��                                                    
 * ����ֵ:    OK����ȷ���ù���ģʽ; NG��δ����ȷ���ù���ģʽ                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/ 
u8_t ut4b0_receiver_acmode_set(u8_t acmode)
{
    
    switch(acmode)
    {
    
    case REV_ACMODE_MIX: /* ���϶�λ */
        /* ����GPS��BDS��GLO��GAL */
        if (NG == ut4b0_receiver_configuration("UNMASK GPS\r\nUNMASK BDS\r\nUNMASK GLO\r\nUNMASK GAL\r\n"))
        {
           return NG;
        }        
        break;
    case REV_ACMODE_GPS: /* ��GPS��λ */
        /* ����BDS��GLO��GAL������GPS */
        if (NG == ut4b0_receiver_configuration("MASK BDS\r\nMASK GLO\r\nMASK GAL\r\nUNMASK GPS\r\n"))
        {
            return NG;
        }                    
        break;
    case REV_ACMODE_BDS: /* ��BDS��λ */
        /* ����GPS��GLO��GAL������BDS */
        if (NG == ut4b0_receiver_configuration("MASK GPS\r\nMASK GLO\r\nMASK GAL\r\nUNMASK BDS\r\n"))
        {
            return NG;
        }              
        break;
    case REV_ACMODE_GLO: /* ��GLO��λ */
        /* ����GPS��BDS��GAL������GLO */
        if (NG == ut4b0_receiver_configuration("MASK GPS\r\nMASK BDS\r\nMASK GAL\r\nUNMASK GLO\r\n"))
        {
            return NG;
        }
                 
        break;
    case REV_ACMODE_GAL: /* ��GAL��λ */
        /* ����GPS��BDS��GLO������GAL */
        if (NG == ut4b0_receiver_configuration("MASK GPS\r\nMASK BDS\r\nMASK GLO\r\nUNMASK GAL\r\n"))
        {
            return NG;
        }
             
        break;
    default:
        break;
    }


    return OK;
}



/*****************************************************************************
 * ��  ��:    ut4b0_receiver_serial_baud_set                                                          
 * ��  ��:    ����UT4B0���ջ����ڲ�����                                                                 
 * ��  ��:    p_baud: ָ��������
 * ��  ��:    ��                                                    
 * ����ֵ:    OK����ȷ���ò�����; NG��δ����ȷ���ò�����                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
u8_t ut4b0_receiver_serial_baud_set(u8_t *p_baud)
{
 
    u8_t buf[20] = {0};

       
    sprintf((char *)buf, "CONFIG COM1 %s", p_baud);   
    /* ���Ͳ���������ָ���UT4B���ջ���������ý�� */
    return ut4b0_receiver_configuration(buf);

  
}

/*****************************************************************************
 * ��  ��:    ut4b0_receiver_statement_set                                                           
 * ��  ��:    ��/�ر�ut4b0���ջ�ָ���������                                                                 
 * ��  ��:    stmt_index: ָ���������
 * ��  ��:    on_off_flag: ��/�ر��������ı�־ 
 * ��  ��:    ��                                                    
 * ����ֵ:    OK: ���óɹ�; NG: ����ʧ��                                                  
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                   
 ****************************************************************************/
u8_t ut4b0_receiver_statement_set(u8_t stmt_index, u8_t on_off_flag)
{
    

   
    if (on_off_flag == GNSS_ON) /*�� */
    {
        /* ����������� */
        switch(stmt_index)
        {
        case GNSS_STMT_ZDA: /* ZDA���      */
            if (NG == ut4b0_receiver_configuration("GPZDA 1"))
            {
                return NG;
            }
           break;            
            
        case GNSS_STMT_GGA: /* GGA���      */
            if (NG == ut4b0_receiver_configuration("GNGGA 1"))
            {
                return NG;
            }
           break;
                
        case GNSS_STMT_GSV: /* GSV���      */
            if (NG == ut4b0_receiver_configuration("GPGSV 1"))
            {
                return NG;
            }
           break;                

        case GNSS_STMT_UTC: /* ������Ϣ��� */
            if (NG == ut4b0_receiver_utc_statement_set())
            {
                return NG;
            }
            break;
            
        case GNSS_STMT_PPS: /* 1PPS��Ϣ��� */
            if (NG == ut4b0_receiver_configuration("BBPPSMSGA ONCHANGED "))
            {
                return NG;
            }                
            break;
            
        case GNSS_STMT_ANT: /* ����״̬��� */
            if (NG == ut4b0_receiver_configuration("ANTENNAA 1"))
            {
                return NG;
            }                
            break; 
            
        case GNSS_STMT_VER: /* �汾��Ϣ��� */
            if (NG == ut4b0_receiver_configuration("VERSIONA 1"))
            {
                return NG;
            }                
            break;             
                
        default:
            break;
        }
              
    }
    else /* �ر� */
    {
        /* ����������� */
        switch(stmt_index)
        {
        case GNSS_STMT_ZDA: /* ZDA���      */
            if (NG == ut4b0_receiver_configuration("UNLOG GPZDA"))
            {
                return NG;
            }  
            
           break;            
            
        case GNSS_STMT_GGA: /* GGA���      */
            if (NG == ut4b0_receiver_configuration("UNLOG GNGGA"))
            {
                return NG;
            }  
            
           break;
           
        case GNSS_STMT_GSV: /* GSV���      */
            if (NG == ut4b0_receiver_configuration("UNLOG GPGSV"))
            {
                return NG;
            }
            
            break;
            
        case GNSS_STMT_UTC: /* ������Ϣ��� */
            MTFS30_TIPS("UT4B0�ݲ�֧�ֹر�UTC���!\n");
            return NG;
            break;
            
        case GNSS_STMT_PPS: /* 1PPS��Ϣ��� */
            MTFS30_TIPS("UT4B0�ݲ�֧�ֹر�1PPS������!\n");
            return NG;
            break;
            
        case GNSS_STMT_ANT: /* ����״̬��� */
            if (NG == ut4b0_receiver_configuration("UNLOG ANTENNAA"))
            {
                return NG;
            }
            
            break;   
            
        case GNSS_STMT_VER: /* �汾��Ϣ��� */
            if (NG == ut4b0_receiver_configuration("UNLOG VERSIONA"))
            {
                return NG;
            } 
            
            break;
            
        default:
            break;
        }    
  
    }
           
    return OK;       
}

/*****************************************************************************
 * ��  ��:    ut4b0_receiver_statement_set                                                           
 * ��  ��:    ��/�ر�ut4b0���ջ����UTC���                                                                 
 * ��  ��:    stmt_index: ָ���������
 * ��  ��:    on_off_flag: ��/�ر��������ı�־ 
 * ��  ��:    ��                                                    
 * ����ֵ:    OK�����óɹ�                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                   
 ****************************************************************************/
static u8_t ut4b0_receiver_utc_statement_set(void)
{

        if (NG == ut4b0_receiver_configuration("GPSUTC 1\r\nBDSUTC 1\r\nGALUTC 1\r\n"))
        {
            return NG;
        }
        

//        if (NG == ut4b0_receiver_configuration("BDSUTC 1"))
//        {
//            return NG;
//        }                
//
//        
//        if (NG == ut4b0_receiver_configuration("GALUTC 1"))
//        {
//            return NG;
//        }                 

    
    return OK;
}

/*****************************************************************************
 * ��  ��:    ut4b0_receiver_configuration                                                          
 * ��  ��:    ��������ָ���UT4B���ջ���������ý��                                                               
 * ��  ��:    p_cmd����ָ��
 * ��  ��:    ��                                                    
 * ����ֵ:    OK: ��ȷִ������ָ��
 *            NG: δ����ȷִ������ָ��
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8_t ut4b0_receiver_configuration(u8_t *p_cmd)
{
    OS_ERR err;
    
    

    /* ������������ */
    if (strlen((const char *)p_cmd)> UT4B0_CONFIG_CMD_MAX_LEN)
    {
        MTFS30_DEBUG("UT4B0��������(%s)����, �Ϊ%d�ֽ�", p_cmd, UT4B0_CONFIG_CMD_MAX_LEN);
        return NG;
    }
        
    /* ������������ */
    ut4b0_receiver_cmd_save(p_cmd);
    //strncpy((char *)gut4b0_config_cmd, (char *)p_cmd, UT4B0_CONFIG_CMD_MAX_LEN);

    //MTFS30_DEBUG("���������: %s\n", gut4b0_config_cmd);
    /* ʹ������ģ�鴮�ڷ����������� */
    GNSS_USART_Send(p_cmd);
     
    /* �����û��������������ģ���־ */
    g_gnss_cmd_send_flag = 1;      
    
    /* �ȴ��������ý�� */
    OS_SEM_CTR sem = OSSemPend ((OS_SEM *) &g_ut4b0_reply_sem,
                                (OS_TICK ) 2000,
                                (OS_OPT  ) OS_OPT_PEND_BLOCKING,
                                (CPU_TS *) NULL,
                                (OS_ERR *) &err);  
   

    /* ����������������ģ���־ */
    g_gnss_cmd_send_flag = 0; 
    
    if (err == OS_ERR_NONE)
    {
        return OK;
    }
        
    
    /* �ȴ���ʱ */
    MTFS30_DEBUG("��������(%s)ʧ��: ", g_save_cmd.buf);
    return NG;
}

/*****************************************************************************
 * ��  ��:    ut4b0_receiver_cmd_save                                                          
 * ��  ��:    �������õ�����                                                               
 * ��  ��:    p_cmd����ָ��
 * ��  ��:    ��                                                    
 * ����ֵ:    OK: 
 *            NG: δ����ȷִ������ָ��
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void ut4b0_receiver_cmd_save(u8_t *p_cmd)
{
    u8_t *p = p_cmd;
    g_save_cmd.num = 0;
    strcpy((char *)g_save_cmd.buf, (const char *)p_cmd); /* ��������*/

    /* ����ÿ������Ĵ���λ�� */
    g_save_cmd.index[0] = 0; /* ��һ������洢λ�� */ 
    g_save_cmd.num = 1;
    while(*p != '\0')
    {
        if(NULL != (p = (u8_t *)strstr((const char *)p, "\r\n")))
        {

            p += 2;
            if (*p != '\0') /* ����Ƿ�Ϊ���һ��\r\n */
            {
                g_save_cmd.index[g_save_cmd.num] = p - p_cmd; /* ��2��~���һ������ �Ĵ洢λ�� */
                g_save_cmd.num++;
            }
        }
    
    }


}
