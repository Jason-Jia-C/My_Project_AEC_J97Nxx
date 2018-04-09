/*
	项目名称:
	编辑日期:2018.xx.xx
	作者	:JasonJia
	PCB负责 :龚为\徐珺\陈大庆\郭薇
	芯片封装:HT66F002 SOP_8\HT66F004 NSOP_16\HT453430 NSOP_16
	软件时钟:128us\1ms
	功能描述:
	修改记录:
/////////////HT编写注意事项//////
	(1)	创建项目名称时不能含有中文
	(2) 编写任何代码时不能省写括号
	(3) 002单片机中不允许使用任何子函数调用
	(4) 004和3430单片机最多允许调用一层子函数
	(5) 电压各种算法必须使用逼近法
	(6) 进入休眠时一定要先关闭所有中断，最后关闭看门狗
	(7) 单行代码不能进行太复杂的计算
	(8) 程序中尽量不要出现乘除法
	(9) 所有调光和ADC计算程序必须要进行反复验证
	(10)所有数据寄存器读写顺序必须从低到高
	(11)main函数开头需要增加空间清除程序
	(12)进入休眠前需要将所有中断使能关闭，如需用到外部中断则在INTEG内打开就可
	(13)使用外部中断唤醒时，唤醒后需要将中断标志位清零
	(14)6001芯片充电状态指示脚是开漏输出，没有输出高的能力，只能拉低
/////////////HT66F002////////////
			 -------
			|VDD VSS|
  			|PA6 PA0| 
        	|PA5 PA1| 
	 		|PA7 PA2| 
			 -------
/////////////HT66F004////////////
	 ------------------------
	|VSS				  VDD|
	|PC0		 PB0/INT0/AN0|
	|PC1		 PB1/INT1/AN1|
	|PC2		PB2/PTCK0/AN2|
	|PA0/PTP0	PA4/PTCK1/AN3| 
	|PA1/PTP0I	 PA5/AN4/VREF|
	|PA2		PA6/AN5/VREFO| 
	|PA3/PTP1I	 PA7/PTP1/AN6| 
	 ------------------------
/////////////HT45F3430////////////
   	-------------------------
   |PB0/STP1_1		 PWML/PC4|  
   |PB1 			 PWMH/PC5| 
   |PB2 				  VDD| +VCC
   |PB3 				  VSS| -VSS
   |PA5 			  AN6/PA6|  
   |PA4 			  AN7/PA7| 
   |PA3 			  AN0/PA0| 
   |PA2 			  AN1/PA1|  
    -------------------------
*/

#include "HT66F004.h"
#define bool	bit
#define uint8 	unsigned char 
#define uint16	unsigned int 

#define MCU_002		0
#define MCU_004		0
#define MCU_3430	0
#define Halt_FLAG	0
#define ADCV_FLAG	1
#define ADCA_FLAG	1
#define PTM_FLAG	1
#define STM_FLAG	0
#define CPWM_FLAG	0
#define EEP_FLAG	1
#define FLASH_FlAG	1
#define SHUT_FlAG	1
#define SP_FlAG 	0
#define PWM_FLAG	0

#define ping0	_pa0
#define ping1 	_pa1
#define ping2 	_pa2
#define ping3 	_pa3
#define ping4 	_pa4
#define ping5 	_pa5
#define ping6 	_pa6
#define ping7 	_pa7

#define ping10	_pb0
#define ping11	_pb1
#define ping12	_pb2
#define ping13	_pb3
#define ping14	_pb4
#define ping15	_pb5
#define ping16	_pb6
#define ping17	_pb7

#define ping20	_pc0
#define ping21	_pc1
#define ping22	_pc2
#define ping23	_pc3
#define ping24	_pc4
#define ping25	_pc5
#define ping26	_pc6
#define ping27	_pc7

