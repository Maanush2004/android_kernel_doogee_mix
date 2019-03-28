/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#define LOG_TAG  "LCM"
#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif

#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/upmu_common.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
/*#include <mach/mt_pm_ldo.h>*/
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#ifdef CONFIG_MTK_LEGACY
#include <mach/mt_gpio.h>
#endif
#endif
#ifdef CONFIG_MTK_LEGACY
#include <cust_gpio_usage.h>
#endif

#ifdef BUILD_LK
#define printk(string, args...)  dprintf(0, "[LK/"LOG_TAG"]"string, ##args)
#define LCM_LOGD(string, args...)  dprintf(1, "[LK/"LOG_TAG"]"string, ##args)
#else
#define printk(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#define LCM_LOGD(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#endif

#define LCM_ID	 0x3821

static LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)	(lcm_util.set_reset_pin((v)))
#define MDELAY(n)		(lcm_util.mdelay(n))
#define UDELAY(n)		(lcm_util.udelay(n))


#define dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update) \
  lcm_util.dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) \
  lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) \
  lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) \
  lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) \
  lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) \
  lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define set_gpio_lcd_enp(cmd) \
  lcm_util.set_gpio_lcd_enp_bias(cmd)

#define LCM_DSI_CMD_MODE									0
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1440)

#ifndef CONFIG_FPGA_EARLY_PORTING
#endif
#define REGFLAG_DELAY             							0xAB
#define REGFLAG_END_OF_TABLE      							0xAA   // END OF REGISTERS MARKER

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
  #define FALSE (0)
#endif

#ifndef TRUE
  #define TRUE  (1)
#endif

static unsigned int lcm_compare_id(void);

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))
#define SET_GPIO_OUT(gpio_num,val)    						(lcm_util.set_gpio_out((gpio_num),(val)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)                                                                                   lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)                                   lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)  

#define GPIO_LCM_RST_1         (GPIO83 | 0x80000000)
#define GPIO_LCM_RST           (83+343)
#define GPIO_LCM_ENP_1         (GPIO106 | 0x80000000)
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

#ifdef BUILD_LK
static void lcm_set_enp_bias_lk(int output)
{
  mt_set_gpio_mode(GPIO_LCM_ENP_1, GPIO_MODE_00);
  mt_set_gpio_dir(GPIO_LCM_ENP_1, GPIO_DIR_OUT);
  mt_set_gpio_out(GPIO_LCM_ENP_1, (output > 0) ? GPIO_OUT_ONE : GPIO_OUT_ZERO);
}
static void lcm_set_rst_lk(int output)
{
  mt_set_gpio_mode(GPIO_LCM_RST_1, GPIO_MODE_00);
  mt_set_gpio_dir(GPIO_LCM_RST_1, GPIO_DIR_OUT);
  mt_set_gpio_out(GPIO_LCM_RST_1, (output > 0) ? GPIO_OUT_ONE : GPIO_OUT_ZERO);
}

