/*****************************************************************************/
/* �ļ���:    dev_usart.c                                                 */
/* ��  ��:    ������ش���                                                   */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#include <includes.h>
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "Debug/mtfs30_debug.h"
#include "Util/util_ringbuffer.h"
#include "bsp_usart.h"




/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
u8 g_gnss_usart_rx_buf[GNSS_USART_BUF_MAX_SIZE];   /* ����ģ�鴮�ڽ��ջ����� */
ringbuffer_t g_gnss_usart_rb;                      /* ������Ϣ������         */
u8 g_debug_usart_rx_buf[DEBUG_USART_BUF_MAX_SIZE]; /* ���Դ��ڽ��ջ�����     */



/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
extern OS_SEM     g_usart_debug_sem;         /* ���ڵ������ź���   */
extern OS_SEM     g_gnss_idle_sem;           /* GNSS�����ж��ź��� */
extern u8_t       g_gnss_init_finished_flag; /* ���ǳ�ʼ����ɱ�־ */
extern OS_Q       g_gnss_msgq;         /* ������Ϣ��Ϣ����   */

/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
static void GNSS_USART_Config(void);
static void DEBUG_USART_Config(void);
static void DEBUG_USART_IRQHandler(void);

/*****************************************************************************
 * ��  ��:    GNSS_USART_Config                                                          
 * ��  ��:    ��������ģ�鴮��                                                                  
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void GNSS_USART_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    
    
    /*--------------------------------*/
    /* ʱ������                       */
    /*--------------------------------*/
    
	/* ����GPIOʱ�� */
    RCC_AHB1PeriphClockCmd(GNSS_USART_RX_GPIO_CLK | GNSS_USART_TX_GPIO_CLK, ENABLE);
	/* ����DMAʱ�� */						   
	RCC_AHB1PeriphClockCmd(GNSS_USART_DMA_CLK, ENABLE);  						
	/* ����USARTʱ�� */
	RCC_APB2PeriphClockCmd(GNSS_USART_CLK, ENABLE);	
    
    

    /*--------------------------------*/
    /* GPIO����                       */
    /*--------------------------------*/    
 
    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    
    /* Configure USART Tx as alternate function  */
    GPIO_InitStructure.GPIO_Pin = GNSS_USART_TX_PIN  ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GNSS_USART_TX_GPIO_PORT, &GPIO_InitStructure);

    /* Configure USART Rx as alternate function  */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GNSS_USART_RX_PIN;
    GPIO_Init(GNSS_USART_RX_GPIO_PORT, &GPIO_InitStructure);
      
    /* Connect PXx to USARTx_Tx*/
    GPIO_PinAFConfig(GNSS_USART_RX_GPIO_PORT, GNSS_USART_RX_SOURCE, GNSS_USART_RX_AF);

    /* Connect PXx to USARTx_Rx*/
    GPIO_PinAFConfig(GNSS_USART_TX_GPIO_PORT, GNSS_USART_TX_SOURCE, GNSS_USART_TX_AF); 
 
    
    /*--------------------------------*/
    /* NVIC����                       */
    /*--------------------------------*/
	NVIC_InitStructure.NVIC_IRQChannel = GNSS_USART_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);    
    
    
    /*--------------------------------*/
    /* USARTģʽ����                  */
    /*--------------------------------*/      
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(GNSS_USART, &USART_InitStructure);

    /* ʹ��USART */
    USART_Cmd(GNSS_USART, ENABLE);   
    
    /* ʹ��USART��DMA���� */
    USART_DMACmd(GNSS_USART, USART_DMAReq_Rx, ENABLE);
    
    /* ����USART�жϷ����� */
    BSP_IntVectSet(GNSS_USART_INT_ID, GNSS_USART_IRQHandler);
    BSP_IntEn(GNSS_USART_INT_ID);  
    
    /* �ж����� */
    USART_ITConfig(GNSS_USART, USART_IT_RXNE, DISABLE);/*  ��ʹ�ܽ����ж� */
    USART_ITConfig(GNSS_USART, USART_IT_IDLE, ENABLE); /*  ʹ��IDLE�ж�   */  
    
    /*--------------------------------*/
    /* DMAģʽ����                    */
    /*--------------------------------*/      
    /* �ָ�Ĭ������ */
	DMA_DeInit(GNSS_DMA_Stream);
    /* �ȴ�DMA������ */
	while (DMA_GetCmdStatus(GNSS_DMA_Stream) != DISABLE);
    
	/* ���� DMA Stream */
	DMA_InitStructure.DMA_Channel = DMA_Channel_5;                          /* ͨ��ѡ��               */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&GNSS_USART->DR;        /* DMA�����ַ            */
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)g_gnss_usart_rx_buf;       /* DMA �洢��0��ַ        */
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;                /* ���赽�洢��ģʽ       */
	DMA_InitStructure.DMA_BufferSize = GNSS_USART_BUF_MAX_SIZE;             /* �������ݴ�С           */
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        /* ��ʹ�������ַ����ģʽ */
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 /* �ڴ��ַΪ����ģʽ     */
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; /* �������ݳ���:8λ       */
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         /* �洢�����ݳ���:8λ     */
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           /* ʹ����ͨģʽ           */
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                   /* �е����ȼ�             */
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;                  /* ����FIFOģʽ           */         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;           /* FIFO��С               */
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;             /* �洢��ͻ�����δ���     */
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;     /* ����ͻ�����δ���       */
    /* ����DMA */
	DMA_Init(GNSS_DMA_Stream, &DMA_InitStructure);
    
	DMA_Cmd(GNSS_DMA_Stream, ENABLE);
    
    
}



