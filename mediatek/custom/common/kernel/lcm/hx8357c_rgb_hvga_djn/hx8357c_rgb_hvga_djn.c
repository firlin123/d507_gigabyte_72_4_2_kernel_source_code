#ifdef BUILD_LK
#include <platform/gpio_const.h>
#include <platform/mt_gpio.h>
#else
    #include <linux/string.h>
    #include <mach/gpio_const.h>
    #if defined(BUILD_UBOOT)
        #include <asm/arch/mt_gpio.h>
    #else
        #include <mach/mt_gpio.h>
    #endif
#endif
#include "lcm_drv.h"
#include "cust_gpio_usage.h"

#include <cust_adc.h>    	// zhoulidong  add for lcm detect
#define MIN_VOLTAGE (1400)     // zhoulidong  add for lcm detect
#define MAX_VOLTAGE (1600)     // zhoulidong  add for lcm detect
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (320)
#define FRAME_HEIGHT (480)


// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))
#define SET_GPIO_OUT(n, v)  (lcm_util.set_gpio_out((n), (v)))

#define SET_GPIO_DIR(n, v)  (lcm_util.set_gpio_dir((n), (v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))



//#define LSA0_GPIO_PIN (GPIO_DISP_LSA0_PIN)
#define LSCE_GPIO_PIN (GPIO_DISP_LSCE_PIN)
#define LSCK_GPIO_PIN (GPIO_DISP_LSCK_PIN)
#define LSDA_GPIO_PIN (GPIO_DISP_LSDA_PIN)

#define SET_LSCE_LOW   SET_GPIO_OUT(LSCE_GPIO_PIN, 0)
#define SET_LSCE_HIGH  SET_GPIO_OUT(LSCE_GPIO_PIN, 1)
#define SET_LSCK_LOW   SET_GPIO_OUT(LSCK_GPIO_PIN, 0)
#define SET_LSCK_HIGH  SET_GPIO_OUT(LSCK_GPIO_PIN, 1)
#define SET_LSDA_LOW   SET_GPIO_OUT(LSDA_GPIO_PIN, 0)
#define SET_LSDA_HIGH  SET_GPIO_OUT(LSDA_GPIO_PIN, 1)

#define SET_LSDA_OUT   SET_GPIO_DIR(LSDA_GPIO_PIN, GPIO_DIR_OUT)
#define SET_LSDA_IN       SET_GPIO_DIR(LSDA_GPIO_PIN, GPIO_DIR_IN)
#define GET_LSDA_IN      (mt_get_gpio_in(LSDA_GPIO_PIN))

#define LCD_WRITE_CMD(cmd) SET_LSCE_LOW;send_ctrl_cmd(cmd);SET_LSCE_HIGH
#define LCD_WRITE_DATA(data) SET_LSCE_LOW;send_data_cmd(data);SET_LSCE_HIGH

extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);

static __inline void spi_write_byte(unsigned int data)
{
            unsigned int i;
            for (i = 0; i < 8; ++ i)
            {
                SET_LSCK_LOW;
                if (data & (1 << 7)) {
                    SET_LSDA_HIGH;
                } else {
                    SET_LSDA_LOW;
                }
                UDELAY(1);
                SET_LSCK_HIGH;
                UDELAY(1);
                data <<= 1;
            }
}

static __inline void send_ctrl_cmd(unsigned int cmd)
{
            unsigned int out = (cmd & 0xFF);
            SET_LSDA_OUT;
            SET_LSCK_HIGH;
            SET_LSDA_HIGH;	
            UDELAY(1);
            SET_LSCK_LOW;
            SET_LSDA_LOW;
            UDELAY(1);
            SET_LSCK_HIGH;
            UDELAY(1);
        	
            spi_write_byte(out);
}

static __inline void send_data_cmd(unsigned int data)
{
            unsigned int out = data & 0xFF;
            SET_LSDA_OUT;
            SET_LSCK_HIGH;
            SET_LSDA_HIGH;
        		
            UDELAY(1);
            SET_LSCK_LOW;
            SET_LSDA_HIGH;
            UDELAY(1);
            SET_LSCK_HIGH;
            UDELAY(1);
        	
            spi_write_byte(out);
}


static __inline void set_lcm_register(unsigned int regIndex,
                                      unsigned int regData)
{
               LCD_WRITE_CMD(regIndex);
               LCD_WRITE_DATA(regData);
}


