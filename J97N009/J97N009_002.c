/*
	项目名称:人体冷却灯
	编辑日期:2018.3.30
	作者	:JasonJia
	PCB负责 :龚为
	芯片封装:HT66F002 SOP_8
	软件时钟:128us
	功能描述:
		弱(灯长亮)-强(灯长亮)-断续(强1分钟-关半分钟 循环 灯长亮)-关(灯灭)
	修改记录:
/////////////HT66F002////////////
			 脚位定义
			  -------
			 |VDD VSS|
		 KEY |PA6 PA0| 
  HALForFULL |PA5 PA1| SN
 		 LED |PA7 PA2| SW
			  -------
*/

#include "HT66F002.h"
#define bool	bit
#define uint8 	unsigned char 
#define uint16	unsigned int 

#define SP_FLAG	0

#define KEY					_pa6
#define FAN_OFF				{_pa1=0;_pa2=0;_pac5=0;_pa5=0;_pa7=1;}
#define FAN_HALF 			{_pa1=1;_pa2=1;_pac5=1;_pa7=0;}
#define FAN_FULL		 	{_pa1=1;_pa2=1;_pac5=0;_pa5=0;_pa7=0;}
#define FAN_INTERMITTENT 	{_pa1=0;_pa2=0;_pac5=0;_pa5=0;_pa7=0;}

#pragma	vector int0_int @ 0x04
#pragma vector tb0_int	@ 0x08 		//tb0中断
#pragma vector tb1_int	@ 0x0C
#pragma vector mf0_int	@ 0x10
#pragma vector eep_int	@ 0x14
#pragma vector adc_int	@ 0x18
void int0_int(void)	{_inte=0;}
void tb1_int(void)	{_tb1e=0;}
void mf0_int(void)	{_mf0e=0;}
void eep_int(void)	{_dee=0;}
void adc_int(void)	{_ade=0;}
#pragma rambank0

//Halt
bool	b_halt_flag;
uint16	u16_tm_halt_1ms;
//TM
bool	b_tm_1ms_flag;
bool	b_tm_10ms_flag;
bool	b_tm_100ms_flag;
bool	b_tm_1s_flag;
uint8	u8_tm_1ms_cnt;
uint8	u8_tm_10ms_cnt;
uint8	u8_tm_100ms_cnt;
//KEY
bool 	b_key1;		
bool 	b_key_trg1;
bool 	b_key_date1;
bool 	b_key_cont1;	
uint8 	u8_tm_key1_1ms;
//FAN
uint8	u8_fan_mode;
uint8	u8_fan_mode1;
bool	b_shutdown_flag;
uint16	u16_tm_shutdown_1ms;
bool	b_intermittent_flag;
uint16	u16_tm_fan_intermittent_10ms;



#pragma norambank

