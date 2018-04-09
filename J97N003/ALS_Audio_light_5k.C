/*
项目名称: ALS 5000 R+C 蓝牙音响灯
芯片型号: G80F960A32 LQFP32 外部晶振 11.0592M
功能: 
内部128K晶振，功耗350uA
外部11.0592M晶振，功耗 3.3V:3.5mA; 5V:4.6mA
休眠功耗23uA
负载PWM频率1.351K
11.1V 6000mAh 3串26980电池
五个按键: -, ON/OFF, +, 蓝牙音箱开关, USB充电宝开关
15个指示灯: 5档流明指示, 10个电量指示

脚位定义
             ----------------------------------
    K1 COM5 |01|P02/AN0/INT45  INT46/AN1/P03|32| COM1 K3
            |02|P01            INT47/AN2/P04|31| ADC_CHG
            |03|P00/OP1OUT      VREF/AN3/P05|30| ADC_BAT
            |04|P10/OP1N             AN4/P06|29| ADC_IIN
            |05|P11/OP1P             AN5/P07|28| ADC_IOUT
    K4 COM4 |06|P12/AN8/INT0/TDO      T1/P27|27| BTS_EN
    K5 COM3 |07|P13/AN9/INT1/TMS      T0/P26|26| SEG4
     USW_TX |08|P14/TDI            PWM21/P25|25| SEG3
           <                                    >
       SEG2 |09|P15/TCK            PWM11/P24|24| BTS_ST
       SEG1 |10|P16/AN10           PWM01/P23|23| CHARGEB
     USB_EN |11|P17                 PWM2/P22|22| LM358_EN
            |12|VDD                 PWM1/P21|21| BT_RST
            |13|GND                 PWM0/P20|20| LD_PWM
      XTAL1 |14|P35/INT44           PWM3/P30|19| LD_EN
      XTAL2 |15|P34/INT43      RXD/INT40/P31|18| MCU_RX
    K2 COM2 |16|P33/INT42      TXD/INT41/P32|17| MCU_TX
             ----------------------------------

烧录器接口
 -------
| 1 | 2 |  蓝 橙 TCK
 -------   VDD
| 3 | 4 |     棕 TDI
 -------
| 5 | 6 |     绿 TMS
 -------
| 7 | 8 |     红 TDO
 -------
| 9 | 10|     黄 GND
 -------

PCB板烧录接口
-------
  PIN1 | 棕 TDI
  PIN2 | 橙 TCK
  PIN3 | 红 TDO
  PIN4 | 绿 TMS
  PIN5 | 黄 GND
  PIN6 | 蓝 VDD
-------

*/


#include "config.h"
//#include "api.h"
#include "g80f903a_eeprom.h"
#define BLE_V2_EN 0
#define SW_UART_EN 1
#define UART_SW_MODE 1
#if (BLE_V2_EN == 1)
//#include "ALS_RXTX.h"
#endif


#if (BLE_V2_EN == 0)
typedef struct mouselet_app_status
{
	unsigned char u8_ble_tx_id;	//0:不发送; 1:可以发送
	unsigned char u8_ble_status;	//0:未连接; 1:已连接
	unsigned char u8_handle[2];		//不同APP发来的handle
	unsigned char u8_bd_addr[6];	//手机的BD_ADDR 随机不确定
	unsigned char u8_uart_err;		//串口数据传输出错
}app_st;

code uint8 u8_app_pwm_value[] = {
	0x00, 10, 25, 50, 75, 100
};
#endif

code uint8 u8_ld_color[7] = {
	0, 25, 119, 194, 238, 251, 255
};

code uint16 adc_vdd_chg_ld_array10[10] = {
//	6.77  6.92	7.02  7.10	7.19  7.34	7.51  7.71	7.93  8.11
	3012, 3080, 3123, 3156, 3196, 3263, 3340, 3429, 3526, 3604
};

code uint16 adc_vdd_ld5_array10[10] = {
//	6.67  6.81	6.91  6.99	7.08  7.21	7.38  7.58	7.80  8.04
	2967, 3028, 3072, 3108, 3150, 3208, 3284, 3372, 3468, 3575
};

code uint16 adc_vdd_ld4_array10[10] = {
//  
	2979, 3041, 3085, 3120, 3160, 3217, 3291, 3375, 3468, 3577
};

code uint16 adc_vdd_ld3_array10[10] = {
//  6.73  6.87  6.97  7.04  7.13  7.25  7.42  7.60  7.80  8.05
	2992, 3055, 3099, 3132, 3170, 3226, 3298, 3379, 3469, 3580
};

code uint16 adc_vdd_ld2_array10[10] = {
//  
	3002, 3067, 3111, 3144, 3183, 3244, 3319, 3404, 3497, 3592
};

code uint16 adc_vdd_ld1_array10[10] = {
//  6.77  6.92  7.02  7.10  7.19  7.34  7.51  7.71  7.93  8.11
	3012, 3080, 3123, 3156, 3196, 3263, 3340, 3429, 3526, 3604
};

/////////////////////////////////////////////////////////////
//输入
#define		ADC_BAT			3			//电池采样电压
#define		ADC_PV			2			//充电器采样电压
#define		ADC_IIN			4			//充电电流
#define		ADC_IOUT		5			//放电电流
#define		BTS_ST			pin24

//输出
#define		CHARGEB			pin23		//充电器地线MOS开关 高:导通; 低:截止
#define		LD_EN			pin19
#define		LM358_EN		pin22
#define		BT_RST			pin21
#define		BTS_EN			pin27
#define		USB_EN			pin11

#define		TM_DIO			pin5
#define		TM_CLK			pin4
#define		TM_STB			pin3
#define		TM_EN			pin2

#define		DEBUG_OUT		pin2

#define		TM_DELAY		{CLR_WDT(); CLR_WDT(); CLR_WDT(); CLR_WDT(); CLR_WDT(); }

#define		LD_OUT_ON		{LD_EN=1;}
#define		LD_OUT_OFF		{LD_EN=0;}

#define		CHARGEB_ON		1
#define		CHARGEB_OFF		0

#define		LM358_ON		0
#define		LM358_OFF		1

#define		BT_ON			1
#define		BT_OFF			0

#define		KEY_DOWN		0
#define		KEY_UP			1
#define		KEY_SHORT_PRESS_DOWN	2		//20ms
#define		KEY_SHORT_PRESS_UP		10		//100ms
#define		KEY_LONG_PRESS_DOWN		250		//20000ms

#define		LED_SEG1		pin10
#define		LED_SEG2		pin9
#define		LED_SEG3		pin25
#define		LED_SEG4		pin26

#define		LED_COM5		pin1
#define		LED_COM5_IN		{P0CR &= ~(1<<2);}
#define		LED_COM5_OUT	{P0CR |= (1<<2);}
#define		LED_COM5_HIGH	{LED_COM5=1;}
#define		LED_COM5_LOW	{LED_COM5=0;}

#define		LED_COM4		pin6
#define		LED_COM4_IN		{P1CR &= ~(1<<2);}
#define		LED_COM4_OUT	{P1CR |= (1<<2);}
#define		LED_COM4_HIGH	{LED_COM4=1;}
#define		LED_COM4_LOW	{LED_COM4=0;}

#define		LED_COM3		pin7
#define		LED_COM3_IN		{P1CR &= ~(1<<3);}
#define		LED_COM3_OUT	{P1CR |= (1<<3);}
#define		LED_COM3_HIGH	{LED_COM3=1;}
#define		LED_COM3_LOW	{LED_COM3=0;}

#define		LED_COM2		pin16
#define		LED_COM2_IN		{P3CR &= ~(1<<3);}
#define		LED_COM2_OUT	{P3CR |= (1<<3);}
#define		LED_COM2_HIGH	{LED_COM2=1;}
#define		LED_COM2_LOW	{LED_COM2=0;}

#define		LED_COM1		pin32
#define		LED_COM1_IN		{P0CR &= ~(1<<3);}
#define		LED_COM1_OUT	{P0CR |= (1<<3);}
#define		LED_COM1_HIGH	{LED_COM1=1;}
#define		LED_COM1_LOW	{LED_COM1=0;}

#define		KEY_1			LED_COM5		//开/关
#define		KEY_2			LED_COM2		//减小
#define		KEY_3			LED_COM1		//增加
#define		KEY_4			LED_COM4		//蓝牙音响开关
#define		KEY_5			LED_COM3		//USB充电宝开关

#define		CHG_CUR_FULL		79		//80:0.42 95 0.5

#define		BAT_CUR_LEVEL_10	84		//0.45
#define		BAT_CUR_LEVEL_9		93		//0.5
#define		BAT_CUR_LEVEL_8		140		//0.75
#define		BAT_CUR_LEVEL_7		200		//1.08
#define		BAT_CUR_LEVEL_6		276		//1.49
#define		BAT_CUR_LEVEL_5		364		//1.96 以行及以下无效
#define		BAT_CUR_LEVEL_4		440		//2.37
#define		BAT_CUR_LEVEL_3		441		//2.38
#define		BAT_CUR_LEVEL_2		444		//2.4

#define		BAT_LEVEL_10		3620	//8.14
#define		BAT_LEVEL_9			3609	//8.11
#define		BAT_LEVEL_8			3575	//8.04
#define		BAT_LEVEL_7			3528	//7.93
#define		BAT_LEVEL_6			3464	//7.79 此行及以上无效
#define		BAT_LEVEL_5			3396	//7.64
#define		BAT_LEVEL_4			3332	//7.49
#define		BAT_LEVEL_3			3275	//7.36
#define		BAT_LEVEL_2			3212	//7.22

#define		BAT_UP_RESET		3160	//10.8V
#define		BAT_LD_FLASH		2800	//6.3V
#define		BAT_LOW_OFF			2925	//2779	//2779:9.5V;2516:8.6V自动关灯电压点 2925:10V
#define		BAT_LOW_OFF2		2666	//2666:6V自动关灯电压点


#define		IDLE_CNT			5		//10秒
#define		IDLE_CNT2			5		//3600秒

#define		BAT_LEVEL_CNT		240

#define		LD_PWM_5			4095//1023
#define		LD_PWM_4			2920//2800//690
#define		LD_PWM_3			2320//2200//510
#define		LD_PWM_2			1720//1700//330
#define		LD_PWM_1			1200//238
#define		LD_PWM_0			0



/////////////////////////////////////////////////////////////
//系统定时器变量
section0	uint8	u8_tm_10ms_cnt;
section0	bool	b_tm_10ms_trig;
section0	uint8	u8_tm_trig_cnt;

section2	uint8	u8_tm_20ms_cnt;
section0	bool	b_tm_20ms_trig;

section2	uint8	u8_tm_120ms_cnt;
section0	bool	b_tm_120ms_flag;

section2	uint8	u8_tm_500ms_cnt;
section0	bool	b_tm_500ms_trig;
section0	bool	b_tm_500ms_flag;


section2	uint16	u8_tm_idle_timeout;

section2	uint8	u8_tm_bts_cnt;
section2	uint8	u8_tm_bts_timeout;

section0	bool	b_bat_low_det_trig;

section0	uint16	u8_selftest_cnt;
section2	uint8	u8_selftest_timeout;
section0	bool	b_selftest_flag;
section0	bool	b_st_key1;
section0	bool	b_st_key2;
section0	bool	b_st_key3;
section0	uint8	u8_st_all;


//系统定时器变量结束

section0	uint8	u8_debug_index;

section0	bool	b_power_status;
section0	uint8	u8_ld_level;

section0	uint8	u8_sw_pwm_tel;

//led显示驱动
section0	uint8	u8_led_update_index;
section0	uint8	u8_led_line1;			//第1行0-5
section0	uint8	u8_led_line2;			//第2行0-10

//*
section0	bool	b_led_seg1;
section0	bool	b_led_seg2;
section0	bool	b_led_seg3;
section0	bool	b_led_seg4;
section0	bool	b_led_seg5;

section0	bool	b_led_com1;
section0	bool	b_led_com2;
section0	bool	b_led_com3;
section0	bool	b_led_com4;
section0	bool	b_led_com5;
//*/

section0	bool	b_tm_update;
section0	bool	b_tm_status;
section0	bool	b_tm_status_old;

section2	byte_type	tm_ram0;
section2	byte_type	tm_ram1;
section2	byte_type	tm_ram2;
section2	byte_type	tm_ram3;
section2	byte_type	tm_ram4;

#define	b_c1_s1	tm_ram0.bits.bit0		//
#define	b_c1_s2	tm_ram0.bits.bit1		//
#define	b_c1_s3	tm_ram0.bits.bit2		//
#define	b_c1_s4	tm_ram0.bits.bit3		//
#define	b_c1_s5	tm_ram0.bits.bit4		//
#define	b_c1_s6	tm_ram0.bits.bit5		//
#define	b_c1_s7	tm_ram0.bits.bit6		//
#define	b_c1_s8	tm_ram0.bits.bit7		//

#define	b_c2_s1	tm_ram1.bits.bit0		//
#define	b_c2_s2	tm_ram1.bits.bit1		//
#define	b_c2_s3	tm_ram1.bits.bit2		//
#define	b_c2_s4	tm_ram1.bits.bit3		//
#define	b_c2_s5	tm_ram1.bits.bit4		//
#define	b_c2_s6	tm_ram1.bits.bit5		//
#define	b_c2_s7	tm_ram1.bits.bit6		//
#define	b_c2_s8	tm_ram1.bits.bit7		//

#define	b_c3_s1	tm_ram2.bits.bit0		//
#define	b_c3_s2	tm_ram2.bits.bit1		//
#define	b_c3_s3	tm_ram2.bits.bit2		//
#define	b_c3_s4	tm_ram2.bits.bit3		//
#define	b_c3_s5	tm_ram2.bits.bit4		//
#define	b_c3_s6	tm_ram2.bits.bit5		//
#define	b_c3_s7	tm_ram2.bits.bit6		//
#define	b_c3_s8	tm_ram2.bits.bit7		//

#define	b_c4_s1	tm_ram3.bits.bit0		//
#define	b_c4_s2	tm_ram3.bits.bit1		//
#define	b_c4_s3	tm_ram3.bits.bit2		//
#define	b_c4_s4	tm_ram3.bits.bit3		//
#define	b_c4_s5	tm_ram3.bits.bit4		//
#define	b_c4_s6	tm_ram3.bits.bit5		//
#define	b_c4_s7	tm_ram3.bits.bit6		//
#define	b_c4_s8	tm_ram3.bits.bit7		//

#define	b_c5_s1	tm_ram4.bits.bit0		//
#define	b_c5_s2	tm_ram4.bits.bit1		//
#define	b_c5_s3	tm_ram4.bits.bit2		//
#define	b_c5_s4	tm_ram4.bits.bit3		//
#define	b_c5_s5	tm_ram4.bits.bit4		//
#define	b_c5_s6	tm_ram4.bits.bit5		//
#define	b_c5_s7	tm_ram4.bits.bit6		//
#define	b_c5_s8	tm_ram4.bits.bit7		//

//led显示驱动结束

//按键驱动
section0	uint8	u8_key_value;
section0	uint8	u8_key_value_old;

section0	bool	b_key1_io;
section0	bool	b_key2_io;
section0	bool	b_key4_io;
section0	bool	b_key8_io;
section0	bool	b_key10_io;
section0	bool	b_key1_up_flag;
section0	bool	b_key2_up_flag;
section0	bool	b_key4_up_flag;
section0	bool	b_key8_up_flag;
section0	bool	b_key10_up_flag;

section0	uint8	u8_key1_down_cnt;
section0	uint8	u8_key2_down_cnt;
section0	uint8	u8_key4_down_cnt;
section0	uint8	u8_key8_down_cnt;
section0	uint8	u8_key10_down_cnt;
section0	uint8	u8_key1_up_cnt;
section0	uint8	u8_key2_up_cnt;
section0	uint8	u8_key4_up_cnt;
section0	uint8	u8_key8_up_cnt;
section0	uint8	u8_key10_up_cnt;
//按键驱动结束

//充电驱动
section0	bool	b_sw_pwm_flag;
section0	uint8	u8_chg_tel;				//占空比
section0	bool	b_chg_mos_st;			//充电MOS管开关状态
section0	bool	b_charge_status;

section0	bool	b_chgv_det_trig;
section0	bool	b_chgv_level;			//不充电时充电电压等级
section0	bool	b_chgv_level_new;
section0	uint8	u8_chgv_level_cnt;

section0	bool	b_chga_det_trig;
section0	bool	b_chga_level;			//充电时充电电流等级
section0	bool	b_chga_level_new;
section0	uint8	u8_chga_level_cnt;

section0	bool	b_chg_enable_by_extv;	//充电电压存在

//充电驱动结束


section0	uint8	u8_ee_addr;
section0	uint8	u8_ee_data;
section0	bool	b_ee_rd_en;
section0	bool	b_ee_wt_en;
section2	uint8	u8_cfg0[8];

section0	bool	b_adc_bat_finish;
section0	bool	b_adc_iin_finish;

section0	bool	b_bat_chg_low_flag;		//0:未发生充电电压低; 1:发生充电电压低

section0	bool	b_ld_out_trig;
section0	uint16	u16_sw_pwm_index;
section0	uint16	u16_sw_pwm_target;

section0	uint16	u16_adc_iout_target;
section0	uint8	u8_iout_need_up_cnt;
section0	uint8	u8_iout_need_down_cnt;