/*****************************************************************************
 * ��  ��:    DEBUG_USART_Config                                                          
 * ��  ��:    ��������ģ�鴮��                                                                  
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void DEBUG_USART_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    /*--------------------------------*/
    /* PeriphClock clock enable       */
    /*--------------------------------*/
    
    /* PeriphClock usart 1 pin, GPIO A9(TX) GPIO A10(RX)  clock enable*/
    RCC_AHB1PeriphClockCmd(DEBUG_USART_RX_GPIO_CLK | DEBUG_USART_TX_GPIO_CLK, ENABLE);
    /* PeriphDMA2 (usart1) clock enable */						   
    RCC_AHB1PeriphClockCmd(DEBUG_USART_DMA_CLK, ENABLE);  						
    /* PeriphUSART1 clock enable */
    RCC_APB2PeriphClockCmd(DEBUG_USART_CLK, ENABLE);	

    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    
    /* Configure USART Tx as alternate function  */
    GPIO_InitStructure.GPIO_Pin = DEBUG_USART_TX_PIN  ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStructure);

    /* Configure USART Rx as alternate function  */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = DEBUG_USART_RX_PIN;
    GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStructure);
      
    /* Connect PXx to USARTx_Tx*/
    GPIO_PinAFConfig(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_SOURCE, DEBUG_USART_RX_AF);

    /* Connect PXx to USARTx_Rx*/
    GPIO_PinAFConfig(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_SOURCE, DEBUG_USART_TX_AF); 
 
    /* NVIC configuration                               */
    NVIC_InitStructure.NVIC_IRQChannel = DEBUG_USART_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);    
    
    /* USART1 baud .databit .stopbit .parity. flowcontrol*/     
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(DEBUG_USART, &USART_InitStructure);

    /* enable USART1 */
    USART_Cmd(DEBUG_USART, ENABLE);   
    
    /* enable USART1 DMA2 chanel 5 as receive */
    USART_DMACmd(DEBUG_USART, USART_DMAReq_Rx, ENABLE);
    
    /* enable usart1 interrupte and irq fuction */
    BSP_IntVectSet(DEBUG_USART_INT_ID, DEBUG_USART_IRQHandler);
    BSP_IntEn(DEBUG_USART_INT_ID);  
    
    /* enable idle interrupte*/
    USART_ITConfig(DEBUG_USART, USART_IT_RXNE, DISABLE);/*  ��ʹ�ܽ����ж� */
    USART_ITConfig(DEBUG_USART, USART_IT_IDLE, ENABLE); /*  ʹ��IDLE�ж�   */  
    
    /* DMA default initalize                    */    
    DMA_DeInit(DEBUG_DMA_Stream);

    /* wait until DMA default initalize end  */
    while (DMA_GetCmdStatus(DEBUG_DMA_Stream) != DISABLE);

    DMA_InitStructure.DMA_Channel = DMA_Channel_4;                          /* ͨ��ѡ��               */
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&DEBUG_USART->DR;       /* DMA�����ַ            */
    DMA_InitStructure.DMA_Memory0BaseAddr = (u32)g_debug_usart_rx_buf;      /* DMA �洢��0��ַ        */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;                /* ���赽�洢��ģʽ       */
    DMA_InitStructure.DMA_BufferSize = DEBUG_USART_BUF_MAX_SIZE;            /* �������ݴ�С           */
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        /* ��ʹ�������ַ����ģʽ */
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 /* �ڴ��ַΪ����ģʽ     */
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; /* �������ݳ���:8λ       */
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         /* �洢�����ݳ���:8λ     */
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           /* ʹ����ͨģʽ           */
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                   /* �е����ȼ�             */
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;                  /* ����FIFOģʽ           */         
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;           /* FIFO��С               */
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;             /* �洢��ͻ�����δ���     */
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;     /* ����ͻ�����δ���       */
    
    DMA_Init(DEBUG_DMA_Stream, &DMA_InitStructure);
    DMA_Cmd(DEBUG_DMA_Stream, ENABLE);
   
}


