/*****************************************************************************/
/* �ļ���:    bsp_spi.c                                                      */
/* ��  ��:    SPI��ش���                                                    */
/* ��  ��:    2018-07-20 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#include <includes.h>
#include "BSP/inc/bsp_spi.h"
#include "Debug/mtfs30_debug.h"
#include "mtfs30.h"



/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
static __IO uint32_t  SPITimeout = SPIT_LONG_TIMEOUT; /* ��ʱʱ�� */  

/*-------------------------------*/
/* ��������                      */
/*-------------------------------*/
static void SPI_FPGA_Init(void);
static u8 SPI_FPGA_SendByte(u8 send_byte, u8 *recv_byte);
static u8 SPI_FPGA_ReadByte(u8 *recv_byte);


/*****************************************************************************
 * ��  ��:    SPI_Init                                                          
 * ��  ��:    SPI��ʼ������                                                                  
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-11-02 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
void BSP_SPI_Init(void)
{
    /* FPGA SPI��ʼ�� */
    SPI_FPGA_Init();
}

/*****************************************************************************
 * ��  ��:    FPGA_SPI_Init                                                          
 * ��  ��:    FPGA SPI��ʼ������                                                                  
 * ��  ��:    ��                          
 * ��  ��:    ��                                                    
 * ����ֵ:    ��                                                    
 * ��  ��:    2018-11-02 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static void SPI_FPGA_Init(void)
{

    SPI_InitTypeDef  FPGA_SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    
    
    /* ʹ��GPIOʱ�� */
    RCC_AHB1PeriphClockCmd (FPGA_SPI_SCK_GPIO_CLK  | 
                            FPGA_SPI_MISO_GPIO_CLK |
                            FPGA_SPI_MOSI_GPIO_CLK | 
                            FPGA_SPI_CS_GPIO_CLK, ENABLE);
    
    /* ʹ��SPIʱ�� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    
    
    /* ���Ÿ��� */
    GPIO_PinAFConfig(FPGA_SPI_SCK_GPIO_PORT, FPGA_SPI_SCK_PINSOURCE, FPGA_SPI_SCK_AF); 
	GPIO_PinAFConfig(FPGA_SPI_MISO_GPIO_PORT, FPGA_SPI_MISO_PINSOURCE, FPGA_SPI_MISO_AF); 
	GPIO_PinAFConfig(FPGA_SPI_MOSI_GPIO_PORT, FPGA_SPI_MOSI_PINSOURCE, FPGA_SPI_MOSI_AF); 
    
    
    /* ����SCK���� */
    GPIO_InitStructure.GPIO_Pin = FPGA_SPI_SCK_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  
  
    GPIO_Init(FPGA_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);
  
	/* ����MISO���� */
    GPIO_InitStructure.GPIO_Pin = FPGA_SPI_MISO_PIN;
    GPIO_Init(FPGA_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);
  
	/* ����MOSI���� */
    GPIO_InitStructure.GPIO_Pin = FPGA_SPI_MOSI_PIN;
    GPIO_Init(FPGA_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);  

	/* ����CS���� */
    GPIO_InitStructure.GPIO_Pin = FPGA_SPI_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(FPGA_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
    SPI_FPGA_CS_HIGH(); /* ֹͣ�ź�: CS���Ÿߵ�ƽ*/    
    
    /* FPGA SPIģʽ����:  ģʽ3*/
    FPGA_SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    FPGA_SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    FPGA_SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    FPGA_SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    FPGA_SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    FPGA_SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    FPGA_SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    FPGA_SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    FPGA_SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(FPGA_SPI, &FPGA_SPI_InitStructure);

   /* ʹ�� FPGA SPI  */
   SPI_Cmd(FPGA_SPI, ENABLE);
    
   
}