static void init_lcm_registers(void)
{

                LCD_WRITE_CMD(0xB9);              // Set EXTC 
                LCD_WRITE_DATA(0xFF); 
                LCD_WRITE_DATA(0x83); 
                LCD_WRITE_DATA(0x57); 
                MDELAY(5); 
                LCD_WRITE_CMD(0xB6);             // 
                LCD_WRITE_DATA(0x20);             //VCOMDC  2C

                LCD_WRITE_CMD(0x11); 
                MDELAY(150); 

                LCD_WRITE_CMD(0x3A);                 
                LCD_WRITE_DATA(0x60);             //262k 

                LCD_WRITE_CMD(0xCC);             //Set Panel 
                LCD_WRITE_DATA(0x09);             // 

                LCD_WRITE_CMD(0xB3);             //COLOR FORMAT 
                LCD_WRITE_DATA(0x43);             //SDO_EN,BYPASS,EPF[1:0],0,0,RM,DM    43
                LCD_WRITE_DATA(0x00);             //DPL,HSPL,VSPL,EPL 
                LCD_WRITE_DATA(0x06);             //RCM, HPL[5:0] 
                LCD_WRITE_DATA(0x06);             //VPL[5:0] 

                LCD_WRITE_CMD(0xB1);             // 
                LCD_WRITE_DATA(0x00);             //DP 
                LCD_WRITE_DATA(0x15);             //BT 
                LCD_WRITE_DATA(0x1C);             //VSPR 
                LCD_WRITE_DATA(0x1C);             //VSNR 
                LCD_WRITE_DATA(0x83);             //AP 
                LCD_WRITE_DATA(0xAA);             //FS 

                LCD_WRITE_CMD(0xC0);             // 
                LCD_WRITE_DATA(0x24);             //OPON 
                LCD_WRITE_DATA(0x24);             //OPON 
                LCD_WRITE_DATA(0x01);             //STBA 
                LCD_WRITE_DATA(0x3C);             //STBA 
                LCD_WRITE_DATA(0x1E);             //STBA 
                LCD_WRITE_DATA(0x08);             //GEN 

                LCD_WRITE_CMD(0xB4);             // 
                LCD_WRITE_DATA(0x01);             //NW 
                LCD_WRITE_DATA(0x40);             //RTN 
                LCD_WRITE_DATA(0x00);             //DIV 
                LCD_WRITE_DATA(0x2A);             //DUM 
                LCD_WRITE_DATA(0x2A);             //DUM 
                LCD_WRITE_DATA(0x0D);             //GDON 
                LCD_WRITE_DATA(0x4F);              //GDOFF 

                LCD_WRITE_CMD(0xE0);              // 
                LCD_WRITE_DATA(0x02);              //1 
                LCD_WRITE_DATA(0x08);              //2 
                LCD_WRITE_DATA(0x11);              //3 
                LCD_WRITE_DATA(0x23);              //4 
                LCD_WRITE_DATA(0x2C);              //5 
                LCD_WRITE_DATA(0x40);              //6 
                LCD_WRITE_DATA(0x4A);              //7 
                LCD_WRITE_DATA(0x52);              //8 
                LCD_WRITE_DATA(0x48);              //9 
                LCD_WRITE_DATA(0x41);              //10 
                LCD_WRITE_DATA(0x3C);              //11 
                LCD_WRITE_DATA(0x33);              //12 
                LCD_WRITE_DATA(0x2E);              //13 
                LCD_WRITE_DATA(0x28);              //14 
                LCD_WRITE_DATA(0x27);              //15 
                LCD_WRITE_DATA(0x1B);              //16 
                LCD_WRITE_DATA(0x02);              //17 
                LCD_WRITE_DATA(0x08);              //18 
                LCD_WRITE_DATA(0x11);              //19 
                LCD_WRITE_DATA(0x23);              //20 
                LCD_WRITE_DATA(0x2C);              //21 
                LCD_WRITE_DATA(0x40);              //22 
                LCD_WRITE_DATA(0x4A);              //23 
                LCD_WRITE_DATA(0x52);              //24 
                LCD_WRITE_DATA(0x48);              //25 
                LCD_WRITE_DATA(0x41);              //26 
                LCD_WRITE_DATA(0x3C);              //27 
                LCD_WRITE_DATA(0x33);              //28 
                LCD_WRITE_DATA(0x2E);              //29 
                LCD_WRITE_DATA(0x28);              //30 
                LCD_WRITE_DATA(0x27);              //31 
                LCD_WRITE_DATA(0x1B);              //32 
                LCD_WRITE_DATA(0x00);              //33 
                LCD_WRITE_DATA(0x01);              //34 

                LCD_WRITE_CMD(0x29);              //Display on 
                MDELAY(25); 	 
                LCD_WRITE_CMD(0x2c);
}


