/*
项目名称:手持式音响灯
	编辑日期:2018.3.19
	作者	:JasonJia
	PCB负责 :徐珺
	芯片封装:HT66F004 NSOP_16
	软件时钟:1ms
	功能描述:蓝牙播放，正面弱-正面强-关
	修改记录:
			2018年3月19日编辑第一版



脚位定义
	         ________________________
            |VSS                  VDD|
	 LD_EN1 |PC0         PB0/INT0/AN0|
     LD_EN2 |PC1         PB1/INT1/AN1| 
     	    |PC2        PB2/PTCK0/AN2| 
		KEY |PA0/PTP0   PA4/PTCK1/AN3| 
        	|PA1/PTP0I   PA5/AN4/VREF| 
   	        |PA2        PA6/AN5/VREFO| BLE_EN
    BLE_LED |PA3/PTP1I   PA7/PTP1/AN6| BLE_STATUS
             ------------------------

*/	
#include "HT66F004.h"
#define bool	bit
#define uint8 	unsigned char 
#define uint16	unsigned int 
	
#define LD_EN1			_pc0
#define LD_EN2			_pc1
#define KEY				_pa0
#define BLE_LED			_pa3
#define BLE_STATUS		_pa7
#define BLE_EN			_pa6

#define LD_HALF				{LD_EN1=1;LD_EN2=0;}
#define LD_FULL				{LD_EN1=1;LD_EN2=1;}
#define LD_OFF				{LD_EN1=0;LD_EN2=0;}

#define BLE_ON				{BLE_EN=1;_pac7=1;_papu7=1;}
#define BLE_CONNECT			{BLE_EN=1;BLE_LED=1;_pac7=1;_papu7=1;}
#define BLE_OFF 			{BLE_EN=0;BLE_LED=0;_papu7=0;_pac7=0;_pa7=0;}

#define HALT_START_TM			4890			//休眠计时5S

#pragma rambank0

bool 	b_halt_flag;
uint16	u16_tm_halt_1ms;

bool 	b_tm_1ms_flag;
bool	b_tm_100ms_flag;
uint8	u8_tm_1ms_cnt;

bool 	b_key1;
bool 	b_key_trg1;
bool 	b_key_date1;
bool 	b_key_cont1;
bool 	b_key_flag1;
uint8 	u8_tm_key1_1ms;
uint16 	u16_tm_key_press_down_1ms;

bool	b_tm_ble_125ms_flag;
uint8 	u8_loader_mode;
uint8 	u8_ble_mode;
uint16	u16_tm_ble_1ms;
uint16	u16_tm_ble_status_1ms;
uint16	u16_tm_ble_connect_100ms;


#pragma norambank

enum loader_mode_t
{
	ld_half_t,
	ld_full_t,
	ld_off_t,
};
enum ble_status_t
{
	ble_on_t,
	ble_connect_t,
	ble_off_t,
};

/////////////////////////////////////////////////////////////
#pragma 	vector		ext0_int	@ 0x04		//外部中断0
#pragma 	vector		tb0_int		@ 0x08		//Time Base 0
#pragma 	vector		tb1_int		@ 0x0C		//Time Base 1
#pragma 	vector		mf0_int		@ 0x10 		//多功能0 stm中断
#pragma		vector		eeprom_int	@ 0x14		//EEPROM
#pragma 	vector		adc_int		@ 0x18		//AD
#pragma 	vector		ext1_int	@ 0x1C		//外部中断1
/////////////////////////////////////////////////////////////

void ext0_int	(void)	{_int0e=0;_int0f=0;}
void tb1_int	(void)	{_tb1e=0;}
void mf0_int	(void)	{_mf0e=0;_mf0f=0;}
void eeprom_int	(void)	{_dee=0;_def=0;}
void adc_int	(void)	{_ade=0;_adf=0;}
void ext1_int	(void)	{_int1e=0;_int1f=0;}

