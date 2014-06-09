/******************************************************************************************************
 *
 *   Copyright (C)  IQOO,2012			All Riights Reserved!
 *
 *   File: bbk_wakesrc.c
 * 
 *   Module: Wakeup record device and driver.
 *
 *   Author: liuchangjian
 *
 *   Date: 2012-11-06
 *
 *   E-mail: liuchangjian@iqoo.com
 *
 *********************************************************************************************************/


/*********************************************************************************************************
 *
 *   Abstract:
 *		Wake up record driver.
 *
 *********************************************************************************************************/


/**********************************************************************************************************
 *
 *   History:
 *
 *	Name		Date		Ver		Action
 *   ------------------------------------------------------------------------------------------------------
 *	liuchangjian	2012-11-06	v1.0		create
 *
 *********************************************************************************************************/

#ifndef __BBK_WAKESRC_C__
#define __BBK_WAKESRC_C__

/** headers */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 

#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/time.h>

/** log switch set */
static int enable_log = 1;
#define LOG_LEVEL	 2
#define LOG_P			printk

#if (LOG_LEVEL == 2)
#define MSG_LOG(...) \
{ \
	if(enable_log) \
		LOG_P(__VA_ARGS__); \
}	

#define DRV_LOG(...) \
{ \
	if(enable_log) \
		LOG_P(__VA_ARGS__); \
}	
#elif (LOG_LEVEL == 1)
#define MSG_LOG(...) 
#define DRV_LOG(...) \
{ \
	if(enable_log) \
		LOG_P(__VA_ARGS__); \
}	
#else
#define MSG_LOG(...) 
#define DRV_LOG(...) 
#endif

/** data struct */
#define WS_DAT_LEN			1024	
typedef struct bbk_wakesrc_data_t
{
	int		enable;
	unsigned int	total;
	unsigned int 	cnt;
	struct rtc_time *rtc_data;
	unsigned int *wakesrc;
}bbk_wakesrc_data;

static bbk_wakesrc_data ws_dat;

MODULE_LICENSE("GPL");

static struct platform_device bbk_wakesrc_dev = {
	.name = "bbk_wakesrc",
	.id = -1,
};

/**************************************************** interface ****************************************************************/
void bbk_wakesrc_set(unsigned int wakesrc)
{
	struct timeval now_time;
	if(0 == ws_dat.enable)
		return;

	/** record wake src */
	*(ws_dat.wakesrc+ws_dat.cnt) = wakesrc;
	MSG_LOG("\n%s: bbk set wake src is %d!\n",__FUNCTION__,wakesrc);
	
	/** record time */
	do_gettimeofday(&now_time);
	rtc_time_to_tm(now_time.tv_sec,ws_dat.rtc_data+ws_dat.cnt);
	MSG_LOG("%s: bbk record time is %02d-%02d %02d:%02d:%02d!\n\n",(ws_dat.rtc_data+ws_dat.cnt)->tm_mon,(ws_dat.rtc_data+ws_dat.cnt)->tm_mday,(ws_dat.rtc_data+ws_dat.cnt)->tm_hour,(ws_dat.rtc_data+ws_dat.cnt)->tm_min,(ws_dat.rtc_data+ws_dat.cnt)->tm_sec);

	ws_dat.cnt++;
	if(ws_dat.cnt >= WS_DAT_LEN)
	{
		ws_dat.cnt = 0;
		ws_dat.total++;
	}
}
EXPORT_SYMBOL(bbk_wakesrc_set);

/************************************************** interface  end****************************************************************/
/* MTK chip wake src SLPCTRL wakeup sources */
#define WAKE_SRC_TS             (1U << 1)
#define WAKE_SRC_KP             (1U << 2)
#define WAKE_SRC_MSDC1          (1U << 3)
#define WAKE_SRC_GPT            (1U << 4)
#define WAKE_SRC_EINT           (1U << 5)
#define WAKE_SRC_RTC            (1U << 6)
#define WAKE_SRC_CCIF_MD        (1U << 7)
#define WAKE_SRC_ACCDET         (1U << 8)
#define WAKE_SRC_LOW_BAT        (1U << 9)
#define WAKE_SRC_CA9_DBG        (1U << 19)
#define WAKE_SRC_AFE            (1U << 21)

#define NUM_WAKE_SRC            22

static int get_wakesrc_reason(unsigned int wakesta,char *str)
{

	if (wakesta & WAKE_SRC_KP)
                strcat(str, "KP ");

        if (wakesta & WAKE_SRC_MSDC1)
                strcat(str, "MSDC1 ");

        if (wakesta & WAKE_SRC_EINT)
                strcat(str, "EINT ");

        if (wakesta & WAKE_SRC_RTC)
                strcat(str, "RTC ");

        if (wakesta & WAKE_SRC_CCIF_MD) {
                strcat(str, "CCIF_MD ");
        }

        if (wakesta & WAKE_SRC_ACCDET)
                strcat(str, "ACCDET ");

        if (wakesta & WAKE_SRC_TS)
                strcat(str, "TS ");

        if (wakesta & WAKE_SRC_GPT)
                strcat(str, "GPT ");

        if (wakesta & WAKE_SRC_LOW_BAT)
                strcat(str, "LOW_BAT ");

        if (wakesta & WAKE_SRC_CA9_DBG)
                strcat(str, "CA9_DBG ");
	
	return 0;
}

