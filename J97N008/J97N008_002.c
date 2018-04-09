/*
	项目名称:ALS拉杆灯充电板
	编辑日期:2018.3.30
	作者	:JasonJia
	PCB负责 :龚为
	芯片封装:HT66F002 SOP_8
	软件时钟:1ms
	功能描述:
	修改记录:
/////////////HT66F002////////////
			-------
		   |VDD VSS|
 KEY/COM15 |PA6 PA0| OUT12
	   VIN |PA5 PA1| OUT3 
	COM234 |PA7 PA2| OUT45
			-------
*/
#include "HT66F002.h"
#define bool	bit
#define uint8 	unsigned char 
#define uint16	unsigned int 

#define SP_FLAG	0

#define COM15   _pa6
#define COM234  _pa7
#define OUT12   _pa0
#define OUT3    _pa1
#define OUT45   _pa2

#define VIN     _pa5
#define KEY     _pa6

#define		BAT_LEVEL_5			3600	//8.1以上
#define		BAT_LEVEL_4			3466	//7.8
#define		BAT_LEVEL_3			3333	//7.5
#define		BAT_LEVEL_2			3200	//7.2

#pragma	vector int0_int @ 0x04
#pragma vector tb0_int	@ 0x08 		//tb0中断
#pragma vector tb1_int	@ 0x0C
#pragma vector mf0_int	@ 0x10
#pragma vector eep_int	@ 0x14
#pragma vector adc_int	@ 0x18
void int0_int(void)	{_inte=0;_intf=0;}
void tb1_int(void)	{_tb1e=0;_tb1f=0;}
void mf0_int(void)	{_mf0e=0;_mf0f=0;}
void eep_int(void)	{_dee=0;_def=0;}
void adc_int(void)	{_ade=0;_adf=0;}
#pragma rambank0

//Halt
bool	b_halt_flag;
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

bool 	b_key_flag;
//ADC
bool	b_adc_cal_flag;
bool	b_adc_mode;
bool	b_adc_bat_final_flag;
uint8	u8_adc_bat_cnt1;
uint8	u8_adc_bat_cnt2;
uint16	u16_adc_bat_vdd;
uint16	u16_adc_bat_sum1;
uint16	u16_adc_bat_sum2;
uint16	u16_adc_bat_max;
uint16	u16_adc_bat_min;
uint16	u16_adc_bat_final;
	//Level_
uint8	b_tm_level_500ms_flag;
uint8	u8_tm_level_100ms_cnt;
uint8	u8_bat_level;

//LED
uint8	u8_led_var[5];
uint8	u8_led_cnt;
uint8	u8_tm_led_100ms;


	
#pragma norambank