#define KEY	ping0
#define Volt_FlashLowV	1000
#define Volt_FlashtRst	1500
#if (MCU_002==1)
/////////////HT66F002////////////
#pragma	vector int0_int @ 0x04
#pragma vector tb0_int	@ 0x08 		
#pragma vector tb1_int	@ 0x0C
#pragma vector mf0_int	@ 0x10
#pragma vector eep_int	@ 0x14
#pragma vector adc_int	@ 0x18
void int0_int(void)		{_inte=0;_intf=0;}
void tb1_int (void)		{_tb1e=0;_tb1f=0;}
void mf0_int (void)		{_mf0e=0;_mf0f=0;}
void eep_int (void)		{_dee=0;_def=0;}
void adc_int (void)		{_ade=0;_adf=0;}
#endif
#if (MCU_004==1)
/////////////HT66F004////////////
#pragma vector	ext0_int	@ 0x04		
#pragma vector	tb0_int 	@ 0x08		
#pragma vector	tb1_int 	@ 0x0C		
#pragma vector	mf0_int 	@ 0x10		
#pragma vector	eeprom_int	@ 0x14		
#pragma vector	adc_int 	@ 0x18		
#pragma vector	ext1_int	@ 0x1C	
void ext0_int	(void)	{_int0e=0;_int0f=0;}
void tb1_int	(void)	{_tb1e=0;_tb1f=0;}
void mf0_int	(void)	{_mf0e=0;_mf0f=0;}
void eeprom_int	(void)	{_dee=0;_def=0;}
void adc_int	(void)	{_ade=0;_adf=0;}
void ext1_int	(void)	{_int1e=0;_int1f=0}
#endif
#if (MCU_3430==1)
/////////////HT66F3430////////////
#pragma	vector	ocp_int		@ 0x04	
#pragma	vector	ovp_int		@ 0x08	
#pragma vector	ext0_int	@ 0x0C	
#pragma vector	mf_int		@ 0x10 
#pragma vector	adc_int		@ 0x18		
#pragma	vector	eeprom_int	@ 0x1C		
#pragma vector	tb0_int		@ 0x20		
#pragma vector	tb1_int		@ 0x24		
#pragma vector	ext1_int	@ 0x28	
void ocp_int	(void)	{_ocpe=0;_ocpf=0;}
void ovp_int	(void)	{_ovpe=0;_ovpf=0;}
void ext0_int	(void)	{_int0e=0;_int0f =0;}
void mf_int		(void)	{_mfe=0;_mff=0;}
void adc_int	(void)	{_ade=0;_adf=0;}
void eeprom_int	(void)	{_dee=0;_def=0;}
void tb1_int	(void)	{_tb1e=0;_tb1f=0;}
void ext1_int	(void)	{_int1e=0;_int1f=0;}
#endif
#pragma rambank0
	bool	b_tm_1ms_flag;
	bool	b_tm_10ms_flag;
	bool	b_tm_100ms_flag;
	bool	b_tm_1s_flag;
	uint8	u8_tm_1ms_cnt;
	uint8	u8_tm_10ms_cnt;
	uint8	u8_tm_100ms_cnt;

	bool	b_key1; 	
	bool	b_key_trg1;
	bool	b_key_date1;
	bool	b_key_cont1;	
	bool	b_key1_flag;	
	uint8	u8_tm_key1_1ms;

	bool	b_key2; 	
	bool	b_key_trg2;
	bool	b_key_date2;
	bool	b_key_cont2;	
	bool	b_key2_flag;	
	uint8	u8_tm_key2_1ms;
	
	uint8	u8_led_step;
	uint8	u8_led_mode;
#if (SHUT_FlAG==1)
	bool	b_shutdown_flag;
	bool	b_tm_shutdown_100ms;
	uint8	u8_shutdown_timer_100ms_cnt;
#endif
	
#if (Halt_FLAG==1)
	bool	b_halt_flag;
	uint8	u8_tm_halt_100ms_cnt;
#endif

#if (ADCV_FLAG==1)
	bool	b_adc_VoltF_flag;
	uint8	u8_adc_VoltF_cnt1;
	uint16	u16_adc_VoltF_vdd1;
	uint16	u16_adc_VoltF_sum1;
	uint16	u16_adc_VoltF_max;
	uint16	u16_adc_VoltF_min;
	uint16	u16_adc_VoltF_value;
#endif