#if(HW_UART0_EN == 1)
#if (BLE_V2_EN == 0)
//uart0 rx tx
section0	bool	b_uart0_rx_finish;
section2	uint16	u16_uart0_rx_i;
section2	uint16	u16_uart0_rx_len;
section2	uint16	u16_uart0_tx_len;
section2	uint8	u8_uart0_rx_buf;
section2	uint8	u8_uart0_rx[32];
section2	uint8	u8_uart0_tx[32];
section2	uint8	u8_uart0_rx_timeout;
section2	uint8	u8_uart0_tx_timeout;
section2	uint8	u8_uart0_tx_tm_cnt;
//uart0 rx tx end

//Bluetooth param
section0	bool	b_ble_power_status;

section0	uint8	u8_app_cnt;
section2	app_st	m_app[2];
section0	bool	b_bt_broadcast_en;
section0	uint16	u8_bt_get_status_cnt;
section0	bool	b_bt_get_stauts_trig;
section2	uint8	u8_bt_get_status_nack_cnt;
//section0	bool	b_bt_enter_low_energy;
//section0	uint8	u8_tm_bt_enter_low_energy;

section0	bool	b_uart0_rx_finish;
section0	bool	b_bt_broadcast_status;			//蓝牙模块状态 0:未广播; 1:广播

section0	bool	b_ble_trig;
section0	uint8	u8_ble_on_cnt;
section0	uint8	u8_ble_off_cnt;
//Bluetooth param end
#endif
#endif

#if (ADC_CONVERT_EN == 1)
//adc param
section2	uint8	u8_adc_loop_index;

section2	uint16	u16_adc_bat_sum,u16_adc_bat_avg;
section2	uint8	u8_adc_bat_cnt;

section2	uint16	u16_adc_adp_sum,u16_adc_adp_avg;
section2	uint8	u8_adc_adp_cnt;

section2	uint16	u16_adc_iin_sum,u16_adc_iin_avg;
section2	uint16	u16_adc_iin_max,u16_adc_iin_min;
section2	uint8	u8_adc_iin_cnt;

section2	uint16	u16_adc_iin_offset;

section2	uint16	u16_adc_iout_sum,u16_adc_iout_avg;
section2	uint8	u8_adc_iout_cnt;

section2	uint16	u16_adc_iout_offset;
//adc param end
#endif
//低电量闪灯驱动
section2	uint8	u8_adc_up_cnt;
section2	uint8	u8_adc_down_cnt;
section0	bool	b_adc_bat_level;
section0	bool	b_adc_bat_level_new;
/*
 * 电池电压等级下降沿发生置1
 * 电池电压等级上升沿清0
 * 按键按下清0
 */
section0	bool	b_adc_bat_fail;
section0	bool	b_adc_bat_fail_1st;

section2	uint8	u8_ld_flash_cnt;			//连续闪烁时间
section0	bool	b_ld_flash_flag;
section0	bool	b_ld_flash_en;
//低电量闪灯驱动 结束


section0	uint8	u8_bat_level;			//电池电压等级
section0	uint8	u8_bat_level_new;		//电池电压等级

section0	uint8	u8_bat_up_cnt;			//电压上升滤波计数器
section0	uint8	u8_bat_down_cnt;		//电压下降滤波计数器

section0	bool	b_bts_en;
section0	bool	b_bts_led;
section0	bool	b_bts_st;

section0	bool	b_usb_en;

#if(SW_UART_EN == UART_SW_MODE)
#define		USW_TX			pin8

section0	bool	b_stx_bit_ing;			//1:1Byte数据发送中; 0:1Byte发送完成
section2	uint16	u16_stx_buf;

section2	uint8	u8_tm_stx_cnt;			//发送字符串周期
section2	uint8	u8_stx_i;				//发送序列
section2	uint8	u8_stx[16];				//发送数据
#endif

#if (BLE_V2_EN == 1)
section0	uint8	u8_buf_rx;
section0	uint8	u8_buf_i;
section0	bool	b_buf_rx_finish;
section0	uint8	u8_buf_rx_timeout;

section2	uint8	u8_rx_len;
section2	uint8	u8_rx_buf[255];
section2	uint8	u8_rx_sum;

section2	uint8	u8_tx_len;
section2	uint8	u8_tx_buf[32];

section2	ST_FIFO_QUEUE,st_fifo_buf;
#endif

/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
//函数声明
#if (BLE_V2_EN == 0)
void mouselet_ble_on();
void mouselet_ble_off();
void mouselet_tx_lamp_status();
#endif

/////////////////////////////////////////////////////////////
//外部中断0
void int0_isr(void) interrupt 0
{
	EA	=	0;
	IE0	=	0;
	//在此加入您要处理的代码
	EA	=	1;
}

/*
 * Timer0中断处理函数 向上增长，增长到65536发生中断
 * SYSCLK:	11.0592M
 * TCON1[3:2] = 11	TMCLK = SYSCLK	  = 11059200	
 * TCON1[3:2] = 00	TMCLK = SYSCLK/12 = 921,600		Selected
 *
 * TMOD[1:0]  = 01	方式1 TH0[7-0]<<8+TL0[7-0]
 * TH0 :  TL0 = 65536 - (Ttarget * TMCLK)
 * 0xFC  0x67			 0.001s
 * 0x00  0x00			 0.071s
 */
void timer0_isr(void) interrupt 1
{
	EA	= 0;
	TF0	= 0;						//清T0中断标志
	TH0 = 0xFC;
	TL0 = 0x67;
	//在此加入您要处理的代码		1ms中断一次

	u8_tm_10ms_cnt++;
	if(u8_tm_10ms_cnt > 9)
	{
		u8_tm_10ms_cnt = 0;
		b_tm_10ms_trig = 1;
	}
//	b_key_scan_trig = 1;

	/////////////////////////////////////////////////////////////
	//dynamic_scan_key_down();
#if 1
	if(u8_led_update_index == 5)
	{
		u8_led_update_index = 0;
		
		LED_SEG1 = 0;
		LED_SEG2 = 0;
		LED_SEG3 = 0;
		LED_SEG4 = 0;

		LED_COM5_HIGH
		LED_COM4_HIGH
		LED_COM3_HIGH
		LED_COM2_HIGH
		LED_COM1_HIGH
		
		LED_COM5_IN
		LED_COM4_IN
		LED_COM3_IN
		LED_COM2_IN
		LED_COM1_IN

		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		b_key1_io = KEY_1;
		b_key2_io = KEY_2;
		b_key4_io = KEY_3;
		b_key8_io = KEY_4;
		b_key10_io = KEY_5;
		
		if(b_key1_io == KEY_DOWN)
		{
			u8_key1_down_cnt++;
			if(u8_key1_down_cnt >= KEY_LONG_PRESS_DOWN)
			{
				u8_key1_down_cnt = KEY_LONG_PRESS_DOWN+1;
				u8_key_value = 0x08;
				u8_key_value_old = 0x80;
			}
			else if((u8_key1_down_cnt >= KEY_SHORT_PRESS_DOWN) && (u8_key1_down_cnt < KEY_SHORT_PRESS_UP))
			{
				u8_key1_down_cnt = KEY_SHORT_PRESS_UP+1;
				u8_key_value = 0x01;
				u8_key_value_old = 0x10;
			}
		}
		else
		{
			u8_key1_down_cnt = 0;
			if(u8_key_value_old == 0x10)
			{
				u8_key_value = 0x11;
			}
		}
		
		if(b_key2_io == KEY_DOWN)
		{
			u8_key2_down_cnt++;
			if(u8_key2_down_cnt >= KEY_LONG_PRESS_DOWN)
			{
				u8_key2_down_cnt = KEY_LONG_PRESS_DOWN+1;
			}
			else if((u8_key2_down_cnt >= KEY_SHORT_PRESS_DOWN) && (u8_key2_down_cnt < KEY_SHORT_PRESS_UP))
			{
				u8_key2_down_cnt = KEY_SHORT_PRESS_UP+1;
				if(b_key2_up_flag)
				{
					b_key2_up_flag = 0;
					u8_key2_up_cnt = 0;
					u8_key_value = 0x02;
					u8_key_value_old = 0x20;
				}
			}
		}
		else
		{
			u8_key2_down_cnt = 0;
			u8_key2_up_cnt++;
			if(u8_key2_up_cnt >= KEY_SHORT_PRESS_UP)
			{
				b_key2_up_flag = 1;
				u8_key2_up_cnt = KEY_SHORT_PRESS_UP;
			}
		}
		if(b_key4_io == KEY_DOWN)
		{
			u8_key4_down_cnt++;
			if(u8_key4_down_cnt >= KEY_LONG_PRESS_DOWN)
			{
				u8_key4_down_cnt = KEY_LONG_PRESS_DOWN+1;
			}
			else if((u8_key4_down_cnt >= KEY_SHORT_PRESS_DOWN) && (u8_key4_down_cnt < KEY_SHORT_PRESS_UP))
			{
				u8_key4_down_cnt = KEY_SHORT_PRESS_UP+1;
				if(b_key4_up_flag)
				{
					b_key4_up_flag = 0;
					u8_key4_up_cnt = 0;
					u8_key_value = 0x04;
					u8_key_value_old = 0x40;
				}
			}
		}
		else
		{
			u8_key4_down_cnt = 0;
			u8_key4_up_cnt++;
			if(u8_key4_up_cnt >= KEY_SHORT_PRESS_UP)
			{
				b_key4_up_flag = 1;
				u8_key4_up_cnt = KEY_SHORT_PRESS_UP;
			}
		}
		
		if(b_key8_io == KEY_DOWN)
		{
			u8_key8_down_cnt++;
			if(u8_key8_down_cnt >= KEY_LONG_PRESS_DOWN)
			{
				u8_key8_down_cnt = KEY_LONG_PRESS_DOWN+1;
			}
			else if((u8_key8_down_cnt >= KEY_SHORT_PRESS_DOWN) && (u8_key8_down_cnt < KEY_SHORT_PRESS_UP))
			{
				u8_key8_down_cnt = KEY_SHORT_PRESS_UP+1;
				if(b_key8_up_flag)
				{
					b_key8_up_flag = 0;
					u8_key8_up_cnt = 0;
					u8_key_value = 0x20;
					u8_key_value_old = 0x02;
				}
			}
		}
		else
		{
			u8_key8_down_cnt = 0;
			u8_key8_up_cnt++;
			if(u8_key8_up_cnt >= KEY_SHORT_PRESS_UP)
			{
				b_key8_up_flag = 1;
				u8_key8_up_cnt = KEY_SHORT_PRESS_UP;
			}
		}
		
		if(b_key10_io == KEY_DOWN)
		{
			u8_key10_down_cnt++;
			if(u8_key10_down_cnt >= KEY_LONG_PRESS_DOWN)
			{
				u8_key10_down_cnt = KEY_LONG_PRESS_DOWN+1;
			}
			else if((u8_key10_down_cnt >= KEY_SHORT_PRESS_DOWN) && (u8_key10_down_cnt < KEY_SHORT_PRESS_UP))
			{
				u8_key10_down_cnt = KEY_SHORT_PRESS_UP+1;
				if(b_key10_up_flag)
				{
					b_key10_up_flag = 0;
					u8_key10_up_cnt = 0;
					u8_key_value = 0x10;
					u8_key_value_old = 0x01;
				}
			}
		}
		else
		{
			u8_key10_down_cnt = 0;
			u8_key10_up_cnt++;
			if(u8_key10_up_cnt >= KEY_SHORT_PRESS_UP)
			{
				b_key10_up_flag = 1;
				u8_key10_up_cnt = KEY_SHORT_PRESS_UP;
			}
		}
		
		if(u8_key_value_old != u8_key_value)
		{
			if(u8_key_value_old == 0x80)			//长按按住
			{
				u8_key_value_old = 0;
				u8_key_value = 0;
			}
			else
			{
				if(u8_key_value == 0x01)			//S1 开关
				{
					if(b_selftest_flag == 0)
					{
						if(b_power_status == 1)
						{
							b_power_status = 0;
							b_adc_bat_fail = 0;

#if (BLE_V2_EN == 0)
							mouselet_ble_off();
#else
							pl_api_ble_off();
#endif
						}
						else
						{
							b_power_status = 1;
							
#if (BLE_V2_EN == 0)
							mouselet_ble_on();
#else
							pl_api_ble_on();
#endif

							b_bat_chg_low_flag = 0;
						}
					//	mouselet_tx_lamp_status();
					}
					else
					{
						b_st_key1 = ~b_st_key1;
					}
					
					u8_key_value = 0;
					u8_key_value_old = 0;
				}
				else if(u8_key_value == 0x02)		//S3 减小
				{
					if(b_selftest_flag == 0)
					{
						if(b_power_status == 1)
						{
							if(u8_ld_level>1)
							{
								u8_ld_level--;

								b_ee_wt_en = 1;
							}
#if (BLE_V2_EN == 0)
							mouselet_tx_lamp_status();
#else
							als_cmd_tx_status();
#endif
						}
					}
					else
					{
						b_st_key2 = ~b_st_key2;
					}
					
					u8_key_value = 0;
					u8_key_value_old = 0;
				}
				else if(u8_key_value == 0x04)		//S2 增加
				{
					if(b_selftest_flag == 0)
					{
						if(b_power_status == 1)			//开灯
						{
							if(u8_ld_level<5)
							{
								u8_ld_level++;
								b_bat_chg_low_flag = 0;

								b_ee_wt_en = 1;
							}
#if (BLE_V2_EN == 0)
							mouselet_tx_lamp_status();
#else
							als_cmd_tx_status();
#endif
						}
						else							//关灯
						{
							if(u8_selftest_cnt < 100)
							{
								u8_selftest_cnt = 100;
								b_selftest_flag = 1;
								u8_selftest_timeout = 20;
								b_st_key1 = 1;
								b_st_key2 = 1;
								b_st_key3 = 1;
							}
						}
					}
					else
					{
						b_st_key3 = ~b_st_key3;
					}
					
					u8_key_value = 0;
					u8_key_value_old = 0;
				}
				else if(u8_key_value == 0x20)		//S4 蓝牙音响开关
				{
					if(b_selftest_flag == 0)
					{
						b_bts_en = ~b_bts_en;

						if(b_bts_en)
						{
							u8_tm_bts_timeout = 240;
						}
					}
					
					u8_key_value = 0;
					u8_key_value_old = 0;
				}
				else if(u8_key_value == 0x10)		//S5 USB充电宝开关
				{
					if(b_selftest_flag == 0)
					{
						b_usb_en = ~b_usb_en;
					}
					
					u8_key_value = 0;
					u8_key_value_old = 0;
				}
			}
		}
	}
	/////////////////////////////////////////////////////////////
//	if(u8_tm_idle_timeout > 0)
	{
	//	led_update();
		b_led_seg1=0;
		b_led_seg2=0;
		b_led_seg3=0;
		b_led_seg4=0;
		
		b_led_com1=1;
		b_led_com2=1;
		b_led_com3=1;
		b_led_com4=1;
		b_led_com5=1;

	//	DEBUG_OUT = 1;

		LED_SEG1 = 0;
		LED_SEG2 = 0;
		LED_SEG3 = 0;
		LED_SEG4 = 0;

		LED_COM1_HIGH
		LED_COM2_HIGH
		LED_COM3_HIGH
		LED_COM4_HIGH
		LED_COM5_HIGH
		
		LED_COM1_OUT
		LED_COM2_OUT
		LED_COM3_OUT
		LED_COM4_OUT
		LED_COM5_OUT

		
		if(u8_led_update_index == 0)		//com1
		{
			u8_led_update_index = 1;

			if(b_usb_en)
			{
				b_led_seg1 = 1;
				b_led_com1 = 0;
			}
			if(u8_led_line1 >= 3)
			{
				b_led_seg2 = 1;
				b_led_com1 = 0;
			}
			
			if(u8_led_line2 >= 8)
			{
				b_led_seg3 = 1;
				b_led_seg4 = 1;
				b_led_com1 = 0;
			}
			else if(u8_led_line2 >= 3)
			{
				b_led_seg3 = 1;
				b_led_com1 = 0;
			}
		}
		else if(u8_led_update_index == 1)	//com2
		{
			u8_led_update_index = 2;
			
			if(b_bts_led)
			{
				b_led_seg1 = 1;
				b_led_com2 = 0;
			}
			if(u8_led_line1 >= 2)
			{
				b_led_seg2 = 1;
				b_led_com2 = 0;
			}
			if(u8_led_line2 >= 7)
			{
				b_led_seg3 = 1;
				b_led_seg4 = 1;
				b_led_com2 = 0;
			}
			else if(u8_led_line2 >= 4)
			{
				b_led_seg3 = 1;
				b_led_com2 = 0;
			}
		}
		else if(u8_led_update_index == 2)	//com3
		{
			u8_led_update_index = 3;
			if(u8_led_line1 >= 5)
			{
				b_led_seg2 = 1;
				b_led_com3 = 0;
			}

			if(u8_led_line2 >= 10)
			{
				if(b_charge_status)
				{
					b_led_seg4 = 1;
					b_led_com3 = 0;
				}
			}
			else
			{
				if(b_charge_status)
				{
					if(b_tm_500ms_flag)
					{
						b_led_seg4 = 1;
						b_led_com3 = 0;
					}
				}
			}
			
			if(u8_led_line2 >= 1)
			{
				b_led_seg3 = 1;
				b_led_com3 = 0;
			}
		}
		else if(u8_led_update_index == 3)	//com4
		{
			u8_led_update_index = 4;
			if(u8_led_line1 >= 4)
			{
				b_led_seg2 = 1;
				b_led_com4 = 0;
			}
			
			if(u8_led_line2 >= 9)
			{
				b_led_seg3 = 1;
				b_led_seg4 = 1;
				b_led_com4 = 0;
			}
			else if(u8_led_line2 >= 2)
			{
				b_led_seg3 = 1;
				b_led_com4 = 0;
			}
		}
		else if(u8_led_update_index == 4)	//com5
		{
			u8_led_update_index = 5;
			
			if(u8_led_line1 >= 1)
			{
				b_led_seg1 = 1;
				b_led_seg2 = 1;
				b_led_com5 = 0;
			}
			if(u8_led_line2 >= 6)
			{
				b_led_seg3 = 1;
				b_led_seg4 = 1;
				b_led_com5 = 0;
			}
			else if(u8_led_line2 >= 5)
			{
				b_led_seg3 = 1;
				b_led_com5 = 0;
			}
		}
		
		/*
		b_led_seg1 = 1;
		b_led_seg2 = 1;
		b_led_seg3 = 1;
		b_led_seg4 = 1;
		b_led_seg5 = 1;
		
		b_led_com1 = 0;
		b_led_com2 = 0;
		b_led_com3 = 0;
		b_led_com4 = 0;
		b_led_com5 = 0;
		//*/

		LED_COM1 = b_led_com1;
		LED_COM2 = b_led_com2;
		LED_COM3 = b_led_com3;
		LED_COM4 = b_led_com4;
		LED_COM5 = b_led_com5;
		
		LED_COM1_OUT
		LED_COM2_OUT
		LED_COM3_OUT
		LED_COM4_OUT
		LED_COM5_OUT

		LED_SEG1 = b_led_seg1;
		LED_SEG2 = b_led_seg2;
		LED_SEG3 = b_led_seg3;
		LED_SEG4 = b_led_seg4;
	}
#endif
	/////////////////////////////////////////////////////////////
	
#if (SW_UART_EN == UART_SW_MODE)
	//发送数据驱动
	if(u16_stx_buf > 0) 			//波特率 970
	{
		USW_TX = (u16_stx_buf & 0x01);
		u16_stx_buf >>= 1;
	}
	else
	{
		b_stx_bit_ing = 0;
		USW_TX = 1;
	}
#endif


	EA=1;
}