void tb0_int()
{
	b_tm_1ms_flag=1;
}
void main()
{
	_bp=0;
	for(_mp0=0x40;_mp0<=0x9F;_mp0++)
	{
		_iar0=0;
	}
	u8_loader_mode=ld_off_t;
	u8_ble_mode=ble_off_t;
/*
	         ________________________
            |VSS                  VDD|
	 LD_EN1 |PC0         PB0/INT0/AN0|
     LD_EN2 |PC1         PB1/INT1/AN1| 
     	    |PC2        PB2/PTCK0/AN2| 
		KEY |PA0/PTP0   PA4/PTCK1/AN3| 
        	|PA1/PTP0I   PA5/AN4/VREF| 
   	        |PA2        PA6/AN5/VREFO| BLE_EN
    BLE_LED |PA3/PTP1I   PA7/PTP1/AN6| BLE_STATUS
             ------------------------
	
		#define LD_EN1			_pc0
		#define LD_EN2			_pc1
		#define KEY				_pa0
		#define BLE_LED			_pa3
		#define BLE_STATUS		_pa7
		#define BLE_EN			_pa6
		
			7		6		5		4		3		2		1		0
	PA 	BLE_EN  BLE_STATUS	/		/ 	BLE_LED		/		/	   KEY
	PB		/		/		/		/		/		/	  	/		/
	PC		/		/		/		/		/	  	/	LED_EN2	  LED_EN1
*/
	_rstc	= 0B01010101;		//复位选择寄存器
	_pasr	= 0B00000000;		//<7:6>PA7;<5:4>PA6;<3:2>PA5;<1>PA4;<0>PA0
	_pbsr	= 0B00000000;		//<5>PB5;<4>PB4;<3>PB3;<2>PB2;<1>PB1;<0>PB0
	_scomc	= 0B00000000;		//<6:5>偏压电阻;<4>SCOMEN;<3>COM3EN;<2>COM2EN;<1>COM1EN;<0>COM0EN
	_pa		= 0B00000001;		//电平		1:高;   0:低
	_pac	= 0B00000001;		//输入输出	1:输入; 0:输出
	_pawu	= 0B00000001;		//唤醒		1:使能; 0:禁止
	_papu	= 0B00000001;		//上拉		1:使能; 0:禁止
	_pa		= 0B00000001;		//电平		1:高;   0:低
	_pb		= 0B00000000;		//电平		1:高;   0:低
	_pbc	= 0B00000000;		//输入输出	1:输入; 0:输出
	_pbpu	= 0B00000000;		//上拉		1:使能; 0:禁止
	_pb		= 0B00000000;		//电平		1:高;   0:低
	_pc		= 0B00000000;		//电平		1:高;   0:低
	_pcc	= 0B00000000;		//输入输出	1:输入; 0:输出
	_pcpu	= 0B00000000;		//上拉		1:使能; 0:禁止
	_pc		= 0B00000000;		//电平		1:高;   0:低	
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
	_clrwdt();
	while(1)
	{
		_clrwdt();
////////////////////////////TIME//////////////////////////////////////////////////////////////////////////
		if(b_tm_1ms_flag==1)
		{
			b_tm_1ms_flag=0;
			u8_tm_1ms_cnt++;
			if(u8_tm_1ms_cnt>99)
			{
				u8_tm_1ms_cnt=0;
				b_tm_100ms_flag=1;
			}
			u8_tm_key1_1ms++;
			if(u16_tm_key_press_down_1ms<3100)						//按键按下计时
			{
				u16_tm_key_press_down_1ms++;	
			}
			u16_tm_ble_1ms++;
			if(u16_tm_ble_1ms>124)									//蓝牙闪烁刷新
			{
				u16_tm_ble_1ms=0;
				b_tm_ble_125ms_flag=1;
			}
			u16_tm_halt_1ms++;
			if(u16_tm_halt_1ms>HALT_START_TM)						//休眠计时
			{
				u16_tm_halt_1ms=0;
				b_halt_flag=1;
			}
			if(u8_ble_mode==ble_on_t)								//蓝牙状态检测消抖计时
			{
				if(BLE_STATUS==0)
				{
					u16_tm_ble_status_1ms++;	
				}
				else
				{
					u16_tm_ble_status_1ms=0;	
				}	
			}
			else if(u8_ble_mode==ble_connect_t)	
			{
				if(BLE_STATUS==1)
				{
					u16_tm_ble_status_1ms+=3;	
				}
				else
				{
					if(u16_tm_ble_status_1ms>0)
					{
						u16_tm_ble_status_1ms--;
					}
				}
			}
			else
			{
				u16_tm_ble_status_1ms=0;	
			}
		}
		if(b_tm_100ms_flag==1)
		{
			b_tm_100ms_flag=0;
			u16_tm_ble_connect_100ms++;
		}
////////////////////////////TIME//////////////////////////////////////////////////////////////////////////

		if(KEY!=b_key1)
		{
			if(u8_tm_key1_1ms>35)
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
		if(b_key_cont1==1&&b_key_trg1==1)
		{
			b_key_flag1=1;
		}
		if(b_key_cont1==0)									
		{
			if(b_key_flag1==1)
			{
				if(u8_loader_mode==ld_half_t)
				{
					u8_loader_mode=ld_full_t;
				}
				else if(u8_loader_mode==ld_full_t)
				{
					u8_loader_mode=ld_off_t;
				}
				else
				{
					u8_loader_mode=ld_half_t;
				}
			}
			b_key_flag1=0;
			u16_tm_key_press_down_1ms=0;
		}
		if(b_key_flag1==1&&u16_tm_key_press_down_1ms>3000)
		{
			b_key_flag1=0;	
			if(u8_ble_mode==ble_on_t||u8_ble_mode==ble_connect_t)
			{
				u8_ble_mode=ble_off_t;
			}
			else
			{
				u8_ble_mode=ble_on_t;	
			}
		}
		if(u8_loader_mode==ld_half_t)
		{
			LD_HALF
		}
		else if(u8_loader_mode==ld_full_t)
		{
			LD_FULL
		}
		else
		{
			LD_OFF
		}
		if(u8_ble_mode==ble_on_t)
		{
			if(u16_tm_ble_status_1ms>1000)
			{
				u16_tm_ble_status_1ms=0;
				u8_ble_mode=ble_connect_t;	
			}
		}
		else if(u8_ble_mode==ble_connect_t)
		{
			if(u16_tm_ble_status_1ms>1000)
			{
				u16_tm_ble_status_1ms=0;
				u8_ble_mode=ble_on_t;	
			}		
		}
		if(u8_ble_mode==ble_on_t)
		{
			BLE_ON
			if(b_tm_ble_125ms_flag==1)
			{
				b_tm_ble_125ms_flag=0;
				BLE_LED=~BLE_LED;
			}
			if(u16_tm_ble_connect_100ms>1200)
			{
				u16_tm_ble_connect_100ms=0;
				u8_ble_mode=ble_off_t;		
			}	
		}
		else if(u8_ble_mode==ble_connect_t)
		{
			u16_tm_ble_connect_100ms=0;
			BLE_CONNECT
			
		}
		else if(u8_ble_mode==ble_off_t)
		{
			u16_tm_ble_connect_100ms=0;
			BLE_OFF
			
		}		
		
////////////////////////////Halt//////////////////////////////////////////////////////////////////////////
		if(u8_loader_mode!=ld_off_t||u8_ble_mode!=ble_off_t)
		{
			u16_tm_halt_1ms=0;
		}
		if(b_halt_flag==1)
		{
			_intc0	= 0B00000000;		//<3>TB1E;<2>TB0E;<1>INT0E;<0>EMI
			_tbc	= 0B00000000;		//<7>TBON; <6>TBCK=fsys/4; <5:4>TB11-TB10; <2-0>TB02-TB00(fTB/2048 1ms)
			_pa		= 0B00000001;		//电平		1:高;   0:低
			_pac	= 0B00000001;		//输入输出	1:输入; 0:输出
			_pawu	= 0B00000001;		//唤醒		1:使能; 0:禁止
			_papu	= 0B00000001;		//上拉		1:使能; 0:禁止
			_pa		= 0B00000001;		//电平		1:高;   0:低
			_pb 	= 0B00000000;		//电平		1:高;	0:低
			_pbc	= 0B00000000;		//输入输出	1:输入; 0:输出
			_pbpu	= 0B00000000;		//上拉		1:使能; 0:禁止
			_pb 	= 0B00000000;		//电平		1:高;	0:低
			_pc 	= 0B00000000;		//电平		1:高;	0:低
			_pcc	= 0B00000000;		//输入输出	1:输入; 0:输出
			_pcpu	= 0B00000000;		//上拉		1:使能; 0:禁止
			_pc 	= 0B00000000;		//电平		1:高;	0:低	
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
			b_halt_flag=0;	
		}
	}
////////////////////////////Halt//////////////////////////////////////////////////////////////////////////
}

