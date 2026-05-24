#include "mdss_dsi.h"
#include "zte_lcd_common.h"

struct zte_lcd_info g_zte_lcd_info;
struct mdss_dsi_ctrl_pdata *G_ctrl_pdata;
bool power_off_remove_to_TP = false;
#ifdef ZTE_LCD_COVERT_BACKLEVEL
/*
if use this func,you can canel the code MDSS_BRIGHT_TO_BL in mdss_fb_set_bl_brightness() of mdss_fb.c
*/
unsigned char zte_backlight_dijing[256]={
0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,
3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,
5,5,5,5,5,5,5,6,6,6,6,6,7,7,7,7,
8,8,8,8,9,9,9,10,10,10,11,11,11,12,12,12,
13,13,14,14,14,15,15,16,16,17,17,17,18,18,19,19,
20,20,21,21,21,22,22,23,23,24,24,24,25,25,26,26,
27,27,27,28,28,29,29,30,30,31,31,31,32,32,33,33,
34,34,35,35,36,37,37,38,38,39,39,40,41,41,42,43,
43,44,45,45,46,47,47,48,49,50,50,51,52,53,54,55,
55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,
71,72,73,74,75,77,78,79,80,81,82,84,85,86,87,88,
90,91,92,93,94,96,97,98,100,101,102,103,105,106,107,109,
111,113,114,116,117,119,120,122,124,125,127,128,130,131,133,135,
136,138,140,141,143,145,146,148,150,151,153,155,156,158,160,162,
163,165,167,168,170,172,174,175,177,179,181,182,184,186,188,190,
};

unsigned char zte_backlight_boe[256]={
0,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,
3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,
4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,
6,6,6,6,6,7,7,7,7,7,8,8,8,8,9,9,
9,10,10,10,11,11,12,12,12,13,13,14,14,15,15,16,
16,17,17,18,18,19,19,20,20,21,21,22,22,23,24,24,
25,25,26,26,27,27,28,28,29,30,30,31,31,32,32,33,
33,34,34,35,35,36,36,37,37,38,39,39,40,40,41,41,
42,43,43,44,45,45,46,47,47,48,49,49,50,51,52,52,
53,54,55,56,57,57,58,59,60,61,62,63,64,65,66,67,
68,69,71,72,73,74,75,76,78,79,80,81,83,84,85,86,
88,89,90,92,93,95,96,97,99,100,102,103,105,106,108,109,
110,112,114,115,117,119,120,122,124,126,127,129,131,133,135,136,
138,140,142,144,146,148,150,152,154,156,158,160,162,164,166,168,
170,172,175,177,179,181,183,185,188,190,192,194,196,199,201,203,
205,208,210,212,214,217,219,221,223,226,228,230,232,235,237,240,
};
int  zte_covert_backlevel_function(int level,u32 bl_max)
{
	int bl,covert_level;

	if(bl_max>255)
	{
		bl = level/16;
		covert_level = 4095*zte_backlight_dijing[bl]/255;
	}
	else
	{
		covert_level = zte_backlight_boe[level];
	}

	printk("[MSM_LCD]%s:%s level=%d  --> covert_level = %d\n",__func__,(bl_max>255)?"dijing":"boe",level,covert_level);
	return covert_level;
}
#endif