//外部中断1
void int1_isr(void) interrupt 2
{
	EA	= 0;
	IE1	= 0;
	//在此加入您要处理的代码
	EA	= 1;
}

/*
 * Timer0中断处理函数 向上增长，增长到65536发生中断
 * SYSCLK:	11.0592M
 * TCON1[3:2] = 11	TMCLK = SYSCLK	  = 11059200	
 * TCON1[3:2] = 00	TMCLK = SYSCLK/12 = 921,600		Selected
 *
 * TMOD[1:0]  = 01	方式1 TH0[7-0]<<8+TL0[7-0]
 * TH0 :  TL0 = 65536 - (Ttarget * TMCLK)
 * 0xFC  0x67			 0.001s
 * 0x00  0x00			 0.071s
 */
void timer1_isr(void) interrupt 3
{  
	EA	= 0;
	TF1	= 0;						//清T1中断标志

	EA	= 1;
}

//Uart0中断处理函数
void uart0_isr(void) interrupt 4  using 3
{
//	ES = 0;
	if(RI)							//接收中断
	{
		//在此加入您要处理接收的代码
		RI = 0x00;
		ACC = SBUF;
#if(HW_UART0_EN == 1)
#if (BLE_V2_EN == 0)
		u8_uart0_rx_buf = ACC;
		
		u8_uart0_rx_timeout = 3;					//有数据接收开始定时

		if(b_uart0_rx_finish == 0)
		{
			u8_uart0_rx[u16_uart0_rx_i] = u8_uart0_rx_buf;
			u16_uart0_rx_i++;
			u16_uart0_rx_len = u16_uart0_rx_i;

			if(u16_uart0_rx_len == (u8_uart0_rx[2]+4))
			{
				b_uart0_rx_finish = 1;
			}
			
			if(u16_uart0_rx_i > 30)
			{
				b_uart0_rx_finish = 1;
			//	u16_uart0_rx_i = 0;
			//	u16_uart0_rx_len = 0;
			}
		}
#else
		QueueIn(&st_fifo_buf, ACC);
#endif
#endif
	}
//	ES = 1;
}


//Timer2中断处理函数
void timer2_isr(void) interrupt 5
{
   EA=0;
   TF2=0;							//清T2中断标志
   //在此加入您要处理的代码
   EA=1;
}

//Int4中断处理函数
void int4_isr(void) interrupt 10 using 2
{
	EA=0;
	EXF1 = 0x00;
	//在此加入您要处理的代码

	EA=1;
}

#if (BLE_V2_EN == 0)

/*
 * 通用A5校验码算法
 */
uint8 calc_str_sum(uint8 * psrc, uint16 len)
{
	uint16 rxsumi;
	uint8 u8_uart_sum = 0;
	//计算接收数据的校验和
	for(rxsumi=0; rxsumi<len; rxsumi++)
	{
		u8_uart_sum += psrc[rxsumi];
	}
	u8_uart_sum ^= 0xA5;
	return u8_uart_sum;
}

/*
 * 计算mouselet校验码
 */
uint8 calc_mouselet_sum(uint8 *psrc, uint8 len)
{
	uint8 u8_ret = 0;
	uint8 i;
	for (i=0; i<len; i++)
	{
		u8_ret ^= psrc[i];
	}
	return u8_ret;
}

/*
 * 蓝牙模块进入低功耗，不广播
*/
void mouselet_tx_deepsleep()
{
	int k=0;
	u16_uart0_tx_len = 7;

	u8_uart0_tx[k++] = 0x77;
	u8_uart0_tx[k++] = 0x01;
	u8_uart0_tx[k++] = 0x03;
	u8_uart0_tx[k++] = 0x11;
	u8_uart0_tx[k++] = 0x0B;
	u8_uart0_tx[k++] = 0x00;
	
	u8_uart0_tx[u16_uart0_tx_len-1] = calc_mouselet_sum(u8_uart0_tx, (u16_uart0_tx_len-1));
	u8_uart0_tx_tm_cnt = 8;
}

/*
 * 开启蓝牙模块，设置广播名字，开启广播
 */
void mouselet_ble_on()
{
	b_ble_power_status = 1;
	u8_ble_on_cnt = 255;
//	BT_RST = 0;
}

/*
 * 关闭蓝牙模块，断开连接，进入低功耗
 */
void mouselet_ble_off()
{
	b_ble_power_status = 0;
	u8_ble_on_cnt = 0;
	u8_ble_off_cnt = 255;
//	BT_RST = 0;
}

/*
 * 下发开启广播指令
 */
void mouselet_tx_broadcast_on()
{
	int k=0;
	u16_uart0_tx_len = 5;

	u8_uart0_tx[k++] = 0x77;
	u8_uart0_tx[k++] = 0x01;
	u8_uart0_tx[k++] = 0x01;
	u8_uart0_tx[k++] = 0x01;
	
	u8_uart0_tx[u16_uart0_tx_len-1] = calc_mouselet_sum(u8_uart0_tx, (u16_uart0_tx_len-1));
	u8_uart0_tx_tm_cnt = 8;
}

/*
 * 下发关闭广播指令
void mouselet_tx_broadcast_off()
{
	int k=0;
	u16_uart0_tx_len = 5;

	u8_uart0_tx[k++] = 0x77;
	u8_uart0_tx[k++] = 0x01;
	u8_uart0_tx[k++] = 0x01;
	u8_uart0_tx[k++] = 0x02;
	
	u8_uart0_tx[u16_uart0_tx_len-1] = calc_mouselet_sum(u8_uart0_tx, (u16_uart0_tx_len-1));
	u8_uart0_tx_tm_cnt = 8;
}
*/


/*
 * 下发断开连接指令
void mouselet_tx_disconnect(uint8 handle0, uint8 handle1)
{
	int k=0;
	u16_uart0_tx_len = 7;

	u8_uart0_tx[k++] = 0x77;
	u8_uart0_tx[k++] = 0x01;
	u8_uart0_tx[k++] = 0x03;
	u8_uart0_tx[k++] = 0x03;
	u8_uart0_tx[k++] = handle0;
	u8_uart0_tx[k++] = handle1;
	
	u8_uart0_tx[u16_uart0_tx_len-1] = calc_mouselet_sum(u8_uart0_tx, (u16_uart0_tx_len-1));
	u8_uart0_tx_tm_cnt = 8;
}
*/

/*
 * 设置模块名字
 */
void mouselet_tx_set_name()
{
	int k=0;
	u16_uart0_tx_len = 12;

	u8_uart0_tx[k++] = 0x77;
	u8_uart0_tx[k++] = 0x01;
	u8_uart0_tx[k++] = u16_uart0_tx_len-4;
	u8_uart0_tx[k++] = 0x0C;
	
	u8_uart0_tx[k++] = 'A';
	u8_uart0_tx[k++] = 'U';
	u8_uart0_tx[k++] = 'D';
	u8_uart0_tx[k++] = '5';
	u8_uart0_tx[k++] = '0';
	u8_uart0_tx[k++] = '1';
	u8_uart0_tx[k++] = 'H';
	
	u8_uart0_tx[u16_uart0_tx_len-1] = calc_mouselet_sum(u8_uart0_tx, (u16_uart0_tx_len-1));
	u8_uart0_tx_tm_cnt = 8;
}

/*
 * 查询模块当前连接状态
 */
void mouselet_tx_get_stauts()
{
	int k=0;
	u16_uart0_tx_len = 5;

	u8_uart0_tx[k++] = 0x77;
	u8_uart0_tx[k++] = 0x01;
	u8_uart0_tx[k++] = 0x01;
	u8_uart0_tx[k++] = 0x0D;
	
	u8_uart0_tx[u16_uart0_tx_len-1] = calc_mouselet_sum(u8_uart0_tx, (u16_uart0_tx_len-1));
	u8_uart0_tx_tm_cnt = 8;
}

/*
 * 上传灯的状态给APP
 */
void mouselet_tx_lamp_status()
{
	if(u8_ble_on_cnt > 0)
		return;
	if(m_app[0].u8_ble_status == 1)
	{
		m_app[0].u8_ble_tx_id = 1;
	}
	
	if(m_app[1].u8_ble_status == 1)
	{
		m_app[1].u8_ble_tx_id = 1;
	}
}

/*
 * 响应低功耗请求应答
void mouselet_tx_low_energy_ack()
{
	if(m_app[0].u8_ble_status == 2)
	{
		m_app[0].u8_ble_tx_id = 2;
	}
	
	if(m_app[1].u8_ble_status == 2)
	{
		m_app[1].u8_ble_tx_id = 2;
	}
}
*/

/*
 * 增加APP节点信息
 */
void mouselet_add(unsigned char* buf)
{
	uint8 i;
	for(i=0; i<2; i++)
	{
		if(m_app[i].u8_ble_status == 0)
		{
			m_app[i].u8_ble_status = 1;
			memcpy(m_app[i].u8_handle, &buf[5], 2);
			memcpy(m_app[i].u8_bd_addr, &buf[7], 6);

			break;
		}
	}
}

/*
 * 删除APP节点信息
 */
void mouselet_del(unsigned char* buf)
{
	uint8 i;
	uint16 handle1, handle2;
	handle1 = (uint16)((uint16)(buf[5]<<8) + buf[6]);
	
	for(i=0; i<2; i++)
	{
		handle2 = (uint16)((uint16)((m_app[i].u8_handle[0])<<8) + (m_app[i].u8_handle[1]));
		
		if(handle1 == handle2)
		{
			m_app[i].u8_ble_status = 0;
			break;
		}
	}
}

/*
 * 节点连接状态更新
 */
void mouselet_refresh()
{
	uint8 i=0;
//	if(b_bt_enter_low_energy == 1)
//		return;
	
	//查询APP连接状态，计算已连接总数
	u8_app_cnt = 0;
	for(i=0; i<2; i++)
	{
		if(m_app[i].u8_ble_status == 1)
		{
			u8_app_cnt++;
		}
	}

	if(u8_app_cnt == 0)				//当前未任何连接
	{
		if(b_bt_broadcast_status == 0)
		{
			b_bt_broadcast_en = 1;
		}
		else
		{
			b_bt_broadcast_en = 0;
		}
	}
	else if(u8_app_cnt == 1)
	{
		if(b_bt_broadcast_status == 0)
		{
			b_bt_broadcast_en = 1;
		}
		else
		{
			b_bt_broadcast_en = 0;
		}
	}
	else if(u8_app_cnt == 2)
	{
		b_bt_broadcast_en = 0;
	}

	//1s 定时开启广播
	if(b_bt_broadcast_en == 1)
	{
		mouselet_tx_broadcast_on();
	}
}

/*
 * 定时获取蓝牙模块状态
 */
void mouselet_cmd_get_status_thread()
{
	if(b_bt_get_stauts_trig == 1)
	{
		b_bt_get_stauts_trig = 0;
		mouselet_tx_get_stauts();
		u8_bt_get_status_nack_cnt++;

		if(u8_bt_get_status_nack_cnt > 4)
		{
			//5秒后无反应
			u8_bt_get_status_nack_cnt = 5;

			m_app[0].u8_ble_status = 0;
			m_app[1].u8_ble_status = 0;
			mouselet_refresh();
		}
	}
}

void mouselet_ble_on_thread()
{
	if(u8_ble_on_cnt>250)
	{
		BT_RST = 0;
	}
	else if((u8_ble_on_cnt>231) && (u8_ble_on_cnt<240))
	{
		BT_RST = 1;
	}
	else if((u8_ble_on_cnt>201) && (u8_ble_on_cnt<210))
	{
		u8_ble_on_cnt = 200;
		mouselet_tx_set_name();
	}
	else if((u8_ble_on_cnt>151) && (u8_ble_on_cnt<160))
	{
		u8_ble_on_cnt = 150;
		mouselet_refresh();
	}
}

void mouselet_ble_off_thread()
{
	if((u8_ble_off_cnt>200) && (u8_ble_off_cnt<210))
	{
		u8_ble_off_cnt = 200;
		mouselet_tx_deepsleep();
		u8_tm_idle_timeout = IDLE_CNT;
	}
	else if((u8_ble_off_cnt>140) && (u8_ble_off_cnt<150))
	{
		BT_RST = 0;
	}
	else if((u8_ble_off_cnt>131) && (u8_ble_off_cnt<140))
	{
		BT_RST = 1;
	}
	else if((u8_ble_off_cnt>101) && (u8_ble_off_cnt<110))
	{
		u8_ble_off_cnt = 100;
		mouselet_tx_deepsleep();
		u8_tm_idle_timeout = IDLE_CNT;
	}
}

/*
 * 蓝牙模块主函数线程
 */
void mouselet_main_thread()
{
	if(b_ble_trig)
	{
		b_ble_trig = 0;

		if(u8_ble_on_cnt > 0)
			u8_ble_on_cnt--;
		if(u8_ble_off_cnt > 0)
			u8_ble_off_cnt--;
		
		u8_bt_get_status_cnt++;
		if(u8_bt_get_status_cnt > 199)
		{
			u8_bt_get_status_cnt = 0;
			if(u8_ble_on_cnt == 0)
			{
				b_bt_get_stauts_trig = 1;
			}
		}
	}

	if(b_ble_power_status)
	{
		mouselet_ble_on_thread();

		mouselet_cmd_get_status_thread();
	}
	else
	{
		mouselet_ble_off_thread();
	}
}

/*
 * 解析透传数据
 * 0  1  2  3  4  5  6  7  8  9  10 11 12 13
 *                   0  1  2  3  4  5  6  7  8
 * 77 04 0A 02 01 40 00 CA 0F 52 B3 2A 10 24 
 * 77 04 0A 06 40 00 00 07 4B 3E 30 3B 5E 18			//查询
 * 77 04 0C 06 40 00 00 09 4B 3E 32 3B 01 64 C1 E8		//开100%
 * 77 04 0C 06 40 00 00 09 4B 3E 32 3B 00 00 5A 16		//关

 
 档位	6K色温
 100%	0x00,0x09,0x4B,0x3E,0x32,0x3C,0x06,0x64,SUM
 75%	0x00,0x09,0x4B,0x3E,0x32,0x3C,0x06,0x4B,SUM
 50% 	0x00,0x09,0x4B,0x3E,0x32,0x3C,0x06,0x32,SUM
 25%	0x00,0x09,0x4B,0x3E,0x32,0x3C,0x06,0x19,SUM
 10%	0x00,0x09,0x4B,0x3E,0x32,0x3C,0x06,0x0A,SUM
 关灯	0x00,0x09,0x4B,0x3E,0x32,0x3C,0x00,0xXX,SUM
 
 档位	4K色温
 100%	0x00,0x09,0x4B,0x3E,0x32,0x3C,0x04,0x64,SUM
 75%	0x00,0x09,0x4B,0x3E,0x32,0x3C,0x04,0x4B,SUM
 50%	0x00,0x09,0x4B,0x3E,0x32,0x3C,0x04,0x32,SUM
 25%	0x00,0x09,0x4B,0x3E,0x32,0x3C,0x04,0x19,SUM
 10%	0x00,0x09,0x4B,0x3E,0x32,0x3C,0x04,0x0A,SUM
 关灯	0x00,0x09,0x4B,0x3E,0x32,0x3C,0x00,0xXX,SUM
 */
