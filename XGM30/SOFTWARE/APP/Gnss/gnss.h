/*****************************************************************************/
/* �ļ���:    gnss.h                                                         */
/* ��  ��:    gnss����ͷ�ļ�                                                 */
/* ��  ��:    2018-07-19 changzehai(DTT)                                     */
/* ��  ��:    ��                                                             */
/* Copyright 1998 - 2018 DTT. All Rights Reserved                            */
/*****************************************************************************/
#ifndef __GNSS_H_
#define __GNSS_H_
#include "arch/cc.h"


/*-------------------------------*/
/* �궨��                        */
/*-------------------------------*/

/* �ڶ��峤��ʱ��Ҫ���һ��λ������ '\0' */
/* ��Ϣ���ȶ��� */
#define    GNSS_LOCATION_INFO_LEN      56    /* ��λ��Ϣ����       */
#define    GNSS_STATELLITE_INFO_LEN    400   /* ����״̬��Ϣ����   */
#define    GNSS_LEAP_INFO_LEN          28    /* ����Ԥ����Ϣ����   */
#define    GNSS_MODE_INFO_LEN          17    /* ����ģʽ��Ϣ����   */
#define    GNSS_1PPS_INFO_LEN          3     /* 1PPS״̬��Ϣ����   */
#define    GNSS_ANTENNA_INFO_LEN       6     /* ����״̬��Ϣ����   */
#define    GNSS_VERSION_INFO_LEN       30    /* GNSS���ջ��汾��Ϣ */
#define    GNSS_TIME_INFO_LEN          22    /* GNSSʱ����Ϣ       */
#define    GNSS_RECVIVER_NAME_LEN      10    /* GNSS���ջ��ͺ����� */

#define    GNSS_LOCATION_SATS          3     /* ��λ�������ݳ���   */
#define    GNSS_LOCATION_TIME          10    /* UTCʱ�����ݳ���    */
#define    GNSS_LOCATION_LAT           14    /* γ�����ݳ���       */
#define    GNSS_LOCATION_LON           15    /* �������ݳ���       */
#define    GNSS_LOCATION_ALT           9     /* �������ݳ���       */

#define    GNSS_SAT_NUM_MAX            36    /* ���յ����������   */
#define    GNSS_SAT_INFO_MAX_LEN       400   /* ������Ϣ��󳤶�   */


#define    GNSS_LEAP_TIME_HMS_LEN      7     /* ���뷢��ʱ���볤�� */

#define    GNSS_GSV_MSG_MAX_LEN        70  /* GSV��Ϣ��󳤶� */

#define    GNSS_MSG_MAX_LEN          2048 /* ������Ϣ��󳤶� */

     

/*-------------------------------*/
/* �ṹ����                      */
/*-------------------------------*/

/* GNSSʱ������ */
typedef struct _TIME
{
    u8_t  hms[GNSS_LEAP_TIME_HMS_LEN]; /* ʱ���� */
    u8_t  day; /* ��   */
    u8_t  mon; /* ��   */
    u16_t year;/* ��   */
    u8_t  zone;/* ʱ�� */ 
}gnss_time_t;


/* ��λ���� */
typedef struct _GNSS_LOCATION_T_
{
    u8_t           sats;                          /* ��λ����        */
    u8_t           time[GNSS_LOCATION_TIME];      /* UTCʱ��         */
    u8_t           lat[GNSS_LOCATION_LAT];        /* γ��            */ 
    u8_t           latdir;                        /* γ�ȷ���        */
    u8_t           lon[GNSS_LOCATION_LON];        /* ����            */
    u8_t           londir;                        /* ���ȷ���        */
    u8_t           alt[GNSS_LOCATION_ALT];        /* ����            */
} gnss_location_t;




/* ��λ��Ϣ */
typedef struct _GNSS_LOCATION_INFO_T_
{

  u8_t info[GNSS_LOCATION_INFO_LEN];

} gnss_location_info_t;

/* ����������Ϣ */
typedef struct _GNSS_SAT_T_
{
    u8_t enable;/* �Ƿ����    */
    u8_t prn;   /* ����PRN��� */
	u8_t snr;   /* �����      */
	u8_t elev;  /* ����        */
    
} gnss_sat_t;

/* ������Ϣ */
typedef struct _GNSS_SAT_INFO_T_ 
{
    u16_t    len;
    u8_t     info[GNSS_STATELLITE_INFO_LEN];
    
} gnss_satellite_info_t;

/* ����Ԥ����Ϣ */
typedef struct _GNSS_LEAP_INFO_T_
{
    u8_t info[GNSS_LEAP_INFO_LEN];
} gnss_leap_info_t;


/* ����ģʽ��Ϣ */
typedef struct _GNSS_MODE_INFO_T_
{
    u8_t info[GNSS_MODE_INFO_LEN];
} gnss_mode_info_t;


/* ����״̬��Ϣ */
typedef struct _ANTENNA_INFO_T_
{
    u8_t info[GNSS_ANTENNA_INFO_LEN];
} gnss_antenna_info_t;


/* 1PPS״̬��Ϣ */
typedef struct _GNSS_PPS_INFO_T_
{
    u8_t info[GNSS_1PPS_INFO_LEN];
} gnss_pps_info_t;


/* GNSS���ջ��汾��Ϣ */
typedef struct _GNSS_VERSION_INFO_T_
{
    u8_t info[GNSS_VERSION_INFO_LEN];

} gnss_version_info_t;

/* GNSSʱ����Ϣ */
typedef struct _GNSS_TIME_INFO_T_
{
    u8_t info[GNSS_TIME_INFO_LEN];

} gnss_time_info_t;


/* GNSS��Ϣ */
typedef struct _GNSS_INFO_T_
{
  gnss_mode_info_t        mode; /* ����ģʽ��Ϣ       */
  gnss_time_info_t        time; /* GNSSʱ����Ϣ       */
  gnss_location_info_t    loc;  /* ��λ��Ϣ           */
  gnss_satellite_info_t   sat;  /* ������Ϣ           */
  gnss_leap_info_t        leap; /* ����Ԥ����Ϣ       */
  gnss_pps_info_t         pps;  /* 1PPS״̬��Ϣ       */
  gnss_antenna_info_t     ant;  /* ����״̬��Ϣ       */
  gnss_version_info_t     ver;  /* GNSS���ջ��汾��Ϣ */
  
} gnss_info_t;


/* GNSS������������ */
typedef struct  _GNSS_STMT_HANDLE_T_
{
	u8_t* p_head;     /* ����ͷ                      */
    u8_t  check_type; /* У������                    */
    void (*parse_func)(u8_t *p_param);   /* �������� */
} gnss_stmt_handle_t;



/* ���ջ���Ϣ */
typedef struct _GNSS_RECEIVER_INFO_T_
{
    u8_t post_sats;                         /* ��λ����        */
    u8_t ac_mode;	                        /* ���ջ�����ģʽ  */
    u8_t ant_sta;					        /* ����״̬        */
    u8_t rec_type;		                    /* ���յ�����      */
    u8_t rec_name[GNSS_RECVIVER_NAME_LEN];  /*���յ��ͺ�����   */
    u8_t pps_status;                        /* 1PPS״̬        */
} gnss_recviver_info_t;


/* ������Ϣ */
typedef struct _GNSS_LEAP_T_
{
    u8_t        now_leap;      /* ��ǰ����      */
    u8_t        next_leap;     /* �µ�����      */
    u8_t        sign;          /* ��/��/�� ���� */    
    gnss_time_t leap_time;     /* ���뷢��ʱ��  */

} gnss_leap_t;

#endif
