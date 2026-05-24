/***********************
 * file : tpd_fw.c
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include "tpd_sys.h"



DECLARE_RWSEM(tp_firmware_list_lock);
LIST_HEAD(tp_firmware_list);

#define MAX_BUF_SIZE 256 * 1024
#define VENDOR_END 0xff

struct tpvendor_t synaptics_vendor_l[] ={
	{0x31, "TPK"},
	{0x32, "Truly"},
	{0x33, "Success"},
	{0x34, "Ofilm"},
	{0x35, "Lead"},
	{0x36, "Wintek"},
	{0x37, "Laibao"},
	{0x38, "CMI"},
	{0x39, "Ecw"},
	{0x41, "Goworld"},
	{0x42, "BaoMing"},
	{0x43, "Eachopto"},
	{0x44, "Mutto"},
	{0x45, "Junda"},	
	{0x46, "BOE"},
	{0x47, "Tianma"},
	{0x48, "Samsung"},
	{0x49, "DiJing"},
	{0x50, "LCE"},
	{VENDOR_END, "Unkown"},
};

struct tpvendor_t focal_vendor_l[] ={
	{0x11, "TeMeiKe"},
	{0x15, "ChuangWei"},
	{0x54, "EELY"},
	{0x55, "LaiBao"},
	{0x55, "LaiBao"},
	{0x57, "Goworld"},
	{0x5a, "Truly"},
	{0x5c, "TPK"},
	{0x5d, "BaoMing"},
	{0x5f, "Success"},
	{0x60, "Lead"},
	{0x67, "DiJing"},
	{0x80, "EACH"},
	{0x82, "HeLiTai"},
	{0x85, "JunDa"},
	{0x87, "LianChuang"},
	{0xda, "DiJingDA"},
	{0xf0, "TongXingDa"},
	{VENDOR_END, "Unkown"},
};

struct tpvendor_t cypress_vendor_l[] ={
	{0x01, "TPK"},
	{0x02, "Truly"},
	{0x03, "Success"},
	{0x04, "Ofilm"},
	{0x05, "Lead"},
	{0x06, "Wintek"},
	{0x07, "Laibao"},
	{0x08, "CMI"},
	{0x09, "Ecw"},
	{0x0a, "Goworld"},
	{0x0b, "BaoMing"},
	{0x0c, "Eachopto"},
	{0x0d, "Mutto"},
	{0x0e, "Junda"},
	{VENDOR_END, "Unkown"},
};

struct tpvendor_t atmel_vendor_l[] ={
	{0x06, "Wintek"},
	{0x07, "Laibao"},
	{0x08, "CMI"},
	{0x09, "Ecw"},
	{VENDOR_END, "Unkown"},
};

struct tpvendor_t goodix_vendor_l[] ={
	{0x00, "Eachopto"},
	{0x01, "Success"},
	{0x02, "TPK"},
	{0x03, "BaoMing"},
	{0x04, "Ofilm"},
	{0x05, "Truly"},
	{0x06, "Wintek"},
	{0x07, "Laibao"},
	{0x08, "CMI"},
	{0x09, "Ecw"},
	{0x0a, "Goworld"},
	{0x0b, "Lead"},
	{0x0c, "TeMeiKe"},
	{0x0d, "Mutto"},
	{0x0e, "Junda"},
	{0x0f, "TianMa"},
	{0x10, "SanXing"},
	{0x11, "GuoXian"},
	{0x12, "BoEn"},
	{VENDOR_END, "Unkown"},
};

struct tpvendor_t mstar_vendor_l[] ={
	{0x01, "FuNaYuanChuang"},
	{0x02, "TeMeiKe"},
	{VENDOR_END, "Unkown"},
};

struct tpvendor_t melfas_vendor_l[] ={
	{VENDOR_END, "Unkown"},
};

int tpd_power_already_on = 0;
bool LCM_esd_recovery = false;
struct tpd_classdev_t tpd_fw_cdev;

static struct class *tsp_fw_class;
//zhangjian add for TP esd check
int TP_ESD_check(void)
{	
	struct tpd_classdev_t *cdev = &tpd_fw_cdev;	
	int ret = 0;
	mutex_lock(&cdev->cmd_mutex);
	if(cdev->TP_ESD_check) {
		ret = cdev->TP_ESD_check(cdev);
	}
	mutex_unlock(&cdev->cmd_mutex);
	
	return ret;
}
//add end
static int get_chip_vendor(struct tpvendor_t * vendor_l, int count, int vendor_id, char *vendor_name)
{
	int i = 0;
	printk("%s: count: %d.\n", __func__, count);

	for(i = 0; i < count; i ++) {
		if(vendor_l[i].vendor_id == vendor_id || VENDOR_END == vendor_l[i].vendor_id) {
			strcpy(vendor_name, vendor_l[i].vendor_name);
			break;
		}
	}

	return 0;
}

static void tpd_get_tp_module_name(struct tpd_classdev_t *cdev)
{
	unsigned int vendor_id = 0;
	int size = 0;
	
	printk("%s \n", __func__);

	vendor_id = cdev->ic_tpinfo.module_id;
	
	if(NULL != strstr(cdev->ic_tpinfo.tp_name, "Synaptics")) {
		size = sizeof(synaptics_vendor_l) / sizeof(struct tpvendor_t);
		get_chip_vendor(synaptics_vendor_l, size, cdev->ic_tpinfo.module_id, cdev->ic_tpinfo.vendor_name);
	} else if (NULL != strstr(cdev->ic_tpinfo.tp_name, "Atmel")) {
		size = sizeof(atmel_vendor_l) / sizeof(struct tpvendor_t);
		get_chip_vendor(atmel_vendor_l, size, cdev->ic_tpinfo.module_id, cdev->ic_tpinfo.vendor_name);
	} else if (NULL != strstr(cdev->ic_tpinfo.tp_name, "Cyttsp")) {
		size = sizeof(cypress_vendor_l) / sizeof(struct tpvendor_t);
		get_chip_vendor(cypress_vendor_l, size, cdev->ic_tpinfo.module_id, cdev->ic_tpinfo.vendor_name);
	} else if (NULL != strstr(cdev->ic_tpinfo.tp_name, "Focal"))	{
		size = sizeof(focal_vendor_l) / sizeof(struct tpvendor_t);
		get_chip_vendor(focal_vendor_l, size, cdev->ic_tpinfo.module_id, cdev->ic_tpinfo.vendor_name);
	} else if (NULL != strstr(cdev->ic_tpinfo.tp_name, "Goodix")) {
		size = sizeof(goodix_vendor_l) / sizeof(struct tpvendor_t);
		get_chip_vendor(goodix_vendor_l, size, cdev->ic_tpinfo.module_id, cdev->ic_tpinfo.vendor_name);
		vendor_id = 0;
	} else if (NULL != strstr(cdev->ic_tpinfo.tp_name, "Melfas")) {
		size = sizeof(melfas_vendor_l) / sizeof(struct tpvendor_t);
		get_chip_vendor(melfas_vendor_l, size, cdev->ic_tpinfo.module_id, cdev->ic_tpinfo.vendor_name);
	} else if (NULL != strstr(cdev->ic_tpinfo.tp_name, "Mstar")) {
		size = sizeof(mstar_vendor_l) / sizeof(struct tpvendor_t);
		get_chip_vendor(mstar_vendor_l, size, cdev->ic_tpinfo.module_id, cdev->ic_tpinfo.vendor_name);
	} else {
		strcpy(cdev->ic_tpinfo.vendor_name, "Unkown.");
	}
	strcpy(cdev->file_tpinfo.vendor_name, cdev->ic_tpinfo.vendor_name);

	printk("fun:%s module name:%s.\n",__func__, cdev->ic_tpinfo.vendor_name);
	
}

static ssize_t tsp_fw_ic_tpinfo_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tpd_classdev_t *cdev = dev_get_drvdata(dev);

	if(cdev->get_tpinfo) {
		cdev->get_tpinfo(cdev);
	}

	return sprintf(buf, "%u 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x %s\n",
		cdev->ic_tpinfo.chip_part_id,   cdev->ic_tpinfo.chip_model_id,
		cdev->ic_tpinfo.chip_ver,        cdev->ic_tpinfo.module_id, 
		cdev->ic_tpinfo.firmware_ver, cdev->ic_tpinfo.config_ver, 
		cdev->ic_tpinfo.i2c_type,        cdev->ic_tpinfo.i2c_addr, 
		cdev->ic_tpinfo.tp_name);
}
static ssize_t tsp_fw_ic_tpinfo_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	return size;
}
static ssize_t tsp_test_save_file_path_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct tpd_classdev_t *cdev = dev_get_drvdata(dev);
	int retval = 0;

	if(cdev->tpd_test_get_save_filepath) {
		retval = cdev->tpd_test_get_save_filepath(cdev, buf);
	}

	return retval;
}

static ssize_t tsp_test_save_file_path_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct tpd_classdev_t *cdev = dev_get_drvdata(dev);
	int retval = 0;

	if(cdev->tpd_test_set_save_filepath) {
		retval = cdev->tpd_test_set_save_filepath(cdev, buf);
	}

	return count;
}


static ssize_t tsp_test_ini_file_path_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct tpd_classdev_t *cdev = dev_get_drvdata(dev);
	int retval = 0;

	if(cdev->tpd_test_get_ini_filepath) {
		retval = cdev->tpd_test_get_ini_filepath(cdev, buf);
	}

	return retval;
}

static ssize_t tsp_test_ini_file_path_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct tpd_classdev_t *cdev = dev_get_drvdata(dev);
	int retval = 0;

	if(cdev->tpd_test_set_ini_filepath) {
		retval = cdev->tpd_test_set_ini_filepath(cdev, buf);
	}

	return count;
}

static ssize_t tsp_test_filename_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct tpd_classdev_t *cdev = dev_get_drvdata(dev);
	int retval = 0;

	if(cdev->tpd_test_get_filename) {
		retval = cdev->tpd_test_get_filename(cdev, buf);
	}

	return retval;
}

static ssize_t tsp_test_filename_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct tpd_classdev_t *cdev = dev_get_drvdata(dev);
	int retval = 0;

	if(cdev->tpd_test_set_filename) {
		retval = cdev->tpd_test_set_filename(cdev, buf);
	}

	return count;
}

static ssize_t tsp_test_cmd_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct tpd_classdev_t *cdev = dev_get_drvdata(dev);
	int retval = 0;

	if(cdev->tpd_test_get_cmd) {
		retval = cdev->tpd_test_get_cmd(cdev, buf);
	}

	return retval;
}

static ssize_t tsp_test_cmd_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct tpd_classdev_t *cdev = dev_get_drvdata(dev);
	int retval = 0;

	if(cdev->tpd_test_set_cmd) {
		retval = cdev->tpd_test_set_cmd(cdev, buf);
	}

	return count;
}

static ssize_t tsp_test_node_data_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct tpd_classdev_t *cdev = dev_get_drvdata(dev);
	int retval = 0;

	if(cdev->tpd_test_get_node_data) {
		retval = cdev->tpd_test_get_node_data(cdev, buf);
	}

	return retval;
}

static ssize_t tsp_test_node_data_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct tpd_classdev_t *cdev = dev_get_drvdata(dev);
	int retval = 0;

	if(cdev->tpd_test_set_node_data_type) {
		retval = cdev->tpd_test_set_node_data_type(cdev, buf);
	}

	return count;
}
static ssize_t tsp_test_channel_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct tpd_classdev_t *cdev = dev_get_drvdata(dev);
	int retval = 0;

	if(cdev->tpd_test_get_channel_info) {
		retval = cdev->tpd_test_get_channel_info(cdev, buf);
	}

	return retval;
}
static ssize_t tsp_test_result_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct tpd_classdev_t *cdev = dev_get_drvdata(dev);
	int retval = 0;

	if(cdev->tpd_test_get_result) {
		retval = cdev->tpd_test_get_result(cdev, buf);
	}

	return retval;
}
//for tpd test
static DEVICE_ATTR(tpinfo, 0644, tsp_fw_ic_tpinfo_show, tsp_fw_ic_tpinfo_store);
static DEVICE_ATTR(tpd_test_result, S_IRUGO|S_IRUSR, tsp_test_result_show, NULL);
static DEVICE_ATTR(tpd_test_channel_setting, S_IRUGO|S_IRUSR, tsp_test_channel_show, NULL);
static DEVICE_ATTR(tpd_test_save_file_path, S_IRUGO|S_IWUSR, tsp_test_save_file_path_show, tsp_test_save_file_path_store);
static DEVICE_ATTR(tpd_test_ini_file_path, S_IRUGO|S_IWUSR, tsp_test_ini_file_path_show, tsp_test_ini_file_path_store);
static DEVICE_ATTR(tpd_test_filename, S_IRUGO|S_IWUSR, tsp_test_filename_show, tsp_test_filename_store);
static DEVICE_ATTR(tpd_test_cmd, S_IRUGO|S_IWUSR, tsp_test_cmd_show, tsp_test_cmd_store);
static DEVICE_ATTR(tpd_test_node_data, S_IRUGO|S_IWUSR, tsp_test_node_data_show, tsp_test_node_data_store);

static struct attribute *tsp_dev_attrs[] = {	

	//for tpd test
	&dev_attr_tpinfo.attr,
	&dev_attr_tpd_test_filename.attr,
	&dev_attr_tpd_test_node_data.attr,
	&dev_attr_tpd_test_cmd.attr,
	&dev_attr_tpd_test_ini_file_path.attr,
	&dev_attr_tpd_test_save_file_path.attr,
	&dev_attr_tpd_test_channel_setting.attr,
	&dev_attr_tpd_test_result.attr,
	NULL,
};

static const struct attribute_group tsp_dev_attribute_group = {
	.attrs = tsp_dev_attrs,
};

#define	TPD_PROC_FILE	"driver/tsc_id"
static struct proc_dir_entry *tpd_proc_entry;

static ssize_t tpd_proc_read_val(struct file *file,
	char __user *buffer, size_t count, loff_t *offset)
{
    ssize_t len = 0;
    uint8_t buffer_tpd[200];
    struct tpd_classdev_t *cdev = &tpd_fw_cdev;
    if(cdev->get_tpinfo) 
    {
        cdev->get_tpinfo(cdev);
    }
    tpd_get_tp_module_name(cdev);    
    len += sprintf(buffer_tpd+len, "TP module: %s(0x%x)\n",cdev->ic_tpinfo.vendor_name,cdev->ic_tpinfo.module_id);
    len += sprintf(buffer_tpd+len, "IC type : %s\n", cdev->ic_tpinfo.tp_name);
    len += sprintf(buffer_tpd+len, "I2C address: 0x%x\n",cdev->ic_tpinfo.i2c_addr); 
    len += sprintf(buffer_tpd+len, "Firmware version : 0x%x \n",cdev->ic_tpinfo.firmware_ver);
    if (cdev->ic_tpinfo.config_ver)
        len += sprintf(buffer_tpd+len,  "Config version:0x%x\n",cdev->ic_tpinfo.config_ver);
    return simple_read_from_buffer(buffer, count, offset, buffer_tpd, len);
}
static ssize_t tpd_proc_write_val(struct file *filp,
					 const char *buff, size_t len,
					 loff_t * off)
{
	return len;
}

static struct file_operations tpd_proc_ops = {
	.read = tpd_proc_read_val,
	.write = tpd_proc_write_val,
};

static void create_tpd_proc_entry(void)
{
	tpd_proc_entry = proc_create(TPD_PROC_FILE, 0644, NULL, &tpd_proc_ops);
	if (tpd_proc_entry) {
		printk(KERN_INFO "create proc file sucess!\n");
	} else
		printk(KERN_INFO "create proc file failed!\n");
}
static ssize_t tp_sleep_mode_read(struct file *file,
					 char __user *buffer, size_t count, loff_t *offset)
{
	ssize_t len = 0;
	uint8_t data_buf[10] = {0};
	struct tpd_classdev_t *cdev = &tpd_fw_cdev;

	if (cdev->get_sleep_mode) {
		cdev->get_sleep_mode(cdev);
	}
	pr_notice("tpd: %s val:%d.\n", __func__, cdev->b_sleep_enable);

	len = snprintf(data_buf, sizeof(data_buf), "%u\n", cdev->b_sleep_enable);
	return simple_read_from_buffer(buffer, count, offset, data_buf, len);
}

static ssize_t tp_sleep_mode_write(struct file *file,
				const char __user *buffer, size_t len, loff_t *off)
{
	int ret;
	unsigned int input;
	char data_buf[10] = {0};
	struct tpd_classdev_t *cdev = &tpd_fw_cdev;
	pr_notice("tpd: %s.\n", __func__);
	ret = copy_from_user(data_buf, buffer, len);
	if (ret)
		return -EINVAL;
	ret = kstrtouint(data_buf, 0, &input);
	if (ret)
		return -EINVAL;
	input = input > 0 ? 1 : 0;
	pr_notice("tpd: %s val %d.\n", __func__, input);
	if (cdev->set_sleep_mode) {
		cdev->set_sleep_mode(cdev, input);
	}
	return len;
}

static const struct file_operations proc_ops_sleep_mode = {
	.owner = THIS_MODULE,
	.read =  tp_sleep_mode_read,
	.write = tp_sleep_mode_write,
};

static void create_tpd_sleep_mode_entry(void)
{
	struct proc_dir_entry *tpd_proc_dir;
	struct proc_dir_entry *tpd_proc_entry;

	tpd_proc_dir = proc_mkdir("touchscreen", NULL);
	tpd_proc_entry = proc_create("sleep_mode_enable", 0664, tpd_proc_dir, &proc_ops_sleep_mode);
	if (tpd_proc_entry == NULL)
		pr_notice("proc create sleep_mode_enable failed!\n");	
}
static struct kobject *tpd_wake_gesture_kobj;
static ssize_t tpd_wake_gesture_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct tpd_classdev_t *cdev = &tpd_fw_cdev;
	int retval = 0;

	mutex_lock(&cdev->cmd_mutex);
	if(cdev->get_gesture) {
		cdev->get_gesture(cdev);
	}
	printk("tpd: %s val:%d.\n", __func__, cdev->b_gesture_enable);

	retval = snprintf(buf, 32, "%d,\n", cdev->b_gesture_enable);
	mutex_unlock(&cdev->cmd_mutex);

	return retval;
}

static ssize_t tpd_wake_gesture_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int input;
	struct tpd_classdev_t *cdev = &tpd_fw_cdev;
	
	if (sscanf(buf, "%u", &input) != 1)
		return -EINVAL;

	input = input > 0 ? 1 : 0;
	
	printk("tpd: %s val %d.\n", __func__, input);

	mutex_lock(&cdev->cmd_mutex);
	if(cdev->wake_gesture) {
		cdev->wake_gesture(cdev, input);
	}
	mutex_unlock(&cdev->cmd_mutex);
	
	return count;
}

static DEVICE_ATTR(wake_gesture, 0664/*S_IRUGO|S_IWUSR|S_IWGRP*/,  tpd_wake_gesture_show,  tpd_wake_gesture_store);