#endif
struct LCM_setting_table
{
    unsigned char cmd;
  unsigned char count;
  unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] =
{

{0xFF, 3, {0x98,0x81,0x05}},
{0xB2, 1, {0x70}},		//panda timing clr delay 1 frame
{0x03, 1, {0x00}},		//VCM1[8]
{0x04, 1, {0x24}},		//VCM1[7:0]
{0x30, 1, {0xF7}},		//-3/3X 
{0x29, 1, {0x00}},
{0x2A, 1, {0x12}},		//sleep VCM1-old
{0x38, 1, {0xA8}},		//IOVCC LVD 1.45
{0x1A, 1, {0x50}},		//debounce 32us
{0x52, 1, {0x5F}},		//VGHO=VGH
{0x54, 1, {0x28}},		//VGH CLP=12.03 
{0x55, 1, {0x25}},		//VGL CLP=-12.03

{0x26, 1, {0x02}},		//Auto 1/2 VCOM-new
{0x3D, 1, {0xA1}},		//default:E1 enable VCI/VSP LVD
{0x1B, 1, {0x01}},		//11, keep LVD function

{0xFF, 3, {0x98,0x81,0x02}},
{0x42, 1, {0x2F}},		//SDT=3us 
{0x01, 1, {0x50}},		//timeout black
{0x15, 1, {0x10}},		//2/3-power mode
//{0x33, 1, {0x01}},		//BIST EN     

{0x57, 1, {0x00}},
{0x58, 1, {0x17}},
{0x59, 1, {0x26}},
{0x5A, 1, {0x14}},
{0x5B, 1, {0x17}},
{0x5C, 1, {0x29}},
{0x5D, 1, {0x1D}},
{0x5E, 1, {0x1F}},
{0x5F, 1, {0x8B}},
{0x60, 1, {0x1E}},
{0x61, 1, {0x2A}},
{0x62, 1, {0x78}},
{0x63, 1, {0x19}},
{0x64, 1, {0x17}},
{0x65, 1, {0x4B}},
{0x66, 1, {0x20}},
{0x67, 1, {0x27}},
{0x68, 1, {0x4A}},
{0x69, 1, {0x5A}},
{0x6A, 1, {0x25}},

{0x6B, 1, {0x00}},
{0x6C, 1, {0x17}},
{0x6D, 1, {0x26}},
{0x6E, 1, {0x14}},
{0x6F, 1, {0x17}},
{0x70, 1, {0x29}},
{0x71, 1, {0x1D}},
{0x72, 1, {0x1F}},
{0x73, 1, {0x8B}},
{0x74, 1, {0x1E}},
{0x75, 1, {0x2A}},
{0x76, 1, {0x78}},
{0x77, 1, {0x19}},
{0x78, 1, {0x17}},
{0x79, 1, {0x4B}},
{0x7A, 1, {0x20}},
{0x7B, 1, {0x27}},
{0x7C, 1, {0x4A}},
{0x7D, 1, {0x5A}},
{0x7E, 1, {0x25}},


{0xFF, 3, {0x98,0x81,0x01}},		//GIP 
{0x01, 1, {0x00}},
{0x02, 1, {0x00}},
{0x03, 1, {0x56}},		//STV_A rise[9:8]=01, phase_stv_a=2, overlap_stv_a=6H
{0x04, 1, {0x13}},		//phase_stv_b=2, overlap_stv_b=4H
{0x05, 1, {0x13}},		//phase_stv_c=2, overlap_stv_c=4H
{0x06, 1, {0x0a}},		//STV_A rise[7:0]
{0x07, 1, {0x05}},		//STV_B rise[7:0]
{0x08, 1, {0x05}},		//STV_C rise[7:0]
{0x09, 1, {0x1D}},		//fti_1_rise:STV
{0x0a, 1, {0x01}},		//fti_2_rise
{0x0b, 1, {0x00}},		//fti_3_rise
{0x0c, 1, {0x3F}},		//fti_1_fall
{0x0d, 1, {0x29}},		//fti_2_fall
{0x0e, 1, {0x29}},		//fti_3_fall
{0x0f, 1, {0x1D}},		//clw_1_rise  29
{0x10, 1, {0x1D}},		//clw_2_rise  29
{0x11, 1, {0x00}},		//clwb_1_rise
{0x12, 1, {0x00}},		//clwb_2_rise
{0x13, 1, {0x08}},		//clw_x_fall
{0x14, 1, {0x08}},		//clwb_x_fall
{0x15, 1, {0x00}},
{0x16, 1, {0x00}},
{0x17, 1, {0x00}},
{0x18, 1, {0x00}},
{0x19, 1, {0x00}},
{0x1a, 1, {0x00}},
{0x1b, 1, {0x00}},
{0x1c, 1, {0x00}},
{0x1d, 1, {0x00}},
{0x1e, 1, {0x40}},		//clk_a_rise[10:8]
{0x1f, 1, {0x88}},		//panda lvd_option, clk_b_rise[10:8], panda_back_vgh_option
{0x20, 1, {0x08}},		//clk_a_rise[7:0]
{0x21, 1, {0x01}},		//clk_a_fall[7:0]
{0x22, 1, {0x00}},		//clk_b_rise[7:0]
{0x23, 1, {0x00}},		//clk_b_fall[7:0]
{0x24, 1, {0x00}},		//clk_keep_pos1[7:0]
{0x25, 1, {0x00}},		//clk_keep_pos2[7:0]
{0x26, 1, {0x00}},		//clkb_keep_pos1[7:0]
{0x27, 1, {0x00}},		//clkb_keep_pos2[7:0]
{0x28, 1, {0x33}},		//clk_x_num[2:0]=8, phase_clk[2:0]=8
{0x29, 1, {0x03}},		//overlap_clk[3:0]=4H
{0x2a, 1, {0x00}},
{0x2b, 1, {0x00}},
{0x2c, 1, {0x00}},
{0x2d, 1, {0x00}},
{0x2e, 1, {0x00}},
{0x2f, 1, {0x00}},
{0x30, 1, {0x00}},
{0x31, 1, {0x00}},
{0x32, 1, {0x00}},
{0x33, 1, {0x00}},
{0x34, 1, {0x00}},
{0x35, 1, {0x00}},
{0x36, 1, {0x00}},
{0x37, 1, {0x00}},
{0x38, 1, {0x00}},
{0x39, 1, {0x0f}},		//post_timing_en, gip_sr_vgl[0], gip_sr_vgh_[1:0]
{0x3a, 1, {0x2a}},
{0x3b, 1, {0xc0}},
{0x3c, 1, {0x00}},
{0x3d, 1, {0x00}},
{0x3e, 1, {0x00}},
{0x3f, 1, {0x00}},
{0x40, 1, {0x00}},
{0x41, 1, {0xe0}},
{0x42, 1, {0x40}},
{0x43, 1, {0x0f}},
{0x44, 1, {0x31}},		//panda timing, default=15
{0x45, 1, {0xa8}},
{0x46, 1, {0x00}},
{0x47, 1, {0x08}},
{0x48, 1, {0x00}},
{0x49, 1, {0x00}},
{0x4a, 1, {0x00}},
{0x4b, 1, {0x00}},
                                       
// ==== GOUT_BW_L[3:0] ====
{0x4c, 1, {0xb2}},		//STVB2
{0x4d, 1, {0x22}},
{0x4e, 1, {0x01}},		//VGL/VGH
{0x4f, 1, {0xf7}},		//STVC2/STVA2
{0x50, 1, {0x29}},		//L9=19=CK8=CLK6
{0x51, 1, {0x72}},		//L12=17=CK6=CLK4
{0x52, 1, {0x25}},		//L13=15=CK4=CLK2
{0x53, 1, {0xb2}},		//L16=1B=CK2=CLK8
{0x54, 1, {0x22}},
{0x55, 1, {0x22}},
{0x56, 1, {0x22}},
                                       
// ==== GOUT_BW_R[3:0] ====
{0x57, 1, {0xa2}},		//STVB1
{0x58, 1, {0x22}},
{0x59, 1, {0x01}},
{0x5a, 1, {0xe6}},		//STVC1/STVA1
{0x5b, 1, {0x28}},		//R9=CK7=CLK5
{0x5c, 1, {0x62}},		//R12=CK5=CLK3
{0x5d, 1, {0x24}},		//R13=CK3=CLK1
{0x5e, 1, {0xa2}},		//R16=CK1=CLK7
{0x5f, 1, {0x22}},
{0x60, 1, {0x22}},
{0x61, 1, {0x22}},

{0x62, 1, {0xee}},
                                       
// ==== GOUT_FW_L ====
{0x63, 1, {0x02}},
{0x64, 1, {0x0b}},
{0x65, 1, {0x02}},
{0x66, 1, {0x02}},
{0x67, 1, {0x01}},
{0x68, 1, {0x00}},
{0x69, 1, {0x0f}},
{0x6a, 1, {0x07}},
{0x6b, 1, {0x55}},		//L9=CK8=CLK2
{0x6c, 1, {0x02}},
{0x6d, 1, {0x02}},
{0x6e, 1, {0x5b}},		//L12=CK6=CLK8
{0x6f, 1, {0x59}},		//L13=CK4=CLK6
{0x70, 1, {0x02}},
{0x71, 1, {0x02}},
{0x72, 1, {0x57}},		//L16=CK2=CLK4
{0x73, 1, {0x02}},
{0x74, 1, {0x02}},
{0x75, 1, {0x02}},
{0x76, 1, {0x02}},
{0x77, 1, {0x02}},
{0x78, 1, {0x02}},
                                       
// ==== GOUT_FW_R ====
{0x79, 1, {0x02}},
{0x7a, 1, {0x0a}},
{0x7b, 1, {0x02}},
{0x7c, 1, {0x02}},
{0x7d, 1, {0x01}},
{0x7e, 1, {0x00}},
{0x7f, 1, {0x0e}},
{0x80, 1, {0x06}},
{0x81, 1, {0x54}},		//R9=CK7=CLK1
{0x82, 1, {0x02}},
{0x83, 1, {0x02}},
{0x84, 1, {0x5a}},		//R12=CK5=CLK7
{0x85, 1, {0x58}},		//R13=CK3=CLK5
{0x86, 1, {0x02}},
{0x87, 1, {0x02}},
{0x88, 1, {0x56}},		//R16=CK1=CLK3
{0x89, 1, {0x02}},
{0x8a, 1, {0x02}},
{0x8b, 1, {0x02}},
{0x8c, 1, {0x02}},
{0x8d, 1, {0x02}},
{0x8e, 1, {0x02}},
                                       
{0x8f, 1, {0x44}},
{0x90, 1, {0x44}},

{0xFF, 3, {0x98,0x81,0x06}},
{0x01, 1, {0x03}},		//LEDPWM/SDO hi-z  
{0x04, 1, {0x70}},		//70：4lane, 71：3lane,
{0x2B, 1, {0x0A}},		//BGR_PANEL+SS_PANEL BW:09   
   
{0xC0, 1, {0xCF}},		//720*1440
{0xC1, 1, {0x2A}},		//720*1440

{0xFF, 3, {0x98,0x81,0x00}},
{0x11, 1, {0x00}},              
{REGFLAG_DELAY, 120, {}},
{0x29, 1, {0x00}}, 
{REGFLAG_DELAY, 200, {}},
{0x35, 1, {0x00}},
{0x36, 1, {0x00}},

};
/*
static struct LCM_setting_table lcm_sleep_mode_in_setting[] =
{
  // Display off sequence
  {0x28, 0, {0x00}},
  {REGFLAG_DELAY, 120, {}},

  // Sleep Mode On
  {0x10, 0, {0x00}},
  {REGFLAG_DELAY, 200, {}},
  {REGFLAG_END_OF_TABLE, 0x00, {}}
};
*/
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
  unsigned int i;

  for(i = 0; i < count; i++)
  {

    unsigned cmd;
    cmd = table[i].cmd;

    switch (cmd)
    {

      case REGFLAG_DELAY :
        MDELAY(table[i].count);
        break;

      case REGFLAG_END_OF_TABLE :
        break;

      default:
        dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        //MDELAY(2);
    }
  }

}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
  memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
  memset(params, 0, sizeof(LCM_PARAMS));

  params->type = LCM_TYPE_DSI;

  params->width = FRAME_WIDTH;
  params->height = FRAME_HEIGHT;

  // enable tearing-free
  params->dbi.te_mode = LCM_DBI_TE_MODE_DISABLED;
  params->dbi.te_edge_polarity = LCM_POLARITY_RISING;

    //params->dsi.mode   = SYNC_PULSE_VDO_MODE;
  params->dsi.mode = SYNC_EVENT_VDO_MODE;//BURST_VDO_MODE;

  // DSI
  /* Command mode setting */
  params->dsi.LANE_NUM				= LCM_FOUR_LANE;
  //The following defined the fomat for data coming from LCD engine.
  params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
  params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
  params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
  params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

  params->dsi.packet_size=256;
  params->dsi.intermediat_buffer_num = 2;

  params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

  params->dsi.vertical_sync_active				= 8;
  params->dsi.vertical_backporch					= 14;
  params->dsi.vertical_frontporch					= 20;
  params->dsi.vertical_active_line				= FRAME_HEIGHT; 

  params->dsi.horizontal_sync_active				= 20;
  params->dsi.horizontal_backporch				= 60;
  params->dsi.horizontal_frontporch				= 60; //70;
  params->dsi.horizontal_active_pixel				= FRAME_WIDTH;


