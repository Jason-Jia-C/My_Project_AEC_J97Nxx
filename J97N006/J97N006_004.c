/*]
项目名称:WorkLight_M(大直板)
	编辑日期:2018.3.28
	作者	:JasonJia
	PCB负责 :龚为
	芯片封装:HT66F004 NSOP_16
	软件时钟:1ms
	功能描述:
		顶部-弱-强-关
		长按1.5S电量显示、
		工厂测试程序
	修改记录:
	脚位定义
/////////////HT66F004////////////
		 ------------------------
		|VSS				  VDD|
 CHG_DET|PC0		 PB0/INT0/AN0| LD2_HALF
	SEG2|PC1		 PB1/INT1/AN1| V5V_IN
	SEG1|PC2		PB2/PTCK0/AN2|
	 KEY|PA0/PTP0	PA4/PTCK1/AN3| 
	SEG3|PA1/PTP0I	 PA5/AN4/VREF| LD_EN2
		|PA2		PA6/AN5/VREFO| LD_EN1
	SEG4|PA3/PTP1I	 PA7/PTP1/AN6| SEG5 
		 ------------------------
*/

#include "HT66F004.h"

#define	bool	bit
#define	uint8	unsigned char
#define	uint16	unsigned int

/////////////////////////////////////////////////////////////

#define		pin2	_pc0
#define		pin3	_pc1
#define		pin4	_pc2
#define		pin5	_pa0
#define		pin6	_pa1
#define		pin7	_pa2
#define		pin8	_pa3
#define		pin9	_pa7
#define		pin10	_pa6
#define		pin11	_pa5
#define		pin12	_pa4
#define		pin13	_pb2
#define		pin14	_pb1
#define		pin15	_pb0

//输入
#define		KEY1			pin5		//按键
#define		CHG_DET			pin2		//充电检测
#define		V5V_IN			pin14		//5V输入

//输出
#define		LD_EN1			pin10
#define		LD_EN2			pin11
#define		LCD_SEG1		pin4
#define		LCD_SEG2		pin3
#define		LCD_SEG3		pin6
#define		LCD_SEG4		pin8
#define		LCD_SEG5		pin9

#define		LD12_OFF		{_pa5 = 0 ; _pbc0 = 1;  _pa6 = 0;}
#define		LD1_TOP			{_pa5 = 1 ; _pbc0 = 1;  _pa6 = 0;}
#define		LD2_HALF		{_pa5 = 0 ; _pbc0 = 1;  _pa6 = 1;}	
#define		LD2_FULL		{_pa5 = 0 ; _pbc0 = 0;  _pb0 = 0; _pa6 = 1;}
#define 	CHG_ON			{_pc0 = 1 ; _pcc0 = 1;  _pcpu0 = 1; _pc0 = 1;}
#define		CHG_OFF			{_pc0 = 0 ; _pcc0 = 0;  _pcpu0 = 0; _pc0 = 0;}


#define		CHG_DET_FULL	0			//充电时已充满
#define		CHG_DET_CHGING	1			//充电中未充满

#define		LCD_ON			0
#define		LCD_OFF			1

#define		OUT_ON			1
#define		OUT_OFF			0

#define		KEY_DOWN		0
#define		KEY_UP			1
#define		KEY_SHORT_PRESS_DOWN	20		//20ms
#define		KEY_SHORT_PRESS_UP		50		//50ms
#define		KEY_LONG_PRESS_DOWN		1500	//3000ms

#define		BAT_CHG_LEVEL_4		1960	//3.98V以上
#define		BAT_CHG_LEVEL_3		1900	//3.86-3.98
#define		BAT_CHG_LEVEL_2		1840	//3.72-3.86

#define		BAT_LEVEL_5			1968	//4.02以上
#define		BAT_LEVEL_4			1920	//3.92-4.02
#define		BAT_LEVEL_3			1850	//3.78-3.92
#define		BAT_LEVEL_2			1760	//3.60-3.78

#define		IDLE_CNT		5000
/////////////////////////////////////////////////////////////
#pragma	rambank0