/* add your attr in here*/
static struct attribute *tpd_bsg_attributes[] = {
	&dev_attr_wake_gesture.attr,	
	NULL
};

static struct attribute_group tpd_bsg_attribute_group = {
	.attrs = tpd_bsg_attributes
};


int tpd_create_wake_gesture_sysfs(void)
{
	int err;

	tpd_wake_gesture_kobj = kobject_create_and_add("tp_wake_gesture", NULL);
	if (!tpd_wake_gesture_kobj) {
		err = -EINVAL;
		printk("%s() - ERROR Unable to create tpd_wake_gesture_kobj.\n", __func__);
		return -EIO;
	}

	err = sysfs_create_group(tpd_wake_gesture_kobj, &tpd_bsg_attribute_group);
	if (0 != err) 
	{
		printk("%s - ERROR sysfs_create_group failed.\n",__func__);
		
		kobject_put(tpd_wake_gesture_kobj);
		return -EIO;
	} else {
		printk("%s succeeded.\n",__func__);
	}
	
	return err;
}

int tpd_remove_wake_gesture_sysfs(void)
{
	sysfs_remove_group(tpd_wake_gesture_kobj, &tpd_bsg_attribute_group);
	kobject_put(tpd_wake_gesture_kobj);
	return 0;
}
/**
 * tpd_classdev_register - register a new object of tpd_classdev_t class.
 * @parent: The device to register.
 * @tsp_fw_cdev: the tpd_classdev_t structure for this device.
 */
