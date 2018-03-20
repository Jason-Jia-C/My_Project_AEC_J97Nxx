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
    LED_BLE |PC0         PB0/INT0/AN0| 
       		|PC1         PB1/INT1/AN1| 
     		|PC2        PB2/PTCK0/AN2| 
    KEY_BLE	|PA0/PTP0   PA4/PTCK1/AN3| CE2
        	|PA1/PTP0I   PA5/AN4/VREF| CE1
    KEY_LED |PA2        PA6/AN5/VREFO| BLE_B
         	|PA3/PTP1I   PA7/PTP1/AN6| BLE_S
             ------------------------
#define LED_BLE		_pc0
#define KEY_BLE 	_pa0
#define KEY_COB		_pa2
#define BLE_S		_pa7
#define BLE_B		_pa6
#define COB_CE1		_pa5
#define COB_CE2		_pa4

*/	
#include "HT66F004.h"
#define bool	bit
#define uint8 	unsigned char 
#define uint16	unsigned int 
	
#define LED_BLE		_pc0
#define KEY_BLE 	_pa0
#define KEY_COB		_pa2
#define COB_CE2		_pa4
#define COB_CE1		_pa5
#define BLE_B		_pa6
#define BLE_S		_pa7

#define HALT_START_TM			200			//休眠计时，25ms为单位			5S

#pragma rambank0

bool b_halt_flag;
uint8 u8_tm_halt_25ms;

bool b_tm_1ms_flag;
bool b_tm_25ms_flag;
bool b_tm_500ms_flag;

uint8 u8_tm_1ms;	
uint8 u8_tm_25ms;

bool b_key1;
bool b_key_trg1;
bool b_key_date1;
bool b_key_cont1;

bool b_key2;
bool b_key_trg2;
bool b_key_date2;
bool b_key_cont2;

uint8 u8_tm_key1_1ms;
uint8 u8_tm_key2_1ms;

uint8 u8_led_mode;