uint8	u8_tm_1ms_cnt;
uint8	u8_tm_1ms_flag;
uint16	u16_tm_3s_timeout;		//电量显示延时
uint16	u16_tm_idle_timeout;
uint8	u8_tm_10ms;
uint8	u8_tm_selftest;
uint8	u8_selftest_flag;

uint8	u8_key_value;
uint8	u8_key_value_old;
uint16	u16_key_cnt1;

uint8	u8_v5v_on_cnt;
uint8	u8_v5v_off_cnt;
/////////////////////////////////////////////////////////////

bool	b_me4056_chg_en;
bool	b_me4056_status;
uint8	u8_chg_full_cnt;
uint8	u8_chg_notfull_cnt;
uint8	u8_loader_mode;

//显示驱动变量
uint8	u8_led_level;				//数码管显示参数
uint8	u8_led_level_temp;

//adc param
uint16	u16_adc_vdd,u16_adc_vdd_avg;
uint16	u16_adc_vdd_min, u16_adc_vdd_max, u16_adc_vdd_sum;
uint8	u8_adc_vdd_cnt;
//adc param end

uint8	u8_bat_level;
uint8	u8_bat_level_new;
uint16	u16_bat_level_up_cnt;		//电压上升滤波计数器
uint16	u16_bat_level_down_cnt;		//电压下降滤波计数器

uint16	u16_bat_level_up_target;
uint16	u16_bat_level_down_target;


uint8	u8_sw_pwm_cnt;			//软件PWM记数器
uint8	u8_sw_pwm_tel;			//软件PWM占空比

#pragma	norambank

/////////////////////////////////////////////////////////////

//电池电量
enum led_level_t
{
	led_level_0,		//电量最低
	led_level_1,
	led_level_2,
	led_level_3,
	led_level_4,
	led_level_5,		//电量最高
};

//负载档位状态
enum loader_mode_t
{
	ld_off,				//负载关
	ld_top,				//顶部
	ld_half,			//半亮
	ld_full,			//全亮
};


enum me4056_chg_t
{
	me4056_chg_off,				//无5V输入，无充电
	me4056_chg_on,				//有5V输入，在充电
};

enum me4056_chg_status_t
{
	me4056_st_charging,			//充电中，电量未满
	me4056_st_chg_full,			//充电完成
};
/////////////HT66F004////////////
#pragma vector	ext0_int	@ 0x04		
#pragma vector	tb0_int 	@ 0x08		
#pragma vector	tb1_int 	@ 0x0C		
#pragma vector	mf0_int 	@ 0x10		
#pragma vector	eeprom_int	@ 0x14		
#pragma vector	adc_int 	@ 0x18		
#pragma vector	ext1_int	@ 0x1C	
void ext0_int	(void)	{_int0e=0;}
void tb1_int	(void)	{_tb1e=0;}
void mf0_int	(void)	{_mf0e=0;}
void eeprom_int	(void)	{_dee=0;}
void adc_int	(void)	{_ade=0;}
void ext1_int	(void)	{_int1e=0;}
/////////////HT66F004////////////

void tb0_int()
{
	u8_tm_1ms_flag = 1;
}