int tpd_classdev_register(struct device *parent, struct tpd_classdev_t *tsp_fw_cdev)
{
	
	tsp_fw_cdev->dev = device_create(tsp_fw_class, NULL, 0, tsp_fw_cdev,
					  "%s", tsp_fw_cdev->name);
	if (IS_ERR(tsp_fw_cdev->dev))
		return PTR_ERR(tsp_fw_cdev->dev);
	mutex_init(&tsp_fw_cdev->cmd_mutex);	
	//tpd_create_wake_gesture_sysfs();
	create_tpd_proc_entry();
	create_tpd_sleep_mode_entry();
	printk("tpd: Registered tsp_fw device: %s\n",tsp_fw_cdev->name);

	return 0;
}
EXPORT_SYMBOL_GPL(tpd_classdev_register);

/**
 * tpd_classdev_unregister - unregisters a object of tsp_fw_properties class.
 * @tsp_fw_cdev: the tsp_fw device to unregister
 *
 * Unregisters a previously registered via tpd_classdev_register object.
 */
void tpd_classdev_unregister(struct tpd_classdev_t *tsp_fw_cdev)
{
	device_unregister(tsp_fw_cdev->dev);
	
}
EXPORT_SYMBOL_GPL(tpd_classdev_unregister);


static int __init tpd_class_init(void)
{
      int err = 0;
	tsp_fw_class = class_create(THIS_MODULE, "tsp_fw");
	if (IS_ERR(tsp_fw_class))
		return PTR_ERR(tsp_fw_class);	
      tpd_fw_cdev.name = "touchscreen";
      tpd_fw_cdev.private = NULL;
      tpd_fw_cdev.read_block = NULL;
      tpd_fw_cdev.write_block = NULL;
      tpd_fw_cdev.get_tpinfo = NULL;	
      tpd_fw_cdev.get_gesture = NULL;
      tpd_fw_cdev.wake_gesture = NULL;
      tpd_fw_cdev.get_sleep_mode = NULL;
      tpd_fw_cdev.set_sleep_mode = NULL;
      tpd_fw_cdev.b_gesture_enable = 0;
      tpd_fw_cdev.b_sleep_enable = 0;
      //for tpd test
      tpd_fw_cdev.tpd_test_set_save_filepath = NULL;
      tpd_fw_cdev.tpd_test_get_save_filepath = NULL;
      tpd_fw_cdev.tpd_test_set_ini_filepath = NULL;
      tpd_fw_cdev.tpd_test_get_ini_filepath = NULL;
      tpd_fw_cdev.tpd_test_set_filename = NULL;
      tpd_fw_cdev.tpd_test_get_filename = NULL;
      tpd_fw_cdev.tpd_test_set_cmd = NULL;
      tpd_fw_cdev.tpd_test_get_cmd = NULL;
      tpd_fw_cdev.tpd_test_set_node_data_type = NULL;
      tpd_fw_cdev.tpd_test_get_node_data = NULL;
      tpd_fw_cdev.tpd_test_get_channel_info = NULL;
      tpd_fw_cdev.tpd_test_get_result = NULL;
      tpd_fw_cdev.TP_ESD_check =NULL; //zhangjian add for TP esd check
      tpd_classdev_register(NULL, &tpd_fw_cdev);
      if (IS_ERR(tpd_fw_cdev.dev)) 
      {
          printk(" tpd_fw_cdev dev null.");
      } 
      else
      {
          err = sysfs_create_group(&tpd_fw_cdev.dev->kobj, &tsp_dev_attribute_group);
          if (0 != err)
          {
              printk( "%s() - ERROR: sysfs_create_group() failed.",  __func__);
          }
          else
          {
              printk("%s() - sysfs_create_group() succeeded.", __func__);
          }
      }
      return 0;
}

static void __exit tpd_class_exit(void)
{
	
	class_destroy(tsp_fw_class);
}

subsys_initcall(tpd_class_init);
module_exit(tpd_class_exit);

MODULE_AUTHOR("John Lenz, Richard Purdie");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TSP FW Class Interface");


