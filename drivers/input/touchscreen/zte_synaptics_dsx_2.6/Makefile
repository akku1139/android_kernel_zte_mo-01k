#
# Makefile for the Synaptics DSX touchscreen driver.
#

# Each configuration option enables a list of files.
ccflags-y += -I$(srctree)/drivers/input/touchscreen/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/zte_synaptics_dsx_2.6/project_config/$(CONFIG_FIRMWARE_CONFIG)/
obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_I2C) += synaptics_dsx_i2c.o
#obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_SPI) += synaptics_dsx_spi.o
#obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_RMI_HID_I2C) += synaptics_dsx_rmi_hid_i2c.o
obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_CORE) += synaptics_dsx_core.o
obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_RMI_DEV) += synaptics_dsx_rmi_dev.o
obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_FW_UPDATE) += synaptics_dsx_fw_update.o
obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_TEST_REPORTING) += synaptics_dsx_test_reporting.o
#obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_PROXIMITY) += synaptics_dsx_proximity.o
#obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_ACTIVE_PEN) += synaptics_dsx_active_pen.o
obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_GESTURE) += synaptics_dsx_gesture.o
obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_VIDEO) += synaptics_dsx_video.o
#obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_DEBUG) += synaptics_dsx_debug.o
obj-$(CONFIG_TOUCHSCREEN_SPECIAL_INTERFACE) += synaptics_dsx_test.o
obj-$(CONFIG_TOUCHSCREEN_SPECIAL_INTERFACE) += synaptics_dsx_test_entry.o
obj-$(CONFIG_TOUCHSCREEN_SPECIAL_INTERFACE) += synaptics_dsx_test_parse_ini.o