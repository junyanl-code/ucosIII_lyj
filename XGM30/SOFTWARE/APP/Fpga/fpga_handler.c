/*****************************************************************************/
/* �ļ���:    fpga_handler.c                                                 */
/* ��  ��:    FPGA��д����                                                   */
/* ��  ��:    2018-11-05 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#include "cc.h"
#include "Debug/user_cmd.h"
#include "Fpga/fpga_handler.h"

#include "mtfs30.h"

#include "Debug/mtfs30_debug.h"
#include "Util/util.h"
#include "BSP/inc/bsp_spi.h"
/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/


/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
static u8_t fpga_write_data_parser(u8_t *p_param, u8_t *p_data, u8_t *p_size);

/*****************************************************************************
 * ��  ��:    fpga_write_cmd_handler                                                           
 * ��  ��:    ��FPGA��д������                                                              
 * ��  ��:    p_param: �������                        
 * ��  ��:    ��                                                 
 * ����ֵ:    ��                                               
 * ��  ��:    2018-11-05 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/ 

u8_t fpga_write_cmd_handler(int num,...)
{
   u8_t *p_param[MAX_PARAM_NUM] = {NULL};
   u32_t write_addr = 0;
   u8_t  write_size = 0;
   u8_t *p_end = NULL;
   u8_t  err_flag = OK;
   u8_t  inx = 0;
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
      write_addr = (u32_t)strtoul((char const *)p_param[0], (char **)&p_end, 16);
      write_size = (u8_t)atoi((char const *)p_param[1]);
      
      if (write_size == 1) /* д��1���ֽ� */
      {
          u8_t write_byte = 0;
          
          /* ��16�����ַ���ת��Ϊ�޷������� */
          write_byte = (u8_t)strtoul((char const *)p_param[2], (char **)&p_end, 16);
  
          
          /* ͨ��SPI���е���д���� */
          if (NG == SPI_FPGA_ByteWrite(write_byte, write_addr))
          {
              MTFS30_DEBUG("SPI_FPGA_ByteWrite failed! д��ַ(%#x), д����(%c)", write_addr, *p_param[2]);
              err_flag = NG;
              goto return_flag;
          }
      
      } 
      else /* д�����ֽ� */
      {
          u8_t write_buf[FPGA_RW_MAX_SIZE]; /* ���ת��������� */    
          u8 write_cnt = 0;                 /* ʵ��д���ֽ���   */
          
          /* ��16�����ַ���ת��Ϊ�޷������� */
          if (NG == fpga_write_data_parser(p_param[2], write_buf, &write_cnt))
          {
              MTFS30_DEBUG("fpga_write_data_parser ERROR! д������Ϊ(%s)", p_param[2]);
              err_flag = NG;
              goto return_flag;
          }
          
          
          /* д�����ֽ�������ָ���ֽ��� */
          if (write_cnt > write_size)
          {
              write_cnt = write_size;
              MTFS30_TIPS("ʵ��д���ֽ���Ϊ(%d)", write_cnt);
          }
        
          
          /* ͨ��SPI��������д���� */
          if (NG == SPI_FPGA_BufferWrite(write_buf, write_addr, write_cnt))
          {
              MTFS30_DEBUG("SPI_FPGA_BufferWrite failed! д��ַ(%#x), д����(%d), д����(%s)", write_addr, write_size, p_param[2]);
              err_flag = NG;
          }
      } 
   }
   