void mouselet_user_adj(unsigned char* buf)
{
	//u8_uart0_rx[4],u8_uart0_rx[5] 是handle
	uint16 u16_ptl_len=0;
	uint8 buf_sum;
	uint8 mptl_len = 6;
	bool b_buf6_0;
	u16_ptl_len = (uint16)((uint16)(buf[6]<<8) + buf[7]);	//用户数据长度
	buf_sum = calc_str_sum(&buf[6],(u16_ptl_len-1));
	if(buf[u16_ptl_len+5] == buf_sum)
	{
		if(buf[mptl_len+2] == 0x4B)						//C75
		{
			if(buf[mptl_len+3] == 0x3E)					//APP -> Bluetooth -> MCU
			{
				if(buf[mptl_len+4] == 0x32)				//SET PARAM
				{
					if(buf[mptl_len+5] == 0x3B)			//NOVA AC 1WLM SCANGRIP
					{
						b_buf6_0 = buf[mptl_len+6] & 0x01;
						if(buf[mptl_len+7] <= 10)
						{
							u8_ld_level = 1;
						}
						else if(buf[mptl_len+7] <= 25)
						{
							u8_ld_level = 2;
						}
						else if(buf[mptl_len+7] <= 50)
						{
							u8_ld_level = 3;
						}
						else if(buf[mptl_len+7] <= 75)
						{
							u8_ld_level = 4;
						}
						else
						{
							u8_ld_level = 5;
						}
						if(b_buf6_0 == 1)
						{
							b_ee_wt_en = 1;
						}
						else
						{
							b_power_status = 0;
							mouselet_ble_off();
						}
						b_bat_chg_low_flag = 0;
						b_ld_out_trig = 1;
						mouselet_tx_lamp_status();
					}
					else if(buf[mptl_len+5] == 0xFA)
					{
						b_power_status = 1;
						u8_ld_level = 0;
						u16_sw_pwm_target = buf[mptl_len+6]<<8 | buf[mptl_len+7];
						u16_sw_pwm_index = u16_sw_pwm_target;
						b_ld_out_trig = 1;
					}
				}
				else if(buf[mptl_len+4] == 0x30)		//GET PARAM
				{
					if(buf[mptl_len+5] == 0x3B)			//NOVA AC 1WLM SCANGRIP
					{
						mouselet_tx_lamp_status();
					}
				}
				/*
				else if(buf[mptl_len+4] == 0x3B)		//低功耗参数
				{
					if(buf[mptl_len+5] == 00)			//开启低功耗模式
					{
						mouselet_tx_low_energy_ack();
						b_bt_enter_low_energy = 1;
						u8_tm_bt_enter_low_energy = 200;	//定时1秒后发送关闭广播指令
					}
				}
				*/
			}
		}
	}
}
#else

uint8 get_bat_level()
{
	return u8_bat_level;
}

bool get_charge_status()
{
	return b_charge_status;
}

bool get_power_status()
{
	return b_power_status;
}

void set_power_status_off()
{
	b_power_status = 0;
}

uint8 get_ld_level()
{
	return u8_ld_level;
}

void set_ld_level(uint8 param)
{
	u8_ld_level = param;
}

void set_ld_out_trig()
{
	b_ld_out_trig = 1;
}

#endif

/////////////////////////////////////////////////////////////
//串口与蓝牙连接

#if(HW_UART0_EN == 1)

#if (BLE_V2_EN == 0)

void uart0_clr_rx()
{
//	uint8	rx_i;
//	for(rx_i=0; rx_i<40; rx_i++)
//		u8_uart0_rx[rx_i] = 0;
	u16_uart0_rx_i = 0;
	u16_uart0_rx_len = 0;
	memset(u8_uart0_rx, 0, 32);
}

/*
 * 发送u8_uart0_tx_len个字符串
 */
void uart0_tx_str()
{
	uint16	tx_i = 0;
	ES = 0;
	//*
	for(tx_i=0; tx_i<u16_uart0_tx_len; tx_i++)
	{
		SBUF = u8_uart0_tx[tx_i];	//发送一个字节
		while(TI == 0)
		{
			CLR_WDT();
		}
		TI=0;
	}
	//*/
	ES = 1;

	//发送完成等待1秒后再启动状态检测
	u8_bt_get_status_cnt = 0;
}


/*
 * 串口接收数据主线程函数
 */
void uart0_rx_thread()
{
	uint16 handle1, handle2;
	if(b_uart0_rx_finish == 1)
	{
		if (u8_uart0_rx[u16_uart0_rx_len-1] == calc_mouselet_sum(u8_uart0_rx, (u16_uart0_rx_len-1)))
		{
			if (u8_uart0_rx[0] == 0x77)
			{
				if (u8_uart0_rx[1] == 0x01)			//Command
				{
				}
				else if (u8_uart0_rx[1] == 0x02)	//Reserved
				{
				}
				else if (u8_uart0_rx[1] == 0x03)	//Response
				{
					if (u8_uart0_rx[3] == 0x01)		//Set Pairing Mode Enter in the Pairing mode
					{
						if (u8_uart0_rx[4] == 0x01)
						{
							//开启广播成功
							u8_bt_get_status_nack_cnt = 0;
						//	b_bt_broadcast_en = 1;
						}
						else if(u8_uart0_rx[4] == 0x03)
						{
							//开启广播失败
						//	b_bt_broadcast_en = 0;
						}
					}
					else if (u8_uart0_rx[3] == 0x02)	//Cancel the pairing mode
					{
						if (u8_uart0_rx[4] == 0x01)
						{
							//关闭广播成功
						}
					}
					else if (u8_uart0_rx[3] == 0x03)	//Set Disconnected
					{
						if (u8_uart0_rx[4] == 0x01)
						{
							//关闭连接成功
						}
					}
					else if (u8_uart0_rx[3] == 0x0D)	//parse BT STATUS
					{
						//77 03 06 0D 00 40 00 41 00 SUM
						b_bt_broadcast_status = u8_uart0_rx[4];
						handle1 = (uint16)((uint16)(u8_uart0_rx[5]<<8) + u8_uart0_rx[6]);
						handle2 = (uint16)((uint16)(u8_uart0_rx[7]<<8) + u8_uart0_rx[8]);

						if(handle1 != 0)
						{
							m_app[0].u8_ble_status = 1;
							m_app[0].u8_handle[0] = u8_uart0_rx[5];
							m_app[0].u8_handle[1] = u8_uart0_rx[6];
						}
						else
						{
							m_app[0].u8_ble_status = 0;
							m_app[0].u8_handle[0] = 0;
							m_app[0].u8_handle[1] = 0;
						}

						if(handle2 != 0)
						{
							m_app[1].u8_ble_status = 1;
							m_app[1].u8_handle[0] = u8_uart0_rx[7];
							m_app[1].u8_handle[1] = u8_uart0_rx[8];
						}
						else
						{
							m_app[1].u8_ble_status = 0;
							m_app[1].u8_handle[0] = 0;
							m_app[1].u8_handle[1] = 0;
						}
						u8_bt_get_status_nack_cnt = 0;
						mouselet_refresh();
					}
				}
				else if (u8_uart0_rx[1] == 0x04)	//Event
				{
					if (u8_uart0_rx[3] == 0x02)		//OpCode
					{
						if (u8_uart0_rx[4] == 0x01)		//BLE Startup 建立连接
						{
							mouselet_add(u8_uart0_rx);
						//	mouselet_refresh();
							u8_bt_get_status_cnt = 0;
							b_bt_get_stauts_trig = 0;
						}
						else if (u8_uart0_rx[4] == 0x02)	//Pairing Status 断开连接
						{
							mouselet_del(u8_uart0_rx);
						//	mouselet_refresh();
							u8_bt_get_status_cnt = 0;
							b_bt_get_stauts_trig = 0;
						}
					}
					else if (u8_uart0_rx[3] == 0x06)	//Send Data透传
					{
						mouselet_user_adj(u8_uart0_rx);
						u8_bt_get_status_cnt = 0;
						b_bt_get_stauts_trig = 0;
					}
				}
			}
		}
		else
		{
			//发送重发指令
			m_app[0].u8_uart_err = 1;
			m_app[1].u8_uart_err = 1;
			mouselet_tx_lamp_status();
		}
		uart0_clr_rx();
		b_uart0_rx_finish = 0;
		u8_uart0_rx_timeout = 0;
	}
	//串口接收超时20ms，重新接收数据
	if(u8_uart0_rx_timeout == 1)
	{
		u8_uart0_rx_timeout = 0;
		uart0_clr_rx();
		b_uart0_rx_finish = 1;
	//	m_app[0].u8_uart_err = 1;
	//	m_app[1].u8_uart_err = 1;
	//	mouselet_tx_lamp_status();
	}
}

void uart0_tx_thread()
{
	if(u8_uart0_tx_tm_cnt>1 && u8_uart0_tx_tm_cnt<4)
	{
		uart0_tx_str();
		u8_uart0_tx_tm_cnt = 0;
	}
}

/*
 * 用户发送数据类似邮件槽线程
 * 准备数据，启动发送
 */
void uart0_tx_thread_user()
{
	if(u8_uart0_tx_tm_cnt == 0)
	{
		//当串口数据发送完成，再次准备数据，启动发送
		if(m_app[0].u8_ble_tx_id == 1)
		{
			if(m_app[0].u8_ble_status == 1)
			{
				u16_uart0_tx_len = 16;
				
				u8_uart0_tx[0] = 0x77;						//Header
				u8_uart0_tx[1] = 0x01;						//Command
				u8_uart0_tx[2] = 0x0C;						//Length
				u8_uart0_tx[3] = 0x0B;						//Opcode: Data
				u8_uart0_tx[4] = m_app[0].u8_handle[0];		//Handle
				u8_uart0_tx[5] = m_app[0].u8_handle[1];

				if(m_app[0].u8_uart_err == 0)
				{
					u8_uart0_tx[6] = 0x00;					//PTL
					u8_uart0_tx[7] = 0x09;
					u8_uart0_tx[8] = 0x4B;
					u8_uart0_tx[9] = 0x3F;
					u8_uart0_tx[10] = 0x31;
					u8_uart0_tx[11] = 0x3B;
					u8_uart0_tx[12] = (u8_bat_level<<4)|b_power_status;
					u8_uart0_tx[13] = u8_app_pwm_value[u8_ld_level];
				}
				else if(m_app[0].u8_uart_err == 1)
				{
					u8_uart0_tx[6] = 0x00;					//PTL
					u8_uart0_tx[7] = 0x09;
					u8_uart0_tx[8] = 0x4B;
					u8_uart0_tx[9] = 0x3F;
					u8_uart0_tx[10] = 0x3F;					//?
					u8_uart0_tx[11] = 0x3B;
					u8_uart0_tx[12] = (u8_bat_level<<4)|b_power_status;
					u8_uart0_tx[13] = u8_app_pwm_value[u8_ld_level];

					m_app[0].u8_uart_err = 0;
				}
				
				u8_uart0_tx[14] = calc_str_sum(&u8_uart0_tx[6], 8);		//PTL SUM
				u8_uart0_tx[15] = calc_mouselet_sum(u8_uart0_tx, 15);	//SUM

				u8_uart0_tx_tm_cnt = 8;
				u8_bt_get_status_cnt = 0;
				b_bt_get_stauts_trig = 0;
			}
			m_app[0].u8_ble_tx_id = 0;
			return;
		}
		else if(m_app[0].u8_ble_tx_id == 2)
		{
			if(m_app[0].u8_ble_status == 1)
			{
				u16_uart0_tx_len = 14;
				
				u8_uart0_tx[0] = 0x77;						//Header
				u8_uart0_tx[1] = 0x01;						//Command
				u8_uart0_tx[2] = 0x0A;						//Length
				u8_uart0_tx[3] = 0x0B;						//Opcode: Data
				u8_uart0_tx[4] = m_app[0].u8_handle[0];		//Handle
				u8_uart0_tx[5] = m_app[0].u8_handle[1];

				if(m_app[0].u8_uart_err == 0)
				{
					u8_uart0_tx[6] = 0x00;					//PTL
					u8_uart0_tx[7] = 0x07;
					u8_uart0_tx[8] = 0x4B;
					u8_uart0_tx[9] = 0x3F;
					u8_uart0_tx[10] = 0x3B;
					u8_uart0_tx[11] = 00;
				}
				else if(m_app[0].u8_uart_err == 1)
				{
					u8_uart0_tx[6] = 0x00;					//PTL
					u8_uart0_tx[7] = 0x07;
					u8_uart0_tx[8] = 0x4B;
					u8_uart0_tx[9] = 0x3F;
					u8_uart0_tx[10] = 0x3F;					//?
					u8_uart0_tx[11] = 00;

					m_app[0].u8_uart_err = 0;
				}
				
				u8_uart0_tx[12] = calc_str_sum(&u8_uart0_tx[6], 6);		//PTL SUM
				u8_uart0_tx[13] = calc_mouselet_sum(u8_uart0_tx, 13);	//SUM

				u8_uart0_tx_tm_cnt = 8;
				u8_bt_get_status_cnt = 0;
				b_bt_get_stauts_trig = 0;
			}
			m_app[0].u8_ble_tx_id = 0;
			return;
		}

		if(m_app[1].u8_ble_tx_id == 1)
		{
			if(m_app[1].u8_ble_status == 1)
			{
				u16_uart0_tx_len = 16;
				
				u8_uart0_tx[0] = 0x77;
				u8_uart0_tx[1] = 0x01;
				u8_uart0_tx[2] = 0x0C;
				u8_uart0_tx[3] = 0x0B;
				u8_uart0_tx[4] = m_app[1].u8_handle[0];
				u8_uart0_tx[5] = m_app[1].u8_handle[1];

				if(m_app[1].u8_uart_err == 0)
				{
					u8_uart0_tx[6] = 0x00;
					u8_uart0_tx[7] = 0x09;
					u8_uart0_tx[8] = 0x4B;
					u8_uart0_tx[9] = 0x3F;
					u8_uart0_tx[10] = 0x31;
					u8_uart0_tx[11] = 0x3B;
					u8_uart0_tx[12] = (u8_bat_level<<4)|b_power_status;
					u8_uart0_tx[13] = u8_app_pwm_value[u8_ld_level];
				}
				else if(m_app[1].u8_uart_err == 1)
				{
					u8_uart0_tx[6] = 0x00;					//PTL
					u8_uart0_tx[7] = 0x09;
					u8_uart0_tx[8] = 0x4B;
					u8_uart0_tx[9] = 0x3F;
					u8_uart0_tx[10] = 0x3F;					//?
					u8_uart0_tx[11] = 0x3B;
					u8_uart0_tx[12] = (u8_bat_level<<4)|b_power_status;
					u8_uart0_tx[13] = u8_app_pwm_value[u8_ld_level];

					m_app[1].u8_uart_err = 0;
				}
				
				u8_uart0_tx[14] = calc_str_sum(&u8_uart0_tx[6], 8);
				u8_uart0_tx[15] = calc_mouselet_sum(u8_uart0_tx, 15);

				u8_uart0_tx_tm_cnt = 8;
				u8_bt_get_status_cnt = 0;
				b_bt_get_stauts_trig = 0;
			}
			m_app[1].u8_ble_tx_id = 0;
			return;
		}
		else if(m_app[1].u8_ble_tx_id == 2)
		{
			if(m_app[1].u8_ble_status == 1)
			{
				u16_uart0_tx_len = 14;
				
				u8_uart0_tx[0] = 0x77;						//Header
				u8_uart0_tx[1] = 0x01;						//Command
				u8_uart0_tx[2] = 0x0A;						//Length
				u8_uart0_tx[3] = 0x0B;						//Opcode: Data
				u8_uart0_tx[4] = m_app[1].u8_handle[0];		//Handle
				u8_uart0_tx[5] = m_app[1].u8_handle[1];

				if(m_app[1].u8_uart_err == 0)
				{
					u8_uart0_tx[6] = 0x00;					//PTL
					u8_uart0_tx[7] = 0x07;
					u8_uart0_tx[8] = 0x4B;
					u8_uart0_tx[9] = 0x3F;
					u8_uart0_tx[10] = 0x3B;
					u8_uart0_tx[11] = 00;
				}
				else if(m_app[1].u8_uart_err == 1)
				{
					u8_uart0_tx[6] = 0x00;					//PTL
					u8_uart0_tx[7] = 0x07;
					u8_uart0_tx[8] = 0x4B;
					u8_uart0_tx[9] = 0x3F;
					u8_uart0_tx[10] = 0x3F;					//?
					u8_uart0_tx[11] = 00;

					m_app[1].u8_uart_err = 0;
				}
				
				u8_uart0_tx[12] = calc_str_sum(&u8_uart0_tx[6], 6);		//PTL SUM
				u8_uart0_tx[13] = calc_mouselet_sum(u8_uart0_tx, 13);	//SUM

				u8_uart0_tx_tm_cnt = 8;
				u8_bt_get_status_cnt = 0;
				b_bt_get_stauts_trig = 0;
			}
			m_app[1].u8_ble_tx_id = 0;
			return;
		}
		/*
		if(b_bt_enter_low_energy == 1)
		{
			if((u8_tm_bt_enter_low_energy > 1) && (u8_tm_bt_enter_low_energy < 50))
			{
				u8_tm_bt_enter_low_energy = 0;
				mouselet_tx_broadcast_off();
			}
		}
		*/
	}
}


/*
 * Uart0 初始化
 */
