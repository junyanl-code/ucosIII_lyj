/*****************************************************************************/
/* �ļ���:    user_cmd.c                                                     */
/* ��  ��:    �û�������ش���                                               */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#include "user_cmd.h"
#include "mtfs30.h"
#include "Gnss/gnss_handler.h"
#include "Fpga/fpga_handler.h"
#include "Roll/roll_print.h"
#include "Output/out_ptp.h"
#include "Debug/mtfs30_debug.h"





/*******************************************************************************
*command handle.
********************************************************************************/
static user_cmdhandle_t user_cmdhandle[] =
{
    {USER_CMD_SET, (u8_t *)"$DEBUGPRINT",     mtfs30_debug_set_handler},         /* ��/�ر�GNSSģ�������Ϣ */  
    {USER_CMD_SET, (u8_t *)"$ROLLPRINT",      roll_print_set_handler},           /* ��/�ر�ѭ����ӡ��Ϣ     */
    {USER_CMD_SET, (u8_t *)"$SATEPRINT",      gnss_echo_set_handler},            /* ��/�ر�������         */
    {USER_CMD_SET, (u8_t *)"$G-SET-MODE",    gnss_acmode_set_handler},          /* ����GNSS����ģʽ          */
    {USER_CMD_SET, (u8_t *)"$G-SET-BAUD",    gnss_serial_baud_set_handler},     /* ����GNSS���ڲ�����        */
    {USER_CMD_SET, (u8_t *)"$G-SET-STMT",    gnss_stmt_set_handler},            /* ��/�ر�GNSS���ָ����� */
 //   {USER_CMD_GET, (u8_t *)"$G-GET-INFO",    gnss_get_info_handler},          /* ��ѯGNSS��ǰ��Ϣ          */ 
    {USER_CMD_SET, (u8_t *)"$FPGA-W",        fpga_write_cmd_handler},           /* ��FPGAд������            */
    {USER_CMD_GET, (u8_t *)"$FPGA-R",        fpga_read_cmd_handler},            /* ��FPGA��������            */
    {USER_CMD_GET, (u8_t *)"gnss",           gnss_helper},                      /* gnss help                 */
    
//    {USER_CMD_GET, (u8_t *)"$PTP-SET",        ptp_set_handler},               /* ����PTP                   */
    


};




/*****************************************************************************
 * ��  ��:    user_cmd_parser                                                          
 * ��  ��:    �����������û�����                                                             
 * ��  ��:     *p_cmd: �û������������Ϣ                        
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void user_cmd_parser(u8_t *p_cmd)
{
    u8_t i = 0;
    u8_t cnt = 0;          /* ��������������� */
    u8_t *p_head = NULL;   /* ����ͷ           */
    u8_t *p_param[MAX_PARAM_NUM] = {NULL}; /* command1         */
    u8_t  tmp_buffer[RTRV_BUF_MAX_SIZE] = {0}; 
 //   u8_t *p_param2 = NULL; /* command2         */
    u8_t p_param_num = 0;
    u8_t inx = 0;
    u8_t len = 0;

    /* ������� */
    if (p_cmd == NULL)
    {
        MTFS30_ERROR("�û������������ϢΪNULL");
        return;
    }
  
    /* ignore the ' '*/
    while(*p_cmd == ' ')
    {
        p_cmd++;
    }
    
    /* ��鳤�� */
    if(strstr(p_cmd,"\r\n") || strstr(p_cmd,"\r") || strstr(p_cmd,"\n"))
        p_cmd = (u8_t *)strtok((char *)p_cmd, "\r\n"); /* ȥ�������\r\n */
    len = strlen((const char *)p_cmd);
    MTFS30_DEBUG("########cmd len = %d", len);
    if (len > USER_CMD_MAX_LEN)
    {
        MTFS30_ERROR("����(%s)�������Ϊ128�ֽ�", p_cmd);
        return ;
    }
    
//    /* �����ʽ��� */
//    if ((*p_cmd != '$') ||  (';' != p_cmd[len-1]))
//    {
//        MTFS30_ERROR("����(%s)��ʽ����! Usage($XXXX:XXXX;)", p_cmd);
//        //return;
//    }
    
    
    //MTFS30_DEBUG("���������: %s", pmsg);
    /* ȡ������ͷ��������� */
    for(inx = 0; inx < len; inx++)
    {
      if(*(p_cmd+inx) == '\0')
        break;
      
      if(*(p_cmd+inx) == ' ')
        p_param_num++;
    }
    
    if(p_param_num >= MAX_PARAM_NUM)
    {
        MTFS30_ERROR("����(%s)��ʽ����! Usage($XXXX:XXXX;)", p_cmd);
        return;
    }  
    else
        MTFS30_DEBUG("########param num = %d", p_param_num);
    
    p_head = (u8_t *)strtok((char *)p_cmd, " ");
    
    for(inx = 0; inx < p_param_num; inx++)
    {
       p_param[inx] = (u8_t *)strtok(NULL, " ");
       MTFS30_DEBUG("########param = %s", p_param[inx]);
    }
    
	/* ������������ */
    cnt = sizeof(user_cmdhandle) / sizeof(user_cmdhandle[0]);

	/* ����ָ������ */
    for (i = 0; i < cnt; i++)
    {
        
		if(strcmp((char *)user_cmdhandle[i].p_head, (char*)p_head) == 0)
		{
			break;
		}
    } 
    
    
    /* ���������û���ҵ������� */
    if (i == cnt)
    {
	    /* ȡ�����ָ�� */
	    MTFS30_ERROR("ָ��(%s)����!", p_head);        
        
        return;
        
    }

    
    /* �ж��������� */
    if (user_cmdhandle[i].cmd_type == USER_CMD_SET) /* �������� */
    {
        //u8_t set_buf[RTRV_BUF_MAX_SIZE] = {0};
        p_param[p_param_num] = tmp_buffer;
        if (OK == user_cmdhandle[i].cmd_fun(p_param_num +1,p_param[0],p_param[1],
                                                 p_param[2],p_param[3],p_param[4],p_param[5])) /* ���óɹ� */
        {
            MTFS30_TIPS("%s Success!", p_head);
        }
        else 
        {
            MTFS30_TIPS("%s Failure!", p_head);
        }
          
    } 
    else /* ��ѯ���� */ 
    {

        
        p_param[p_param_num] = tmp_buffer;
        if (OK == user_cmdhandle[i].cmd_fun(p_param_num+1,p_param[0], p_param[1], p_param[2],
                                            p_param[3], p_param[4], p_param[5]))  /* ��ѯ�ɹ� */
        {
            
            
            if (0 != strcmp("$FPGA-R", (char const *)user_cmdhandle[i].p_head)) /* $FPGA-R֮��Ĳ�ѯ���� */
            {  
                MTFS30_DEBUG("%s Success!", p_head);
                MTFS30_DEBUG("��ѯ���:%s", tmp_buffer); 
            }
        } 
        else
        {
            MTFS30_TIPS("%s Failure!", p_head);
        }   
    }
}