bool b_ble_flag;
bool b_ble_status_flag;
uint8 u8_tm_ble_status_1ms;
uint8 u8_tm_ble_status_con_1ms;	
#pragma norambank

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
	_rstc	= 0b01010101;
	_pa		= 0b00000101;	
	_pac	= 0b10000101;	
	_papu	= 0b00000101;	
	_pawu	= 0b00000101;
	_pasr	= 0b00000000;	
	_pa		= 0b00000101;	
	_pb		= 0b00000000;	
	_pbc	= 0b00000000;			
	_pbpu	= 0b00000000;	
	_pbsr	= 0b00000000;	
	_pb		= 0b00000000;	
	_pc		= 0b00000000;
	_pcc	= 0b00000000;
	_pcpu	= 0b00000000;
	_pc		= 0b00000000;
	_smod	= 0b00000001; 	
	_smod1	= 0b00000000;	
	_sadc0	= 0B00000000;
	_sadc1	= 0B00000000;
	_sadc2	= 0B00000000;	
	_ptm0c0	= 0b00000000;	
	_ptm0c1 = 0b00000000;
	_ptm1c0	= 0b00000000;	
	_ptm1c1 = 0b00000000;
	_integ	= 0b00000000;
	_intc1	= 0b00000000;	
	_mfi0	= 0b00000000;
	_tbc	= 0B11000011; 
	_intc0	= 0b00000101; 	
	_wdtc	= 0b01010111; 
	_clrwdt();
	while(1)
	{
		_clrwdt();
////////////////////////////TIME//////////////////////////////////////////////////////////////////////////
		if(b_tm_1ms_flag==1)
		{
			u8_tm_1ms++;
			if(u8_tm_1ms>=25)
			{
				u8_tm_1ms=0;
				u8_tm_25ms++;
				b_tm_25ms_flag=1;
				if(u8_tm_25ms>19)
				{
					u8_tm_25ms=0;
					b_tm_500ms_flag=1;
				}
			}
		}
		if(b_tm_1ms_flag==1)
		{
			b_tm_1ms_flag=0;
			u8_tm_key1_1ms++;	
			u8_tm_key2_1ms++;	
			if(b_ble_flag==1)
			{
				if(BLE_S==0)
				{
					if(b_ble_status_flag==0)
					{
						u8_tm_ble_status_1ms++;
						u8_tm_ble_status_con_1ms=0;
					}
				}
				else
				{
					u8_tm_ble_status_1ms=0;
					u8_tm_ble_status_con_1ms++;
				}
			}
			else
			{
				u8_tm_ble_status_1ms=0;
				u8_tm_ble_status_con_1ms=0;
			}
		}
		if(b_tm_25ms_flag==1)
		{
			b_tm_25ms_flag=0;
			u8_tm_halt_25ms++;		
		}
		if(b_tm_500ms_flag==1)
		{
			b_tm_500ms_flag=0;
			if(b_ble_flag==1&&b_ble_status_flag==0)
			{
				LED_BLE=~LED_BLE;
			}
		}
////////////////////////////TIME//////////////////////////////////////////////////////////////////////////

////////////////////////////KEY&Contrl//////////////////////////////////////////////////////////////////////////
		if(KEY_COB!=b_key1)
		{
			if(u8_tm_key1_1ms>35)
			{
				b_key1=KEY_COB;
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
		if(KEY_BLE!=b_key2)
		{
			if(u8_tm_key2_1ms>35)
			{
				b_key2=KEY_BLE;
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
			u8_led_mode++;
			if(u8_led_mode>2)
			{
				u8_led_mode=0;
			}
		}
		if(b_key_cont2==1&&b_key_trg2==1)
		{
			b_ble_flag=~b_ble_flag;
		}
		if(u8_led_mode==1)
		{
			COB_CE1=1;
			COB_CE2=0;
		}
		else if(u8_led_mode==2)
		{
			COB_CE1=1;
			COB_CE2=1;
		}
		else
		{
			COB_CE1=0;
			COB_CE2=0;
		}

		if(b_ble_flag==1)
		{
			BLE_B=1;
			if(b_ble_status_flag==1)
			{
				LED_BLE=1;
			}
		}
		else
		{
			BLE_B=0;
			LED_BLE=0;
			b_ble_status_flag=0;
		}
		if(u8_tm_ble_status_1ms>150)
		{
			b_ble_status_flag=1;
			u8_tm_ble_status_1ms=0;
		}
		if(u8_tm_ble_status_con_1ms>250)
		{
			b_ble_status_flag=0;
			u8_tm_ble_status_con_1ms=0;
		}
////////////////////////////KEY&Contrl//////////////////////////////////////////////////////////////////////////

////////////////////////////Halt//////////////////////////////////////////////////////////////////////////
		if(u8_led_mode!=0||b_ble_flag==1)
		{
			u8_tm_halt_25ms=0;
		}
		if(u8_tm_halt_25ms>=HALT_START_TM)
		{
			u8_tm_halt_25ms=0;
			b_halt_flag=1;
		}
		if(b_halt_flag==1)
		{
			_intc0	= 0b00000000; 	//中断设置
			_tbc	= 0B00000000;  	//时基设置  tb0  976hz   1ms  
			_clrwdt();
			_wdtc	= 0b10101000; 
			_pa		= 0b00000101;	
			_pac	= 0b00000101;	
			_papu	= 0b00000101;	
			_pawu	= 0b00000101;
			_pasr	= 0b00000000;	
			_pa		= 0b00000101;	
			_nop();
			_nop();
			_halt();
			_nop();
			_nop();
			_pa		= 0b00000101;	
			_pac	= 0b10000101;	
			_papu	= 0b00000101;	
			_pawu	= 0b00000101;
			_pasr	= 0b00000000;	
			_pa		= 0b00000101;
			_tbc	= 0B11000011;  	//时基设置  tb0  976hz   1ms  
			_intc0	= 0b00000101; 	//中断设置
			_wdtc	= 0b01010111; 
			_clrwdt();
			b_halt_flag=0;	
		}
	}
////////////////////////////Halt//////////////////////////////////////////////////////////////////////////
}

