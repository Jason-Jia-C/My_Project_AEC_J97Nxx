/**
	 项目名称:防爆头灯
	 编辑日期:2018.03.23
	 作者	 :JasonJia
	 PCB负责 :徐珺
	 芯片封装:HT453430 NSOP_16
	 软件时钟:1ms
	 功能描述:
	 修改记录:
 /////////////HT45F3430////////////
		 -------------------------
	Send|PB0/STP1_1 	  PWML/PC4| PWM2
 Receive|PB1			  PWMH/PC5| PWM1 
	 LED|PB2				   VDD| +VCC
		|PB3				   VSS| -VSS
	KEY2|PA5			   AN6/PA6| IREF1 
	KEY1|PA4			   AN7/PA7| 
		|PA3			   AN0/PA0| 
        |PA2			   AN1/PA1| IREF2 
		 -------------------------
*/
#include "HT45F3430.h"
#define bool	bit
#define uint8 	unsigned char 
#define uint16	unsigned int

#define SP_FLAG			0	

#define KEY1			_pa4
#define	KEY2			_pa5
#define IFR_Send		_pb0	
#define IFR_Receive 	_pb1
#define IFR_LED			_pb2
#define IREF1			_pa6	
#define IREF2			_pa1	
#define LED_PWM_OUT1	_pc4	
#define LED_PWM_OUT2	_pc5

#define	ADC_BAT			0
#define	ADC_OCP			1
#define IOUT_1			0x02DA
#define IOUT_2			0x0280
#define IOUT_3			0x04CA
#define VIN_1			0x0700
#define VIN_2			0x077C
#define	VIN_3			0x077C
#define VIN_M			0x0686
#define VIN_LOW			0x061A
#define VIN_REC			0x07CD
/////////////////////////////////////////////////////////////
#pragma		vector		ocp_int		@ 0x04	
#pragma		vector		ovp_int		@ 0x08	
#pragma 	vector		ext0_int	@ 0x0C	
#pragma 	vector		mf_int		@ 0x10 
#pragma 	vector		adc_int		@ 0x18		//ADC
#pragma		vector		eeprom_int	@ 0x1C		//EEPROM
#pragma 	vector		tb0_int		@ 0x20		//Time Base 0
#pragma 	vector		tb1_int		@ 0x24		//Time Base 1
#pragma 	vector		ext1_int	@ 0x28	
/////////////////////////////////////////////////////////////
void	ocp_int			(void){_ocpe=0;}
void	ovp_int			(void){_ovpe=0;}
void	ext0_int		(void){_int0e = 0; _int0f = 0;}
void	mf_int			(void){_mfe = 0; _mff = 0;}
void    adc_int		    (void){_ade = 0;}
void	eeprom_int		(void){_dee = 0;}
void	tb1_int			(void){_tb1e = 0;}
void 	ext1_int		(void){_int1e = 0; _int1f = 0;}
//////////////////////////////////////////////////////////////
#pragma rambank0
//Halt
	bool	b_halt_flag;
	uint8	u8_tm_halt_100ms_cnt;
//TM
	bool	b_tm_1ms_flag;
	bool	b_tm_10ms_flag;
	bool	b_tm_100ms_flag;
	bool	b_tm_1s_flag;

	uint8	u8_tm_1ms_cnt;
	uint8	u8_tm_10ms_cnt;
	uint8	u8_tm_100ms_cnt;
//LED
	bool	b_led_sw_flag;
	bool	b_led_pwm_flag;
	bool	b_tm_led_20ms_flag;
	bool	b_led_shutdown_flag;
	uint8	u8_led_mode;
	uint8	u8_led_mode1;
	uint8   u8_led_step;
	uint8	u8_tm_led_1ms_cnt;	
//PWM
	uint8	u8_pwm_add_cnt;
	uint8	u8_pwm_add_ref;
	uint8	u8_pwm_cut_cnt;
	uint8	u8_pwm_cut_ref;
	uint8	u8_pwm_duty_cycle;
//pre
	bool	b_pwm_pre_flag;
	uint8	u8_pwm_pre_var;
//flash
	bool	b_led_flash_flag;
	bool	b_bat_low_flag;
	bool	b_tm_bat_low_1s_flag;
	bool	b_led_flash_two_flag;	
	uint8	u8_bat_low_cnt;
	uint8	u8_tm_flash_100ms_cnt;
	uint8	u8_bat_rec_cnt;
	uint8	u8_led_flash_cnt;
	uint8	u8_tm_bat_low_1s_cnt;
//IFR	
	bool	b_ifr_sw_flag;
	bool	b_ifr_res_flag;
	bool	b_ifr_con_led_flag;
	bool	b_tm_ifr_1ms_flag;
	uint8	u8_ifr_step;
	uint8	u8_tm_ifr_1ms;
	uint8	u8_tm_ifr_10ms;

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
	bool	b_adc_ocp_final_flag;
	uint8	u8_adc_ocp_cnt1;
	uint8	u8_adc_ocp_cnt2;
	uint8	u8_adc_ocp_cnt3;
	uint16	u16_adc_ocp_vdd;
	uint16	u16_adc_ocp_sum1;
	uint16	u16_adc_ocp_sum2;
	uint16	u16_adc_ocp_sum3;
	uint16	u16_adc_ocp_max;
	uint16	u16_adc_ocp_min;
	uint16	u16_adc_ocp_final;
//KEY
	bool 	b_key1;		
	bool 	b_key_trg1;
	bool 	b_key_date1;
	bool 	b_key_cont1;	
	bool 	b_key1_flag;	
	uint8 	u8_tm_key1_1ms;
	bool 	b_key2;		
	bool 	b_key_trg2;
	bool 	b_key_date2;
	bool 	b_key_cont2;	
	bool 	b_key2_flag;	
	uint8 	u8_tm_key2_1ms;	
	uint8 	u8_tm_key1_100ms_cnt;
#if (SP_FLAG==1)
//SP
	bool 	b_sp_flag;
	uint8	u8_sp_value[11];
	uint8 	u8_tm_sp_10ms;
	uint8 	u8_sp_cnt;
	uint16	u16_sp_buf_value;