/*****************************************************************************
 * ��  ��:    SPI_FPGA_SendByte                                                          
 * ��  ��:    ʹ��SPI����һ���ֽڵ�����                                                                  
 * ��  ��:    send_byte: Ҫ���͵�����                          
 * ��  ��:    ��                                                    
 * ����ֵ:    OK: ���ͳɹ�; NG: ����ʧ��                                                    
 * ��  ��:    2018-11-02 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8 SPI_FPGA_SendByte(u8 send_byte, u8 *recv_byte)
{
    SPITimeout = SPIT_FLAG_TIMEOUT;

    /* �ȴ����ͻ�����Ϊ�գ�TXE�¼� */
    while (SPI_I2S_GetFlagStatus(FPGA_SPI, SPI_I2S_FLAG_TXE) == RESET)
    {
        /* �ȴ���ʱ */
        if((SPITimeout--) == 0) 
        {
              MTFS30_ERROR("SPI ���͵ȴ���ʱ!");
              return NG;
        }
    }

    /* д�����ݼĴ�������Ҫд�������д�뷢�ͻ����� */
    //MTFS30_DEBUG("�����ֽ�: %#x", send_byte);
    SPI_I2S_SendData(FPGA_SPI, send_byte);

    
    SPITimeout = SPIT_FLAG_TIMEOUT;
    /* �ȴ����ջ������ǿգ�RXNE�¼� */
    while (SPI_I2S_GetFlagStatus(FPGA_SPI, SPI_I2S_FLAG_RXNE) == RESET)
    {
        /* �ȴ���ʱ */
        if((SPITimeout--) == 0) 
        {
              MTFS30_ERROR("SPI ���յȴ���ʱ!");
              return NG;
        }
    }


    /* ��ȡ���ݼĴ�������ȡ���ջ��������� */
    *recv_byte = SPI_I2S_ReceiveData(FPGA_SPI);
    //MTFS30_DEBUG("SPI ��������: %#x", *recv_byte);
    
    return OK;
}

/*****************************************************************************
 * ��  ��:    SPI_FPGA_ReadByte                                                          
 * ��  ��:    ʹ��SPI��ȡһ���ֽڵ�����                                                                  
 * ��  ��:    ��                          
 * ��  ��:    recv_byte�� ��ȡ����1�ֽ�����                                                    
 * ����ֵ:    OK: ��ȡ�ɹ�; NG:��ȡʧ��                                                   
 * ��  ��:    2018-11-02 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
static u8 SPI_FPGA_ReadByte(u8 *recv_byte)
{ 
  
  return SPI_FPGA_SendByte(Dummy_Byte, recv_byte);   
}


/*****************************************************************************
 * ��  ��:    SPI_FPGA_ByteWrite                                                          
 * ��  ��:    ͨ��SPI���е���д����                                                                  
 * ��  ��:    write_byte: Ҫд�������    
 *            write_addr: Ҫд��ĵ�ַ
 * ��  ��:    ��                                                    
 * ����ֵ:    OK: д��ɹ�; NG: д��ʧ��                                                    
 * ��  ��:    2018-11-02 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
u8 SPI_FPGA_ByteWrite(u8 write_byte, u32 write_addr)
{
    u8 ret = OK;
    
    
    /* ѡ��FPGA: CS�͵�ƽ */
    SPI_FPGA_CS_LOW();
  
    /* ����д����*/
    if (NG == SPI_FPGA_SendByte(FPGA_WRITE_BYTE, NULL)) 
    {
        MTFS30_DEBUG("SPI_FPGA_SendByte ERROR! (%#x)", FPGA_WRITE_BYTE);
        SPI_FPGA_CS_HIGH();
        return NG;
    }

    /*����д��ĵ�ַ*/
    ret |= SPI_FPGA_SendByte(((write_addr & 0xFF000000) >> 24), NULL); 
    ret |= SPI_FPGA_SendByte(((write_addr & 0xFF0000) >> 16), NULL);
    ret |= SPI_FPGA_SendByte(((write_addr & 0xFF00) >> 8), NULL);
    ret |= SPI_FPGA_SendByte((write_addr & 0xFF), NULL);
    
    if (ret != OK)
    {
        MTFS30_DEBUG("SPI_FPGA_SendByte ERROR! (%#x)", write_addr);
        SPI_FPGA_CS_HIGH();
        return NG;
    }



    /* ����Ҫд����ֽ����� */
    if (NG == SPI_FPGA_SendByte(write_byte, NULL))
    {
        MTFS30_DEBUG("SPI_FPGA_SendByte ERROR! (%c)", write_byte);
        SPI_FPGA_CS_HIGH();
        return NG;
    }


    /* ֹͣ�ź� FPGA: CS �ߵ�ƽ */
    SPI_FPGA_CS_HIGH();

    return OK;
}

