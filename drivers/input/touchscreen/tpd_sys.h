/*
 * FILE:__TSP_FW_CLASS_H_INCLUDED
 *
 */
#ifndef __TPD_FW_H_INCLUDED
#define __TPD_FW_H_INCLUDED

#include <linux/device.h>
#include <linux/rwsem.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/input.h>




struct tpvendor_t {
	int vendor_id;
	char * vendor_name;
};
/*chip_model_id synaptics 1,atmel 2,cypress 3,focal 4,goodix 5,mefals 6,mstar 7,himax 8;
 *
 */
struct tpd_tpinfo_t {
	unsigned int chip_model_id;
	unsigned int chip_part_id;
	unsigned int chip_ver;
	unsigned int module_id;
	unsigned int firmware_ver;
	unsigned int config_ver;
	unsigned int i2c_addr;
	unsigned int i2c_type;
	char tp_name[20];
	char vendor_name[20];
};

struct tpd_classdev_t {
     const char *name;	
     int b_gesture_enable;
     int b_sleep_enable;
     int b_force_upgrade;
     int (*read_block)(struct tpd_classdev_t *cdev, u16 addr, u8 *buf, int len);
     int (*write_block)(struct tpd_classdev_t *cdev, u16 addr, u8 *buf, int len);
     int (*flash_fw)(struct tpd_classdev_t *cdev, unsigned char * data, unsigned int size, int force_upg);
     int (*compare_tp_version)(struct tpd_classdev_t *cdev, unsigned char *data);
     int (*get_gesture)(struct tpd_classdev_t *cdev);
     int (*wake_gesture)(struct tpd_classdev_t *cdev, int enable);
     int (*get_sleep_mode)(struct tpd_classdev_t *cdev);
     int (*set_sleep_mode)(struct tpd_classdev_t *cdev, int enable);
     int (*TP_ESD_check)(struct tpd_classdev_t *cdev); //zhangjian add for TP esd check       
     int (*tpd_test_set_save_filepath)(struct tpd_classdev_t *cdev, const char *buf);
     int (*tpd_test_get_save_filepath)(struct tpd_classdev_t *cdev, char *buf);
     int (*tpd_test_set_ini_filepath)(struct tpd_classdev_t *cdev, const char *buf);
     int (*tpd_test_get_ini_filepath)(struct tpd_classdev_t *cdev, char *buf);
     int (*tpd_test_set_filename)(struct tpd_classdev_t *cdev, const char *buf);
     int (*tpd_test_get_filename)(struct tpd_classdev_t *cdev, char *buf);
     int (*tpd_test_set_cmd)(struct tpd_classdev_t *cdev, const char *buf);
     int (*tpd_test_get_cmd)(struct tpd_classdev_t *cdev, char *buf);
     int (*tpd_test_set_node_data_type)(struct tpd_classdev_t *cdev, const char *buf);
     int (*tpd_test_get_node_data)(struct tpd_classdev_t *cdev, char *buf);
     int (*tpd_test_get_channel_info)(struct tpd_classdev_t *cdev, char *buf);
     int (*tpd_test_get_result)(struct tpd_classdev_t *cdev, char *buf);
     int (*get_tpinfo)(struct tpd_classdev_t *cdev);
     void *private;
     void *test_node;    //added for tp test.
     struct mutex cmd_mutex;	
     struct tpd_tpinfo_t ic_tpinfo;
     struct tpd_tpinfo_t file_tpinfo;
     struct device		*dev;
     struct list_head	 node;
};
extern  int TP_ESD_check(void);
extern bool LCM_esd_recovery;
extern int msm_dss_vreg_power_off(void);
extern bool power_off_remove_to_TP;
extern struct tpd_classdev_t tpd_fw_cdev;
extern int tpd_classdev_register(struct device *parent, struct tpd_classdev_t *tsp_fw_cdev);
#endif	/* __TSP_FW_CLASS_H_INCLUDED */

