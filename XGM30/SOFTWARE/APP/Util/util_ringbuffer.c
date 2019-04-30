/*****************************************************************************/
/* �ļ���:    util_ringbuffer.c                                              */
/* ��  ��:    ���λ�������ش���                                             */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#include "Util/util_ringbuffer.h"
#include "arch/cc.h"
#include "Debug/mtfs30_debug.h"





/*****************************************************************************
 * ��  ��:    util_ringbuffer_create                                                          
 * ��  ��:    �������λ�����                                                                
 * ��  ��:    rb_size :  ���λ�������С              
 * ��  ��:    *rb : ָ�򴴽��Ļ��λ�����                                                    
 * ����ֵ:                                                        
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void util_ringbuffer_create(ringbuffer_t *rb, u16_t rb_size)
{
 
    /* Ϊ���λ���������ռ� */
    rb->prb_buf = (u8_t *)malloc(rb_size);
    if (rb->prb_buf == NULL)
    {     
        MTFS30_DEBUG("Ϊ���λ���������ռ�ʧ�ܣ�");
        return;
    }
   
    /* ��ʼ�� */
    rb->rb_write = 0;
    rb->rb_read = 0;
    rb->rb_size = rb_size ;   

    MTFS30_DEBUG("�������ζ��гɹ�������");     
      
}


/*****************************************************************************
 * ��  ��:    util_ringbuffer_can_read                                                          
 * ��  ��:    ��ȡ���λ������ɶ��Ŀռ��С                                                               
 * ��  ��:    *rb : ָ�򴴽��Ļ��λ�����               
 * ��  ��:                                                       
 * ����ֵ:    ���λ������ɶ��Ŀռ��С                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
u16_t util_ringbuffer_can_read(ringbuffer_t *rb)
{

    if (rb->rb_write == rb->rb_read)
    {
        return 0;
    } 
    else if (rb->rb_write > rb->rb_read)
    {
        return (rb->rb_write - rb->rb_read);
    }
    else
    {
        return (rb->rb_size - (rb->rb_read - rb->rb_write));
    }
    
    
}

/*****************************************************************************
 * ��  ��:    util_ringbuffer_can_write                                                          
 * ��  ��:    ��ȡ���λ�������д�Ŀռ��С                                                               
 * ��  ��:    *rb : ָ�򴴽��Ļ��λ�����               
 * ��  ��:                                                       
 * ����ֵ:    ���λ�������д�Ŀռ��С                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
u16_t util_ringbuffer_can_write(ringbuffer_t *rb)
{

    return  (rb->rb_size - util_ringbuffer_can_read(rb));
}


/*****************************************************************************
 * ��  ��:    util_ringbuffer_read                                                          
 * ��  ��:    �ӻ��λ������ж�ȡָ����С������                                                              
 * ��  ��:    *rb        : ָ�򴴽��Ļ��λ�����  
 *            *data      : ��Ŀ���ַ
 *            read_size  : ָ�����Ĵ�С
 * ��  ��:                                                       
 * ����ֵ:    ʵ�ʶ����Ĵ�С                                                   
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
u16_t util_ringbuffer_read(ringbuffer_t *rb, u8_t *data, u16_t read_size)
{
 

    u16_t can_read = 0;
    u16_t tail_size = 0;
    
    
    
    /* ȡ�ÿɶ���С */
    can_read = util_ringbuffer_can_read(rb);
//    MTFS30_DEBUG("��index: %d, дindex: %d, ����С: %d, �ɶ���С: %d, ��д��С:%d\n", 
//                 rb->rb_read, rb->rb_write, read_size, can_read, util_ringbuffer_can_write(rb));     
    
    /* �޿ɶ������ݣ�ֱ�ӷ��� */
    if (can_read == 0)
    {
        return 0;
    }
    
    /* ʵ�ʶ���С */
    read_size = min(read_size, can_read);
    
    
    /* ���ʵ�ʶ��Ĵ�СС�ڡ���index��������ĩβ���Ŀռ��С����ֱ�Ӷ�;
     *  ���򣬷����ζ����ȶ�����index��������ĩβ���Ŀռ��С�����ݣ�Ȼ��
     *  �ٴӻ�������ͷ���Ŷ�ʣ�������
     */ 
    tail_size = rb->rb_size - rb->rb_read;
    if (read_size < tail_size)
    {
        memcpy(data, rb->prb_buf+rb->rb_read, read_size);
        rb->rb_read = rb->rb_read + read_size;
    }
    else
    {
        memcpy(data, rb->prb_buf+rb->rb_read, tail_size);
        memcpy(data+tail_size, rb->prb_buf, read_size-tail_size);
        rb->rb_read =  (rb->rb_read + read_size) % rb->rb_size;
    }
    
    
//    MTFS30_DEBUG("�¶�index: %d, дindex: %d, ����С: %d, �ɶ���С: %d, ��д��С:%d\n", 
//                 rb->rb_read, rb->rb_write, read_size, util_ringbuffer_can_read(rb), util_ringbuffer_can_write(rb));  
    
    return read_size; /* ����ʵ�ʶ��Ĵ�С */

}