/*****************************************************************************
 * ��  ��:    SPI_FPGA_ByteRead                                                          
 * ��  ��:    ͨ��SPI���е��ֶ�����                                                                  
 * ��  ��:    p_byte: ָ�����������    
 *            read_addr: Ҫд��ĵ�ַ
 * ��  ��:    ��                                                    
 * ����ֵ:    OK: ��ȡ�ɹ�; NG:��ȡʧ��                                                    
 * ��  ��:    2018-11-02 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
u8 SPI_FPGA_ByteRead(u8 *p_byte, u32 read_addr)
{
    
    u8 ret = OK;
    
    
    /* ѡ��FPGA: CS�͵�ƽ */
    SPI_FPGA_CS_LOW();

    /* ���Ͷ�ָ�� */
    if (NG == SPI_FPGA_SendByte(FPGA_READ_BYTE, NULL))
    {
        MTFS30_DEBUG("SPI_FPGA_SendByte ERROR! (%#x)", FPGA_READ_BYTE);
        SPI_FPGA_CS_HIGH();
        return NG;
    }

    /* ���Ͷ���ַ */
    ret |= SPI_FPGA_SendByte(((read_addr & 0xFF000000) >> 24), NULL); 
    ret |= SPI_FPGA_SendByte(((read_addr & 0xFF0000) >> 16), NULL);
    ret |= SPI_FPGA_SendByte(((read_addr& 0xFF00) >> 8), NULL);
    ret |= SPI_FPGA_SendByte((read_addr & 0xFF), NULL);
    
    if (ret != OK) 
    {
        MTFS30_DEBUG("SPI_FPGA_SendByte ERROR! (%#x)", read_addr);
        SPI_FPGA_CS_HIGH();
        return NG;        
    }

    /* ��ȡһ���ֽ�*/
    //MTFS30_DEBUG("SPI_FPGA_ReadByte Start!");
    if (NG == SPI_FPGA_ReadByte(p_byte))
    {
        MTFS30_DEBUG("SPI_FPGA_ReadByte ERROR!");
        SPI_FPGA_CS_HIGH();
        return NG;
    }

    /* ֹͣ�ź� FPGA: CS �ߵ�ƽ */
    SPI_FPGA_CS_HIGH();

    return OK;
}