void tb0_int()
{
	b_tm_1ms_flag=1;
	if(b_tm_level_500ms_flag==1&&u8_bat_level>0)
	{
		_pa=0b11000000;
		_pa=u8_led_var[u8_led_cnt];
		u8_led_cnt++;
		if(u8_led_cnt==u8_bat_level)
			u8_led_cnt=0;			
	}
	else{
		_pa=0b11000000;
	}

}
void main()
{
	_bp=0;
	for(_mp0=0x40;_mp0<0x7F;_mp0++)
	{
		_iar0=0;
	}
	_rstc	=0B01010101;		//复位选择寄存器
	_smod	=0b00000001; 		// 系统时钟选择  FSYS=FH
	_smod1	=0b00000000;	
	_intc1	=0b00000000;		//中断设置
	_integ	=0b00000000;	
	_mfi0	=0b00000000;		//多功能中断设置
    _wdtc	=0b01010111; 
    _clrwdt();
    
	b_halt_flag=1;
	u8_led_var[0]=0x81;
	u8_led_var[1]=0x41;
	u8_led_var[2]=0x02;
	u8_led_var[3]=0x44;
	u8_led_var[4]=0x84;
	while(1)
	{
		if(b_halt_flag==1)
		{
			_clrwdt();
			_intc0	=0b00000000;	//中断设置
			_tbc	=0B00000000;	//时基设置
			_sadc0	=0B00000000; 
			_sadc1	=0B00000000;
			_sadc2	=0B00000000;
			_pa 	=0b01000000;	//
			_pac	=0b01100000;	//输入输出设置	0输出 1输入
			_papu	=0b01000000;	//上拉电阻		0除能 1使能 
			_pawu	=0b01000000;	//唤醒功能位	0除能 1使能 
			_pasr	=0b11000000;	//功能选择	 
			_pa 	=0b01000000;	
			_wdtc	=0b10101111;	
			_nop();
			_nop();
			_halt();
			_nop();
			_nop();
	        _intc0	=0b00000101; 	//中断设置
	        _tbc	=0B11000011;  	//时基设置  tb0  976hz   1ms  
			_wdtc	=0b01010111;	
			b_halt_flag=0;
			//TM
			b_tm_1ms_flag=0;
			b_tm_10ms_flag=0;
			b_tm_100ms_flag=0;
			b_tm_1s_flag=0;
			u8_tm_1ms_cnt=0;
			u8_tm_10ms_cnt=0;
			u8_tm_100ms_cnt=0;
			//KEY
			b_key1=1;
			b_key_trg1=0;
			b_key_date1=0;
			b_key_cont1=0;	
			u8_tm_key1_1ms=0;
			b_key_flag=0;
			//ADC
			b_adc_bat_final_flag=0;
			u8_adc_bat_cnt1=0;
			u16_adc_bat_vdd=0;
			u16_adc_bat_sum1=0;
			u16_adc_bat_max=0;
			u16_adc_bat_min=0xFFFF;
			u16_adc_bat_final=0;
			//Level_
			b_tm_level_500ms_flag=0;
			u8_tm_level_100ms_cnt=0;
			u8_bat_level=0;
			//LED
			u8_led_cnt=0;
			u8_tm_led_100ms=0;
		}
		_clrwdt();
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
		if(b_tm_1ms_flag==1)
		{
			u8_tm_1ms_cnt++;
		}
		if(u8_tm_1ms_cnt>=10)
		{
			u8_tm_1ms_cnt=0;
			b_tm_10ms_flag=1;
			u8_tm_10ms_cnt++;
		}
		if(u8_tm_10ms_cnt>=10)
		{
			u8_tm_10ms_cnt=0;
			b_tm_100ms_flag=1;
			u8_tm_100ms_cnt++;
		}
		if(u8_tm_100ms_cnt>=10)
		{
			u8_tm_100ms_cnt=0;
			b_tm_1s_flag=1;
		}
/////////////////////////////////////////////////////////
		if(b_tm_1ms_flag==1)
		{
			b_tm_1ms_flag=0;
			u8_tm_key1_1ms++;
		}
		if(b_tm_10ms_flag==1)
		{
			b_tm_10ms_flag=0;	
		}
		if(b_tm_100ms_flag==1)
		{
			b_tm_100ms_flag=0;
			u8_tm_level_100ms_cnt++;
			if(u8_tm_level_100ms_cnt>5)
			{
				b_tm_level_500ms_flag=1;
			}
			u8_tm_led_100ms++;
		}
		if(b_tm_1s_flag==1)
		{
			b_tm_1s_flag=0;
		}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
		if(b_key_flag==0)
		{
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
			if(b_key_cont1==1)
			{
				b_key_flag=1;
				_pa		=0b11000000;	
				_pac	=0b00100000;	
				_papu	=0b00000000;
				_pawu	=0b00000000;	 
				_pasr	=0b11000000;		//将PA5设置成AN3输入状态			   
				_pa		=0b11000000;	
			}
		}
/////////////////////////////////////////////////////
		_sadc1	=	0b00000100; 	//[2:0]Fad=Fsys/16
		_sadc0	=	0b00110011; 	//[5]Open ENADC 	[4]SADOH=[11:8] SADOL=[7:0] 	[1:0]AN3
		_sadc2	=	0b00000000; 	//[7]ENOPA	[6]VBGEN	[3:0]Vf=VDD 
		_start	=0;
		_start	=1;
		_start	=0;
		while(_adbz)
		{
			_clrwdt();
		}
		u16_adc_bat_vdd=((uint16)_sadoh<<8)+(_sadol);
		u16_adc_bat_sum1+=u16_adc_bat_vdd;
		if(u16_adc_bat_vdd>u16_adc_bat_max)
		{
			u16_adc_bat_max=u16_adc_bat_vdd;
		}
		if(u16_adc_bat_vdd<u16_adc_bat_min)
		{
			u16_adc_bat_min=u16_adc_bat_vdd;
		}
		u8_adc_bat_cnt1++;
		if(u8_adc_bat_cnt1>=10) 		
		{
			u16_adc_bat_sum1	-=	u16_adc_bat_max;
			u16_adc_bat_sum1	-=	u16_adc_bat_min;
			u16_adc_bat_sum1	>>= 3;
			u16_adc_bat_final	=	u16_adc_bat_sum1;
			b_adc_bat_final_flag=	1;
			u8_adc_bat_cnt1 	=	0;
			u16_adc_bat_sum1	=	0;
			u16_adc_bat_max 	=	0;
			u16_adc_bat_min 	=	0xFFFF;
			u16_adc_bat_vdd 	=	0;
		}	
		if(b_tm_level_500ms_flag==0&&b_key_flag==1)
		{
			if(u16_adc_bat_final >= BAT_LEVEL_5)
			{
				u8_bat_level = 5;
			}
			else if(u16_adc_bat_final >= BAT_LEVEL_4)
			{
				u8_bat_level = 4;
			}
			else if(u16_adc_bat_final >= BAT_LEVEL_3)
			{
				u8_bat_level = 3;
			}
			else if(u16_adc_bat_final >= BAT_LEVEL_2)
			{
				u8_bat_level = 2;
			}
			else
			{
				u8_bat_level = 1;
			}
		}
		if(u8_tm_led_100ms>36)
		{
			b_halt_flag=1;
		}
	}
}