void uart0_init()
{
	/*	晶振	11.0592M
	 *	9600	0xD7
	 11059200/(16*9600) = 72
	 (32768 - 72) = 32,696 = 0x7FB8
	 */
	SCON=0x50;			//MODE 1;REN=1;
	SBRTH=0x7F;
	SBRTL=0xB8;
	BFINE=0;			//UART波特率发生器微调寄存器
	SBRTH|=0x80;
}
#else
void uart0_gen_payload_thread()
{
	uint8 fifo_status = 0;
	if(b_buf_rx_finish == 0)
	{
		fifo_status = QueueOut(&st_fifo_buf, &u8_buf_rx);
		if(fifo_status == QueueOperateOk)
		{
			u8_rx_buf[u8_buf_i] = u8_buf_rx;
			u8_buf_i++;
			u8_rx_len = u8_buf_i;

			if(u8_rx_len == (u8_rx_buf[PL_BLE_A_LENGTH] + PL_BLE_PKG_LEN) )			//Pairlink protocol
			{
				u8_rx_sum = calc_mouselet_sum(u8_rx_buf, (u8_rx_len-1));
				b_buf_rx_finish = 1;
			}
			
			u8_buf_rx_timeout = 5;
		}
		else if(fifo_status == QueueEmpty)
		{
			//当有任意数据接收到后，并且不符合协议规则，即产生一次超时完成通知
			if(u8_buf_rx_timeout == 1)		//20ms
			{
				u8_buf_rx_timeout = 0;
				b_buf_rx_finish = 1;
			}
		}
	}
	
	//数据组包完成，解析数据
	if(b_buf_rx_finish)
	{
		if(u8_rx_buf[u8_rx_len-1] == u8_rx_sum)
		{
			pl_api_parse(u8_rx_buf, u8_rx_len);
			u8_buf_rx_timeout = 0;
		}
		else
		{
			als_cmd_tx_error();
		}
		u8_buf_i = 0;
		u8_rx_len = 0;
		u8_rx_sum = 0;
		b_buf_rx_finish = 0;
		QueueInit(&st_fifo_buf);
	}
}

#endif
#endif

#if (ADC_CONVERT_EN == 1)

#define     ADDGT_INIT         0x0000
#define     ADDLT_INIT         0x0000	  
#define     ADC_CH0            0	   
#define     ADC_CH1            1
#define     ADC_CH2            2
#define     ADC_CH3            3
#define     ADC_CH4            4
#define     ADC_CH5            5
#define     ADC_CH6            6
#define     ADC_CH7            7
#define     ADC_CH8            8
#define     ADC_CH9            9
#define     ADC_CH10           10
#define     ADC_CH11           11
#define     ADC_OP1OUT         12	//提前100us打开OP
#define     ADC_OP2OUT         13	//提前100us打开OPOUT
#define     ADC_VBG            14	//提前100us打开参考源

void adc_init(void)
{
	/*
	tADC[2-0] ADC时钟周期选择位
	0000: 1tSYS
	0001: 2tSYS
	0010: 3tSYS
	0011: 4tSYS
	0100: 6tSYS
	0101: 8tSYS
	0110: 12tSYS
	0111: 16tSYS
	1000: 24tSYS
	1001: 32tSYS
	1010: 48tSYS
	1011: 64tSYS
	1100: 96tSYS
	1101: 128tSYS
	1110: 192tSYS
	1111: 1.5tSYS
	TS[3-0] 采样时间选择位
	2tAD <= 采样时间 = (TS[3:0]+1)*tAD <= 15tAD
	*/
	ADT    = B01111110;		//<7-4>TADC; <3-0>TS
	ADCON2 = B00000000;		//<7:4>GRP; <3>MODE; <2:0>TGAP
	SEQCON = B00000000;		//<7>ALR; <4>REGSEL; <3:0>REG

	ADCH1  = B00000000;		//<7:0>CH7-CH0 1:ADC;  0:IO
	ADCH2  = B00000000;		//<7>CH11; <6>CH10; <5>CH9; <4>CH8; <0>BGCHOP
	//---------关闭运算放大器-------------------
	CMP1CON  = 0x00;
	CMP2CON0 = 0x00;
	CMP2CON1 = B00000000;	//<7:6>PWMTRGS;<5>TRGPOL;<4>BGEN;<3>AMP1EN;<2>AMP2EN;<1:0>C2IFS
	CMP2CON2 = B00000000;
	//-----------ADC compare-----------------------------   
	ADCMPCON = 0x00;
	ADDGTL = 0x00;  //高于此值 ADGIF置1
	ADDGTH = 0x00;
	ADDLTL = 0x00;  //低于此值 ADLIF置1
	ADDLTH = 0x00;	
}

/*******************************************************************************************
** 函数名称: adc_convert
** 函数描述: AD初始化
** 输入参数: ch  :采样的通道号	  P4.0-P4.3	P3.4-P3.7
** 输出参数: 无 	(用多个AD时，每次单独将一个channel设定为 ADC口，并将转换channel
设定为该channel,其他 channel设为IO)
*******************************************************************************************/
uint16 adc_convert(uint8 ch)
{
	uint16 ad_value;

	ADCON2 = 0x00;			//<7:4>GRP; <3>MODE; <2:0>TGAP
	SEQCON = 0x00;			//<7>ALR; <4>REGSEL; <3:0>REG
	SEQCON |= 0x90;			//<7>ALR; <4>REGSEL; <3:0>REG
	if(ch < 8)
	{
		ADCH1 = 1<<ch;
		ADCH2 = 0x00;
	}
	else if(ch < 12)
	{
		ADCH1 = 0x00;
		ADCH2 = 1<<(ch-4);
	}
	SEQCHx  = ch;

	ADCON1 = B10000000;		//<7>ADON; <6>ADCIF; <5>SC; <4>REFC; <3>ADCIE;<2>PWMTRGEN;<1>TIMTRGEN;<0>GO/DONE
	ADCON1|= B00000001;		//<7>ADON; <6>ADCIF; <5>SC; <4>REFC; <3>ADCIE;<2>PWMTRGEN;<1>TIMTRGEN;<0>GO/DONE
	while(!(ADCON1&0x40));
	ADCON1 = ADCON1&0xBF;	//<7>ADON; <6>ADCIF; <5>SC; <4>REFC; <3>ADCIE;<2>PWMTRGEN;<1>TIMTRGEN;<0>GO/DONE
	SEQCON = SEQCON&0x80;	//<7>ALR; <4>REGSEL; <3:0>REG

	ad_value = ((uint16)ADDxH<<8)|ADDxL;

	return(ad_value);
}

/*
 * ADC采样函数 每次调用只完成一次ADC转换，节省CPU时间
 */
void adc_convert_thread()
{
	uint16	u16_adc_once;
	if(u8_adc_loop_index == ADC_BAT)
	{
		u16_adc_once = adc_convert(u8_adc_loop_index);
	//	u16_adc_once = 3600;
		u16_adc_bat_sum += u16_adc_once;
		u8_adc_bat_cnt++;
		if(u8_adc_bat_cnt >= 8)
		{
			u16_adc_bat_avg = 0x10 + (u16_adc_bat_sum>>3);
			u16_adc_bat_sum = 0;
			u8_adc_bat_cnt = 0;
			b_adc_bat_finish = 1;
		}
		u8_adc_loop_index = ADC_PV;
	}
	else if(u8_adc_loop_index == ADC_PV)
	{
		if(b_chg_mos_st == 0)				//充电MOS管截止时检测充电电压
		{
			u16_adc_once = adc_convert(u8_adc_loop_index);
			u16_adc_adp_sum += u16_adc_once;
			u8_adc_adp_cnt++;
			if(u8_adc_adp_cnt >= 8)
			{
			//	DEBUG_OUT = ~DEBUG_OUT; 	//5ms
				u16_adc_adp_avg = (u16_adc_adp_sum>>3);
				u16_adc_adp_sum = 0;
				u8_adc_adp_cnt = 0;
			}
		}
		u8_adc_loop_index = ADC_IIN;
	}
	else if(u8_adc_loop_index == ADC_IIN)
	{
		if(b_chg_mos_st == 1)				//充电MOS管导通时检测充电电流
		{
			u16_adc_once = adc_convert(u8_adc_loop_index);
			if(u16_adc_once >= u16_adc_iin_max)
				u16_adc_iin_max = u16_adc_once;
			if(u16_adc_once <= u16_adc_iin_min)
				u16_adc_iin_min = u16_adc_once;
			u16_adc_iin_sum += u16_adc_once;
			u8_adc_iin_cnt++;
			if(u8_adc_iin_cnt >= 10)
			{
			//	DEBUG_OUT2 = ~DEBUG_OUT2;		//6.6ms
				u16_adc_iin_sum -= u16_adc_iin_max;
				u16_adc_iin_sum -= u16_adc_iin_min;
				u16_adc_iin_avg = u16_adc_iin_sum>>3;
				u16_adc_iin_sum = 0;
				u16_adc_iin_max = 0;
				u16_adc_iin_min = 0xFFFF;
				u8_adc_iin_cnt = 0;
				b_adc_iin_finish = 1;

				if(u16_adc_iin_avg > u16_adc_iin_offset)
				{
					u16_adc_iin_avg -= u16_adc_iin_offset;
				}
			}
		}
		u8_adc_loop_index = ADC_IOUT;
	}
	else if(u8_adc_loop_index == ADC_IOUT)
	{
		u16_adc_once = adc_convert(u8_adc_loop_index);
		u16_adc_iout_sum += u16_adc_once;
		u8_adc_iout_cnt++;
		if(u8_adc_iout_cnt >= 8)
		{
			u16_adc_iout_avg = u16_adc_iout_sum>>3;
			u16_adc_iout_sum = 0;
			u8_adc_iout_cnt = 0;

			if(u16_adc_iout_avg > u16_adc_iout_offset)
			{
				u16_adc_iout_avg -= u16_adc_iout_offset;
			}
		}
		u8_adc_loop_index = ADC_BAT;
	}
}


#endif

/*
 * 通过电池电压及充电电流获取充电时新的电量等级
 */
uint8 get_vdd_chg0_new_level()
{
	uint8 ret = 1;
	
	if(u16_adc_bat_avg >= BAT_LEVEL_5)
	{
		if(u16_adc_iin_avg < BAT_CUR_LEVEL_10)
		{
			ret = 10;
		}
		else if(u16_adc_iin_avg < BAT_CUR_LEVEL_9)
		{
			ret = 9;
		}
		else if(u16_adc_iin_avg < BAT_CUR_LEVEL_8)
		{
			ret = 8;
		}
		else if(u16_adc_iin_avg < BAT_CUR_LEVEL_7)
		{
			ret = 7;
		}
		else if(u16_adc_iin_avg < BAT_CUR_LEVEL_6)
		{
			ret = 6;
		}
		else
		{
			ret = 5;
		}
	}
	else if(u16_adc_bat_avg >= BAT_LEVEL_4)
	{
		ret = 4;
	}
	else if(u16_adc_bat_avg >= BAT_LEVEL_3)
	{
		ret = 3;
	}
	else if(u16_adc_bat_avg >= BAT_LEVEL_2)
	{
		ret = 2;
	}
	else
	{
		ret = 1;
	}

	return ret;
}

uint8 get_vdd_chg_ld_new_level()
{
	uint8 ret = 1;
	for(ret=1; ret<10; ret++)
	{
		if(u16_adc_bat_avg < adc_vdd_chg_ld_array10[ret])
		{
			break;
		}
	}
	return ret;
}

uint8 get_vdd_ld5_new_level()
{
	uint8 ret = 0;
	for(ret=1; ret<10; ret++)
	{
		if(u16_adc_bat_avg < adc_vdd_ld5_array10[ret])
		{
			break;
		}
	}
	return ret;
}

uint8 get_vdd_ld4_new_level()
{
	uint8 ret = 1;
	for(ret=1; ret<10; ret++)
	{
		if(u16_adc_bat_avg < adc_vdd_ld4_array10[ret])
		{
			break;
		}
	}
	return ret;
}

uint8 get_vdd_ld3_new_level()
{
	uint8 ret = 1;
	for(ret=1; ret<10; ret++)
	{
		if(u16_adc_bat_avg < adc_vdd_ld3_array10[ret])
		{
			break;
		}
	}
	return ret;
}

uint8 get_vdd_ld2_new_level()
{
	uint8 ret = 1;
	for(ret=1; ret<10; ret++)
	{
		if(u16_adc_bat_avg < adc_vdd_ld2_array10[ret])
		{
			break;
		}
	}
	return ret;
}

uint8 get_vdd_ld1_new_level()
{
	uint8 ret = 1;
	for(ret=1; ret<10; ret++)
	{
		if(u16_adc_bat_avg < adc_vdd_ld1_array10[ret])
		{
			break;
		}
	}
	return ret;
}

/////////////////////////////////////////////////////////////
/*
 * 电池电压检测函数
 * 根据电池电压采样值获得当前电量等级
 */
void battery_detect()
{
	if(b_adc_bat_finish == 1)
	{
		b_adc_bat_finish = 0;

		//根据电池电压转换充电电量等级
		if(b_power_status == 0)
		{
			if(b_charge_status == 1)
			{
				u8_bat_level_new = get_vdd_chg0_new_level();
			}
			else
			{
				u8_bat_level_new = get_vdd_ld1_new_level();
			}
		}
		else
		{
			if(b_charge_status == 1)
			{
				u8_bat_level_new = get_vdd_chg_ld_new_level();
			}
			else
			{
				if(u8_ld_level == 5)
				{
					u8_bat_level_new = get_vdd_ld5_new_level();
				}
				else if(u8_ld_level == 4)
				{
					u8_bat_level_new = get_vdd_ld4_new_level();
				}
				else if(u8_ld_level == 3)
				{
					u8_bat_level_new = get_vdd_ld3_new_level();
				}
				else if(u8_ld_level == 2)
				{
					u8_bat_level_new = get_vdd_ld2_new_level();
				}
				else if(u8_ld_level == 1)
				{
					u8_bat_level_new = get_vdd_ld1_new_level();
				}
			}
		}
		
		//充电电压稳定算法
		if(u8_bat_level_new == u8_bat_level)
		{
			u8_bat_down_cnt = 0;
			u8_bat_up_cnt = 0;
		}
		else if(u8_bat_level_new < u8_bat_level) 	//电压下降
		{
			u8_bat_up_cnt = 0;
			if(u8_bat_down_cnt > BAT_LEVEL_CNT)			//2分钟
			{
				u8_bat_down_cnt = 0;
				if(u8_bat_level > 0)
				{
					u8_bat_level--;
#if (BLE_V2_EN == 0)
					mouselet_tx_lamp_status();
#else
					als_cmd_tx_status();
#endif
				}
			}
		}
		else if(u8_bat_level_new > u8_bat_level)	//电压上升
		{
			u8_bat_down_cnt = 0;
			if(u8_bat_up_cnt > BAT_LEVEL_CNT)			//2分钟
			{
				u8_bat_up_cnt = 0;
				if(u8_bat_level < 10)
				{
					u8_bat_level++;
#if (BLE_V2_EN == 0)
					mouselet_tx_lamp_status();
#else
					als_cmd_tx_status();
#endif
				}
			}
		}

		//电池电压低于限制电压自动关机
		if(u16_adc_bat_avg < BAT_LOW_OFF2)
		{
			b_power_status = 0;
			
#if (BLE_V2_EN == 0)
			mouselet_ble_off();
#else
			pl_api_ble_off();
#endif
		}

	}
}


/*
 * 10ms无条件调用一次
 * 充电器电压检测函数 - 不充电时有效
 * 输出: b_chgv_level
 */
void adapter_voltage_detect()
{
	if(b_chgv_det_trig == 1)					//10ms检测一次
	{
		b_chgv_det_trig = 0;
		
		if(b_chg_mos_st == 0)					//充电MOS未导通检测充电器电压
		{
			if((u16_adc_adp_avg > 3500) && (u16_adc_adp_avg < 3800))		//12V ~ 13V
			{
				//充电器电压在正常电压范围可以充电
				b_chgv_level_new = 1;
				LM358_EN = LM358_ON;
				u8_tm_idle_timeout = IDLE_CNT;
			}
			else
			{
				//充电器电压低于电池电压, 停止充电
				b_chgv_level_new = 0;
			}
			
			if(b_chgv_level_new == b_chgv_level)
			{
				u8_chgv_level_cnt = 0;
			}
			
			u8_chgv_level_cnt++;
			
			if(u8_chgv_level_cnt > 4)			//电压稳定50ms
			{
				b_chgv_level = b_chgv_level_new;
			}
		}
	}
}

/*
 * 充电时充电器电流检测函数
 * 返回u8_spa_level
 */
void adapter_current_detect()
{
	if(b_adc_iin_finish == 1)
	{
		b_adc_iin_finish = 0;
		
		if(u16_adc_iin_avg < CHG_CUR_FULL)
		{
			b_chga_level_new = 0;			//小于最小检测电流，MOS管间断充电
		}
		else
		{
			b_chga_level_new = 1;			//高于最小检测电流, MOS管一直导通
		}

		if(b_chga_level_new == b_chga_level)
		{
			u8_chga_level_cnt = 0;
		}
		
		u8_chga_level_cnt++;

		if(u8_chga_level_cnt > 9)			//电流稳定100ms，更新新的等级
		{
			u8_chga_level_cnt = 10;
			b_chga_level = b_chga_level_new;
		}
	}
}