#endif
#pragma norambank
void ADC_Cal()
{
	if(b_adc_cal_flag!=b_led_pwm_flag)
	{
		u8_adc_bat_cnt1		=	0;
		u8_adc_bat_cnt2		=	0;
		u16_adc_bat_sum1	=	0;
		u16_adc_bat_sum2	=	0;
		u16_adc_bat_max 	=	0;
		u16_adc_bat_min 	=	0xFFFF;
		u8_adc_ocp_cnt1		=	0;
		u8_adc_ocp_cnt2		=	0;
		u8_adc_ocp_cnt3		=	0;
		u16_adc_ocp_sum1	=	0;
		u16_adc_ocp_sum2	=	0;
		u16_adc_ocp_sum3	=	0;
		u16_adc_ocp_max		=	0;
		u16_adc_ocp_min		=	0xFFFF;
		b_adc_cal_flag=b_led_pwm_flag;
	}
	if(b_adc_cal_flag==1)							//复杂模式
	{
		if(b_adc_mode==ADC_BAT)						//电池电压计算，合计运算160次
		{
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
			if(u8_adc_bat_cnt1>=9)			
			{
				u16_adc_bat_sum1	-=	u16_adc_bat_min;
				u16_adc_bat_sum1	>>=	3;
				u16_adc_bat_sum2	+=	u16_adc_bat_sum1;
				u8_adc_bat_cnt2		++;
				u8_adc_bat_cnt1		=	0;
				u16_adc_bat_sum1	=	0;
				u16_adc_bat_max		=	0;
				u16_adc_bat_min		=	0xFFFF;
			}
			if(u8_adc_bat_cnt2>=8)			
			{
				u16_adc_bat_sum2	>>=	3;
				u16_adc_bat_final	=	u16_adc_bat_sum2;
				b_adc_bat_final_flag=	1;
				u8_adc_bat_cnt2		=	0;
				u16_adc_bat_sum2	=	0;
			}
		}
		else if(b_adc_mode==ADC_OCP)				//放电电流计算							
		{
			u16_adc_ocp_sum1+=u16_adc_ocp_vdd;
			if(u16_adc_ocp_vdd>u16_adc_ocp_max)
			{
				u16_adc_ocp_max=u16_adc_ocp_vdd;
			}
			if(u16_adc_ocp_vdd<u16_adc_ocp_min)
			{
				u16_adc_ocp_min=u16_adc_ocp_vdd;
			}
			u8_adc_ocp_cnt1++;
			if(u8_adc_ocp_cnt1>=5)
			{
				u16_adc_ocp_sum1	-=	u16_adc_ocp_min;
				u16_adc_ocp_sum1	>>=	2;
				u16_adc_ocp_sum2	+=	u16_adc_ocp_sum1;
				u8_adc_ocp_cnt2		++;
				u8_adc_ocp_cnt1		=	0;
				u16_adc_ocp_sum1	=	0;
				u16_adc_ocp_max		=	0;
				u16_adc_ocp_min		=	0xFFFF;
			}
			if(u8_adc_ocp_cnt2>=8)
			{
				u16_adc_ocp_sum2	>>=	3;
				u16_adc_ocp_sum3	+=	u16_adc_ocp_sum2;
				u8_adc_ocp_cnt3		++;
				u8_adc_ocp_cnt2		=	0;
				u16_adc_ocp_sum2	=	0;
			}
			if(u8_adc_ocp_cnt3>=4)
			{
				u16_adc_ocp_sum3	>>=	2;
				u16_adc_ocp_final	=	u16_adc_ocp_sum3;
				b_adc_ocp_final_flag=	1;
				u8_adc_ocp_cnt3		=	0;
				u16_adc_ocp_sum3	=	0;
			}
		}
	}	
	else											//简单模式			
	{
		if(b_adc_mode==ADC_BAT)						//电池电压计算
		{
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
				u16_adc_bat_sum1	>>=	3;
				u16_adc_bat_final	=	u16_adc_bat_sum1;
				b_adc_bat_final_flag=	1;
				u8_adc_bat_cnt1		=	0;
				u16_adc_bat_sum1	=	0;
				u16_adc_bat_max		=	0;
				u16_adc_bat_min		=	0xFFFF;
			}
		}	
		else if(b_adc_mode==ADC_OCP)				//放电电流计算
		{
			u16_adc_ocp_sum1+=u16_adc_ocp_vdd;
			if(u16_adc_ocp_vdd>u16_adc_ocp_max)
			{
				u16_adc_ocp_max=u16_adc_ocp_vdd;
			}
			if(u16_adc_ocp_vdd<u16_adc_ocp_min)
			{
				u16_adc_ocp_min=u16_adc_ocp_vdd;
			}
			u8_adc_ocp_cnt1++;
			if(u8_adc_ocp_cnt1>=10)
			{
				u16_adc_ocp_sum1	-=	u16_adc_ocp_max;
				u16_adc_ocp_sum1	-=	u16_adc_ocp_min;
				u16_adc_ocp_sum1	>>=	3;
				u16_adc_ocp_final	=	u16_adc_ocp_sum1;
				b_adc_ocp_final_flag=	1;
				u8_adc_ocp_cnt1		=	0;
				u16_adc_ocp_sum1	=	0;
				u16_adc_ocp_max		=	0;
				u16_adc_ocp_min		=	0xFFFF;
			}
		}
	}		
}
void adc_bat()
{
	_sadc1	= 0b00110100;    					//<7-4>SAINS3-SAINS0(VDD/4); <2-0>SACKS2-SACKS0(Fsys/16)	
	_sadc0	= 0b00110000;						//<7>START; <6>ADBZ; <5>ENADC; <4>ADRFS <3-0>SACS3-SACS0   
	_sadc2	= 0b10011101;   					//<7>ADPGAEN; <4>PGAIS(VBG); <3-2>SAVRS1-SAVRS0(VBG); <1-0>PGAGS1-PGAGS0(Gain==2)  
	_start	=0;
	_start	=1;
	_start	=0;
	while(_adbz)
	{
		_clrwdt();
	}
	u16_adc_bat_vdd=((uint16)_sadoh<<8)+(_sadol);
}
void adc_ocp()
{
	_sadc1	 = 0b00000100;    					//<7-4>SAINS3-SAINS0(AN?); <2-0>SACKS2-SACKS0(Fsys/16)  
	_sadc0   = 0B00110001;						//<7>START; <6>ADBZ; <5>ENADC; <4>ADRFS; <3:0>SACS3-SACS0 Open AN1
	_sadc2	 = 0b10011101;       				//<7>ADPGAEN; <4>PGAIS(VBG); <3-2>SAVRS1-SAVRS0(VBG); <1-0>PGAGS1-PGAGS0(Gain==2)
	_start   = 0;
	_start   = 1;
	_start   = 0;
	while(_adbz)
	{
		_clrwdt();
	}
	u16_adc_ocp_vdd=((uint16)_sadoh<<8)+(_sadol);
}
void ADC_Con()
{
	if(b_adc_mode==ADC_BAT)
	{
		adc_bat();
	}
	else if(b_adc_mode==ADC_OCP)
	{
		adc_ocp();
	}
	ADC_Cal();
	if(b_led_sw_flag==1)
	{
		b_adc_mode	=	~b_adc_mode;
	}
	else
	{
		b_adc_mode	=	ADC_BAT;
	}
}
void PWM_Drive()
{
	if(u8_led_mode==0)
	{
		_cpr	=	0;
		_cpor	= 	0;
		_pwmd	=	0;
		_pwmp	=	0;
		_pcs0	=	0b00000000;
		_pcc	=	0b00110000;
		_pcpu	=	0b00110000;
	}
	if(u8_led_mode==1)
	{
		_pcs0	=	0b00100000;		//<5>PCS05(0:PC5 1:PWMH); <5>PCS04(0:PC4 1:PWML)	
		_pcc	=	0b00010000;
		_pcpu	=	0b00010000;		
		_cpr	= 	0B00111000;		//<7>DLLLKEN; <6>DLLEN; <5:4>PWMCK(FH/8); <3>PWMON; <2:0>DT	
		_cpor	=	0b10110000;		//<7>PWMACT; <6>CNVTYP; <5>INVH; <4>INVL; <3>OCPHEN; <2>OCPLEN; <1>OVPHEN; <0>OVPLEN;
		_pwmp	=	0b11111111;
		_dll	=	0x00;
	}
	else if(u8_led_mode>1)
	{
		_pcs0	=	0b00010000;		//<5>PCS05(0:PC5 1:PWMH); <5>PCS04(0:PC4 1:PWML)
		_pcc	=	0b00100000;
		_pcpu	=	0b00100000;
		_cpr	= 	0B00111000;		//<7>DLLLKEN; <6>DLLEN; <5:4>PWMCK(FH/8); <3>PWMON; <2:0>DT	
		_cpor	=	0b11110000;		//<7>PWMACT; <6>CNVTYP; <5>INVH; <4>INVL; <3>OCPHEN; <2>OCPLEN; <1>OVPHEN; <0>OVPLEN;
		_pwmp	=	0xFF;
		_dll	=	0x00;
	}
	u8_adc_bat_cnt1		=	0;
	u8_adc_bat_cnt2		=	0;
	u16_adc_bat_sum1	=	0;
	u16_adc_bat_sum2	=	0;
	u16_adc_bat_max 	=	0;
	u16_adc_bat_min 	=	0xFFFF;
	u8_adc_ocp_cnt1		=	0;
	u8_adc_ocp_cnt2		=	0;
	u8_adc_ocp_cnt3		=	0;
	u16_adc_ocp_sum1	=	0;
	u16_adc_ocp_sum2	=	0;
	u16_adc_ocp_sum3	=	0;
	u16_adc_ocp_max		=	0;
	u16_adc_ocp_min		=	0xFFFF;
}
void PWM_Pre()
{
	if(b_led_sw_flag==1)						//如果当前灯打开着，那么下一档直接按比例继承上一档的值
	{
		u8_pwm_pre_var=u8_pwm_duty_cycle;
		if(u8_led_mode==0)
		{
			u8_pwm_pre_var=0;
		}
		b_pwm_pre_flag=1;
	}
	else
	{
		if(b_adc_bat_final_flag==1)
		{
			b_adc_bat_final_flag=0;
			if(u16_adc_bat_final<VIN_M)				//电量过低，直接满载运行
			{
				u8_pwm_pre_var	=	0xFF;
				b_pwm_pre_flag=1;
				b_led_pwm_flag=0;
			}
			else
			{
				b_led_pwm_flag=1;
				if(u8_led_mode==1)
				{
					if(u16_adc_bat_final>VIN_3)
					{
						u16_adc_bat_final-=VIN_3;
						u16_adc_bat_final>>=3;
						if(u16_adc_bat_final<0xFF)
						{
							u8_pwm_pre_var	=	((0xFF-u16_adc_bat_final)&0xFF);
						}	
						else
						{
							u8_pwm_pre_var=60;
						}
						if(u8_pwm_pre_var<60)
						{
							u8_pwm_pre_var=60;
						}
					}
					else
					{
						u8_pwm_pre_var	=	0xFF;	
					}
					b_pwm_pre_flag=1;	
				}
				else if(u8_led_mode>1)
				{
					if(u16_adc_bat_final>VIN_1)
					{
						u16_adc_bat_final-=VIN_1;
						u16_adc_bat_final>>=3;
						if(u16_adc_bat_final<0xFF)
						{
							u8_pwm_pre_var	=	((0xFF-u16_adc_bat_final)&0xFF);
						}	
						else
						{
							u8_pwm_pre_var=60;
						}
						if(u8_pwm_pre_var<60)
						{
							u8_pwm_pre_var=60;
						}
					}
					else
					{
						u8_pwm_pre_var	=	0xFF;	
					}
					b_pwm_pre_flag=1;
				}
				else
				{
					u8_pwm_pre_var	=	0x00;
					b_pwm_pre_flag=1;					
				}
			}
		}
	}
}
void LED_adj()
{
	uint16	u16_iout_ref[2];
	u16_iout_ref[0]=IOUT_3;
	u16_iout_ref[1]=IOUT_1;
	if(b_led_sw_flag==1)
	{
		if(b_tm_led_20ms_flag==1)				//20ms进来一次
		{
			b_tm_led_20ms_flag=0;
			if(u16_adc_ocp_final>u16_iout_ref[u8_led_mode-1])
			{
				if(u16_adc_ocp_final>u16_iout_ref[u8_led_mode-1]+20)
				{
					u8_pwm_cut_ref=0;
					if(u8_pwm_duty_cycle>128)
					{
						u8_pwm_duty_cycle--;
					}
				}
				else
				{
					u8_pwm_cut_ref=25;	
				}
				if(u8_pwm_add_cnt>0)
				{
					u8_pwm_add_cnt--;
				}
				u8_pwm_cut_cnt++;	

			}
			else if(u16_adc_ocp_final<u16_iout_ref[u8_led_mode-1])
			{
				if(u16_adc_ocp_final<u16_iout_ref[u8_led_mode-1]-20)
				{
					u8_pwm_add_ref=0;
					if(u8_pwm_duty_cycle<128)
					{
						u8_pwm_duty_cycle++;
					}
				}	
				else
				{
					u8_pwm_add_ref=25;
				}
				if(u8_pwm_cut_cnt>0)
				{
					u8_pwm_cut_cnt--;	
				}
				u8_pwm_add_cnt++;
			}
			if(u8_pwm_add_cnt>u8_pwm_add_ref)
			{
				u8_pwm_add_cnt=0;
				if(u8_pwm_duty_cycle<255)
				{
					u8_pwm_duty_cycle++;
					_pwmd=u8_pwm_duty_cycle;
					if(u8_pwm_duty_cycle==255)
					{
						b_led_pwm_flag=0;
					}
					else
					{
						b_led_pwm_flag=1;
					}
				}
			}
			if(u8_pwm_cut_cnt>u8_pwm_cut_ref)
			{
				u8_pwm_cut_cnt=0;
				if(u8_pwm_duty_cycle>0)
				{
					u8_pwm_duty_cycle--;
				}
				_pwmd=u8_pwm_duty_cycle;
			}
		}
	}
	else
	{
		u8_pwm_duty_cycle=0;
	}	
}
void LED_Flash()
{
	if(b_led_flash_flag==0)
	{
		if(b_led_sw_flag==1)
		{
			if(u16_adc_bat_final<VIN_LOW)
			{
				if(b_tm_bat_low_1s_flag==1)
				{
					u8_bat_low_cnt++;	
					b_tm_bat_low_1s_flag=0;
				}
			}
			else
			{
				if(u8_bat_low_cnt>0)
				{
					u8_bat_low_cnt--;
				}
			}
			if(u8_bat_low_cnt>60)
			{
				u8_bat_low_cnt=0;
				b_bat_low_flag=1;
				b_led_flash_flag=1;	
			}
		}	
	}
	else
	{
		if(u16_adc_bat_final>VIN_REC)
		{
			if(b_tm_bat_low_1s_flag==1)
			{
				u8_bat_rec_cnt++;
				b_tm_bat_low_1s_flag=0;
			}
			if(u8_bat_rec_cnt>60)
			{
				b_led_flash_flag=0;
				u8_bat_rec_cnt=0;	
			}
		}
		else
		{
			u8_bat_rec_cnt=0;	
		}
	}
	if(b_bat_low_flag==1)
	{
		b_tm_led_20ms_flag=0;	
	}
}
void LED_Con()
{
	if(u8_led_step==1)
	{
		PWM_Pre();
		u8_pwm_duty_cycle=u8_pwm_pre_var;
		if(b_pwm_pre_flag==1)
		{
			b_pwm_pre_flag=0;
			u8_led_step++;
			_pwmd	=	0;
			u8_adc_bat_cnt1		=	0;
			u8_adc_bat_cnt2		=	0;
			u16_adc_bat_sum1	=	0;
			u16_adc_bat_sum2	=	0;
			u16_adc_bat_max 	=	0;
			u16_adc_bat_min 	=	0xFFFF;
			u8_adc_ocp_cnt1		=	0;
			u8_adc_ocp_cnt2		=	0;
			u8_adc_ocp_cnt3		=	0;
			u16_adc_ocp_sum1	=	0;
			u16_adc_ocp_sum2	=	0;
			u16_adc_ocp_sum3	=	0;
			u16_adc_ocp_max		=	0;
			u16_adc_ocp_min		=	0xFFFF;
			u16_adc_bat_final	=	0x00;
			b_adc_bat_final_flag=	0;
		}	
	}
	else if(u8_led_step==2)
	{
		PWM_Drive();
		_pwmd=u8_pwm_duty_cycle;
		if(u8_led_mode>0)
		{
			u8_led_step++;
			b_led_sw_flag=1;
		}	
		else
		{
			u8_led_step+=2;
		}
	}
	else if(u8_led_step==3)
	{
			LED_Flash();
			LED_adj();
	}
	else
	{
		u8_led_step=0;
	}
}
void IFR_SW(bool b_ifr_sw)
{
	if(b_ifr_sw==1)
	{
		_stmdl	=0x00;
		_stmdh	=0x00;
		_stmal	=0xD4;
		_stmah	=0x00;
		_stmc0	=0b00011001;	
		_stmc1	=0b10100011;
		
		IFR_LED=1;	
	}
	else if(b_ifr_sw==0)
	{
		_stmdl	=0x00;
		_stmdh	=0x00;
		_stmal	=0xD4;
		_stmah	=0x00;
		_stmc0	=0b00000000;	
		_stmc1	=0b00000000;	
		IFR_LED=0;		
	}
	
}
void IFR_Con()
{
	if(b_tm_ifr_1ms_flag==1)
	{
		b_tm_ifr_1ms_flag=0;
		if(u8_ifr_step==1)
		{
			if(b_ifr_sw_flag==1)
			{
				if(IFR_Receive==1)
				{
					if(u8_tm_ifr_1ms>20)
					{
						u8_ifr_step=2;
					}
				}
				else
				{
					u8_tm_ifr_1ms=0;
				}
			}
			else
			{
				u8_ifr_step=0;	
			}	
		}
		else if(u8_ifr_step==2)
		{
			if(IFR_Receive==0)
			{
				u8_ifr_step=3;
			}
			u8_tm_ifr_1ms=0;
		}
		else if(u8_ifr_step==3)
		{
			if(IFR_Receive==0)
			{
				if(u8_tm_ifr_1ms>6)
				{
					u8_ifr_step=4;
					u8_tm_ifr_1ms=0;
				}
			}
			else
			{
				if(u8_tm_ifr_1ms>1)
				{
					u8_tm_ifr_1ms--;
				}
				if(u8_tm_ifr_1ms<1)
				{
					u8_ifr_step=2;
					u8_tm_ifr_1ms=0;
				}
			}
		}
		
		else if(u8_ifr_step==4)
		{
			if(IFR_Receive==0)
			{
				if(u8_tm_ifr_10ms>49)
				{
					u8_ifr_step=1;
					u8_tm_ifr_10ms=0;
					u8_tm_ifr_1ms=0;
				}
			}
			else
			{
				u8_ifr_step=5;
				u8_tm_ifr_10ms=0;
				u8_tm_ifr_1ms=0;
			}
		}
		else if(u8_ifr_step==5)
		{
			if(IFR_Receive==1)
			{
				if(u8_tm_ifr_1ms>8)
				{
					u8_ifr_step=6;
					u8_tm_ifr_1ms=0;
				}
			}
			else
			{
				u8_tm_ifr_1ms=0;	
			}
			if(u8_tm_ifr_10ms>99)				//如果1S后还没通过这次检测就返回
			{	
				u8_tm_ifr_10ms=0;
				u8_tm_ifr_1ms=0;
				u8_ifr_step=1;	
			}
		}
		else if(u8_ifr_step==6)
		{
			b_ifr_res_flag=1;	
			u8_ifr_step=1;
			u8_tm_ifr_1ms=0;
			u8_tm_ifr_10ms=0;
		}
	}
	if(b_ifr_res_flag==1)
	{
		if(b_led_sw_flag==1)
		{
			b_ifr_con_led_flag=0;
		}
		else
		{
			b_ifr_con_led_flag=1;
		}
		if(b_ifr_con_led_flag==0)
		{
			u8_led_mode1=u8_led_mode;
			u8_led_mode=0;
			b_led_shutdown_flag=0;
		}
		else 
		{
			u8_led_mode=u8_led_mode1;
			u8_led_mode1=0;
			b_led_shutdown_flag=0;
		}
		u8_led_step=1;
		b_bat_low_flag=0;
		b_ifr_res_flag=0;
	}
}