static ssize_t show_wakesrc_data(struct device *dev,struct device_attribute *attr,char *buf)
{
	unsigned int len=0,str_len=0,total_len=0;
	unsigned int i;
	unsigned int ret = 0;
	char *data;
	char str[36] = { 0 };

	data = buf;
	
	if(ws_dat.total)
		len = WS_DAT_LEN;
	else
		len = ws_dat.cnt;
	MSG_LOG("\n");
	for(i=0;i<len;i++)
	{
		memset(str,0,36);
		get_wakesrc_reason(*(ws_dat.wakesrc+i),str);
		if(0 == strlen(str))
		{
			strcat(str,"SLP_TMR ");
		}
		str_len = sprintf(data,"%02d-%02d %02d:%02d:%02d Wakesrc is %s \n",(ws_dat.rtc_data+i)->tm_mon,(ws_dat.rtc_data+i)->tm_mday,((ws_dat.rtc_data+i)->tm_hour+8)%24,(ws_dat.rtc_data+i)->tm_min,(ws_dat.rtc_data+i)->tm_sec,str);
		data += str_len;
		total_len += str_len;
		
		MSG_LOG("%s: data len is %d\n",__FUNCTION__,str_len);
		MSG_LOG("%02d-%02d %02d:%02d:%02d Wakesrc is %s \n",(ws_dat.rtc_data+i)->tm_mon,(ws_dat.rtc_data+i)->tm_mday,((ws_dat.rtc_data+i)->tm_hour+8)%24,(ws_dat.rtc_data+i)->tm_min,(ws_dat.rtc_data+i)->tm_sec,str);
	}
	MSG_LOG("\n%s: read data totoal is %d!\n\n",__FUNCTION__,total_len);

	ret = total_len;
	return ret;
}
static DEVICE_ATTR(wakesrc_data,0444,show_wakesrc_data,NULL);

static int bbk_wakesrc_probe(struct platform_device *dev)
{
	int ret;

	LOG_P("%s: bbk wake src probe!\n",__FUNCTION__);

	ws_dat.rtc_data = (struct rtc_time *)kmalloc(sizeof(struct rtc_time)*WS_DAT_LEN,GFP_KERNEL);
	if(NULL == ws_dat.rtc_data)
	{
		LOG_P("%s(ERROR): Can't kmalloc rtc_time memory buffer!!!\n",__FUNCTION__);
		return -1;
	}
	memset(ws_dat.rtc_data,0,sizeof(struct rtc_time)*WS_DAT_LEN);	
	
	ws_dat.wakesrc = kmalloc(sizeof(unsigned int)*WS_DAT_LEN,GFP_KERNEL);
	if(NULL == ws_dat.rtc_data)
	{
		LOG_P("%s(ERROR): Can't kmalloc wakesrc data memory buffer!!!\n",__FUNCTION__);
		goto err;
	}
	memset(ws_dat.wakesrc,0,sizeof(unsigned int)*WS_DAT_LEN);	

	ret = device_create_file(&(dev->dev),&dev_attr_wakesrc_data);
	if(ret)
	{
		LOG_P("%s(ERROR): Can't create sysfs interface!!!\n",__FUNCTION__);
		goto err1;
	}

	ws_dat.enable = 1;	
	ws_dat.total = 0;
	ws_dat.cnt = 0;
	return 0;
err1:
	kfree(ws_dat.wakesrc);
err:
	kfree(ws_dat.rtc_data);
	return -1;
}


static struct platform_driver bbk_wakesrc_drv = {
	.probe = bbk_wakesrc_probe,
	.driver ={
		.name = "bbk_wakesrc",
	},
};

static int __init bbk_wakesrc_init(void)
{
	int ret;
	DRV_LOG("%s: init bbk wakesrc driver!!!\n",__FUNCTION__);

	ret = platform_device_register(&bbk_wakesrc_dev);
	if(ret)
	{
		LOG_P("%s(ERROR): Can't register bbk_wakesrc_dev!!!\n",__FUNCTION__);
		return -1;
	}
	
	ret = platform_driver_register(&bbk_wakesrc_drv);
	if(ret)
	{
		LOG_P("%s(ERROR): Can't register bbk_wakesrc_drv!!!\n",__FUNCTION__);
		return -1;
	}
	LOG_P("%s: init bbk wakesrc driveri OK!!!\n",__FUNCTION__);
	
	return 0;
}

static void __exit bbk_wakesrc_exit(void)
{
	platform_device_unregister(&bbk_wakesrc_dev);
	platform_driver_unregister(&bbk_wakesrc_drv);
	kfree(ws_dat.wakesrc);
	kfree(ws_dat.rtc_data);
	ws_dat.enable = 0;	
	ws_dat.total = 0;
	ws_dat.cnt = 0;
}

module_init(bbk_wakesrc_init);
module_exit(bbk_wakesrc_exit);
MODULE_AUTHOR("liuchangjian");


#endif