#ifdef ZTE_CHECK_LCD_TYPE
static void  zte_check_lcd_type(struct device_node *node)
{
	const char * panel_name ;
	panel_name = of_get_property(node,	"qcom,mdss-dsi-panel-name", NULL);

        if (!strncmp(panel_name, A01_ST7789V_PANEL, strnlen(A01_ST7789V_PANEL, PANEL_NAME_MAX_LENTH)))
                g_zte_lcd_info.lcd_type = LCD_TYPE_ST7789V;
        else if (!strncmp(panel_name, K50_HX8394F_PANEL, strnlen(K50_HX8394F_PANEL, PANEL_NAME_MAX_LENTH)))
                g_zte_lcd_info.lcd_type = LCD_TYPE_HX8394F;
        else if (!strncmp(panel_name, S10_ILI9881C_DIJING_PANEL, strnlen(S10_ILI9881C_DIJING_PANEL, PANEL_NAME_MAX_LENTH)))
                g_zte_lcd_info.lcd_type = LCD_TYPE_ILI9881C_DIJING;
        else if (!strncmp(panel_name, S10_ILI9881C_HOLITECH_PANEL, strnlen(S10_ILI9881C_HOLITECH_PANEL, PANEL_NAME_MAX_LENTH)))
                g_zte_lcd_info.lcd_type = LCD_TYPE_ILI9881C_HOLITECH;
        else if (!strncmp(panel_name, S10_ILI9881C_YASHI_PANEL, strnlen(S10_ILI9881C_YASHI_PANEL, PANEL_NAME_MAX_LENTH)))
                g_zte_lcd_info.lcd_type = LCD_TYPE_ILI9881C_YASHI;
        else if (!strncmp(panel_name, S10_ILI9881C_SKYWORTH_PANEL, strnlen(S10_ILI9881C_SKYWORTH_PANEL, PANEL_NAME_MAX_LENTH)))
                g_zte_lcd_info.lcd_type = LCD_TYPE_ILI9881C_SKYWORTH;
        else if (!strncmp(panel_name, S10_TD4100_PANEL, strnlen(S10_TD4100_PANEL, PANEL_NAME_MAX_LENTH)))
                g_zte_lcd_info.lcd_type = LCD_TYPE_TD4100;
        else if (!strncmp(panel_name, F20_JD9161B_DIJING_PANEL, strnlen(F20_JD9161B_DIJING_PANEL, PANEL_NAME_MAX_LENTH)))
                g_zte_lcd_info.lcd_type = LCD_TYPE_JD9161B_DIJING;
        else if (!strncmp(panel_name, F20_JD9161B_JINGTAI_PANEL, strnlen(F20_JD9161B_JINGTAI_PANEL, PANEL_NAME_MAX_LENTH)))
                g_zte_lcd_info.lcd_type = LCD_TYPE_JD9161B_JINGTAI;
        else if (!strncmp(panel_name, F20_JD9161B_TONGXINGDA_PANEL, strnlen(F20_JD9161B_TONGXINGDA_PANEL, PANEL_NAME_MAX_LENTH)))
                g_zte_lcd_info.lcd_type = LCD_TYPE_JD9161B_TONGXINGDA;
        else if (!strncmp(panel_name, F20_ILI98806E_PANEL, strnlen(F20_ILI98806E_PANEL, PANEL_NAME_MAX_LENTH)))
                g_zte_lcd_info.lcd_type = LCD_TYPE_ILI98806E;
        else
                g_zte_lcd_info.lcd_type = LCD_TYPE_UNKNOWN;

        printk("[MSM_LCD] g_lcd_type=%d\n",g_zte_lcd_info.lcd_type);
}
#endif

#ifdef ZTE_GET_BOOT_MODE
char g_zte_boot_mode;
int zte_get_boot_mode_func(void)
{
	char *t=NULL;
	char *search="androidboot.mode";

	t=strstr(saved_command_line,search);
	if(0==strncmp(t+17,"ffbm-99",7)) //17=androidboot.mode=
	{
		g_zte_boot_mode=BOOT_MODE_FTM;
	}
	else if (0==strncmp(t+17,"recovery",8))
	{
		g_zte_boot_mode=BOOT_MODE_RECOVERY;
	}
	else if (0==strncmp(t+17,"charger",7))
	{
		g_zte_boot_mode=BOOT_MODE_CHARGER;
	}
	else if (0==strncmp(t+17,"normal",6))
	{
		g_zte_boot_mode=BOOT_MODE_NORMAL;
	}
	else if (0==strncmp(t+17,"ffbm-01",7))
	{
		g_zte_boot_mode=BOOT_MODE_FFBM;
	}
	else
		g_zte_boot_mode=BOOT_MODE_UNKNOWN;

	printk("[MSM_LCD]%s:g_zte_boot_mode is %d\n",__FUNCTION__,g_zte_boot_mode);
	
	return g_zte_boot_mode;
}
#endif

static int zte_lcd_proc_info_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s\n", g_zte_lcd_info.lcd_panel_name);
	return 0;
}

static int zte_lcd_proc_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, zte_lcd_proc_info_show, NULL);
}

static const struct file_operations zte_lcd_common_func_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= zte_lcd_proc_info_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

extern void zte_lcd_reg_debug_func(struct mdss_dsi_ctrl_pdata *ctrl_pdata,struct device_node *node);
void  zte_lcd_common_func(struct mdss_dsi_ctrl_pdata *ctrl_pdata,struct device_node *node)
{
	const char * panel_name;

	G_ctrl_pdata = ctrl_pdata;
	#ifdef ZTE_GET_BOOT_MODE
		zte_get_boot_mode_func();
	#endif

	#ifdef ZTE_CHECK_LCD_TYPE
		zte_check_lcd_type(node);
	#endif

	#ifdef ZTE_LCD_DEBUG
		zte_lcd_reg_debug_func(ctrl_pdata,node);
	#endif
	proc_create_data("driver/lcd_id", 0, NULL, &zte_lcd_common_func_proc_fops, NULL);
	panel_name = of_get_property(node,"zte,lcd-proc-panel-name", NULL);
	if (!panel_name)
	{
		pr_info("[MSM_LCD] %s:%d, panel name not found!\n",__func__, __LINE__);
		strcpy(g_zte_lcd_info.lcd_panel_name,"0");
	}
	else
	{
		pr_info("[MSM_LCD]%s: Panel Name = %s\n", __func__, panel_name);
		strcpy(g_zte_lcd_info.lcd_panel_name,panel_name);
	}

	g_zte_lcd_info.is_close_dimming = of_property_read_bool(node,"zte,is-close-dimming");
	pr_err("[MSM_LCD] g_zte_lcd_info.is_close_dimming = %s\n",g_zte_lcd_info.is_close_dimming?"true":"false");

}