/*****************************************************************************
 * ��  ��:    GNSS_USART_IRQHandler                                                          
 * ��  ��:    ����1���պ���                                                                  
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void GNSS_USART_IRQHandler(void)
{
       u16 num;
       OS_ERR err;

     /* check usart6 interrupt idle flag  */
      if (USART_GetITStatus(GNSS_USART, USART_IT_IDLE) != RESET)
      {

      
              /* �ȶ�SR��Ȼ���DR������� */
              num = GNSS_USART->SR;		
              num = GNSS_USART->DR;	

              /* �ر�DMA,��ֹ������������� */
              DMA_Cmd(GNSS_DMA_Stream, DISABLE);								 

              /* �õ������������ݸ��� */
              num = GNSS_USART_BUF_MAX_SIZE - DMA_GetCurrDataCounter(GNSS_DMA_Stream); 
              if (num >= GNSS_USART_BUF_MAX_SIZE)
              {
                  MTFS30_DEBUG("GNSS��Ϣ����");
                  goto GNSS_DMA_SET;
              }
              
              g_gnss_usart_rx_buf[num++] = '\0';        
              /* ���յ���������Ϣ���뵽���λ����� */
              if (num != 0)
              {
                  //MTFS30_DEBUG("num = %d, ������Ϣ: %s\n", num, g_gnss_usart_rx_buf);
                  
                  OSQPost ((OS_Q*      ) &g_gnss_msgq,
                           (void*      ) g_gnss_usart_rx_buf,
                           (OS_MSG_SIZE) num,
                           (OS_OPT     ) OS_OPT_POST_FIFO,
                           (OS_ERR*    ) &err);
              }
        
        
GNSS_DMA_SET:
        
              /* �����־λ */
              DMA_ClearFlag(GNSS_DMA_Stream, GNSS_DMA_CLEAR_FLAG);
                      
              /* ��������ģʽ��Ҫÿ�ζ��������ý������ݸ��� */		
              DMA_SetCurrDataCounter(GNSS_DMA_Stream, GNSS_USART_BUF_MAX_SIZE);
              
              /* ����DMA */
              DMA_Cmd(GNSS_DMA_Stream, ENABLE);
      }
}