/*****************************************************************************
 * ��  ��:    SPI_FPGA_BufferWrite                                                          
 * ��  ��:    ͨ��SPI��������д����                                                                  
 * ��  ��:    pbuf: Ҫд�������    
 *            write_addr: Ҫд��ĵ�ַ
 *            write_size: Ҫд������ݳ���
 * ��  ��:    ��                                                    
 * ����ֵ:    OK: д��ɹ�; NG: д��ʧ��                                                    
 * ��  ��:    2018-11-02 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
u8 SPI_FPGA_BufferWrite(u8 *pbuf, u32 write_addr, u8 write_size)
{
    
    u8 ret = OK;
    
    
    /* ѡ��FPGA: CS�͵�ƽ */
    SPI_FPGA_CS_LOW();
  
    /* ����д����*/
    if (NG == SPI_FPGA_SendByte(FPGA_WRITE_MULBYTE, NULL))
    { 
        MTFS30_DEBUG("SPI_FPGA_ReadByte ERROR! (%#x)", FPGA_WRITE_MULBYTE);
        SPI_FPGA_CS_HIGH();
        return NG;
    }

    /*����д��ĵ�ַ*/
    ret |= SPI_FPGA_SendByte(((write_addr & 0xFF000000) >> 24), NULL);
    ret |= SPI_FPGA_SendByte(((write_addr & 0xFF0000) >> 16), NULL);
    ret |= SPI_FPGA_SendByte(((write_addr & 0xFF00) >> 8), NULL);
    ret |= SPI_FPGA_SendByte((write_addr & 0xFF), NULL);
 
    if (ret != OK) 
    {
        MTFS30_DEBUG("SPI_FPGA_SendByte ERROR! (%#x)", write_addr);
        SPI_FPGA_CS_HIGH();
        return NG;        
    }    
    
    /* ����д������ݳ��� */
    if (NG == SPI_FPGA_SendByte(write_size, NULL))
    {
        MTFS30_DEBUG("SPI_FPGA_SendByte ERROR! (%d)", write_size);
        SPI_FPGA_CS_HIGH();
        return NG;          
    }


    ret = OK;
    /* ����Ҫд����ֽ����� */
    while(write_size--)
    {
        ret |= SPI_FPGA_SendByte(*pbuf++, NULL);
    }
    
    if (ret != OK)
    {
        MTFS30_DEBUG("д��FPGA����������");
        SPI_FPGA_CS_HIGH();
        return NG;
    }
    

    
    /* ֹͣ�ź� FPGA: CS �ߵ�ƽ */
    SPI_FPGA_CS_HIGH();
    
    return OK;
}

/*****************************************************************************
 * ��  ��:    SPI_FPGA_BufferRead                                                          
 * ��  ��:    ͨ��SPI��������������                                                                  
 * ��  ��:    pbuf: ָ�����������    
 *            read_addr: Ҫ���ĵ�ַ
 *            read_size: Ҫ�������ݳ���
 * ��  ��:    ��                                                    
 * ����ֵ:    OK:�������ɹ�; NG��������ʧ��                                                    
 * ��  ��:    2018-11-02 changzehai(DTT)                            
 * ��  ��:    ��                                       
 ****************************************************************************/
u8 SPI_FPGA_BufferRead(u8 *pbuf, u32 read_addr, u8 read_size)
{
    
    u8 ret = OK;
    
    /* ѡ��FPGA: CS�͵�ƽ */
    SPI_FPGA_CS_LOW();
  
    /* ����д����*/
    if (NG == SPI_FPGA_SendByte(FPGA_READ_MULBYTE, NULL))
    {
        MTFS30_DEBUG("SPI_FPGA_ReadByte ERROR! (%#x)", FPGA_READ_MULBYTE);
        SPI_FPGA_CS_HIGH();
        return NG;        
    }

    /*����д��ĵ�ַ*/
    ret |= SPI_FPGA_SendByte(((read_addr & 0xFF000000) >> 24), NULL);
    ret |= SPI_FPGA_SendByte(((read_addr & 0xFF0000) >> 16), NULL);
    ret |= SPI_FPGA_SendByte(((read_addr & 0xFF00) >> 8), NULL);
    ret |= SPI_FPGA_SendByte((read_addr & 0xFF), NULL);

    if (ret != OK) 
    {
        MTFS30_DEBUG("SPI_FPGA_SendByte ERROR! (%#x)", read_addr);
        SPI_FPGA_CS_HIGH();
        return NG;        
    } 
    
    /* ����д������ݳ��� */
    if (NG == SPI_FPGA_SendByte(read_size, NULL))
    {
        MTFS30_DEBUG("SPI_FPGA_SendByte ERROR! (%d)", read_size);
        SPI_FPGA_CS_HIGH();
        return NG;     
    }


    ret = OK;
    /* �������� */
    while(read_size--)
    {
        
        ret |= SPI_FPGA_ReadByte(pbuf++);   
    }

    if (ret != OK)
    {
        MTFS30_DEBUG("��FPGA��ȡ����������");
        SPI_FPGA_CS_HIGH();
        return NG;                
    }   
    
    /* ֹͣ�ź� FPGA: CS �ߵ�ƽ */
    SPI_FPGA_CS_HIGH();

    return OK;
}