/*
 * 充电器充电核心主线程
 */
void adapter_charge_core_thread()
{
	if(b_chgv_level == 1)
	{
		u8_tm_idle_timeout = IDLE_CNT;
		b_charge_status = 1;
		if(b_chga_level == 0)
		{
			u8_chg_tel	= 1;
		}
		else
		{
			u8_chg_tel	= 2;
		}
	}
	else
	{
		b_charge_status = 0;
		u8_chg_tel	= 0;
	}
}

/////////////////////////////////////////////////////////////
/*
 * 电池电量低，关灯前5－10分钟闪烁线程
 */
void bat_low_flash_thread()
{
	if(b_bat_low_det_trig)
	{
		b_bat_low_det_trig = 0;
		
		if(u16_adc_bat_avg > BAT_UP_RESET)
		{
			b_adc_bat_fail = 0;
			b_adc_bat_fail_1st = 0;
			b_adc_bat_level = 1;
			b_adc_bat_level_new = 1;
			b_ld_flash_en = 0;
			u8_ld_flash_cnt = 0;
			u8_adc_up_cnt++;
			u8_adc_down_cnt=0;
		}
		else if(u16_adc_bat_avg > BAT_LD_FLASH)			//2800: 6.3V
		{
			u8_adc_down_cnt=0;
			u8_adc_up_cnt=0;
		}
		else
		{
			u8_adc_up_cnt=0;
			u8_adc_down_cnt++;
		}

		if(u8_adc_up_cnt >= 50)
		{
			u8_adc_up_cnt = 200;
			b_adc_bat_level_new = 1;
		}

		if(u8_adc_down_cnt >= 50)
		{
			u8_adc_down_cnt = 200;
			b_adc_bat_level_new = 0;
		}

		if(b_adc_bat_level_new != b_adc_bat_level)
		{
			b_adc_bat_level = b_adc_bat_level_new;
			if(b_adc_bat_level == 0)
			{
				b_adc_bat_fail = 1;
				b_adc_bat_fail_1st = 1;
			}
		}
		
		if(b_adc_bat_level_new == 0)
		{
			b_bts_en = 0;
			b_usb_en = 0;
		}
	}

	if(b_adc_bat_fail == 1)
	{
		if(u8_ld_flash_cnt == 0)
		{
			if(b_adc_bat_fail_1st)
			{
				b_adc_bat_fail_1st = 0;
				u8_ld_flash_cnt = 60;		//30s
				b_ld_flash_en = 1;
			}
			else
			{
				u8_ld_flash_cnt = 126;		//60+3s
				b_ld_flash_en = 0;
			}
		}
	}
	else
	{
		u8_ld_flash_cnt = 0;
		b_ld_flash_en = 0;
	}
}

/*
 * 负载输出调节
 * 10ms调用一次
 */
void ld_out_adj()
{
	if(b_ld_out_trig == 1)
	{
		b_ld_out_trig = 0;
		
		if(b_power_status == 0)
		{
			LD_EN = 0;			
			PWMRLDEN	= 0x55; 		//允许软件对模块寄存器的修改
			
			PWM0DL	= 0x00;			//<7:0>PWM0占空比低7位
			PWM0DH	= 0x00;			//<3:0>PWM0占空比高4位
			
			PWMRLDEN	= 0xAA; 		//允许模块带缓冲的寄存器的重载
		}
		else
		{
			LD_EN = 1;

			if(u8_ld_level == 5)
			{
				u16_sw_pwm_target = LD_PWM_5;
			}
			else if(u8_ld_level == 4)
			{
				u16_sw_pwm_target = LD_PWM_4;
			}
			else if(u8_ld_level == 3)
			{
				u16_sw_pwm_target = LD_PWM_3;
			}
			else if(u8_ld_level == 2)
			{
				u16_sw_pwm_target = LD_PWM_2;
			}
			else if(u8_ld_level == 1)
			{
				u16_sw_pwm_target = LD_PWM_1;
			}
			u16_sw_pwm_index = u16_sw_pwm_target;
			
			if(u16_sw_pwm_index > u16_sw_pwm_target)
			{
				u16_sw_pwm_index--;
			}
			else if(u16_sw_pwm_index < u16_sw_pwm_target)
			{
				u16_sw_pwm_index++;
			}
			
			PWMRLDEN	= 0x55; 		//允许软件对模块寄存器的修改

			if(b_ld_flash_flag == 0)
			{
				//12位PWM
				PWM0DL	= u16_sw_pwm_index&0xFF;		//<7:0>PWM0占空比低7位
				PWM0DH	= (u16_sw_pwm_index>>8)&0x0F;	//<3:0>PWM0占空比高4位
				
				//8位PWM
			//	PWM0DL	= (u16_sw_pwm_index>>4)&0xFF;		//<7:0>PWM0占空比低7位
			//	PWM0DH	= 0;	//<3:0>PWM0占空比高4位
			//	PWM1DL  = u8_ld_color[u8_sw_pwm_tel-1];	//混色占空比
			//	PWM1DH	= 0;
			}
			else
			{
				PWM0DL	= 0;		//<7:0>PWM0占空比低7位
				PWM0DH	= 0;		//<3:0>PWM0占空比高4位
			}
			
			PWMRLDEN	= 0xAA; 		//允许模块带缓冲的寄存器的重载
			
			u8_tm_idle_timeout = IDLE_CNT;
			/*
			//电池电压低于限制电压自动关机
			if(u16_adc_bat_avg < BAT_LOW_OFF2)
			{
				b_power_status = 0;
				
#if (BLE_V2_EN == 0)
				mouselet_ble_off();
#else
				pl_api_ble_off();
#endif
			}
			else if(u16_adc_bat_avg < BAT_LOW_OFF)
			{
				if(b_charge_status == 0)
				{
				//	b_power_status = 0;
				}
				else
				{
					b_bat_chg_low_flag = 1;
				}
			}

			if(u16_adc_bat_avg > BAT_UP_RESET)	//电压高于10.8V解除限制
			{
				b_bat_chg_low_flag = 0;
			}
			*/
		}
	}
}


/////////////////////////////////////////////////////////////
#if 0
/********串行发送8位数据*************************/
void tm_wt(uint8 dat)
{
	uint8 i = 0;
	P1CR |= B00000010;
	TM_DELAY;
	
	TM_STB = 0;
	for(i=0; i<8; i++)
	{
		TM_CLK = 0;
		TM_DELAY;
		if((dat & 0x01)==0x01)
			TM_DIO = 1;
		else
			TM_DIO = 0;
		TM_CLK = 1;
		dat = dat>>1;
		TM_DELAY;
	}
}

/********串行读取8位数据*************************/
uint8 tm_rd()
{
	uint8 i = 0;
	uint8 dat = 0;
//	TM_DIO = 1;
	P1CR &= B11111101;
	TM_DELAY;
	
	for(i=0; i<8; i++)
	{
		TM_CLK = 0;
		TM_DELAY;
		dat = dat>>1;
		if(TM_DIO)
			dat = dat|0x80;
		TM_CLK = 1;
		TM_DELAY;
	}
	return dat;
}

void tm_init()
{
	TM_EN = 1;
	TM_DELAY;
	
	TM_STB = 0;
	tm_wt(B00000011);		//7SEG 10COM
	TM_STB = 1;
	TM_DELAY;

	TM_STB = 0;
	tm_wt(B10001000);		//显示控制命令 ON, 10/16
	TM_STB = 1;
	TM_DELAY;
}

void tm_clr()
{
	//写数据给显示寄存器
	TM_STB = 0;
	tm_wt(B01000100);
	TM_STB = 1;
	TM_DELAY;

	TM_STB = 0;
	tm_wt(0xC0);
	tm_wt(0);
	TM_STB = 1;

	TM_STB = 0;
	tm_wt(0xC2);
	tm_wt(0);
	TM_STB = 1;

	TM_STB = 0;
	tm_wt(0xC4);
	tm_wt(0);
	TM_STB = 1;

	TM_STB = 0;
	tm_wt(0xC6);
	tm_wt(0);
	TM_STB = 1;

	TM_STB = 0;
	tm_wt(0xC8);
	tm_wt(0);
	TM_STB = 1;
}

void tm_on()
{
	TM_EN = 1;
	TM_DELAY;
	
	TM_STB = 0;
	tm_wt(B10001010);		//显示控制命令 ON, 10/16
	TM_STB = 1;
	TM_DELAY;

}

void tm_off()
{
//	TM_EN = 0;
/*
	TM_STB = 0;
	TM_CLK = 0;
	TM_DIO = 0;
	TM_DELAY;
	*/

	tm_clr();
}

void tm_update()
{	
	tm_ram0.byte = 0x00;
	tm_ram1.byte = 0x00;
	tm_ram2.byte = 0x00;
	tm_ram3.byte = 0x00;
	tm_ram4.byte = 0x00;
	
	if(u8_led_line1 >= 1)	tm_ram0.bits.bit0 = 1;
	if(u8_led_line1 >= 2)	tm_ram0.bits.bit1 = 1;
	if(u8_led_line1 >= 3)	tm_ram0.bits.bit2 = 1;
	if(u8_led_line1 >= 4)	tm_ram0.bits.bit3 = 1;
	if(u8_led_line1 >= 5)	tm_ram0.bits.bit4 = 1;
	
	if(u8_led_line2 >= 1)	tm_ram1.bits.bit0 = 1;
	if(u8_led_line2 >= 2)	tm_ram1.bits.bit1 = 1;
	if(u8_led_line2 >= 3)	tm_ram1.bits.bit2 = 1;
	if(u8_led_line2 >= 4)	tm_ram1.bits.bit3 = 1;
	if(u8_led_line2 >= 5)	tm_ram1.bits.bit4 = 1;
	if(u8_led_line2 >= 6)	tm_ram2.bits.bit0 = 1;
	if(u8_led_line2 >= 7)	tm_ram2.bits.bit1 = 1;
	if(u8_led_line2 >= 8)	tm_ram2.bits.bit2 = 1;
	if(u8_led_line2 >= 9)	tm_ram2.bits.bit3 = 1;
	
	if(u8_led_line2 >= 9)
	{
		if(b_charge_status)		tm_ram2.bits.bit4 = 1;
	}
	else
	{
		if(b_charge_status)		tm_ram2.bits.bit4 = b_tm_500ms_flag;
	}
	
	if(u8_led_line3 == 1)
	{
		tm_ram3.bits.bit0 = 1;
	}
	else if(u8_led_line3 == 2)
	{
		tm_ram3.bits.bit0 = 1;
		b_c5_s1 = 1;
	}
	else if(u8_led_line3 == 3)
	{
		b_c5_s1 = 1;
		tm_ram3.bits.bit1 = 1;
	}
	else if(u8_led_line3 == 4)
	{
		tm_ram3.bits.bit1 = 1;
		tm_ram3.bits.bit2 = 1;
	}
	else if(u8_led_line3 == 5)
	{
		tm_ram3.bits.bit2 = 1;
		tm_ram3.bits.bit3 = 1;
	}
	else if(u8_led_line3 == 6)
	{
		tm_ram3.bits.bit3 = 1;
		tm_ram3.bits.bit4 = 1;
	}
	else if(u8_led_line3 == 7)
	{
		tm_ram3.bits.bit4 = 1;
	}
	
	b_c5_s3 = b_bts_led;
	b_c5_s4 = b_usb_en;
	
	//写数据给显示寄存器
	TM_STB = 0;
	tm_wt(B01000100);
	TM_STB = 1;
	TM_DELAY;
	
	TM_STB = 0;
	tm_wt(0xC0);
	tm_wt(tm_ram0.byte);
	TM_STB = 1;

	TM_STB = 0;
	tm_wt(0xC2);
	tm_wt(tm_ram1.byte);
	TM_STB = 1;

	TM_STB = 0;
	tm_wt(0xC4);
	tm_wt(tm_ram2.byte);
	TM_STB = 1;

	TM_STB = 0;
	tm_wt(0xC6);
	tm_wt(tm_ram3.byte);
	TM_STB = 1;

	TM_STB = 0;
	tm_wt(0xC8);
	tm_wt(tm_ram4.byte);
	TM_STB = 1;

}

void tm_read_key()
{
	uint8 i=0, j=0;
	uint8 b1=0, b2=0, b3=0, b4=0, b5=0;

	TM_STB = 0;
	tm_wt(B01000010);
	TM_DELAY;

	b1 = tm_rd();
	b2 = tm_rd();
	b3 = tm_rd();
	b4 = tm_rd();
	b5 = tm_rd();

	TM_STB = 1;
		
	if((b1 & 0x02) == 0x02)
	{
		u8_tm_key1_down_cnt++;
		if(u8_tm_key1_down_cnt >= KEY_LONG_PRESS_DOWN)
		{
			u8_tm_key1_down_cnt = KEY_LONG_PRESS_DOWN+1;
		}
		else if((u8_tm_key1_down_cnt >= 2) && (u8_tm_key1_down_cnt < KEY_SHORT_PRESS_UP))
		{
			u8_tm_key1_down_cnt = KEY_SHORT_PRESS_UP+1;
			if(b_tm_key1_up_flag)
			{
				b_tm_key1_up_flag = 0;
				u8_tm_key1_up_cnt = 0;
				u8_tm_key_value = 0x01;
				u8_tm_key_value_old = 0x10;
			}
		}
	}
	else
	{
		u8_tm_key1_down_cnt = 0;
		u8_tm_key1_up_cnt++;
		if(u8_tm_key1_up_cnt >= KEY_SHORT_PRESS_UP)
		{
			b_tm_key1_up_flag = 1;
			u8_tm_key1_up_cnt = KEY_SHORT_PRESS_UP;
		}
	}

	if((b1 & 0x10) == 0x10)
	{
		u8_tm_key2_down_cnt++;
		if(u8_tm_key2_down_cnt >= KEY_LONG_PRESS_DOWN)
		{
			u8_tm_key2_down_cnt = KEY_LONG_PRESS_DOWN+1;
		}
		else if((u8_tm_key2_down_cnt >= 2) && (u8_tm_key2_down_cnt < KEY_SHORT_PRESS_UP))
		{
			u8_tm_key2_down_cnt = KEY_SHORT_PRESS_UP+1;
			if(b_tm_key2_up_flag)
			{
				b_tm_key2_up_flag = 0;
				u8_tm_key2_up_cnt = 0;
				u8_tm_key_value = 0x02;
				u8_tm_key_value_old = 0x20;
			}
		}
	}
	else
	{
		u8_tm_key2_down_cnt = 0;
		u8_tm_key2_up_cnt++;
		if(u8_tm_key2_up_cnt >= KEY_SHORT_PRESS_UP)
		{
			b_tm_key2_up_flag = 1;
			u8_tm_key2_up_cnt = KEY_SHORT_PRESS_UP;
		}
	}

	if((b2 & 0x02) == 0x02)
	{
		u8_tm_key3_down_cnt++;
		if(u8_tm_key3_down_cnt >= KEY_LONG_PRESS_DOWN)
		{
			u8_tm_key3_down_cnt = KEY_LONG_PRESS_DOWN+1;
		}
		else if((u8_tm_key3_down_cnt >= 2) && (u8_tm_key3_down_cnt < KEY_SHORT_PRESS_UP))
		{
			u8_tm_key3_down_cnt = KEY_SHORT_PRESS_UP+1;
			if(b_tm_key3_up_flag)
			{
				b_tm_key3_up_flag = 0;
				u8_tm_key3_up_cnt = 0;
				u8_tm_key_value = 0x04;
				u8_tm_key_value_old = 0x40;
			}
		}
	}
	else
	{
		u8_tm_key3_down_cnt = 0;
		u8_tm_key3_up_cnt++;
		if(u8_tm_key3_up_cnt >= KEY_SHORT_PRESS_UP)
		{
			b_tm_key3_up_flag = 1;
			u8_tm_key3_up_cnt = KEY_SHORT_PRESS_UP;
		}
	}

	if((b2 & 0x10) == 0x10)
	{
		u8_tm_key4_down_cnt++;
		if(u8_tm_key4_down_cnt >= KEY_LONG_PRESS_DOWN)
		{
			u8_tm_key4_down_cnt = KEY_LONG_PRESS_DOWN+1;
		}
		else if((u8_tm_key4_down_cnt >= 2) && (u8_tm_key4_down_cnt < KEY_SHORT_PRESS_UP))
		{
			u8_tm_key4_down_cnt = KEY_SHORT_PRESS_UP+1;
			if(b_tm_key4_up_flag)
			{
				b_tm_key4_up_flag = 0;
				u8_tm_key4_up_cnt = 0;
				u8_tm_key_value = 0x08;
				u8_tm_key_value_old = 0x80;
			}
		}
	}
	else
	{
		u8_tm_key4_down_cnt = 0;
		u8_tm_key4_up_cnt++;
		if(u8_tm_key4_up_cnt >= KEY_SHORT_PRESS_UP)
		{
			b_tm_key4_up_flag = 1;
			u8_tm_key4_up_cnt = KEY_SHORT_PRESS_UP;
		}
	}

	if(u8_tm_key_value != u8_tm_key_value_old)
	{
		u8_tm_key_value_old = u8_tm_key_value;

		if(u8_tm_key_value == 1)			//S3 流明减小
		{
			if(b_selftest_flag == 0)
			{
				if(b_power_status == 1)
				{
					if(u8_ld_level>1)
					{
						u8_ld_level--;
						if(u8_ld_level == 1)
						{
						//	u16_sw_pwm_index = u8_cfg0[1];
						//	u16_sw_pwm_target = u16_sw_pwm_index;
						}
						b_ee_wt_en = 1;
						mouselet_tx_lamp_status();
					}
				}
			}
			u8_tm_key_value = 0;
			u8_tm_key_value_old = 0;
		}
		else if(u8_tm_key_value == 4)		//S3 色温减小
		{
			if(b_selftest_flag == 0)
			{
				if(b_power_status == 1)
				{
					if(u8_sw_pwm_tel > 1)
					{
						u8_sw_pwm_tel--;
						b_ee_wt_en = 1;
						mouselet_tx_lamp_status();
					}
				}
			}
			u8_tm_key_value = 0;
			u8_tm_key_value_old = 0;
		}
		else if(u8_tm_key_value == 2)		//S2 流明增加
		{
			if(b_selftest_flag == 0)
			{
				if(b_power_status == 1)			//开灯
				{
					if(u8_ld_level<5)
					{
						u8_ld_level++;
						b_bat_chg_low_flag = 0;
						if(u8_ld_level == 5)
						{
							u16_sw_pwm_index = LD_PWM_5;
							u16_sw_pwm_target = LD_PWM_5;
						}
						b_ee_wt_en = 1;
						mouselet_tx_lamp_status();
					}
				}
			}
			
			u8_tm_key_value = 0;
			u8_tm_key_value_old = 0;
		}
		else if(u8_tm_key_value == 8)		//S2 色温增加
		{
			if(b_selftest_flag == 0)
			{
				if(b_power_status == 1)			//开灯
				{
					if(u8_sw_pwm_tel < 7)
					{
						u8_sw_pwm_tel++;
						b_ee_wt_en = 1;
						mouselet_tx_lamp_status();
					}
				}
			}
			
			u8_tm_key_value = 0;
			u8_tm_key_value_old = 0;
		}
	}
}
/////////////////////////////////////////////////////////////
#endif