/*****************************************************************************
 * ��  ��:    DEBUG_USART_IRQHandler                                                          
 * ��  ��:    ����1���պ���                                                                  
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void DEBUG_USART_IRQHandler(void)
{
       u16 num;
       OS_ERR err;

      /* check usart1 interrupt idle flag  */
	if (USART_GetITStatus(DEBUG_USART, USART_IT_IDLE) != RESET)
	{
		/* �ȶ�SR��Ȼ���DR������� */
		num = DEBUG_USART->SR;		
		num = DEBUG_USART->DR;	

		/* �ر�DMA,��ֹ������������� */
		DMA_Cmd(DEBUG_DMA_Stream, DISABLE);								 

		/* �õ������������ݸ��� */
		num = DEBUG_USART_BUF_MAX_SIZE - DMA_GetCurrDataCounter(DEBUG_DMA_Stream); 
                if ((num >= DEBUG_USART_BUF_MAX_SIZE) || num <= 0)
                {
                    goto DEBUG_DMA_SET;
                }
                
                g_debug_usart_rx_buf[num] = '\0';
        
                /* post�����ź��� */
                OSSemPost ((OS_SEM* ) &g_usart_debug_sem,
                           (OS_OPT  ) OS_OPT_POST_1,
                           (OS_ERR *) &err);
        
 DEBUG_DMA_SET:
                
                /* �����־λ */
                DMA_ClearFlag(DEBUG_DMA_Stream, DEBUG_DMA_CLEAR_FLAG);
                        
                /* ��������ģʽ��Ҫÿ�ζ��������ý������ݸ��� */		
                DMA_SetCurrDataCounter(DEBUG_DMA_Stream, DEBUG_USART_BUF_MAX_SIZE);
                
                /* ����DMA */
                DMA_Cmd(DEBUG_DMA_Stream, ENABLE);
         }
	

}

/*****************************************************************************
 * ��  ��:    GNSS_USART_Send                                                          
 * ��  ��:    ����ģ�鴮�ڷ��ͺ���                                                                  
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void GNSS_USART_Send(u8_t *pdata)
{
	uint16_t i;
	u8_t buf[GNSS_USART_SEND_LEN_MAX]; /* ���ڴ�Ÿ�ʽ���ɴ������ݸ�ʽ������ */
	u8_t len = 0;                      /* ����������ַ�������               */


    snprintf((char *)buf, GNSS_USART_SEND_LEN_MAX - 3, "%s", pdata);
    
    /* ��ʽ��Ϊ������Ϣ��ʽ */
    len = strlen((const char *)buf);
    buf[len] = '\r';
    buf[len + 1] = '\n';
    buf[len + 2] = '\0';
  
    /* �������� */
    len = len + 2;
	for (i = 0; i < len; i++)
	{	
        /* �ȴ����ݷ��ͷ������ */
		while(USART_GetFlagStatus(GNSS_USART, USART_FLAG_TC) == RESET);
		/* �������� */
		USART_SendData(GNSS_USART, (uint16_t)buf[i]);
		
	}
    
}


/*****************************************************************************
 * ��  ��:    DEBUG_USART_Send                                                          
 * ��  ��:    ���Դ��ڷ��ͺ���                                                                  
 * ��  ��:    pdata: ָ��Ҫ���͵�����
 *            len  : ���͵����ݳ���  
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void DEBUG_USART_Send(u8_t *pdata, u16_t len)
{
    u16_t i = 0;
    
    
    /* �������� */
	for (i = 0; i < len; i++)
	{
		/* �ȴ����ݷ��ͷ������ */
		while(USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);        
		/* �������� */
		USART_SendData(DEBUG_USART, (uint16_t)pdata[i]);

	}
    
}



/*****************************************************************************
 * ��  ��:    BSP_InitUsart                                                          
 * ��  ��:    ���ڳ�ʼ��                                                                  
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-07-18 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void BSP_InitUsart(void)
{

    /* ����ģ�鴮�ڳ�ʼ�� */
    GNSS_USART_Config();
    
    /* ���Դ��ڳ�ʼ�� */
    DEBUG_USART_Config();
    
    
//    /* ����GNSS������Ϣ���� */
    util_ringbuffer_create(&g_gnss_usart_rb, GNSS_USART_BUF_MAX_SIZE * 2);    
}