enum fan_mode_t
{
	fan_off_t=0,
	fan_half_t,
	fan_full_t,
	fan_intermittent_t,
};
void tb0_int()
{
	b_tm_1ms_flag=1;
}
void main()
{
	_bp=0;
	for(_mp0=0x40;_mp0<=0x7F;_mp0++)
	{
		_iar0=0;
	}
	_rstc	= 0B01010101;		//复位选择寄存器
	_pasr	= 0B00000000;		//PA5,PA2,PA1,PA0功能选择 IO
	_ifs0	= 0B00000000;		//<5>STCK0PS; <4>STP0IPS; <1:0>INT脚位
	_pa		= 0B11000000;		//电平		1:高;   0:低
	_pac	= 0B01000000;		//输入输出	1:输入; 0:输出
	_pawu	= 0B01000000;		//唤醒		1:使能; 0:禁止
	_papu	= 0B01000000;		//上拉		1:使能; 0:禁止
	_pa		= 0B11000000;		//电平		1:高;   0:低
	_sadc0	= 0B00000000;    	//<7>START; <6>ADBZ; <5>ENADC; <4>ADRFS; <1:0>CH
	_sadc1	= 0B00000000;		//<7-5>输入信号选择; <2-0>时钟源
	_sadc2	= 0B00000000;		//<7>ENOPA; <6>VBGEN; <3-0>参考电压
	_stm0c0	= 0B00000000;		//<7>ST0PAU; <6-4>fSYS; <3>ST0ON; <2-0>CCRP STM[9-7]比较匹配周期256
	_stm0c1	= 0B00000000;		//<7:6>工作模式; <5:4>输出功能; <3>输出控制位; <2>输出极性; <1>周期/占空比; <0>P/A匹配
	_smod	= 0B00000001;		//<7-5>分频选择; <3>LTO; <2>HTO; <1>IDLEN; <0>HLCLK
	_smod1	= 0B00000000;		//<7>FSYSON; <3>RSTF; <2>LVRF; <0>WRF
	_wdtc	= 0B01010111;		//<7-3>WE; <2-0>WS
	_clrwdt();
	_mfi0	= 0B00000000;		//<1>STMA0E; <0>STMP0E
	_integ	= 0B00000000;		//<1:0>外部中断边沿选择位
	_intc1	= 0B00000000;		//<2>ADE; <1>DEE; <0>MF0E
	_tbc	= 0B11000011;		//<7>TBON; <6>TBCK=fsys/4; <5:4>TB1=4096/fTB; <2-0>TB0=2048/fTB 
	_intc0	= 0B00000101;		//<3>TB1E; <2>TB0E; <1>INTE; <0>EMI
	b_halt_flag=0;
	u16_tm_halt_1ms=0;
//TM
	b_tm_1ms_flag=0;
	b_tm_10ms_flag=0;
	b_tm_100ms_flag=0;
	b_tm_1s_flag=0;

	u8_tm_1ms_cnt=0;
	u8_tm_10ms_cnt=0;
	u8_tm_100ms_cnt=0;
//KEY
	b_key1=0;		
	b_key_trg1=0;
	b_key_date1=0;
	b_key_cont1=0;	
	u8_tm_key1_1ms=0;
//FAN
	u8_fan_mode=0;
	u8_fan_mode1=0;
	u16_tm_fan_intermittent_10ms=0;
	b_intermittent_flag=0;
	b_shutdown_flag=0;
	u16_tm_shutdown_1ms=0;
	while(1)
	{
		_clrwdt();
		if(b_tm_1ms_flag==1)
		{
			b_tm_1ms_flag=0;
			u8_tm_1ms_cnt++;	
			if(u8_tm_1ms_cnt>=10)
			{
				u8_tm_1ms_cnt=0;
				b_tm_10ms_flag=1;
			}
			u8_tm_key1_1ms++;
			u16_tm_halt_1ms++;
			if((u8_fan_mode!=fan_off_t)&&(b_shutdown_flag==0))
			{
				u16_tm_shutdown_1ms++;
			}
			else
			{
				u16_tm_shutdown_1ms=0;
			}
		}
		if(b_tm_10ms_flag==1)
		{
			b_tm_10ms_flag=0;
			u16_tm_fan_intermittent_10ms++;
		}
		if(KEY!=b_key1)
		{
			if(u8_tm_key1_1ms>15)
			{
				b_key1=KEY;
				u8_tm_key1_1ms=0;
			} 
		}
		else
		{
			u8_tm_key1_1ms=0;
			b_key_date1=b_key1^0x01;
			b_key_trg1=b_key_date1&(b_key_date1^b_key_cont1);
			b_key_cont1=b_key_date1;
		}
		if((b_key_cont1==1)&&(b_key_trg1==1))
		{
			if(b_shutdown_flag==0)
			{
				if(u8_fan_mode1==fan_off_t)
				{
					if(u8_fan_mode==fan_off_t)
					{
						u8_fan_mode=fan_half_t;	
					}
					else if(u8_fan_mode==fan_half_t)
					{
						u8_fan_mode=fan_full_t;	
					}
					else if(u8_fan_mode==fan_full_t)
					{
						u8_fan_mode=fan_intermittent_t;
					}
					else
					{
						u8_fan_mode=fan_off_t;	
					}
				}
				else
				{
					u8_fan_mode=u8_fan_mode1;
					u8_fan_mode1=fan_off_t;
					u16_tm_fan_intermittent_10ms=0;					//记忆恢复后重新计时
				}	
			}
			else
			{
				b_shutdown_flag=0;
				u8_fan_mode1=u8_fan_mode;
				u8_fan_mode=fan_off_t;	
			}
			u16_tm_shutdown_1ms=0;
		}
		if(u16_tm_shutdown_1ms>5000)
		{
			u16_tm_shutdown_1ms=0;
			b_shutdown_flag=1;
		}
		if(u8_fan_mode==fan_intermittent_t)
		{
			if(u16_tm_fan_intermittent_10ms<6000)
			{
				b_intermittent_flag=1;	
			}
			else if(u16_tm_fan_intermittent_10ms<9000)
			{
				b_intermittent_flag=0;	
			}
			else
			{
				u16_tm_fan_intermittent_10ms=0;	
			}
		}
		else
		{
			u16_tm_fan_intermittent_10ms=0;	
		}
		if(u8_fan_mode==fan_off_t)
		{
			FAN_OFF
		}
		else if(u8_fan_mode==fan_half_t)
		{
			 FAN_HALF
		}
		else if(u8_fan_mode==fan_full_t)
		{
			FAN_FULL
		}
		else if(u8_fan_mode==fan_intermittent_t)
		{
			if(b_intermittent_flag==1)
			{
				FAN_FULL
			}
			else 
			{
				FAN_INTERMITTENT
			}
		}
		if(u8_fan_mode!=fan_off_t)
		{
			u16_tm_halt_1ms=0;
		}
		if(u16_tm_halt_1ms>5000)
		{
			u16_tm_halt_1ms=0;
			b_halt_flag=1;
		}
		if(b_halt_flag==1)
		{
			_intc0	= 0B00000000;		//<3>TB1E; <2>TB0E; <1>INTE; <0>EMI
			_tbc	= 0B00000000;		//<7>TBON; <6>TBCK=fsys/4; <5:4>TB1=4096/fTB; <2-0>TB0=2048/fTB 
			_pasr	= 0B11000000;
			_pa 	= 0B11000000;		//电平		1:高;	0:低
			_pac	= 0B01100000;		//输入输出	1:输入; 0:输出
			_pawu	= 0B01000000;		//唤醒		1:使能; 0:禁止
			_papu	= 0B01000000;		//上拉		1:使能; 0:禁止
			_pa 	= 0B11000000;		//电平		1:高;	0:低
			_clrwdt();
			_wdtc	= 0B10101111;		//<7-3>WE; <2-0>WS
			_nop();
			_nop();
			_halt();
			_nop();
			_nop();
			_wdtc	= 0B01010111;		//<7-3>WE; <2-0>WS
			_clrwdt();
			_tbc	= 0B11000011;		//<7>TBON; <6>TBCK=fsys/4; <5:4>TB1=4096/fTB; <2-0>TB0=2048/fTB 
			_intc0	= 0B00000101;		//<3>TB1E; <2>TB0E; <1>INTE; <0>EMI
			_pasr	= 0B10000000;
 			_pa 	= 0B11000000;		//电平		1:高;	0:低
			_pac	= 0B01000000;		//输入输出	1:输入; 0:输出
			_pawu	= 0B01000000;		//唤醒		1:使能; 0:禁止
			_papu	= 0B01000000;		//上拉		1:使能; 0:禁止
			_pa 	= 0B11000000;		//电平		1:高;	0:低

			b_halt_flag=0;
		}
	}
}