/*////////////HT66F004////////////
		 ------------------------
		|VSS				  VDD|
 CHG_DET|PC0		 PB0/INT0/AN0| LD2_HALF
	SEG2|PC1		 PB1/INT1/AN1| V5V_IN
	SEG1|PC2		PB2/PTCK0/AN2|
	 KEY|PA0/PTP0	PA4/PTCK1/AN3| 
	SEG3|PA1/PTP0I	 PA5/AN4/VREF| LD_EN2
		|PA2		PA6/AN5/VREFO| LD_EN1
	SEG4|PA3/PTP1I	 PA7/PTP1/AN6| SEG5 
		 ------------------------

	  ________________________
		 |VSS				   VDD|
		 |PC0		  PB0/INT0/AN0| LD_OUT
	SEG2 |PC1		  PB1/INT1/AN1| V5V_IN
	SEG1 |PC2		 PB2/PTCK0/AN2| CHG_DET
	KEY1 |PA0/PTP0	 PA4/PTCK1/AN3| 
	SEG3 |PA1/PTP0I   PA5/AN4/VREF| 
		 |PA2		 PA6/AN5/VREFO| 
	SEG4 |PA3/PTP1I   PA7/PTP1/AN6| SEG5
		  ------------------------

*/////////////////////////////////////////////////////////////
void main()
{
	_bp=0;
	for(_mp0=0x40;_mp0<=0x9F;_mp0++)
	{
		_iar0=0;
	}
//		7		6		5		4		3		2		1		0
//PA   SEG5	  LD_EN1  LD_EN2    /      SEG4     /      SEG3    KEY
//PB    /		/		/		/		/		/	  V5V_IN LD2_HALF
//PC  	/		/		/		/	    /     SEG1	   SEG2   CHG_DET
	_rstc	= 0B01010101;		//复位选择寄存器
	_pasr	= 0B00000000;		//<7:6>PA7;<5:4>PA6;<3:2>PA5;<1>PA4;<0>PA0
	_pbsr	= 0B00000000;		//<5>PB5;<4>PB4;<3>PB3;<2>PB2;<1>PB1;<0>PB0
	_scomc	= 0B00000000;		//<6:5>偏压电阻;<4>SCOMEN;<3>COM3EN;<2>COM2EN;<1>COM1EN;<0>COM0EN
	_pa		= 0B10001011;		//电平		1:高;   0:低
	_pac	= 0B00000001;		//输入输出	1:输入; 0:输出
	_pawu	= 0B00000001;		//唤醒		1:使能; 0:禁止
	_papu	= 0B00000001;		//上拉		1:使能; 0:禁止
	_pa		= 0B10001011;		//电平		1:高;   0:低
	_pb		= 0B00000000;		//电平		1:高;   0:低
	_pbc	= 0B00000011;		//输入输出	1:输入; 0:输出
	_pbpu	= 0B00000000;		//上拉		1:使能; 0:禁止
	_pb		= 0B00000000;		//电平		1:高;   0:低
	_pc		= 0B00000110;		//电平		1:高;   0:低
	_pcc	= 0B00000000;		//输入输出	1:输入; 0:输出
	_pcpu	= 0B00000000;		//上拉		1:使能; 0:禁止
	_pc		= 0B00000110;		//电平		1:高;   0:低
	_sadc0	= 0B00000000;    	//<7>START; <6>ADBZ; <5>ENADC; <4>ADRFS; <2:0>CH
	_sadc1	= 0B00000000;		//<7-5>输入信号选择; <2-0>时钟源
	_sadc2	= 0B00000000;		//<7>ENOPA; <6>VBGEN; <3-0>参考电压
	_smod	= 0B00000001;		//<7-5>分频选择; <3>LTO; <2>HTO; <1>IDLEN; <0>HLCLK
	_smod1	= 0B00000000;		//<7>FSYSON; <3>RSTF; <2>LVRF; <0>WRF
	_ptm0c0	= 0B00000000;		//<7>PT0PAU; <6-4>PT0CK; <3>PT0ON;
	_ptm0c1	= 0B00000000;		//<7:6>工作模式; <5:4>输出功能; <3>PT0OC; <2>输出极性; <1>捕捉触发源; <0>P/A匹配
	_ptm1c0	= 0B00000000;		//<7>PT1PAU; <6-4>PT1CK; <3>PT1ON;
	_ptm1c1	= 0B00000000;		//<7:6>工作模式; <5:4>输出功能; <3>PT1OC; <2>输出极性; <1>捕捉触发源; <0>P/A匹配
	_wdtc	= 0B01010111;		//<7-3>WE; <2-0>WS
	_clrwdt();	
	_mfi0	= 0B00000000;		//<3>PTMA1E;<2>PTMP1E;<1>PTMA0E;<0>PTMP0E
	_integ	= 0B00000000;		//<3:2>INT1S;<1:0>INT0S
	_intc1	= 0B00000000;		//<3>INT1E;<2>ADE;<1>DEE;<0>MF0E	
	_tbc	= 0B11000011;		//<7>TBON; <6>TBCK=fsys/4; <5:4>TB11-TB10; <2-0>TB02-TB00(fTB/2048 1ms)
	_intc0	= 0B00000101;		//<3>TB1E;<2>TB0E;<1>INT0E;<0>EMI
	/////////////////////////////////////////////////////////////
	u8_loader_mode = ld_off;
	u8_sw_pwm_tel = 0;
	u16_tm_idle_timeout = IDLE_CNT;
	u16_adc_vdd_max = 0;
	u16_adc_vdd_min = 0xFFFF;
	u16_adc_vdd_sum = 0;
	u8_adc_vdd_cnt = 0;
	u8_bat_level_new = led_level_3;
	u8_bat_level = led_level_1;

	u16_bat_level_up_target = 10;
	u16_bat_level_down_target = 10;
	u8_tm_selftest = 0;
	u8_selftest_flag = 0;
	
	while(1) 
	{
		_clrwdt();				//大概1秒多溢出一次
		if(u8_tm_1ms_flag == 1)
		{
			u8_tm_1ms_flag = 0;
			u16_key_cnt1++;
			
			if(u16_tm_3s_timeout>0)
				u16_tm_3s_timeout--;
			
			if(u16_tm_idle_timeout>0)
				u16_tm_idle_timeout--;
			
			u8_tm_10ms++;
			if(u8_tm_10ms > 19) 		//20ms
			{
				u8_tm_10ms = 0;
				u8_v5v_on_cnt++;
				u8_v5v_off_cnt++;
				
				u8_chg_full_cnt++;
				u8_chg_notfull_cnt++;
				u16_bat_level_up_cnt++;
				if(KEY1 == KEY_UP)
				{
					u16_bat_level_down_cnt++;
				}
				
				u8_tm_selftest++;
				if(u8_tm_selftest > 120)
				{
					u8_tm_selftest = 130;
					if(u8_selftest_flag == 0)
						u8_selftest_flag = 2;
				}
			}
		}
		if(u8_selftest_flag == 0)				//开机初始化过程
		{
			u16_bat_level_up_target = 1;
			u16_bat_level_down_target = 1;
		}
		else if(u8_selftest_flag == 1)			//测试模式
		{
			u16_bat_level_up_target = 20;
			u16_bat_level_down_target = 20;
		}
		else									//正常模式
		{
			u16_bat_level_up_target =   30000;	//30000:600秒
			u16_bat_level_down_target = 6000;
		}
		/////////////////////////////////////////////////////////////
		_sadc0 = 0B01110000;
		_sadc1 = 0B01100010;
		_sadc2 = 0B11001010;
		_start = 0;
		_start = 1;
		_start = 0;
		while(_adbz);
		u16_adc_vdd =((uint16)_sadoh<<8)+(_sadol);
		if(u16_adc_vdd >= u16_adc_vdd_max)
			u16_adc_vdd_max = u16_adc_vdd;
		if(u16_adc_vdd <= u16_adc_vdd_min)
			u16_adc_vdd_min = u16_adc_vdd;
		u16_adc_vdd_sum += u16_adc_vdd;
		u8_adc_vdd_cnt++;
		if(u8_adc_vdd_cnt >=10)
		{
			u16_adc_vdd_sum -= u16_adc_vdd_max;
			u16_adc_vdd_sum -= u16_adc_vdd_min;
			u16_adc_vdd_avg = u16_adc_vdd_sum/8;
			u16_adc_vdd_max = 0;
			u16_adc_vdd_min = 0xFFFF;
			u16_adc_vdd_sum = 0;
			u8_adc_vdd_cnt = 0;
		}
		/////////////////////////////////////////////////////////////
		//通过5V输入IO口判断是否有5V充电输入
		if(V5V_IN == 0)
		{
			u8_v5v_on_cnt = 0;
		}
		else
		{
			u8_v5v_off_cnt = 0;
		}

		if(u8_v5v_on_cnt > 10)		//200ms
		{
			if(u8_v5v_on_cnt==11)
			{
				CHG_ON
			}	
			u8_v5v_on_cnt = 12;
			b_me4056_chg_en = me4056_chg_on;
			u8_chg_notfull_cnt = 0;
		}

		if(u8_v5v_off_cnt > 3)		//80ms
		{
			if(u8_v5v_off_cnt==4)
			{
				CHG_OFF
			}
			u8_v5v_off_cnt = 12;
			b_me4056_chg_en = me4056_chg_off;
			u8_chg_full_cnt = 0;
		}
		
		//电池电压等级
		if(b_me4056_chg_en == me4056_chg_on)			//充电中
		{
			if(b_me4056_status == me4056_st_chg_full)
			{
			//	u8_bat_level = led_level_5;
				u8_bat_level_new = led_level_5;
			}
			else
			{
				if(u16_adc_vdd_avg >= BAT_CHG_LEVEL_4)
				{
					u8_bat_level_new = led_level_4;
				}
				else if(u16_adc_vdd_avg >= BAT_CHG_LEVEL_3)
				{
					u8_bat_level_new = led_level_3;
				}
				else if(u16_adc_vdd_avg >= BAT_CHG_LEVEL_2)
				{
					u8_bat_level_new = led_level_2;
				}
				else
				{
					u8_bat_level_new = led_level_1;
				}
			}
		}
		else											//未充电
		{
			if(u16_adc_vdd_avg >= BAT_LEVEL_5)
			{
				u8_bat_level_new = led_level_5;
			}
			else if(u16_adc_vdd_avg >= BAT_LEVEL_4)
			{
				u8_bat_level_new = led_level_4;
			}
			else if(u16_adc_vdd_avg >= BAT_LEVEL_3)
			{
				u8_bat_level_new = led_level_3;
			}
			else if(u16_adc_vdd_avg >= BAT_LEVEL_2)
			{
				u8_bat_level_new = led_level_2;
			}
			else
			{
				u8_bat_level_new = led_level_1;
			}
		}
		
		if(b_me4056_chg_en == me4056_chg_on)			//充电时电压只会上升
		{
			if(u8_selftest_flag == 0 || u8_selftest_flag == 1)
			{
				if(u8_bat_level_new < u8_bat_level) 		//电压下降
				{
					if(u16_bat_level_down_cnt > u16_bat_level_down_target)	//6000:120秒
					{
						u16_bat_level_down_cnt = 0;
						if(u8_bat_level>1)
						{
							u8_bat_level--;
						}
					}
				}
				else if(u8_bat_level_new > u8_bat_level)	//电压上升
				{
					if(u16_bat_level_up_cnt > u16_bat_level_up_target)		//30000:600秒
					{
						u16_bat_level_up_cnt = 0;
						u8_bat_level++;
						if(u8_bat_level > 5)
						{
							u8_bat_level = 5;
						}
					}
				}
				else
				{
					u16_bat_level_down_cnt = 0;
					u16_bat_level_up_cnt = 0;
				}
			}
			else
			{
				if(u8_bat_level_new > u8_bat_level) 		//电压上升
				{
					if(u16_bat_level_up_cnt > u16_bat_level_up_target) 		//30000:600秒
					{
						u16_bat_level_up_cnt = 0;
						u8_bat_level++;
						if(u8_bat_level > 5)
						{
							u8_bat_level = 5;
						}
					}
				}
				else
				{
					u16_bat_level_up_cnt = 0;
				}
			}
		}
		else
		{
			if(u8_bat_level_new < u8_bat_level) 		//电压下降
			{
				if(u16_bat_level_down_cnt > u16_bat_level_down_target) 	//6000:120秒
				{
					u16_bat_level_down_cnt = 0;
					if(u8_bat_level>1)
					{
						u8_bat_level--;
					}
				}
			}
			else if(u8_bat_level_new > u8_bat_level)	//电压上升
			{
				if(u16_bat_level_up_cnt > u16_bat_level_up_target) 		//30000:600秒
				{
					u16_bat_level_up_cnt = 0;
					u8_bat_level++;
					if(u8_bat_level > 5)
					{
						u8_bat_level = 5;
					}
				}
			}
			else
			{
				u16_bat_level_down_cnt = 0;
				u16_bat_level_up_cnt = 0;
			}
		}
		////////////////////////////////////////////////////////////*/
		//充电一直显示电量
		if(b_me4056_chg_en == me4056_chg_on)		//充电时
		{
			//STANDBY脚位判断
			if(CHG_DET == CHG_DET_FULL)
			{
				u8_chg_notfull_cnt = 0;
			}
			else
			{
				u8_chg_full_cnt = 0;
			}
			
			if(u8_chg_full_cnt >= 150)			//3000ms
			{
				u8_chg_full_cnt = 150;
				b_me4056_status = me4056_st_chg_full;	//充电信号延时后转灯
			}
			
			if(u8_chg_notfull_cnt >= 150)
			{
				u8_chg_notfull_cnt = 150;
				b_me4056_status = me4056_st_charging;
			}
			
			if(b_me4056_status == me4056_st_chg_full)
			{
				if(u8_bat_level == led_level_4)
				{
					u8_led_level_temp = led_level_5;
					u8_bat_level = led_level_5;
				}
				else if(u8_bat_level == led_level_5)
				{
					u8_led_level_temp = led_level_5;
					u8_bat_level = led_level_5;
				}
			}
			else
			{
			//	if(u8_bat_level == led_level_5)
			//		u8_led_level_temp = led_level_4;
			//	else
					u8_led_level_temp = u8_bat_level;
			}
			
			u16_tm_3s_timeout = 100;					//充电时一直显示电量
			u8_led_level = u8_led_level_temp;
			u16_tm_idle_timeout = IDLE_CNT;
		}
		else
		{
			u8_chg_full_cnt = 0;
			u8_chg_notfull_cnt = 150;
			b_me4056_status = me4056_st_charging;
		}
		/////////////////////////////////////////////////////////////
		//按键处理
		if(KEY1 == KEY_DOWN)
		{
			u16_tm_idle_timeout = IDLE_CNT;
			
			if(u16_key_cnt1 >= KEY_LONG_PRESS_DOWN)
			{
				u16_key_cnt1 = KEY_LONG_PRESS_DOWN+1;
				u8_key_value = 0x02;
				u8_key_value_old = 0x20;
			}
			else if((u16_key_cnt1 >= KEY_SHORT_PRESS_DOWN) && (u16_key_cnt1 < KEY_SHORT_PRESS_UP))
			{
				u16_key_cnt1 = KEY_SHORT_PRESS_UP+1;
				u8_key_value = 0x01;
				u8_key_value_old = 0x10;
			}
		}
		else
		{
			u16_key_cnt1 = 0;
			if(u8_key_value_old == 0x10)
			{
				u8_key_value = 0x11;
			}
		}
		
		if(u8_key_value_old != u8_key_value)
		{
			if(u8_key_value_old == 0x20)			//key1长按按住
			{
				u8_led_level = u8_bat_level;
				u16_tm_3s_timeout = 3000;
				u16_tm_idle_timeout = IDLE_CNT;

				u8_key_value_old = 0;
				u8_key_value = 0;
			}
			else if(u8_key_value_old == 0x10)
			{
				if(u8_key_value == 0x11)			//key1短按弹起
				{
					//按键触发一次
					if(u8_loader_mode == ld_top)
					{
						u8_loader_mode = ld_half;
					}
					else if(u8_loader_mode == ld_half)
					{
						u8_loader_mode = ld_full;
					}
					else if(u8_loader_mode == ld_full)
					{
						u8_loader_mode = ld_off;
					}
					else
					{
						u8_loader_mode = ld_top;
					}
					
					u8_key_value_old = 0;
					u8_key_value = 0;
				}
				else if(u8_key_value == 0x01)
				{
					if(u8_tm_selftest<3)
					{
						if(u8_bat_level_new == led_level_5)
						{
							u8_selftest_flag = 1;
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////*/
		//负载输出控制
		if(u8_loader_mode == ld_top)
		{
			LD1_TOP
			u16_tm_idle_timeout = IDLE_CNT;
		}
		else if(u8_loader_mode == ld_half)
		{
			LD2_HALF
			u16_tm_idle_timeout = IDLE_CNT;
		}
		else if(u8_loader_mode == ld_full)
		{
			LD2_FULL
			u16_tm_idle_timeout = IDLE_CNT;
		}
		else if(u8_loader_mode == ld_off)
		{
			LD12_OFF
		}

		////////////////////////////////////////////////////////////*/
		//休眠唤醒
		
		//		7		6		5		4		3		2		1		0
		//PA   SEG5   LD_EN1  LD_EN2	/	   SEG4 	/	   SEG3    KEY
		//PB	/		/		/		/		/		/	  V5V_IN LD2_HALF
		//PC	/		/		/		/		/	  SEG1	   SEG2   CHG_DET
		if(u16_tm_idle_timeout == 0)
		{
			//*
			//准备休眠
			u16_tm_3s_timeout = 0;
			u8_led_level = led_level_0;
			_integ	= 0B00001100;		//<3:2>INT1S;<1:0>INT0S	
			_intc0	= 0B00000000;		//<3>TB1E;<2>TB0E;<1>INT0E;<0>EMI
			_intc1	= 0B00000000;		//<3>INT1E;<2>ADE;<1>DEE;<0>MF0E	
			_tbc	= 0B00000000;		//<7>TBON; <6>TBCK=fsys/4; <5:4>TB11-TB10; <2-0>TB02-TB00(fTB/2048 1ms)
			_mfi0	= 0B00000000;		//<3>PTMA1E;<2>PTMP1E;<1>PTMA0E;<0>PTMP0E
			_integ	= 0B00001100;		//<3:2>INT1S;<1:0>INT0S
			_pa 	= 0B10001011;		//电平		1:高;	0:低
			_pac	= 0B00000001;		//输入输出	1:输入; 0:输出
			_pawu	= 0B00000001;		//唤醒		1:使能; 0:禁止
			_papu	= 0B00000001;		//上拉		1:使能; 0:禁止
			_pa 	= 0B10001011;		//电平		1:高;	0:低	
			_pbsr	= 0b00000000;
			_pb 	= 0B00000000;		//电平		1:高;	0:低
			_pbc	= 0B00000011;		//输入输出	1:输入; 0:输出
			_pbpu	= 0B00000000;		//上拉		1:使能; 0:禁止
			_pb 	= 0B00000000;		//电平		1:高;	0:低
			_pc 	= 0B00000110;		//电平		1:高;	0:低
			_pcc	= 0B00000000;		//输入输出	1:输入; 0:输出
			_pcpu	= 0B00000000;		//上拉		1:使能; 0:禁止
			_pc 	= 0B00000110;		//电平		1:高;	0:低			
			_sadc0	= 0B00000000;
			_sadc1	= 0B00000000;
			_sadc2	= 0B00000000;
			_clrwdt();
			_wdtc	= 0B10101111;		//<7-3>WE; <2-0>WS
			_nop();
			_nop();
			_nop();
			_halt();
			_nop();
			_nop();
			_nop();
			_wdtc	= 0B01010111;
			_clrwdt();
			_tbc	= 0B11000011;		//<7>TBON; <6>TBCK=fsys/4; <5:4>TB11-TB10; <2-0>TB02-TB00(fTB/2048 1ms)
			_intc0	= 0B00000101;		//<3>TB1E;<2>TB0E;<1>INT0E;<0>EMI
			u16_tm_idle_timeout = IDLE_CNT;
			//*/
		}
		////////////////////////////////////////////////////////////*/
		//显示电量
		if(u16_tm_3s_timeout == 0)
			u8_led_level = led_level_0;

	//	u8_led_level = u8_bat_level;
		if(u8_selftest_flag == 1)
		{
			u16_tm_3s_timeout = 3000;
			u16_tm_idle_timeout = IDLE_CNT;
			if(u8_tm_selftest<60)
			{
				LCD_SEG5 = LCD_ON;
				LCD_SEG4 = LCD_OFF;
				LCD_SEG3 = LCD_ON;
				LCD_SEG2 = LCD_OFF;
				LCD_SEG1 = LCD_ON;
			}
			else if(u8_tm_selftest<120)
			{
				LCD_SEG5 = LCD_OFF;
				LCD_SEG4 = LCD_ON;
				LCD_SEG3 = LCD_OFF;
				LCD_SEG2 = LCD_ON;
				LCD_SEG1 = LCD_OFF;
			}
			else
			{
				u8_led_level = u8_bat_level;
				if(u8_led_level == led_level_5)
				{
					LCD_SEG5 = LCD_ON;
					LCD_SEG4 = LCD_ON;
					LCD_SEG3 = LCD_ON;
					LCD_SEG2 = LCD_ON;
					LCD_SEG1 = LCD_ON;
				}
				else if(u8_led_level == led_level_4)
				{
					LCD_SEG5 = LCD_OFF;
					LCD_SEG4 = LCD_ON;
					LCD_SEG3 = LCD_ON;
					LCD_SEG2 = LCD_ON;
					LCD_SEG1 = LCD_ON;
				}
				else if(u8_led_level == led_level_3)
				{
					LCD_SEG5 = LCD_OFF;
					LCD_SEG4 = LCD_OFF;
					LCD_SEG3 = LCD_ON;
					LCD_SEG2 = LCD_ON;
					LCD_SEG1 = LCD_ON;
				}
				else if(u8_led_level == led_level_2)
				{
					LCD_SEG5 = LCD_OFF;
					LCD_SEG4 = LCD_OFF;
					LCD_SEG3 = LCD_OFF;
					LCD_SEG2 = LCD_ON;
					LCD_SEG1 = LCD_ON;
				}
				else if(u8_led_level == led_level_1)
				{
					LCD_SEG5 = LCD_OFF;
					LCD_SEG4 = LCD_OFF;
					LCD_SEG3 = LCD_OFF;
					LCD_SEG2 = LCD_OFF;
					LCD_SEG1 = LCD_ON;
				}
				else if(u8_led_level == led_level_0)
				{
					LCD_SEG5 = LCD_OFF;
					LCD_SEG4 = LCD_OFF;
					LCD_SEG3 = LCD_OFF;
					LCD_SEG2 = LCD_OFF;
					LCD_SEG1 = LCD_OFF;
				}
			}
		}
		else
		{
			if(u8_led_level == led_level_5)
			{
				LCD_SEG5 = LCD_ON;
				LCD_SEG4 = LCD_ON;
				LCD_SEG3 = LCD_ON;
				LCD_SEG2 = LCD_ON;
				LCD_SEG1 = LCD_ON;
			}
			else if(u8_led_level == led_level_4)
			{
				LCD_SEG5 = LCD_OFF;
				LCD_SEG4 = LCD_ON;
				LCD_SEG3 = LCD_ON;
				LCD_SEG2 = LCD_ON;
				LCD_SEG1 = LCD_ON;
			}
			else if(u8_led_level == led_level_3)
			{
				LCD_SEG5 = LCD_OFF;
				LCD_SEG4 = LCD_OFF;
				LCD_SEG3 = LCD_ON;
				LCD_SEG2 = LCD_ON;
				LCD_SEG1 = LCD_ON;
			}
			else if(u8_led_level == led_level_2)
			{
				LCD_SEG5 = LCD_OFF;
				LCD_SEG4 = LCD_OFF;
				LCD_SEG3 = LCD_OFF;
				LCD_SEG2 = LCD_ON;
				LCD_SEG1 = LCD_ON;
			}
			else if(u8_led_level == led_level_1)
			{
				LCD_SEG5 = LCD_OFF;
				LCD_SEG4 = LCD_OFF;
				LCD_SEG3 = LCD_OFF;
				LCD_SEG2 = LCD_OFF;
				LCD_SEG1 = LCD_ON;
			}
			else if(u8_led_level == led_level_0)
			{
				LCD_SEG5 = LCD_OFF;
				LCD_SEG4 = LCD_OFF;
				LCD_SEG3 = LCD_OFF;
				LCD_SEG2 = LCD_OFF;
				LCD_SEG1 = LCD_OFF;
			}
		}
		////////////////////////////////////////////////////////////*/
	}
}