/*
 * UI 显示
 * 参数: 
 * b_power_status	开关灯状态
 * u8_ld_level		负载档位
 * u8_bat_level		电池电量
 * 输出控制:
 * u8_led_line1, u8_led_line2, u8_led_line3
 */
void led_show()
{
	uint8 led1=0, led2=0, led3=0;
	b_tm_status = 0;
	if(b_power_status == 0)
	{
		led1 = 0;
		led2 = 0;
	}
	else
	{
		led1 = u8_ld_level;
		led2 = u8_bat_level;
	}
	
	if(b_charge_status == 1)
	{
		led2 = u8_bat_level;
	}

	if(b_bts_en == 1)
	{
		led2 = u8_bat_level;
	}
	
	if(b_usb_en == 1)
	{
		led2 = u8_bat_level;
	}
	
	if(b_selftest_flag == 0)
	{
		u8_led_line1 = led1;
		u8_led_line2 = led2;
	}
	else
	{
		if(b_st_key1 == 1)
		{
			u8_led_line1 = 10;
			u8_st_all |= 0x04;
		}
		else
		{
			u8_led_line1 = 0;
			u8_st_all |= 0x08;
		}
		
		if(b_st_key2 == 1)
		{
			u8_led_line2 = 10;
			u8_st_all |= 0x10;
		}
		else
		{
			u8_led_line2 = 0;
			u8_st_all |= 0x20;
		}

		if(u8_st_all == 0x3C)
		{
			b_selftest_flag = 0;
			u8_selftest_timeout = 0;
		}
		u8_tm_idle_timeout = IDLE_CNT;
		
	}

}

/////////////////////////////////////////////////////////////
#if 0
void scan_key_down()
{
	//按键处理
	if(b_key_scan_trig == 1)
	{
		b_key_scan_trig = 0;
		b_key1_io = KEY_1;
		b_key2_io = KEY_2;
		b_key3_io = KEY_3;
		
		if(b_key1_io == KEY_DOWN)
		{
			u8_key1_down_cnt++;
			if(u8_key1_down_cnt >= KEY_LONG_PRESS_DOWN)
			{
				u8_key1_down_cnt = KEY_LONG_PRESS_DOWN+1;
			}
			else if((u8_key1_down_cnt >= KEY_SHORT_PRESS_DOWN) && (u8_key1_down_cnt < KEY_SHORT_PRESS_UP))
			{
				u8_key1_down_cnt = KEY_SHORT_PRESS_UP+1;
				if(b_key1_up_flag)
				{
					b_key1_up_flag = 0;
					u8_key1_up_cnt = 0;
					u8_key_value = 0x01;
					u8_key_value_old = 0x10;
				}
			}
		}
		else
		{
			u8_key1_down_cnt = 0;
			u8_key1_up_cnt++;
			if(u8_key1_up_cnt >= KEY_SHORT_PRESS_UP)
			{
				b_key1_up_flag = 1;
				u8_key1_up_cnt = KEY_SHORT_PRESS_UP;
			}
		}
		
		if(b_key2_io == KEY_DOWN)
		{
			u8_key2_down_cnt++;
			if(u8_key2_down_cnt >= KEY_LONG_PRESS_DOWN)
			{
				u8_key2_down_cnt = KEY_LONG_PRESS_DOWN+1;
			}
			else if((u8_key2_down_cnt >= KEY_SHORT_PRESS_DOWN) && (u8_key2_down_cnt < KEY_SHORT_PRESS_UP))
			{
				u8_key2_down_cnt = KEY_SHORT_PRESS_UP+1;
				if(b_key2_up_flag)
				{
					b_key2_up_flag = 0;
					u8_key2_up_cnt = 0;
					u8_key_value = 0x02;
					u8_key_value_old = 0x20;
				}
			}
		}
		else
		{
			u8_key2_down_cnt = 0;
			u8_key2_up_cnt++;
			if(u8_key2_up_cnt >= KEY_SHORT_PRESS_UP)
			{
				b_key2_up_flag = 1;
				u8_key2_up_cnt = KEY_SHORT_PRESS_UP;
			}
		}
		
		if(b_key3_io == KEY_DOWN)
		{
			u8_key3_down_cnt++;
			if(u8_key3_down_cnt >= KEY_LONG_PRESS_DOWN)
			{
				u8_key3_down_cnt = KEY_LONG_PRESS_DOWN+1;
			}
			else if((u8_key3_down_cnt >= KEY_SHORT_PRESS_DOWN) && (u8_key3_down_cnt < KEY_SHORT_PRESS_UP))
			{
				u8_key3_down_cnt = KEY_SHORT_PRESS_UP+1;
				if(b_key3_up_flag)
				{
					b_key3_up_flag = 0;
					u8_key3_up_cnt = 0;
					u8_key_value = 0x04;
					u8_key_value_old = 0x40;
				}
			}
		}
		else
		{
			u8_key3_down_cnt = 0;
			u8_key3_up_cnt++;
			if(u8_key3_up_cnt >= KEY_SHORT_PRESS_UP)
			{
				b_key3_up_flag = 1;
				u8_key3_up_cnt = KEY_SHORT_PRESS_UP;
			}
		}
		
		if(u8_key_value_old != u8_key_value)
		{
			if(u8_key_value == 0x01)				//S1 开关
			{
				if(b_selftest_flag == 0)
				{
					if(b_power_status == 1)
					{
						b_power_status = 0;
						b_adc_bat_fail = 0;
						mouselet_ble_off();
					}
					else
					{
						b_power_status = 1;
						mouselet_ble_on();
						b_bat_chg_low_flag = 0;
					}
					mouselet_tx_lamp_status();
				}
				else
				{
					b_st_key1 = ~b_st_key1;
				}
				
				u8_key_value = 0;
				u8_key_value_old = 0;
			}
			else if(u8_key_value == 0x02)			//S2 蓝牙音响开关
			{
				if(b_selftest_flag == 0)
				{
					b_bts_en = ~b_bts_en;
				
					if(b_bts_en)
					{
						u8_tm_bts_timeout = 240;
					}
				}
				
				u8_key_value = 0;
				u8_key_value_old = 0;
			}
			else if(u8_key_value == 0x04)			//S3 充电宝开关
			{
				if(b_selftest_flag == 0)
				{
					b_usb_en = ~b_usb_en;
				}
				
				u8_key_value = 0;
				u8_key_value_old = 0;
			}
		}
	}
}
#endif

/*
 * 来自timer0中断源的软件定时器
 */
void sw_tm_isr()
{
	if(b_tm_10ms_trig == 1)
	{
		b_tm_10ms_trig = 0;
		/////////////////////////////////////////////////////////////
		// 10ms 软件中断
		b_tm_update = 1;
		b_bat_low_det_trig = 1;
		
#if (BLE_V2_EN == 0)
		b_ble_trig = 1;
#else
		if(u8_buf_rx_timeout)
			u8_buf_rx_timeout--;

		pl_api_main_trig();
#endif

		u8_tm_20ms_cnt++;
		if(u8_tm_20ms_cnt > 1)
		{
			u8_tm_20ms_cnt = 0;
			b_tm_20ms_trig = 1;

			u8_tm_120ms_cnt++;
			if(u8_tm_120ms_cnt > 5)
			{
				u8_tm_120ms_cnt = 0;
				b_tm_120ms_flag = ~b_tm_120ms_flag;
			}
		}
		
#if(HW_UART0_EN == 1)
#if (BLE_V2_EN == 0)
		if(u8_uart0_rx_timeout>0)
			u8_uart0_rx_timeout--;
		if(u8_uart0_tx_timeout>0)
			u8_uart0_tx_timeout--;
		if(u8_uart0_tx_tm_cnt>0)
			u8_uart0_tx_tm_cnt--;
#endif
#endif

		u8_tm_trig_cnt++;
		if(u8_tm_trig_cnt > 9)
		{
			u8_tm_trig_cnt = 0;
			b_ld_out_trig = 1;
		}
		
		b_chgv_det_trig = 1;
		b_chga_det_trig = 1;
		
		u8_selftest_cnt++;
		if(u8_selftest_cnt > 100)
		{
			u8_selftest_cnt = 100;
		}
		
		u8_tm_500ms_cnt++;
		if(u8_tm_500ms_cnt > 49)	//500ms
		{
			u8_tm_500ms_cnt = 0;
			b_tm_500ms_flag = ~b_tm_500ms_flag;
			b_tm_500ms_trig = 1;

			if(b_tm_500ms_flag)
			{
				u8_bat_up_cnt++;
				u8_bat_down_cnt++;
				
				if(u8_tm_idle_timeout>0)
					u8_tm_idle_timeout--;
			}
			
			//软件PWM驱动
			b_sw_pwm_flag = ~b_sw_pwm_flag;
		
			if(u8_tm_bts_timeout)
				u8_tm_bts_timeout--;
			
			if(u8_chg_tel == 0)
			{
				CHARGEB = CHARGEB_OFF;
				b_chg_mos_st = 0;
			}
			else if(u8_chg_tel == 1)
			{
				if(b_sw_pwm_flag == 1)
				{
					CHARGEB = CHARGEB_ON;
					b_chg_mos_st = 1;
				}
				else
				{
					CHARGEB = CHARGEB_OFF;
					b_chg_mos_st = 0;
				}
			}
			else
			{
				CHARGEB = CHARGEB_ON;
				b_chg_mos_st = 1;
			}
		
			if(u8_selftest_timeout > 2)
			{
				u8_selftest_timeout--;
			}
			else if(u8_selftest_timeout > 0)
			{
				b_selftest_flag = 0;
				u8_selftest_timeout = 0;
			}
			else
			{
				u8_selftest_timeout = 0;
			}

			//电池电量低闪灯驱动
			if(u8_ld_flash_cnt > 0)
			{
				u8_ld_flash_cnt--;
				if(b_ld_flash_en)
				{
					b_ld_flash_flag = ~b_ld_flash_flag;
				}
				else
				{
					if(u8_ld_flash_cnt < 6)
					{
						b_ld_flash_flag = ~b_ld_flash_flag;
					}
					else
					{
						b_ld_flash_flag = 0;
					}
				}
			}
			else
			{
				b_ld_flash_flag = 0;
			}

			
#if (SW_UART_EN == UART_SW_MODE)
			//发送数据API调用
			u8_tm_stx_cnt++;
			if(u8_tm_stx_cnt > 9)
			{
				u8_tm_stx_cnt = 0;
				
				u8_stx[0] = 0x10;
				u8_stx[1] = 0x41;	//PTL_TRAN_MCU_SWUART_PC	= 0x41	//MCU 软件串口发送到电脑 970 bps
				u8_stx[2] = 0x37;	//PTL_MCU_G80F960A			= 0x37	//PTL_MCU_G80F960A
				u8_stx[3] = (u16_adc_bat_avg>>8)&0xFF;
				u8_stx[4] = u16_adc_bat_avg&0xFF;
				u8_stx[5] = (u16_adc_adp_avg>>8)&0xFF;
				u8_stx[6] = u16_adc_adp_avg&0xFF;
				u8_stx[7] = (u16_adc_iin_avg>>8)&0xFF;
				u8_stx[8] = u16_adc_iin_avg&0xFF;
				u8_stx[9] = (u16_adc_iout_avg>>8)&0xFF;
				u8_stx[10] = u16_adc_iout_avg&0xFF;
				u8_stx[11] = ((uint8)u8_bat_level<<4)|u8_bat_level_new;
				u8_stx[12] = ((uint8)b_power_status<<4)|u8_ld_level;
				u8_stx[13] = (u8_bat_level<<4)|((uint8)b_charge_status<<1)|b_power_status;;
				u8_stx[14] = 0xEE;
				u8_stx[15] = u8_tm_idle_timeout;
				u8_stx_i = 0;
			}
#endif
		}
		
#if (SW_UART_EN == UART_SW_MODE)
		//发送数据驱动
		if(b_stx_bit_ing == 0)
		{
			if(u8_stx_i < 16)
			{
				u16_stx_buf = ((uint16)u8_stx[u8_stx_i]<<1)|0x0200;
				b_stx_bit_ing = 1;
				u8_stx_i++;
			}
		}
#endif

		/////////////////////////////////////////////////////////////
	}
}