static void config_gpio(void)
{
            const unsigned int USED_GPIOS[] = 
            {
                LSCE_GPIO_PIN,
                LSCK_GPIO_PIN,
                LSDA_GPIO_PIN
            };

            unsigned int i;

            //lcm_util.set_gpio_mode(LSA0_GPIO_PIN, GPIO_DISP_LSA0_PIN_M_GPIO);
            lcm_util.set_gpio_mode(LSCE_GPIO_PIN, 0);//GPIO_DISP_LSCE_PIN_M_GPIO);
            lcm_util.set_gpio_mode(LSCK_GPIO_PIN, 0);//GPIO_DISP_LSCK_PIN_M_GPIO);
            lcm_util.set_gpio_mode(LSDA_GPIO_PIN, 0);//GPIO_DISP_LSDA_PIN_M_GPIO);

            for (i = 0; i < ARY_SIZE(USED_GPIOS); ++ i)
            {
                lcm_util.set_gpio_dir(USED_GPIOS[i], 1);               // GPIO out
                lcm_util.set_gpio_pull_enable(USED_GPIOS[i], 0);
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

        params->type   = LCM_TYPE_DPI;
        params->ctrl   = LCM_CTRL_GPIO;
        params->width  = FRAME_WIDTH;
        params->height = FRAME_HEIGHT;
       //params->io_select_mode = 0;//0:dpi,1:cpu
       

        /* RGB interface configurations */
        params->dpi.mipi_pll_clk_ref  = 0;
        params->dpi.mipi_pll_clk_div1 = 6;
        params->dpi.mipi_pll_clk_div2 = 4;
        params->dpi.mipi_pll_clk_fbk_div=20;
        params->dpi.dpi_clk_div       = 8; 
        params->dpi.dpi_clk_duty      = 4;

        params->dpi.clk_pol           = LCM_POLARITY_FALLING;
        params->dpi.de_pol            = LCM_POLARITY_RISING;
        params->dpi.vsync_pol         = LCM_POLARITY_FALLING;
        params->dpi.hsync_pol         = LCM_POLARITY_FALLING;

        params->dpi.hsync_pulse_width = 6;  
        params->dpi.hsync_back_porch  = 19;
        params->dpi.hsync_front_porch = 39;
        params->dpi.vsync_pulse_width = 5;
        params->dpi.vsync_back_porch  = 14;
        params->dpi.vsync_front_porch = 14;

        params->dpi.format            = LCM_DPI_FORMAT_RGB666;
        params->dpi.rgb_order         = LCM_COLOR_ORDER_RGB;
    	
        params->dpi.intermediat_buffer_num = 2;

        params->dpi.io_driving_current = LCM_DRIVING_CURRENT_6575_4MA;
        params->dpi.i2x_en = 0;
        params->dpi.i2x_edge = 0;

}


static void lcm_init(void)
{
        config_gpio();
        SET_RESET_PIN(1);
        SET_RESET_PIN(0);
        SET_LSCE_HIGH;
        SET_LSCK_HIGH;
        MDELAY(25);
        SET_RESET_PIN(1);
        MDELAY(150);

        init_lcm_registers();
}


static void lcm_suspend(void)
{
	
        LCD_WRITE_CMD(0x28);
        MDELAY(10);
        LCD_WRITE_CMD(0x10);
        MDELAY(120);
}


static void lcm_resume(void)
{
	        
        LCD_WRITE_CMD(0x11);
        MDELAY(120);
        LCD_WRITE_CMD(0x29);
        MDELAY(10);
}

//add start
static int rgk_lcm_compare_id(void)
{
    int data[4] = {0,0,0,0};
    int res = 0;
    int rawdata = 0;
    int lcm_vol = 0;

#ifdef AUXADC_LCM_VOLTAGE_CHANNEL
    res = IMM_GetOneChannelValue(AUXADC_LCM_VOLTAGE_CHANNEL,data,&rawdata);
    if(res < 0)
    { 
	#ifdef BUILD_LK
	printf("[adc_uboot]: get data error\n");
	#endif
	return 0;
		   
    }
#endif
    lcm_vol = data[0]*1000+data[1]*10;

	
    #ifdef BUILD_LK
    printf("[adc_uboot]: lcm_vol= %d\n",lcm_vol);
	#else
	printk("[adc_uboot]: lcm_vol= %d\n",lcm_vol);
    #endif
	
    if (lcm_vol>=MIN_VOLTAGE &&lcm_vol <= MAX_VOLTAGE )
    {
	return 1;
    }

    return 0;
	
}
//add (end)
// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hx8357c_rgb_hvga_djn_lcm_drv = 
{
   .name = "hx8357c_rgb_hvga_djn",
   .set_util_funcs = lcm_set_util_funcs,
   .get_params     = lcm_get_params,
   .init           = lcm_init,
   .suspend        = lcm_suspend,
   .resume         = lcm_resume,
	.compare_id    =  rgk_lcm_compare_id,
};