#if (ADCA_FLAG==1)	
	bool	b_adc_AmpF_flag;
	uint8	u8_adc_AmpF_cnt1;
	uint16	u16_adc_AmpF_vdd1;
	uint16	u16_adc_AmpF_sum1;
	uint16	u16_adc_AmpF_max;
	uint16	u16_adc_AmpF_min;
	uint16	u16_adc_AmpF_value;
#endif
	
#if (SP_FlAG==1)
	bool	b_sp_flag;
	uint8	u8_sp_value[1];
	uint8	u8_tm_sp_10ms;
	uint8	u8_sp_cnt;
	uint16	u16_sp_buf_value;
#endif

#if (EEP_FLAG==1)
	uint8	u8_eep_read_var;
	uint8	u8_eep_write_var;
#endif

#if (FLASH_FlAG==1)
	bool	b_flash_en_flag;
	bool	b_flash_start_flag;	
	bool	b_tm_flash_scan_10ms;
	bool	b_flash_status_sw;
	uint8	u8_flash_scan_10ms_cnt;
	uint8	u8_flash_timer_10ms_cnt;
	uint8	u8_flash_status_step;
	uint8	u8_flash_status_sw_cnt;
#endif	

#pragma norambank
#if (ADCV_FLAG==1)
void ADC_Volt()
{
	_sadc1	=	0b00000100; 	//[2:0]Fad=Fsys/16
	_sadc0	=	0b00110011; 	//[5]Open ENADC 	[4]SADOH=[11:8] SADOL=[7:0] 	[1:0]AN3
	_sadc2	=	0b00000000; 	//[7]ENOPA	[6]VBGEN	[3:0]Vf=VDD 
	_start	=	0;
	_start	=	1;
	_start	=	0;
	while(_adbz)
	{
		_clrwdt();
	}
	u16_adc_VoltF_vdd1=((uint16)_sadoh<<8)+(_sadol);
	u16_adc_VoltF_sum1+=u16_adc_VoltF_vdd1;
	if(u16_adc_VoltF_vdd1>u16_adc_VoltF_max)
	{
		u16_adc_VoltF_max=u16_adc_VoltF_vdd1;
	}
	if(u16_adc_VoltF_vdd1<u16_adc_VoltF_min)
	{
		u16_adc_VoltF_min=u16_adc_VoltF_vdd1;
	}
	u8_adc_VoltF_cnt1++;
	if(u8_adc_VoltF_cnt1>=10) 		
	{
		u16_adc_VoltF_sum1	-=	u16_adc_VoltF_max;
		u16_adc_VoltF_sum1	-=	u16_adc_VoltF_min;
		u16_adc_VoltF_sum1	>>= 3;
		u16_adc_VoltF_value	=	u16_adc_VoltF_sum1;
		u16_adc_VoltF_value	=	1;
		u8_adc_VoltF_cnt1 	=	0;
		u16_adc_VoltF_sum1	=	0;
		u16_adc_VoltF_max 	=	0;
		u16_adc_VoltF_min 	=	0xFFFF;
		u16_adc_VoltF_vdd1 	=	0;
	}		
}
#endif
#if (ADCA_FLAG==1)
void ADC_Amp()
{
	_sadc1	 = 0b00000100;    
	_sadc0   = 0B00110001;		//<5>ENADC; <4>ADRFS;<3:0>Open AN1		 
	_sadc2	 = 0b10011101;       //3-0 Vbg(1.04V)* 2 =2.08		(xxxx/4096*2.08,2048)
	_start   = 0;
	_start   = 1;
	_start   = 0;
	while(_adbz)
		_clrwdt();
	u16_adc_AmpF_vdd1=((uint16)_sadoh<<8)+(_sadol);
	u16_adc_AmpF_sum1+=u16_adc_AmpF_vdd1;
	if(u16_adc_AmpF_vdd1>u16_adc_AmpF_max)
	{
		u16_adc_AmpF_max=u16_adc_AmpF_vdd1;
	}
	if(u16_adc_AmpF_vdd1<u16_adc_AmpF_min)
	{
		u16_adc_AmpF_min=u16_adc_AmpF_vdd1;
	}
	u8_adc_AmpF_cnt1++;
	if(u8_adc_AmpF_cnt1>=10)
	{
		u16_adc_AmpF_sum1	-=	u16_adc_AmpF_max;
		u16_adc_AmpF_sum1	-=	u16_adc_AmpF_min;
		u16_adc_AmpF_sum1	>>=	3;
		u16_adc_AmpF_value	=	u16_adc_AmpF_sum1;
		b_adc_AmpF_flag		=	1;
		u8_adc_AmpF_cnt1	=	0;
		u16_adc_AmpF_sum1	=	0;
		u16_adc_AmpF_max	=	0;
		u16_adc_AmpF_min	=	0xFFFF;
	}
		
}
#endif
void Key_San()
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
}
#if (PTM_FLAG==1)				//HT66F004
void PTM_Set(bool b_PTM_flag)
{
	if(b_PTM_flag==1)	
	{
		_ptm1c0 = 0b00101000;	
		_ptm1c1 = 0b10101000;
		_ptm1dl = 0b00000000;
		_ptm1dh = 0b00000000;
		_ptm1rpl= 0b11111111;	//Cycle Set 
		_ptm1rph= 0b00000011;	
		_ptm1al = 0b00000000;
		_ptm1ah = 0b00000000;	//Duty cycle Set
	}
	else		
	{
		_ptm1c0 = 0b00000000;	
		_ptm1c1 = 0b00000000;
		_ptm1dl = 0b00000000;
		_ptm1dh = 0b00000000;
		_ptm1rpl= 0b00000000;
		_ptm1rph= 0b00000000;
		_ptm1al = 0b00000000;
		_ptm1ah = 0b00000000;
	}	
}
#endif
#if (STM_FLAG==1)				//HT66F002\HT45F3430		
void STM_Set(bool b_STM_flag)
{
	if(b_STM_flag==1)
	{
		_stmdl	=0x00;
		_stmdh	=0x00;
		_stmal	=0xD4;
		_stmah	=0x00;
		_stmc0	=0b00011001;	
		_stmc1	=0b10100011;
		
	}
	else if(b_STM_flag==0)
	{
		_stmdl	=0x00;
		_stmdh	=0x00;
		_stmal	=0xD4;
		_stmah	=0x00;
		_stmc0	=0b00000000;	
		_stmc1	=0b00000000;		
	}
}
#endif
#if (CPWM_FLAG)					//HT45F3430
void CPWM_Set(uint8 u8_mode_select)
{
	if(u8_mode_select==0)
	{
		_pcc	=	0b00110000;
		_pcpu	=	0b00110000;
		_pcs0	=	0b00000000;
		_cpr	=	0;
		_cpor	= 	0;
		_pwmd	=	0;
		_pwmp	=	0;
	}
	if(u8_mode_select==1)
	{
		_pcc	=	0b00010000;
		_pcpu	=	0b00010000;
		_pcs0	=	0b00100000;
		_cpr	= 	0B00111000;		//<7>DLLLKEN; <6>DLLEN; <5:4>PWMCK; <3>PWMON; <2:0>DT
		_cpor	=	0b10110000;
		_pwmp	=	0b11111111;
		_dll	=	0x00;
	}
	else if(u8_mode_select>1)
	{
		_pcc	=	0b00100000;
		_pcpu	=	0b00100000;
		_pcs0	=	0b00010000;
		_cpr	= 	0B00111000;		//<7>DLLLKEN; <6>DLLEN; <5:4>PWMCK; <3>PWMON; <2:0>DT
		_cpor	=	0b11110000;
		_pwmp	=	0xFF;
		_dll	=	0x00;
	}
}
#endif
#if (Halt_FLAG==1)
void Halt_Con()
{
	if(u8_tm_halt_100ms_cnt>=50)
	{
		b_halt_flag=1;
		u8_tm_halt_100ms_cnt=0;
	}
	if(b_halt_flag==1)
	{
#if (MCU_002==1)
		_clrwdt();
		_intc0	=	0b00000000;
		_intc1	=	0b00000000;
		_integ	=	0b00000000;
		_mfi0	=	0b00000000;
		_tbc	=	0b00000000;
		_sadc0	=	0b00000000;
		_sadc1	=	0b00000000;
		_sadc2	=	0b00000000;
		_clrwdt();
		_wdtc	=	0b10101111;
		_pa 	=	0b00000000;
		_pac	=	0b00000000;
		_papu	=	0b00000000;
		_pawu	=	0b00000000;
		_pasr	=	0b00000000;
		_isf0	=	0b00000000;
#endif
#if (MCU_004==1)
		_clrwdt();
		_intc0	=	0b00000000;
		_intc1	=	0b00000000;
		_integ	=	0b00000000;
		_mfi0	=	0b00000000;
		_tbc	=	0b00000000;
		_sadc0	=	0b00000000;
		_sadc1	=	0b00000000;
		_sadc2	=	0b00000000;
		_clrwdt(); 
		_wdtc	=	0b10101111;
		_pa		=	0b00000000;
		_pac	=	0b00000000;
		_papu	=	0b00000000;
		_pawu	=	0b00000000;
		_pa 	=	0b00000000;
		_pasr	=	0b00000000;
 		_pb 	=	0b00000000;
		_pbc	=	0b00000000;
		_pbpu	=	0b00000000;
		_pb 	=	0b00000000;
		_pbsr	=	0b00000000;
		_pc 	=	0b00000000;
		_pcc	=	0b00000000;
		_pcpu	=	0b00000000;
		_pc 	=	0b00000000;
		_pbsr	=	0b00000000;

#endif
#if(MCU_3430==1)
		_clrwdt();
		_intc0	=	0b00000001;
		_intc1	=	0b00000000;
		_intc2	=	0b00000001;
		_integ	=	0b00000000;
		_mfi	=	0b00000000;
 		_tbc	=	0b11000011;
 		_sadc0	=	0b00000000;
		_sadc1	=	0b00000000;
		_sadc2	=	0b00000000;
		_clrwdt();
		_pa 	=	0b00000000;
		_pac	=	0b00000000;
		_papu	=	0b00000000;
		_pawu	=	0b00000000;
		_pa 	=	0b00000000;
		_pas0	=	0b00000000;
		_pas1	=	0b00000000;
		_pb 	=	0b00000000;
		_pbc	=	0b00000000;
		_pbpu	=	0b00000000;
		_pb 	=	0b00000000;
		_pbs0	=	0b00000000;
		_pc 	=	0b00000000;
		_pcc	=	0b00000000;
		_pcpu	=	0b00000000;
		_pc 	=	0b00000000;
		_pcs0	=	0b00000000;
		_isf0	=	0b00000000;
#endif
		_nop();
		_nop();
		_halt();
		_nop();
		_nop();
		b_halt_flag=0;
#if(MCU_002==1)
		_wdtc	=	0b01010111;
		_clrwdt();
		_intc0		=	0b00000101;
		_tbc		=	0b11000011;
#endif

#if(MCU_004==1)
		_wdtc	=	0b01010111;
		_clrwdt();
		_intc0		=	0b00000101;
		_tbc		=	0b11000011;
#endif
	
#if(MCU_3430==1)	
		_clrwdt();
		_intc0		=	0b00000001;
		_intc2		=	0b00000001;
		_tbc		=	0b11000011;
#endif
	}
	
}
#endif
void Seg_Set()
{
#if(MCU_002==1)
	_smod		=	0b00000001;
	_somd1		=	0b00000000;
	_wdtc		=	0b01010111;
	_clrwdt();
	_stm0c0		=	0b00000000;
	_stm0c1		=	0b00000000;
	_stm0al		=	0b00000000;
	_stm0ah		=	0b00000000;
	_sadc0		=	0b00000000;
	_sadc1		=	0b00000000;
	_sadc2		=	0b00000000;
	_clrwdt();
	_integ		=	0b00000000;
	_intc0		=	0b00000101;
	_intc1		=	0b00000000;
	_mfi0		=	0b00000000;
	_tbc		=	0b11000011;
#endif
	
#if(MCU_004==1)
	_smod		=	0b00000001;
	_somd1		=	0b00000000;
	_wdtc		=	0b01010111;
	_clrwdt(); 
	_ptm0c0		=	0b00000000;
	_ptm0c1		=	0b00000000;
	_ptm1c0		=	0b00000000;
	_ptm1c1		=	0b00000000;
	_ptm0al		=	0b00000000;
	_ptm0ah		=	0b00000000;
	_ptm0rpl	=	0b00000000;
	_ptm0rph	=	0b00000000;
	_ptm1al		=	0b00000000;
	_ptm1ah		=	0b00000000;
	_ptm1rpl	=	0b00000000;
	_ptm1rph	=	0b00000000;
	_sadc0		=	0b00000000;
	_sadc1		=	0b00000000;
	_sadc2		=	0b00000000;
	_clrwdt();
	_integ		=	0b00000000;
	_intc0		=	0b00000101;
	_intc1		=	0b00000000;
	_mfi0		=	0b00000000;
	_tbc		=	0b11000011;
	_scomc		=	0b00000000;
#endif
		
#if(MCU_3430==1)	
	_smod		=	0b00000001;
	_somd1		=	0b00000000;
	_wdtc		=	0b01010000;
	_clrwdt();
	_stmc0		=	0b00000000;
	_stmc1		=	0b00000000;
	_stmal		=	0b00000000;
	_stmah		=	0b00000000;
	_sadc0		=	0b00000000;
	_sadc1		=	0b00000000;
	_sadc2		=	0b00000000;
	_ocpc0		=	0b00000000;
	_ocpc1		=	0b00000000;
	_ocpocal	=	0b00000000;
	_ocpccal	=	0b00000000;
	_ovpc0		=	0b00000000;
	_ovpc1		=	0b00000000;
	_cpr		=	0b00000000;
	_cpor		=	0b00000000;
	_pwmp		=	0b00000000;
	_pwmd		=	0b00000000;
	_scomc		=	0b00000000;
	_clrwdt();
	_integ		=	0b00000000;
	_intc0		=	0b00000001;
	_intc1		=	0b00000000;
	_intc2		=	0b00000001;
	_mfi		=	0b00000000;
	_tbc		=	0b11000011;
#endif
}
void IO_Set()
{
#if	(MCU_002==1)
	_pa		=	0b00000000;
	_pac	=	0b00000000;
	_papu	=	0b00000000;
	_pawu	=	0b00000000;
	_pasr	=	0b00000000;
	_isf0	=	0b00000000;
#endif

#if	(MCU_004==1)
	_pa 	=	0b00000000;
	_pac	=	0b00000000;
	_papu	=	0b00000000;
	_pawu	=	0b00000000;
	_pa		=	0b00000000;
	_pasr	=	0b00000000;

	
	_pb 	=	0b00000000;
	_pbc	=	0b00000000;
	_pbpu	=	0b00000000;
	_pb		=	0b00000000;
	_pbsr	=	0b00000000;

	
	_pc 	=	0b00000000;
	_pcc	=	0b00000000;
	_pcpu	=	0b00000000;
	_pc		=	0b00000000;
	_pbsr	=	0b00000000;
#endif

#if	(MCU_3430==1)
	_pa 	=	0b00000000;
	_pac	=	0b00000000;
	_papu	=	0b00000000;
	_pawu	=	0b00000000;
	_pa 	=	0b00000000;
	_pas0	=	0b00000000;
	_pas1	=	0b00000000;
	

	_pb 	=	0b00000000;
	_pbc	=	0b00000000;
	_pbpu	=	0b00000000;
	_pb 	=	0b00000000;
	_pbs0	=	0b00000000;

	_pc 	=	0b00000000;
	_pcc	=	0b00000000;
	_pcpu	=	0b00000000;
	_pc 	=	0b00000000;
	_pcs0	=	0b00000000;

	_isf0	=	0b00000000;
#endif
}
#if (EEP_FLAG==1)
void EEP_Read()
{
	_eea	=	0x01;
	_mp1	=	0x40;
	_bp		=	0X01;
	_iar1	|=	0x02;
	_iar1	|=	0x01;
	while((_iar1&0x01) == 0x01)
	{
		_clrwdt();
	}
	_iar1	=	0;
	_bp		=	0;
	u8_eep_read_var = _eed;
}
void EEP_Write()
{
	_eea	=	0x01;
	_eed	=	u8_eep_write_var;
	_mp1	=	0x40;
	_bp		=	0X01;
	_emi	=	0; 		//写周期执行前总中断位除能
	_iar1	|=	0x08;
	_iar1	|=	0x04;
	_emi	=	1; 		//写周期执行后总中断位使能
	while((_iar1&0x04) == 0x04)
	{
		_clrwdt();
	}
	_emi	=	0;
	_iar1	=	0;
	_bp		=	0;
}
#endif
#if (FLASH_FlAG==1)
void Flash_LowV()
{
	if(b_tm_flash_scan_10ms==1)
	{
		b_tm_flash_scan_10ms=0;
////////////////////////////////////////////////闪烁状态处理部分//////////////////////////////////////////////////
		if(b_flash_en_flag==0)						
		{
			if(u16_adc_VoltF_value<Volt_FlashLowV)			//当AD值低于下限，开始识别
			{
				u8_flash_scan_10ms_cnt++;
			}
			else
			{
				u8_flash_scan_10ms_cnt=0;
			}
			if(u8_flash_scan_10ms_cnt>250)					//当AD值连续低于下限超过2.5s，进入闪烁
			{
				u8_flash_scan_10ms_cnt=0;
				b_flash_en_flag=1;
				b_flash_start_flag=1;
				b_flash_status_sw=0;
				u8_flash_scan_10ms_cnt=0;
				u8_flash_timer_10ms_cnt=0;
				u8_flash_status_sw_cnt=0;
				u8_flash_status_step=0;
			}
		}
		else
		{
			if(u16_adc_VoltF_value>Volt_FlashtRst)			//当AD值高于上线，开始识别
			{
				u8_flash_scan_10ms_cnt++;
			}
			else
			{
				u8_flash_scan_10ms_cnt=0;
			}
			if(u8_flash_scan_10ms_cnt>250)					//当AD值连续高于上线超过2.5s，闪烁状态复位
			{
				u8_flash_scan_10ms_cnt=0;
				b_flash_en_flag=0;
				b_flash_start_flag=0;
			}
		}
////////////////////////////////////////////////闪烁状态执行部分//////////////////////////////////////////////////
		if(b_flash_start_flag==1)
		{
/*			简洁模式闪烁
			u8_flash_timer_10ms_cnt++;
			if(u8_flash_timer_10ms_cnt>49)
			{
				u8_flash_timer_10ms_cnt=0;
				b_flash_status_sw=~b_flash_status_sw;
				u8_led_step=3;
			}
*/
			u8_flash_timer_10ms_cnt++;
			if(u8_flash_status_step==0||u8_flash_status_step==2)
			{
				if(u8_flash_timer_10ms_cnt>49)
				{
					u8_flash_timer_10ms_cnt=0;
					b_flash_status_sw=~b_flash_status_sw;
					u8_flash_status_sw_cnt++;
					if(u8_flash_status_step==0)
					{
						if(u8_flash_status_sw_cnt<61)
						{
							u8_led_step==3;
						}
						else
						{
							u8_flash_status_sw_cnt=0;
							u8_flash_status_step=2;
						}
					}
					else
					{
						if(u8_flash_status_sw_cnt<7)
						{
							u8_led_step==3;
						}
						else
						{
							u8_flash_status_sw_cnt=0;
							u8_flash_status_step=2;
						}
					}
				}
			}
			else if(u8_flash_status_step==1)
			{
				u8_led_step=2;
				if(u8_flash_timer_10ms_cnt>99)
				{
					u8_flash_timer_10ms_cnt=0;
					u8_flash_status_sw_cnt++;
					if(u8_flash_status_sw_cnt>59)
					{
						u8_flash_status_sw_cnt=0;	
						u8_flash_status_step=2;
					}
				}
			}
		}
	}
}
#endif
#if (SHUT_FlAG==1)
void ShutDown()
{
	if(b_tm_shutdown_100ms==1)
	{
		b_tm_shutdown_100ms=0;
		u8_shutdown_timer_100ms_cnt++;
		if(u8_led_mode==0||b_shutdown_flag==1)
		{
			u8_shutdown_timer_100ms_cnt=0;	
		}
		if(u8_shutdown_timer_100ms_cnt>49)
		{
			u8_shutdown_timer_100ms_cnt=0;
			b_shutdown_flag=1;
		}
	}
}
#endif
void LED_Handle()
{
	if(b_key_cont1==1&&b_key_trg1==1)
	{
		if(b_shutdown_flag==0)
		{
			u8_led_mode++;
			if(u8_led_mode>3)
			{
				u8_led_mode=0;
			}
		}
#if (SHUT_FlAG==1)
		else
		{
			if(u8_led_mode>0)
			{
				u8_led_mode1=u8_led_mode;
				u8_led_mode=0;
			}
			else
			{
				u8_led_mode=u8_led_mode1;
				b_shutdown_flag=0;
			}
		}
		else
		{
			u8_led_mode++;
			if(u8_led_mode>3)
			{
				u8_led_mode=0;
			}
		}
#endif
		u8_led_step=2;
		
	}
}
void LED_Ex()
{
	if(u8_led_step==2)
	{
		if(u8_led_mode==1)
		{
			
		}
		else if(u8_led_mode==2)
		{
			
		}
		else if(u8_led_mode==3)
		{
			
		}
		else
		{
			
		}
		u8_led_step=5;	
	}
#if (FLASH_FlAG==1)
	else if(u8_led_step==1)
	{
		if(b_flash_start_flag==1)
		{
			b_flash_start_flag=0;	
			b_flash_status_sw=0;
			u8_flash_status_sw_cnt=0;
			u8_flash_status_step=0;
			u8_flash_scan_10ms_cnt=0;
		}
		u8_led_step=2;
	}
	else if(u8_led_step==3)
	{
		if(b_flash_status_sw==0)
		{
			u8_led_step=2;
		}
		else
		{
			u8_led_step=4;
		}	
	}
	else if(u8_led_step==4)
	{
		if(u8_led_mode==1)
		{
			
		}
		else if(u8_led_mode==2)
		{
			
		}
		else if(u8_led_mode==3)
		{
			
		}	
		u8_led_step=5;
	}
#endif
}
/*
	adj的设想:
	调节需要一个比较值，一个参考值，一般比较值由AD得来，所以这个比较程序10ms进入一次
	在急速调节时10ms调节一次，高速时40ms调节一次，中速时100ms进入一次，低速时2S进入一次
*/
void LED_adj()
{
	uint8	u8_x1;//比较值
	uint8	u8_x2;//参考值
	uint8	u8_c1,u8_c2,u8_c3,u8_c4;
	if(b_tm_led_adj_10ms==1)
	{
		b_tm_led_adj_10ms=0;
///////////////////////加//////////////////////////////////
		if(u8_x2>(u8_x1+u8_c1)) 					//急速
		{
			u8_led_adj_add_cnt+=201;
			u8_led_adj_cut_cnt=0;
		}
		else if(u8_x2>(u8_x1+u8_c2))				//高速
		{
			u8_led_adj_add_cnt+=50;
			u8_led_adj_cut_cnt=0;
		}
		else if(u8_x2>(u8_x1+u8_c3))				//中速
		{
			u8_led_adj_add_cnt+=20;
			u8_led_adj_cut_cnt=0;
		}
		else if(u8_x2>(u8_x1+u8_c4))				//低速
		{
			u8_led_adj_add_cnt++;
			u8_led_adj_cut_cnt=0;
		}
////////////////////////减/////////////////////////////////
		if(u8_x1>(u8_x2+u8_c1))						//急速
		{
			u8_led_adj_cut_cnt+=201;
			u8_led_adj_add_cnt=0;
		}
		else if(u8_x1>(u8_x2+u8_c2))				//高速
		{ 
			u8_led_adj_cut_cnt+=50;
			u8_led_adj_add_cnt=0;
		}
		else if(u8_x1>(u8_x2+u8_c3))				//中速
		{
			u8_led_adj_cut_cnt+=20;
			u8_led_adj_add_cnt=0;
		}
		else if(u8_x1>(u8_x2+u8_c4))				//低速
		{
			u8_led_adj_cut_cnt++;
			u8_led_adj_add_cnt=0;
		}
		if(u8_led_adj_add_cnt>200)
		{
			
		}
		if(u8_led_adj_cut_cnt>200)
		{
			
		}
	}
}
void Date_Send()
{
	
}
void main()
{
	
}   