void main()
{
	//使能外部晶振
	CLKCON = B00001000;	//<6:5>CLKS;<4>SCMIF;<3>OSCXON;<2>FS
	CLR_WDT();
	
	P0CR	= B00000011;	//输入输出	1:输出; 0:输入
	P0PCR	= B00001100;	//上拉		1:开启; 0:关闭
	P0		= B00001100;	//电平		1:高;	0:低
	
	P1CR	= B11110011;	//输入输出	1:输出; 0:输入
	P1PCR	= B00001100;	//上拉		1:开启; 0:关闭
	P1		= B00001100;	//电平		1:高;	0:低
	
	P2CR	= B11101111;	//输入输出	1:输出; 0:输入
	P2PCR	= B00010000;	//上拉		1:开启; 0:关闭
	P2		= B00010100;	//电平		1:高;	0:低
	
	P3CR	= B11000101;	//输入输出	1:输出; 0:输入
	P3PCR	= B00001010;	//上拉		1:开启; 0:关闭
	P3		= B00001110;	//电平		1:高;	0:低

	TCON1	= B00000000;	//<6:5>TCLKSn;<3:2>TCLKPn;<1:0>TCn
	TCON	= B00010001;	//<7>TF1;<6>TR1;<5>TF0;<4>TR0;<3>IE1;<2>IT1;<1>IE0;<0>IT0
	TMOD	= B00010001; 	//<7>GATE1;<6>C/T1;<5>M11;<4>M10;<3>GATE0;<2>C/T0;<1>M01;<0>M00
	
	TH0 	= 0x00;			//T0定时器
	TL0 	= 0x00;
	TH1 	= 0x00;			//T1定时器
	TL1 	= 0x00;

	T2CON	= B00000100;	//<7>TF2;<6>EXF2;<5>RCLK;<3>EXEN2;<2>TR2;<1>C/T2;<0>CP/RL2
	T2MOD	= B10000000;	//<7>TCLKP2;<1>T2OE;<0>DCEN
	RCAP2H	= 0xFF;			//T2重载数据
	RCAP2L	= 0xD7;
	TH2 	= 0xFF;			//T2定时器
	TL2 	= 0xD7;
	
	T3CON	= B00000000;
	SCON	= B00000000;
	PWM3CON	= B00000000;
	BUZCON	= B00000000;
	PWMRLDEN	= 0x55;			//允许软件对模块寄存器的修改
	
	PMANUALCON2 = B00000000;	//<5:3>POUTx1; <2:0>POUTx	0:输出0; 1:输出1
	PMANUALCON1 = B00000000;	//<5:3>PMANUALx1; <2:0>PMANUALx
	PWMOE		= B00000000;	//<7>PWMEN; <5>PWM21OE;<4>PWM11OE;<3>PWM01OE;<2>PWM2OE;<1>PWM1OE;<0>PWM0OE

	PTCON		= B00000100;	//<5:4>PTMOD; <3:2>PTCLK; <1:0>POSTPS
	PWMCON1		= B00010010;	//<7>POUTMOD; <5>PWM21S;<4>PWM11S;<3>PWM01S;<2>PWM2S;<1>PWM1S;<0>PWM0S
	PWMCON2		= B00001000;	//<6>ZETIM; <5>PEAD;<4>ZEAD;<3>PDLDEN;<1:0>DT
	
	PWMDT0L		= 0x00;			//PWM死区控制寄存器0的低位
	PWMDT0H		= 0x00;			//PWM死区控制寄存器0的高位
	PWMDT1L		= 0x00;			//PWM死区控制寄存器1的低位
	PWMDT1H		= 0x00;			//PWM死区控制寄存器1的高位

	PWMPL		= 0xFF;			//PWM周期
	PWMPH		= 0x0F;

	PWM0DL		= 0x00;			//PWM占空比
	PWM0DH		= 0x00;
	PWM1DL		= 0x00;
	PWM1DH		= 0x00;
	PWM2DL		= 0x00;
	PWM2DH		= 0x00;

	FLTCON		= 0x00;			//PWM0/1/2故障检测保护寄存器

	PWMINTEN	= B00000000;	//<7>PWMPIE;<6>PWMZIE;<5>PTDD2IE;<4>PTUD2IE;<3>PTDD1IE;<2>PTUD1IE;<1>PTDD0IE;<1>PTUD0IE
	PWMINTF		= 0x00;

	PWMOE		= B10000001;	//<7>PWMEN; <5>PWM21OE;<4>PWM11OE;<3>PWM01OE;<2>PWM2OE;<1>PWM1OE;<0>PWM0OE
	PWMRLDEN	= 0xAA;			//允许模块带缓冲的寄存器的重载

	/*中断优先级
	IPHx	IPLx
	0		0		等级0 最低
	0		1		等级1
	1		0		等级2
	1		1		等级3 最高
	*/
	IPL0	= B00000000;	//<6>PADCL;<5>PT2L;<4>PSL;<3>PT1L;<2>PX1L;<1>PT0L;<0>PX0L
	IPH0	= B00000000;	//<6>PADCH;<5>PT2H;<4>PSH;<3>PT1H;<2>PX1H;<1>PT0H;<0>PX0H
	IPL1	= B00000000;	//<7>PSCML;<6>PPWML;<5>PMCML;<4>PT3L;<3>PX4L;<2>PCMP2L;<1>PCMP1L;<0>PSPIL
	IPH1	= B00000000;	//<7>PSCMH;<6>PPWMH;<5>PMCMH;<4>PT3H;<3>PX4H;<2>PCMP2H;<1>PCMP1H;<0>PSPIH

	PCON	= B00110000;	//<7>SMOD;<6>SSTAT;<5:4>IT4;<3:2>GF;<1>PD;<0>IDL
	IENC	= B11100111;	//<7>EXS47;<6>EXS46;<5>EXS45;<4>EXS44;<3>EXS43;<2>EXS42;<1>EXS41;<0>EXS40
	EXF1	= B00000000;	//<7> IF47;<6> IF46;<5> IF45;<4> IF44;<3> IF43;<2> IF42;<1> IF41;<0> IF40
	
	IEN1	= B00001000;	//<7>ESCM;<6>EPWM3;<5>EMCM;<4>ET3;<3>EX4;<2>ECMP2;<1>ECMP1;<0>ESPI
	IEN0	= B10000010;	//<7>EA;<6>EADC;<5>ET2;<4>ES;<3>ET1;<2>EX1;<1>ET0;<0>EX0

	//系统时钟选择外部晶振
	CLKCON = B00001100;	//<6:5>CLKS;<4>SCMIF;<3>OSCXON;<2>FS
	/////////////////////////////////////////////////////////////
	

#if(ADC_CONVERT_EN == 1)
	adc_init();

	u8_adc_loop_index = ADC_BAT;
	u16_adc_bat_avg = 0;
	u16_adc_bat_sum = 0;
	u8_adc_bat_cnt = 0;

	u16_adc_adp_avg = 0;
	u16_adc_adp_sum = 0;
	u8_adc_adp_cnt = 0;

	u16_adc_iin_avg = 0;
	u16_adc_iin_sum = 0;
	u16_adc_iin_max = 0;
	u16_adc_iin_min = 0xFFFF;
	u8_adc_iin_cnt = 0;
	u16_adc_iin_offset = 0;

	u16_adc_iout_sum = 0;
	u8_adc_iout_cnt = 0;
	u16_adc_iout_offset = 0;

#endif
	b_adc_bat_level = 1;
	b_adc_bat_level_new = 1;
	b_ld_flash_flag = 0;
	u8_ld_flash_cnt = 0;

#if(HW_UART0_EN == 1)
	uart0_init();

#if (BLE_V2_EN == 0)
	u8_uart0_rx_buf = 0;
	u16_uart0_tx_len = 0;
	u16_uart0_rx_len = 0;
	memset(u8_uart0_rx, 0, 32);
	memset(u8_uart0_tx, 0, 32);
	u8_uart0_rx_timeout = 0;
	u8_uart0_tx_timeout = 0;
	u8_uart0_tx_tm_cnt = 0;
#else
	QueueInit(&st_fifo_buf);

	pl_api_init();
	pl_api_ble_off();
	memset(u8_rx_buf, 0, 32);
#endif
#endif

//	u8_led_ram0 = 3;
	memset(u8_cfg0, 0, 8);

	b_power_status = 0;
	u8_ld_level = 3;
	u8_sw_pwm_tel = 1;
	b_ee_rd_en = 1;

	b_chg_mos_st = 0;
	b_chgv_level = 0;
	b_chgv_level_new = 0;
	b_chga_level = 0;
	b_chga_level_new = 0;
	u8_chg_tel = 0;
	u8_bat_level = 1;
	u8_bat_level_new = 1;
	u8_bat_down_cnt = 0;
	u8_bat_up_cnt = 0;
	b_charge_status = 0;

	u8_led_line1 = 0;
	u8_led_line2 = 0;
	u8_adc_loop_index = ADC_BAT;

	u8_tm_idle_timeout = IDLE_CNT;
	u8_selftest_cnt = 10000;
	b_selftest_flag = 0;
	u8_st_all = 0;
	b_key1_up_flag = 1;
	b_key2_up_flag = 1;
	b_key4_up_flag = 1;
	b_key8_up_flag = 1;
	b_key10_up_flag = 1;
	u8_key1_down_cnt = 0;
	u8_key2_down_cnt = 0;
	u8_key4_down_cnt = 0;
	u8_key8_down_cnt = 0;
	u8_key10_down_cnt = 0;

#if (BLE_V2_EN == 0)
	b_bt_broadcast_en = 1;
	m_app[0].u8_ble_status = 0;
	m_app[1].u8_ble_status = 0;
	m_app[0].u8_uart_err = 0;
	m_app[1].u8_uart_err = 0;
//	mouselet_refresh();
	mouselet_ble_off();	
#endif

	b_bts_en = 0;
	b_usb_en = 0;

	b_tm_120ms_flag = 0;

	while(u8_selftest_cnt > 0) 
	{
		CLR_WDT();
#if(ADC_CONVERT_EN == 1)
		u8_adc_loop_index = ADC_BAT;
		adc_convert_thread();
		adc_convert_thread();
		b_chgv_det_trig = 1;
		b_chg_mos_st = 0;
		adapter_voltage_detect();
		adapter_charge_core_thread();
#endif
		u8_selftest_cnt--;
	}
	
	if(b_charge_status == 0)
	{
		u8_bat_level_new = get_vdd_ld1_new_level();
		u8_bat_level = u8_bat_level_new;
	}
	
	b_charge_status = 0;
	u8_chg_tel	= 0;
	
#if (BLE_V2_EN == 1)
	get_chip_number();
#endif	
	/////////////////////////////////////////////////////////////
	while(1)
	{
		CLR_WDT();

		/////////////////////////////////////////////////////////////
		//软件定时器，系统运行时钟 10ms触发一次
		sw_tm_isr();
		/////////////////////////////////////////////////////////////
#if (ADC_CONVERT_EN == 1)
		adc_convert_thread();
#endif
		bat_low_flash_thread();
		/////////////////////////////////////////////////////////////
		battery_detect();
		adapter_voltage_detect();
		adapter_current_detect();
		adapter_charge_core_thread();
		/////////////////////////////////////////////////////////////
		led_show();
#if 0
		if(b_tm_update)
		{
			b_tm_update = 0;
			if(b_tm_status == 1)
			{
				if(b_tm_status_old != b_tm_status)
				{
					b_tm_status_old = b_tm_status;
					
					tm_clr();
				}
				u8_tm_idle_timeout = IDLE_CNT;
				tm_on();
				tm_update();
				tm_read_key();
			}
			else
			{
				if(b_tm_status_old != b_tm_status)
				{
					b_tm_status_old = b_tm_status;
					
					tm_clr();
				}
				tm_off();
			}
		}
#endif
		ld_out_adj();
		/////////////////////////////////////////////////////////////
#if(HW_UART0_EN == 1)
#if (BLE_V2_EN == 0)
		uart0_rx_thread();
		uart0_tx_thread();
		uart0_tx_thread_user();
		mouselet_main_thread();
#else
		uart0_gen_payload_thread();
		pl_api_main_thread();
#endif
#endif

		/////////////////////////////////////////////////////////////
		USB_EN = b_usb_en;
		if(b_usb_en)
		{
			u8_tm_idle_timeout = IDLE_CNT;
		}
		////////////////////////////////////////////////////////////*/
		//蓝牙状态检测
		//未连接30秒后关闭蓝牙电源
		if(b_bts_en)
		{
			u8_tm_idle_timeout = IDLE_CNT;
			P2CR  &= B11101111;
			P2PCR |= B00010000;
			P2    |= B00010000;
			
			BTS_EN = 1;

			if(b_tm_20ms_trig)
			{
				b_tm_20ms_trig = 0;

				if(BTS_ST == 1)
				{
					//蓝牙未连接
					u8_tm_bts_cnt = 0;
					b_bts_st = 0;
				}
				else
				{
					u8_tm_bts_cnt++;
				}

				if(u8_tm_bts_cnt > 15)			//150*20ms=3000ms
				{
					u8_tm_bts_cnt = 150;
					
					u8_tm_bts_timeout = 240;
					b_bts_st = 1;
				}
			}
			if(b_bts_st)
			{
				b_bts_led = 1;
			}
			else
			{
				b_bts_led = b_tm_120ms_flag;
			}
		}
		else
		{
			P2CR  &= B11101111;
			P2PCR &= B11101111;
			P2    &= B11101111;
			BTS_EN = 0;
			b_bts_led = 0;
		}
		
		if(u8_tm_bts_timeout == 0)
		{
			b_bts_en = 0;
		}

		////////////////////////////////////////////////////////////*/
		//存储器读写
#if(EEPROM_RD_WT_EN == 1)
		//读EEPROM内容
		if(b_ee_rd_en)
		{
			b_ee_rd_en = 0;

			eeprom_rd(0, 0, u8_cfg0, 8);
			if((u8_cfg0[0]>0) && (u8_cfg0[0]<6))
			{
				u8_ld_level = u8_cfg0[0];
			}
		}
		/////////////////////////////////////////////////////////////
		//写EEPROM内容
		if(b_ee_wt_en)
		{
			b_ee_wt_en = 0;
			
			u8_cfg0[0] = u8_ld_level;
			
			SSP_Flag1 = 0x55;
			SSP_Flag2 = 0xAA;
			eeprom_clr(0);
			SSP_Flag1 = 0x55;
			SSP_Flag2 = 0xAA;
			eeprom_wt(0, 0, u8_cfg0, 8);
		}
#endif
		/////////////////////////////////////////////////////////////
		//休眠
#if 1
//		u8_tm_idle_timeout = IDLE_CNT;

		if(u8_tm_idle_timeout == 0)
		{
			ADCH1  = B00001000; 	//<7:0>CH7-CH0 1:ADC;  0:IO
			ADCH2  = B00000000; 	//<7>CH11; <6>CH10; <5>CH9; <4>CH8; <0>BGCHOP
			ADCON1 = B00000000; 	//<7>ADON; <6>ADCIF; <5>SC; <4>REFC; <3>ADCIE;<2>PWMTRGEN;<1>TIMTRGEN;<0>GO/DONE
			
			SCON	= B01000000; 	//SM0 SM1 SM2 REN TB8 RB8 TI RI 
			
			IEN1	= B00000000;	//<5>EPWM;<3>EX4;<0>ESPI
			IEN0	= B00000000;	//<7>EA;<6>EADC;<5>ET2;<4>ES0;<3>ET1;<2>EX1;<1>ET0;<0>EX0
			
			PWMRLDEN	= 0x55; 		//允许软件对模块寄存器的修改
			PWMOE		= B00000000;	//<7>PWMEN; <5>PWM21OE;<4>PWM11OE;<3>PWM01OE;<2>PWM2OE;<1>PWM1OE;<0>PWM0OE
			PWMRLDEN	= 0xAA; 		//允许模块带缓冲的寄存器的重载
			
			P0CR	= B00000011;	//输入输出	1:输出; 0:输入
			P0PCR	= B00001100;	//上拉		1:开启; 0:关闭
			P0		= B00001100;	//电平		1:高;	0:低
			
			P1CR	= B11110011;	//输入输出	1:输出; 0:输入
			P1PCR	= B00001100;	//上拉		1:开启; 0:关闭
			P1		= B00001100;	//电平		1:高;	0:低
			
			P2CR	= B11101111;	//输入输出	1:输出; 0:输入
			P2PCR	= B00010000;	//上拉		1:开启; 0:关闭
			P2		= B00010110;	//电平		1:高;	0:低
			
			P3CR	= B11000101;	//输入输出	1:输出; 0:输入
			P3PCR	= B00001010;	//上拉		1:开启; 0:关闭
			P3		= B00001110;	//电平		1:高;	0:低

			IEN1	= B00001000;	//<7>ESCM;<6>EPWM3;<5>EMCM;<4>ET3;<3>EX4;<2>ECMP2;<1>ECMP1;<0>ESPI
			IEN0	= B10000101;	//<7>EA;<6>EADC;<5>ET2;<4>ES0;<3>ET1;<2>EX1;<1>ET0;<0>EX0

			PCON	|= B00110000;	//<7>SMOD;<6>SSTAT;<5:4>IT4;<3:2>GF;<1>PD;<0>IDL
			IENC	= B11100001;	//<7>EXS47;<6>EXS46;<5>EXS45;<4>EXS44;<3>EXS43;<2>EXS42;<1>EXS41;<0>EXS40
			EXF1	= B00000000;	//<7> IF47;<6> IF46;<5> IF45;<4> IF44;<3> IF43;<2> IF42;<1> IF41;<0> IF40
			

			//进入IDLE模式 1.5mA
			/*
			SUSLO = 0x55;
			PCON |= 0x01;
			_nop_();
			_nop_();
			_nop_();
			_nop_();
			_nop_();
			_nop_();
			//*/
			
			//进入POWER DOWN模式 7uA
			//*
			SUSLO = 0x55;
			PCON |= 0x02;
			_nop_();
			_nop_();
			_nop_();
			_nop_();
			_nop_();
			_nop_();
			//*/
			
			IENC	= B00000000;	//<7>EXS47;<6>EXS46;<5>EXS45;<4>EXS44;<3>EXS43;<2>EXS42;<1>EXS41;<0>EXS40
			EXF1	= B00000000;	//<7> IF47;<6> IF46;<5> IF45;<4> IF44;<3> IF43;<2> IF42;<1> IF41;<0> IF40
			
			P0CR	= B00000011;	//输入输出	1:输出; 0:输入
			P0PCR	= B00001100;	//上拉		1:开启; 0:关闭
			P0		= B00001100;	//电平		1:高;	0:低
			
			P1CR	= B11110011;	//输入输出	1:输出; 0:输入
			P1PCR	= B00001100;	//上拉		1:开启; 0:关闭
			P1		= B00001100;	//电平		1:高;	0:低
			
			P2CR	= B11101111;	//输入输出	1:输出; 0:输入
			P2PCR	= B00010000;	//上拉		1:开启; 0:关闭
			P2		= B00010110;	//电平		1:高;	0:低
			
			P3CR	= B11000101;	//输入输出	1:输出; 0:输入
			P3PCR	= B00001010;	//上拉		1:开启; 0:关闭
			P3		= B00001110;	//电平		1:高;	0:低

			adc_init();
			
			PWMRLDEN	= 0x55; 		//允许软件对模块寄存器的修改
			PWMOE		= B10000001;	//<7>PWMEN; <5>PWM21OE;<4>PWM11OE;<3>PWM01OE;<2>PWM2OE;<1>PWM1OE;<0>PWM0OE
			PWMRLDEN	= 0xAA; 		//允许模块带缓冲的寄存器的重载

			SCON	= B01010000; 	//SM0 SM1 SM2 REN TB8 RB8 TI RI 
			IEN1	= B00001000;	//<7>ESCM;<6>EPWM3;<5>EMCM;<4>ET3;<3>EX4;<2>ECMP2;<1>ECMP1;<0>ESPI
			IEN0	= B10000010;	//<7>EA;<6>EADC;<5>ET2;<4>ES;<3>ET1;<2>EX1;<1>ET0;<0>EX0
			
			u8_tm_idle_timeout = IDLE_CNT;
#if (HW_UART0_EN == 1)
#if (BLE_V2_EN == 0)
			uart0_init();
			u8_uart0_rx_buf = 0;
			u16_uart0_tx_len = 0;
			u16_uart0_rx_len = 0;
			memset(u8_uart0_rx, 0, 32);
			memset(u8_uart0_tx, 0, 32);
			u8_uart0_rx_timeout = 0;
			u8_uart0_tx_timeout = 0;
			u8_uart0_tx_tm_cnt = 0;
#else
			QueueInit(&st_fifo_buf);

			pl_api_init();

#endif
#endif

		}
#endif
		/////////////////////////////////////////////////////////////
	}
}