void tb0_int()
{
	b_tm_1ms_flag=1;
#if (SP_FLAG==1)
    if(u16_sp_buf_value > 0)	
    {
        _pb3 = (u16_sp_buf_value & 0x01);
        u16_sp_buf_value >>= 1;
    }
    else
    {
        b_sp_flag = 0;
        _pb3=1;
    }	
#endif
}
void Tm_Con()
{
	if(b_tm_1ms_flag==1)
	{
/////////////////////////////////////////////////////
		b_tm_1ms_flag=0;
		u8_tm_1ms_cnt++;
		if(u8_tm_1ms_cnt>=10)
		{
			u8_tm_1ms_cnt=0;
			b_tm_10ms_flag=1;
		}
/////////////////////////////////////////////////////
		u8_tm_key1_1ms++;
		u8_tm_key2_1ms++;
		u8_tm_led_1ms_cnt++;
		if(u8_tm_led_1ms_cnt>19)
		{
			u8_tm_led_1ms_cnt=0;
			b_tm_led_20ms_flag=1;
		}	
		if(b_ifr_sw_flag==1)
		{
			if(u8_tm_ifr_1ms<255)
			{
				u8_tm_ifr_1ms++;
			}
		}
		else
		{
			u8_tm_ifr_1ms=0;
		}
		b_tm_ifr_1ms_flag=1;
	}
	if(b_tm_10ms_flag==1)
	{
/////////////////////////////////////////////////////
		b_tm_10ms_flag=0;	
		u8_tm_10ms_cnt++;
		if(u8_tm_10ms_cnt>=10)
		{
			u8_tm_10ms_cnt=0;
			b_tm_100ms_flag=1;
		}
/////////////////////////////////////////////////////
#if (SP_FLAG==1)
		u8_tm_sp_10ms++;
#endif
		if(u8_ifr_step==4||u8_ifr_step==5)
		{
			u8_tm_ifr_10ms++;
		}
		else
		{
			u8_tm_ifr_10ms=0;
		}
	}
	if(b_tm_100ms_flag==1)
	{
/////////////////////////////////////////////////////
		b_tm_100ms_flag=0;
		u8_tm_100ms_cnt++;
		if(u8_tm_100ms_cnt>=10)
		{
			u8_tm_100ms_cnt=0;
			b_tm_1s_flag=1;
		}
/////////////////////////////////////////////////////
		u8_tm_halt_100ms_cnt++;
		if(b_led_shutdown_flag==0)
		{
			if(b_led_sw_flag==1&&b_key_cont1==0)
			{
				u8_tm_key1_100ms_cnt++;
			}	
			else
			{
				u8_tm_key1_100ms_cnt=0;
			}
		}
		else
		{
			u8_tm_key1_100ms_cnt=0;	
		}
		if(b_led_flash_flag==1&&b_bat_low_flag==1)
		{
			if(u8_led_flash_cnt<30)
			{
				u8_tm_flash_100ms_cnt++;
				if(u8_tm_flash_100ms_cnt<=5)
				{
					_pwmd=0;
				}
				else if(u8_tm_flash_100ms_cnt<=9)
				{
					_pwmd=u8_pwm_duty_cycle;	
				}
				else
				{
					u8_tm_flash_100ms_cnt=0;
					if(b_led_flash_two_flag==0)
					{
 						u8_led_flash_cnt++;	
					}
					else
					{
						u8_led_flash_cnt+=10;
					}
				}
			}
		}
		else
		{
			u8_tm_flash_100ms_cnt=0;
		}
	}
	if(b_tm_1s_flag==1)
	{
		b_tm_1s_flag=0;
		b_tm_bat_low_1s_flag=1;
		if(b_led_flash_flag==1&&b_bat_low_flag==1)
		{
			if(u8_led_flash_cnt>29)
			{
				_pwmd=u8_pwm_duty_cycle;
				u8_tm_bat_low_1s_cnt++;
				if(u8_tm_bat_low_1s_cnt>60)
				{
					u8_tm_bat_low_1s_cnt=0;	
					u8_led_flash_cnt=0;
					b_led_flash_two_flag=1;
				}
			}
		}
		else
		{
			u8_tm_bat_low_1s_cnt=0;
			u8_led_flash_cnt=0;
			b_led_flash_two_flag=0;
		}
	}
}
#if (SP_FLAG==1)
void Sp_Con()
{
	
	if(u8_tm_sp_10ms>=50)
	{
		u8_sp_value[0]	= ((u16_adc_bat_final>>8)&0xFF);
		u8_sp_value[1]	= (u16_adc_bat_final&0xFF);
		u8_sp_value[2]	= ((u16_adc_ocp_final>>8)&0xFF);
		u8_sp_value[3]	= (u16_adc_ocp_final&0xFF);
		u8_sp_value[4]	= u8_pwm_duty_cycle;
		u8_sp_value[5]	= b_led_pwm_flag;
		u8_sp_value[6]	= b_led_flash_flag;
		u8_sp_value[7]	= b_bat_low_flag;
		u8_sp_value[8]	= b_led_flash_two_flag;
		u8_sp_value[9]	= u8_led_flash_cnt;
		u8_sp_value[10]	= u8_pwm_cut_ref;
		u8_tm_sp_10ms=0;
		u8_sp_cnt=0;
	}
	if(u8_sp_cnt<11&b_sp_flag==0)
	{
		u16_sp_buf_value=(((uint16)u8_sp_value[u8_sp_cnt]<<1)|0x0200);
		u8_sp_cnt++;
		b_sp_flag=1;
	}	
}
#endif
void main()
{
	if(_pdf==1)
	{
		_clrwdt();
		_halt();
		_clrwdt();
	}
	_smod   = 0b00000001;  		//<0>HLCLK(Fsys=Fh)  
	_smod1  = 0b00000000;   
	_wdtc   = 0b01010111;
	_clrwdt();
	_sledc0	= 0B00000000;		
	_sledc1	= 0B00000000;	
	_pas0	= 0b00001100;		//<7:6>(00:PA3 11:OVP/AN3); <5:4>(00:PA2 10:STP 11:AN2); <3:2>(00:PA1 11:OCP_0/AN1); <1:0>(00:PA0 11:AN0)
	_pas1	= 0b00000000;		//<7:6>(00:PA7 11:AN7); <5:4>(00:PA6 11:OCP_1/AN6); <3:2>(00:PA5 11:AN5); <1:0>(00:PA4 11:VBAT/AN4)
    _pbs0	= 0b00000001;		//<0>PBS00(0:PB0 1:STP)
	_pcs0	= 0b00000000;		//<5>PCS05(0:PC5 1:PWMH); <5>PCS04(0:PC4 1:PWML)
	_ifs0	= 0b00000000;		//<5>IFS05; <4>IFS04; <3>IFS03; <2>IFS02; <1>IFS01; <0>IFS00;
	_pa 	= 0b00110000;		//电平		1:高;   0:低	
	_pac	= 0b00110010;		//输入输出	1:输入; 0:输出
	_papu	= 0b00110000;		//上拉		1:使能; 0:禁止
	_pawu	= 0b00110000;		//唤醒		1:使能; 0:禁止
	_pa 	= 0b00110000;		//电平		1:高;   0:低	
	_pb 	= 0b00000000;		//电平		1:高;   0:低	
	_pbc	= 0b00000010;		//输入输出	1:输入; 0:输出
	_pbpu	= 0b00000000;		//上拉		1:使能; 0:禁止
	_pb 	= 0b00000000;		//电平		1:高;   0:低	
	_pc 	= 0b00110000;		//电平		1:高;   0:低	
	_pcc	= 0b00110000;		//输入输出	1:输入; 0:输出
	_pcpu	= 0b00110000;		//上拉		1:使能; 0:禁止
	_pc 	= 0b00110000;		//电平		1:高;   0:低	
	_stmc0	= 0b00000000;		//<7>STPAU; <6-4>STCK2-STCK0; <3>STON; <2-0>STRP2-STRP0;
	_stmc1	= 0b00000000;		//<7-6>STM1-STM0; <5-4>STIO1-STIO0; <3>STOC; <2>STPOL; <1>STDPX; <0>STCCLR;					
	_sadc0	= 0b00000000;		//<7>START; <6>ADBZ; <5>ENADC; <4>ADRFS <3-0>SACS3-SACS0   
	_sadc1	= 0b00000000;		//<7-4>SAINS3-SAINS0; <2-0>SACKS2-SACKS0
	_sadc2	= 0b00000000;		//<7>ADPGAEN; <4>PGAIS; <3-2>SAVRS1-SAVRS0; <1-0>PGAGS1-PGAGS0
	_ocpc0	= 0b00000000;		//<7-6>OCPEN1-OCPEN0; <5-4>OCPVRS1-OCPVRS0; <3>OCPCHY; <0>OCP0;
	_ocpc1	= 0b00000000;		//<5-3>G2-G0; <2-0>OCPDEB2-OCPDEB0;
	_ocpocal= 0b00000000;		//<7>OOFM; <6>ORSP; <5:0>OOF
	_ocpccal= 0b00000000;		//<7>CPOUT; <6>COFM; <5>CRSP; <4:0>COF
	_ovpc0	= 0b00000000;		//<5>OVPEN; <4>OVPCHY; <3:2>OVPVRS; <1:0>OVPDEB
	_ovpc1	= 0b00000000;		//<7>OVPO; <6>OVPCOFM; <5>OVPCRS; <4:0>OVPCOF
	_ovpda	= 0b00000000;		//OVP DAC输出电压控制位 DAC参考电压*D[7:0]/256
	_pwmp	= 0b00000000;
	_pwmd	= 0b00000000;
	_dll	= 0b00000000;
	_cpr	= 0b00000000;		//<7>DLLLKEN; <6>DLLEN; <5:4>PWMCK(FH/8); <3>PWMON; <2:0>DT
	_cpor	= 0b00000000;		//<7>PWMACT; <6>CNVTYP; <5>INVH; <4>INVL; <3>OCPHEN; <2>OCPLEN; <1>OVPHEN; <0>OVPLEN;
    _integ	= 0b00000000;		//<3:2>INT1S;<1:0>INT0S
    _intc1	= 0b00000000;		//<3>DEE;<2>ADE;<0>MFE
    _mfi	= 0b00000000;		//<5>STMAF;<4>STMPF;<1>STMAE;<0>STMPE
    _tbc	= 0b11000011;		//<7>TBON; <6>TBCK=fsys/4; <5:4>TB1=4096/fTB; <2-0>TB0=2048/fTB 
    _intc2	= 0b00000001;		//<2>INT1E;<1>TB1E;<0>TB0E
    _intc0	= 0b00000001;		//<3>INT0E;<2>OVPE;<1>OCPE;<0>EMI
    _clrwdt();
//Halt
	b_halt_flag=0;
	u8_tm_halt_100ms_cnt=0;
//TM
	b_tm_1ms_flag=0;
	b_tm_10ms_flag=0;
	b_tm_100ms_flag=0;
	b_tm_1s_flag=0;
	u8_tm_1ms_cnt=0;
	u8_tm_10ms_cnt=0;
	u8_tm_100ms_cnt=0;
//LED
	b_led_sw_flag=0;
	b_led_pwm_flag=0;
	b_tm_led_20ms_flag=0;
	b_led_shutdown_flag=0;

	u8_led_mode=0;
	u8_led_mode1=0;
	u8_led_step=0;
	u8_tm_led_1ms_cnt=0;
//IFR
	b_ifr_sw_flag=0;
	b_ifr_res_flag=0;
	b_ifr_con_led_flag=0;
	b_tm_ifr_1ms_flag=0;
	u8_ifr_step=0;
	u8_tm_ifr_1ms=0;
//PWM
	u8_pwm_add_cnt=0;
	u8_pwm_add_ref=0;
	u8_pwm_cut_cnt=0;
	u8_pwm_cut_ref=0;
	u8_pwm_duty_cycle;
//pre
	b_pwm_pre_flag=0;
	u8_pwm_pre_var=0;
//flash
	b_led_flash_flag=0;
	b_bat_low_flag=0;
	b_tm_bat_low_1s_flag=0;
	b_led_flash_two_flag=0;
	u8_bat_low_cnt=0;
	u8_tm_flash_100ms_cnt=0;
	u8_bat_rec_cnt=0;
	u8_led_flash_cnt=0;
	u8_tm_bat_low_1s_cnt=0;
//ADC
	b_adc_cal_flag=0;
	b_adc_mode=0;
	b_adc_bat_final_flag=0;
	u8_adc_bat_cnt1=0;
	u8_adc_bat_cnt2=0;
	u16_adc_bat_vdd=0;
	u16_adc_bat_sum1=0;
	u16_adc_bat_sum2=0;
	u16_adc_bat_max=0;
	u16_adc_bat_min=0xFFFF;
	u16_adc_bat_final=0;
	b_adc_ocp_final_flag=0;
	u8_adc_ocp_cnt1=0;
	u8_adc_ocp_cnt2=0;
	u8_adc_ocp_cnt3=0;
	u16_adc_ocp_vdd=0;
	u16_adc_ocp_sum1=0;
	u16_adc_ocp_sum2=0;
	u16_adc_ocp_sum3=0;
	u16_adc_ocp_max=0;
	u16_adc_ocp_min=0xFFFF;
	u16_adc_ocp_final=0;
//KEY
 	b_key1=0;
 	b_key_trg1=0;
 	b_key_date1=0;
 	b_key_cont1=0;	
 	b_key1_flag=0;
 	u8_tm_key1_1ms=0;
  	b_key2=0;		
 	b_key_trg2=0;
 	b_key_date2=0;
 	b_key_cont2=0;	
 	b_key2_flag=0;	
 	u8_tm_key2_1ms=0;
 	u8_tm_key1_100ms_cnt=0;
#if (SP_FLAG==1)
//SP
	b_sp_flag=0;
	u8_tm_sp_10ms=0;
	u8_sp_cnt=0;
	u16_sp_buf_value=0;
#endif
	while(1)
	{
		_clrwdt();
		if(b_halt_flag==1)
		{
			_intc0	= 0b00000000;		//<3>INT0E;<2>OVPE;<1>OCPE;<0>EMI	
			_intc1	= 0b00000000;		//<3>DEE;<2>ADE;<0>MFE
			_intc2	= 0b00000000;		//<2>INT1E;<1>TB1E;<0>TB0E
			_integ	= 0b00000000;		//<3:2>INT1S;<1:0>INT0S
			_tbc	= 0b00000000;		//<7>TBON; <6>TBCK; <5:4>TB1; <2-0>TB0
			_mfi	= 0b00000000;		//<5>STMAF;<4>STMPF;<1>STMAE;<0>STMPE
			_sadc0	= 0b00000000;		//<7>START; <6>ADBZ; <5>ENADC; <4>ADRFS <3-0>SACS3-SACS0   
			_sadc1	= 0b00000000;		//<7-4>SAINS3-SAINS0; <2-0>SACKS2-SACKS0
			_sadc2	= 0b00000000;		//<7>ADPGAEN; <4>PGAIS; <3-2>SAVRS1-SAVRS0; <1-0>PGAGS1-PGAGS0
			_pas0	= 0b00000000;		//<7:6>(00:PA3 11:OVP/AN3); <5:4>(00:PA2 10:STP 11:AN2); <3:2>(00:PA1 11:OCP_0/AN1); <1:0>(00:PA0 11:AN0)
			_pas1	= 0b00000000;		//<7:6>(00:PA7 11:AN7); <5:4>(00:PA6 11:OCP_1/AN6); <3:2>(00:PA5 11:AN5); <1:0>(00:PA4 11:VBAT/AN4)
			_pbs0	= 0b00000000;		//<0>PBS00(0:PB0 1:STP)
			_pcs0	= 0b00000000;		//<5>PCS05(0:PC5 1:PWMH); <5>PCS04(0:PC4 1:PWML)
			_pa 	= 0b00110000;		//电平		1:高;   0:低	
			_pac	= 0b00110000;		//输入输出	1:输入; 0:输出
			_papu	= 0b00110000;		//上拉		1:使能; 0:禁止
			_pawu	= 0b00110000;		//唤醒		1:使能; 0:禁止
			_pa 	= 0b00110000;		//电平		1:高;   0:低	
			_pb 	= 0b00000000;		//电平		1:高;   0:低	
			_pbc	= 0b00000010;		//输入输出	1:输入; 0:输出
			_pbpu	= 0b00000000;		//上拉		1:使能; 0:禁止
			_pb 	= 0b00000000;		//电平		1:高;   0:低	
			_pc 	= 0b00110000;		//电平		1:高;   0:低	
			_pcc	= 0b00110000;		//输入输出	1:输入; 0:输出
			_pcpu	= 0b00110000;		//上拉		1:使能; 0:禁止
			_pc 	= 0b00110000;		//电平		1:高;   0:低	
			_clrwdt();
			_halt();
			_clrwdt();
			b_halt_flag=0;
			_pas0	= 0b00001100;		//<7:6>(00:PA3 11:OVP/AN3); <5:4>(00:PA2 10:STP 11:AN2); <3:2>(00:PA1 11:OCP_0/AN1); <1:0>(00:PA0 11:AN0)
			_pas1	= 0b00000000;		//<7:6>(00:PA3 11:OVP/AN3); <5:4>(00:PA2 10:STP 11:AN2); <3:2>(00:PA1 11:OCP_0/AN1); <1:0>(00:PA0 11:AN0)			
			_pbs0	= 0b00000001;		//<0>PBS00(0:PB0 1:STP)
			_pa 	= 0b00110000;		//电平		1:高;   0:低	
			_pac	= 0b00110010;		//输入输出	1:输入; 0:输出
			_papu	= 0b00110000;		//上拉		1:使能; 0:禁止
			_pawu	= 0b00110000;		//唤醒		1:使能; 0:禁止
			_pa 	= 0b00110000;		//电平		1:高;   0:低	
			_integ	= 0b00000000;		//<3:2>INT1S;<1:0>INT0S
			_intc1	= 0b00000000;		//<3>DEE;<2>ADE;<0>MFE
			_mfi	= 0b00000000;		//<5>STMAF;<4>STMPF;<1>STMAE;<0>STMPE
			_tbc	= 0b11000011;		//<7>TBON; <6>TBCK=fsys/4; <5:4>TB1=4096/fTB; <2-0>TB0=2048/fTB 		
			_intc2	= 0b00000001;		//<2>INT1E;<1>TB1E;<0>TB0E
			_intc0	= 0b00000001;		//<3>INT0E;<2>OVPE;<1>OCPE;<0>EMI	
		}
		if(b_led_sw_flag==1||b_ifr_sw_flag==1)
		{
			u8_tm_halt_100ms_cnt=0;
		}
		if(u8_tm_halt_100ms_cnt>=50)
		{
			b_halt_flag=1;
			u8_tm_halt_100ms_cnt=0;
		}
		if(u8_led_mode==0&&b_led_sw_flag==1)
		{
			b_led_sw_flag=0;
			b_led_pwm_flag=0;
			u8_adc_ocp_cnt1		=	0;
			u8_adc_ocp_cnt2		=	0;
			u8_adc_ocp_cnt3		=	0;
			u16_adc_ocp_sum1	=	0;
			u16_adc_ocp_sum2	=	0;
			u16_adc_ocp_sum3	=	0;
			u16_adc_ocp_max		=	0;
			u16_adc_ocp_min		=	0xFFFF;
		}	
		Tm_Con();
		ADC_Con();
		if(KEY1!=b_key1)
		{
			if(u8_tm_key1_1ms>35)
			{
				b_key1=KEY1;
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
		if(KEY2!=b_key2)
		{
			if(u8_tm_key2_1ms>35)
			{
				b_key2=KEY2;
				u8_tm_key2_1ms=0;
			} 
		}
		else
		{
			u8_tm_key2_1ms=0;
			b_key_date2=b_key2^0x01;
			b_key_trg2=b_key_date2&(b_key_date2^b_key_cont2);
			b_key_cont2=b_key_date2;
		}
		if(b_key_cont1==1&&b_key_trg1==1)
		{
			if(b_led_shutdown_flag==0)
			{
				if(u8_led_mode1==0)
				{
					u8_led_mode++;
					if(u8_led_mode>2)
					{
						u8_led_mode=0;
					}
				}
				else
				{
					u8_led_mode=u8_led_mode1;
					u8_led_mode1=0;
				}
				u8_led_step=1;
			}
			else
			{
				u8_led_mode=0;
				u8_led_step=1;	
				b_led_shutdown_flag=0;
			}
			b_bat_low_flag=0;
		}
		if(u8_tm_key1_100ms_cnt>49)				//5S触发记忆功能
		{
			b_led_shutdown_flag=1;
			u8_led_mode1=u8_led_mode;	
			u8_tm_key1_100ms_cnt=0;
		}
		if(b_key_cont2==1&&b_key_trg2==1)
		{
			b_ifr_sw_flag=~b_ifr_sw_flag;
			IFR_SW(b_ifr_sw_flag);
			u8_ifr_step=1;
		}
		LED_Con();
		IFR_Con();
		if(b_ifr_sw_flag==0)
		{
			IFR_LED=0;
		}
		
	}
}