/*****************************************************************************
 * ��  ��:    util_ringbuffer_end_ch_read                                                          
 * ��  ��:    �ӻ��λ������ж�ȡ��ָ���ַ�����                                                             
 * ��  ��:    *rb        : ָ�򴴽��Ļ��λ�����  
 *            *data      : ��Ŀ���ַ
 *            read_size : ָ�����Ĵ�С
 *            *byte      : �����ַ�
 * ��  ��:                                                       
 * ����ֵ:    �����Ĵ�С                                                   
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
u16_t util_ringbuffer_end_ch_read(ringbuffer_t *rb, u8_t *data, u16_t read_size, u8_t byte)
{
    u16_t can_read = 0;
    u16_t read_cnt = 0;
    
    /* ȡ�ÿɶ���С */
    can_read = util_ringbuffer_can_read(rb);

//    MTFS30_DEBUG("��index: %d, дindex: %d, ����С: %d, �ɶ���С: %d, ��д��С:%d\n", 
//                 rb->rb_read, rb->rb_write, read_size, can_read, util_ringbuffer_can_write(rb));     
    /* �޿ɶ������ݣ�ֱ�ӷ��� */
    if (can_read == 0)
    {
        data[0]='\0';
        return 0;
    }    
    
    
    read_size = min(read_size, can_read);
    
    while(read_cnt < read_size)
    {
            if (rb->prb_buf[rb->rb_read] == byte)
            {
                data[read_cnt] = rb->prb_buf[rb->rb_read];
                read_cnt++;
                rb->rb_read++;
                
                /* �ﵽ������ĩβ�����Ŵӻ�������ͷ���� */
                if (rb->rb_read == rb->rb_size) {
                    rb->rb_read = 0;
                }                  
//    MTFS30_DEBUG("�¶�index: %d, дindex: %d, ����С: %d, �ɶ���С: %d, ��д��С:%d\n", 
//                 rb->rb_read, rb->rb_write, read_size, util_ringbuffer_can_read(rb), util_ringbuffer_can_write(rb));                
                //return read_cnt;
                break;
            }
            
            
            data[read_cnt] = rb->prb_buf[rb->rb_read];
            read_cnt++; 
            
            
            rb->rb_read++;
            /* �ﵽ������ĩβ�����Ŵӻ�������ͷ���� */
            if (rb->rb_read == rb->rb_size) {
                rb->rb_read = 0;
            }        
    }

//    MTFS30_DEBUG("�¶�index: %d, дindex: %d, ����С: %d, �ɶ���С: %d, ��д��С:%d\n", 
//                 rb->rb_read, rb->rb_write, read_size, util_ringbuffer_can_read(rb), util_ringbuffer_can_write(rb));    
    /* û���ҵ�ָ���ַ� */    
    return read_cnt;

}

/*****************************************************************************
 * ��  ��:    util_ringbuffer_write                                                         
 * ��  ��:    ���λ�������д��ָ����С������                                                               
 * ��  ��:    *rb         : ָ�򴴽��Ļ��λ�����  
 *            *data       : д���ݵ�ַ
 *            *write_size : ָ��д�Ĵ�С
 * ��  ��:                                                       
 * ����ֵ:    ʵ��д��С                                                   
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
u16_t util_ringbuffer_write(ringbuffer_t *rb, u8_t *data, u16_t write_size)
{
   
    u16_t can_write = 0;
    u16_t tail_size = 0;

    /* ȡ�ÿ�д�ռ��С */
    can_write = util_ringbuffer_can_write(rb);

//    MTFS30_DEBUG("дindex: %d, ��index: %d, д���С: %d, ��д��С: %d, �ɶ���С:%d\n", 
//                 rb->rb_write, rb->rb_read, write_size, can_write, util_ringbuffer_can_read(rb));    

    
    /* ������д�ռ��С����д */
    if (write_size > can_write)
    {
       return 0; 
    }
    
    /* ���ʵ��д�Ĵ�СС�ڡ�дindex��������ĩβ���Ŀռ��С����ֱ��д;
     *  ���򣬷�����д����д��дindex��������ĩβ���Ŀռ��С�����ݣ�Ȼ��
     *  �ٴӻ�������ͷ����дʣ�������
     */   
    tail_size = rb->rb_size - rb->rb_write;
    if (write_size < tail_size)
    {
        memcpy(rb->prb_buf + rb->rb_write, data, write_size);
        rb->rb_write = rb->rb_write + write_size;
    }
    else
    {
        memcpy(rb->prb_buf + rb->rb_write, data, tail_size);
        memcpy(rb->prb_buf, data+tail_size, write_size-tail_size);
        rb->rb_write = (rb->rb_write + write_size) %  (rb->rb_size);
    }
    
//    MTFS30_DEBUG("��дindex: %d, ��index: %d, д���С: %d, �¿�д��С: %d, �¿ɶ���С:%d\n", 
//                 rb->rb_write, rb->rb_read, write_size, util_ringbuffer_can_write(rb), util_ringbuffer_can_read(rb));     
    
    return write_size; /* ����ʵ��д�Ĵ�С */
}