params->dsi.PLL_CLOCK = 258 ;//260;//212;//208;	
params->dsi.ssc_disable = 1;	
}

static void lcm_init(void)
{

#ifdef BUILD_LK
  lcm_set_enp_bias_lk(1);
#else
  set_gpio_lcd_enp(1);
#endif

 #ifdef BUILD_LK
  lcm_set_rst_lk(1);
  MDELAY(10);
  lcm_set_rst_lk(0);
  MDELAY(10);
  lcm_set_rst_lk(1);
  MDELAY(80);
#else
  gpio_set_value_cansleep(GPIO_LCM_RST, 1);
  MDELAY(10);
  gpio_set_value_cansleep(GPIO_LCM_RST, 0);
  MDELAY(10);
  gpio_set_value_cansleep(GPIO_LCM_RST, 1);
  MDELAY(80);
#endif

  push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
 #ifdef BUILD_LK

  lcm_set_rst_lk(0);
  MDELAY(120);
  lcm_set_enp_bias_lk(0);
#else
  gpio_set_value_cansleep(GPIO_LCM_RST, 0);
  MDELAY(120);
  set_gpio_lcd_enp(0);
#endif
}

static void lcm_resume(void)
{
  lcm_init();
}

static unsigned int lcm_compare_id(void)
{

 	unsigned int array[4];
	unsigned short device_id;
	unsigned char buffer[4];
#ifdef BUILD_LK
  lcm_set_enp_bias_lk(1);
#else
  set_gpio_lcd_enp(1);
#endif

  #ifdef BUILD_LK
  lcm_set_rst_lk(1);
  MDELAY(15);
  lcm_set_rst_lk(0);
  MDELAY(10);
  lcm_set_rst_lk(1);
  MDELAY(50);
#else
  gpio_set_value_cansleep(GPIO_LCM_RST, 1);
  MDELAY(15);
  gpio_set_value_cansleep(GPIO_LCM_RST, 0);
  MDELAY(10);
  gpio_set_value_cansleep(GPIO_LCM_RST, 1);
  MDELAY(50);
#endif
  /*
     array[0]=0x00063902;
     array[1]=0x000177ff;
     array[2]=0x00001000;
     dsi_set_cmdq(array, 3, 1);
   */
  array[0] = 0x00033700;
  dsi_set_cmdq(array, 1, 1);
	MDELAY(10); 

  read_reg_v2(0x04, buffer, 2);	
  device_id = (buffer[0]<<8) | (buffer[1]);

#if defined(BUILD_LK)
	printf("st7701_ivo50_hongzhan_fwvga  %s,line=%d, id = 0x%x, buffer[0]=0x%08x,buffer[1]=0x%08x\n",__func__,__LINE__, device_id, buffer[0],buffer[1]);
#else
	printk("st7701_ivo50_hongzhan_fwvga  %s,line=%d, id = 0x%x, buffer[0]=0x%08x,buffer[1]=0x%08x\n",__func__,__LINE__, device_id, buffer[0],buffer[1]);
#endif
  return (LCM_ID == device_id)?1:0;


}

LCM_DRIVER ili9881p_panda55_hf_lhd_lcm_drv =
{
  .name			= "ili9881p_panda55_hf_lhd",
  .set_util_funcs = lcm_set_util_funcs,
  .get_params     = lcm_get_params,
  .init           = lcm_init,
  .suspend        = lcm_suspend,
  .resume         = lcm_resume,
  .compare_id     = lcm_compare_id,
};