return_flag:    
    return err_flag;
}
/*****************************************************************************
 * ��  ��:    fpga_read_cmd_handler                                                           
 * ��  ��:    ��FPGA�ж�������                                                               
 * ��  ��:    p_param: �������                        
 * ��  ��:    p_rtrv : ָ�����������                                                  
 * ����ֵ:    ��                                               
 * ��  ��:    2018-11-05 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/ 
u8_t fpga_read_cmd_handler(int num,...)
{
   u8_t *p_param[MAX_PARAM_NUM] = {NULL};
   u32_t read_addr = 0;
   u8_t  read_size = 0;
   u8_t *p_end = NULL;
   u8_t  err_flag = OK;
   u8_t  inx = 0;
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
      /* ȡ�ö���ַ */
      read_addr = (u32_t)strtoul((char const *)p_param[0], (char **)&p_end, 16);
  
      
      /* ȡ�ö����� */
      read_size = (u8_t)atoi((char const *)p_param[1]);
          
      
      /* ��FPGA */
      if (read_size == 1) /* ��ȡ1���ֽ� */
      {
          
          /* ͨ��SPI���е��ֶ����� */
          if (NG == SPI_FPGA_ByteRead(p_param[2], read_addr))
          {
              MTFS30_DEBUG("SPI_FPGA_ByteRead failed! ����ַ(%#x), ������(%c)", read_addr, *p_param[2]);
              err_flag = NG;
              goto return_flag;
          }
      
          /* �������� */
          MTFS30_TIPS("����������: %#x", *p_param[2]);
          
      } 
      else /* ��ȡ����ֽ� */
      {
          /* ͨ��SPI�������������� */
          if (NG == SPI_FPGA_BufferRead(p_param[2], read_addr, read_size))
          {
              MTFS30_DEBUG("SPI_FPGA_BufferRead failed! ����ַ(%#x), ������(%d), ������(%s)", read_addr, read_size, p_param[2]);
              err_flag = NG;
              goto return_flag;
          }
          *(p_param[2]+read_size) = '\0';
          
          /* �������� */
          u8_t i = 0;
          
          MTFS30_TIPS("����������:");
          for (i = 0; i < read_size; i++)
          {
              
              MTFS30_TIPS("��%d���ֽ�: %#x", i+1, p_param[2]);
          }
          
      } 
   }
    
return_flag:    
    return err_flag;
}

/*****************************************************************************
 * ��  ��:    fpga_write_data_parser                                                           
 * ��  ��:    ����ת���û�Ҫд��FPGA������                                                               
 * ��  ��:    p_param: ָ��Ҫд�������                        
 * ��  ��:    p_data : ָ�����ת��������� 
 *            p_size : �������ֽ���
 * ����ֵ:    OK������ת���ɹ�; NG������ת��ʧ��                                               
 * ��  ��:    2018-11-05 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/ 
static u8_t fpga_write_data_parser(u8_t *p_param, u8_t *p_data, u8_t *p_size)
{
    u8_t i = 0;
    u8_t param_len = 0;     /* ����ÿ�������ĳ���   */
    u8_t param_buf[4]= {0}; /* ����ÿ������         */
    u8_t cnt = 0;           /* �ѽ������ֽ���       */
    u8_t *p_end = NULL;
  
    
    if (p_param == NULL || p_data == NULL || p_size == NULL)
    {
        MTFS30_DEBUG("fpga_write_data_parser()��������");
        return NG;
    }
    
    
    cnt = 0;

    /* ����ÿ������ */
    while(p_param[i])
    {
        
        /* ���Կո� */
        if (p_param[i] == ' ')
        {
            continue;
        }
        
        /* �ֽڷָ��� */
        if (p_param[i] == ',' || p_param[i] == '.')
        {
            param_buf[param_len] = '\0';
            p_data[cnt] = (u8_t)strtoul((char const *)param_buf, (char **)&p_end, 16);

            
            cnt++;
            param_len = 0;    
        }
        else 
        {
            /* һ���ֽ�����ʾ3λ��,����3λ���� */
            if (param_len >= 3) 
            {
                MTFS30_DEBUG("д�����ݳ���һ���ֽڱ�ʾ�����ֵ" );
                return NG;
            }
            
            param_buf[param_len++] = p_param[i];
        }
       
        i++;
    }  
    
    /* ���1�����ź�������� */
    p_data[cnt] = (u8_t)strtoul((char const *)param_buf, (char **)&p_end, 16);

    cnt++;    
    *p_size = cnt;
    
    return OK;
}


