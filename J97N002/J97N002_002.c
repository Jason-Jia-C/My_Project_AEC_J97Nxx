/*
��Ŀ����: С���ӵ� +UV��
оƬ�ͺ�: HT66F002 �ڲ�8M ָ������4��Ƶ
����:
һ������
�������棭����UV����
�͵�ѹ��˸���ص�ǰ5���ӣ�1Hz��˸20�Ρ�
��λ�л���5���ٰ������صƣ����ƴ����书�ܡ�
		2		5		3		7
��		P-MOS	5711
����					5711
����UV			5711			P-MOS

��λ����
	         -------
	        |VDD VSS|
	 LD_TOP |PA6 PA0| LD_UV
	LD_OUT2 |PA5 PA1| KEY1
   LD2_MODE |PA7 PA2| LD_OUT1 ��
	         -------

*/

#include "HT66F002.h"

#define		ROM_Rd_Wt_EN	1
#define		ADC_CONVERT_EN	1

#define		bool	bit
#define		uint8	unsigned char
#define		uint16	unsigned int
#define		section0 

/////////////////////////////////////////////////////////////
#define		pin2	_14_6
#define		pin3	_14_5
#define		pin4	_14_7
#define		pin5	_14_2
#define		pin6	_14_1
#define		pin7	_14_0

//����
#define		KEY_IN			pin6			//����

//���
#define		LD_OUT1			pin5			//5711
#define		LD_OUT2			pin3			//5711
#define		LD3_EN			pin7			//����UV P-MOS
#define		LD1_EN			pin2			//���� P-MOS
#define		LD2_MODE		pin4

#define		KEY_DOWN		0
#define		KEY_UP			1
#define		KEY_SHORT_PRESS_DOWN	20		//20ms
#define		KEY_SHORT_PRESS_UP		50		//50ms
#define		KEY_LONG_PRESS_DOWN		250		//2000ms

#define		SW_PWM_OFF		0
#define		SW_PWM_HALF		132
#define		SW_PWM_FULL		256

#define		IDLE_CNT		10

/////////////////////////////////////////////////////////////
#pragma	rambank0

bool	b_tm_1ms_trig;

bool	b_tm_1ms_flag2;

uint8	u8_tm_20ms_cnt;
uint8	u8_tm_500ms_cnt;
bool	b_tm_500ms_flag;
uint8	u8_tm_5s_timeout;

uint8	u8_tm_idle_timeout;

uint8	u8_key_value;
uint8	u8_key_value_old;
uint8	u8_key_down_cnt;
uint8	u8_key_up_cnt;
bool	b_key_up_flag;

uint8	u8_loader_mode;
uint8	u8_loader_mode_eeprom;
uint8	u8_loader_mode_last;

bool	b_ld_out1;
bool	b_ld_out2;
bool	b_ld_out3;

//adc param
uint16	u16_adc_vdd_avg;
//adc param end

uint8	u8_adc_up_cnt;
uint8	u8_adc_down_cnt;
bool	b_adc_bat_level;
bool	b_adc_bat_level_new;
/*
 * ��ص�ѹ�ȼ��½��ط�����1
 * ��ص�ѹ�ȼ���������0
 * ����������0
 */
bool	b_adc_bat_fail;
bool	b_adc_bat_fail_1st;

uint8	u8_ld_flash_cnt;			//������˸ʱ��
bool	b_ld_flash_flag;
bool	b_ld_flash_en;

uint8	u8_ld_mode_change_cnt;

uint16	u16_sw_pwm_tel_app;

uint8	u8_hw_pwm_tel;			//Ӳ��PWMռ�ձ�
uint8	u8_hw_pwm_teh;

bool	b_ee_rd_en;
bool	b_ee_wt_en;

#pragma	norambank
/////////////////////////////////////////////////////////////
#pragma 	vector		ext_int		@ 0x04		//�ⲿ�ж�
#pragma 	vector		tb0_int		@ 0x08		//Time Base 0
#pragma 	vector		tb1_int		@ 0x0C		//Time Base 1
#pragma 	vector		mf0_int		@ 0x10 		//�๦��0 stm�ж�
#pragma		vector		eeprom_int	@ 0x14		//EEPROM
#pragma 	vector		adc_int		@ 0x18		//AD

/////////////////////////////////////////////////////////////

void	ext_int			(void){_inte = 0; _intf = 0;}
void	tb1_int			(void){_tb1e = 0;}
void	mf0_int			(void){_mf0e = 0; _mf0f = 0;}
void	eeprom_int		(void){_dee = 0;}
void    adc_int		    (void){_ade = 0;}


/*
 * time base 0
 */
void	tb0_int			(void)
{
	b_tm_1ms_trig = 1;
}


/////////////////////////////////////////////////////////////
void main()
{
	/////////////////////////////////////////////////////////////
//	ramclr();
	_bp=0;
	for(_mp0=0x40;_mp0<=0x7F;_mp0++)
	{
		_iar0=0;
	}
	/////////////////////////////////////////////////////////////
//	init();
	_rstc	= 0B01010101;		//��λѡ��Ĵ���

	_pasr	= 0B00000000;		//PA5,PA2,PA1,PA0����ѡ�� IO
	_pa		= 0B01000011;		//��ƽ		1:��;   0:��
	_pac	= 0B00000010;		//�������	1:����; 0:���
	_pawu	= 0B00000010;		//����		1:ʹ��; 0:��ֹ
	_papu	= 0B00000010;		//����		1:ʹ��; 0:��ֹ
	_pa		= 0B00000011;		//��ƽ		1:��;   0:��
	_ifs0	= 0B01000000;		//<5>STCK0PS; <4>STP0IPS; <1:0>INT��λ
	
	_sadc0	= 0B00000000;    	//<7>START; <6>ADBZ; <5>ENADC; <4>ADRFS; <1:0>CH
	_sadc1	= 0B00000100;		//<7-5>�����ź�ѡ��; <2-0>ʱ��Դ
	_sadc2	= 0B00000000;		//<7>ENOPA; <6>VBGEN; <3-0>�ο���ѹ

	_smod	= 0B00000001;		//<7-5>��Ƶѡ��; <3>LTO; <2>HTO; <1>IDLEN; <0>HLCLK
	_smod1	= 0B00000000;		//<7>FSYSON; <3>RSTF; <2>LVRF; <0>WRF
	_tbc	= 0B11000011;		//<7>TBON; <6>TBCK=fsys/4; <5:4>TB1=4096/fTB; <2-0>TB0=2048/fTB 
	
	/*
	PTnCK=fSYS:		001 7.8K
	PTnCK=fSYS/4:	000 2K
	PTnCK=fH/16:	010 500Hz
	PTnCK=fH/64:	011 128Hz
	PTnCK=fTBC:		100 26.6Hz;
	*/
	_stm0c0	= 0B00000000;		//<7>ST0PAU; <6-4>fSYS; <3>ST0ON; <2-0>CCRP STM[9-7]�Ƚ�ƥ������256
	_stm0c1	= 0B00000000;		//<7:6>����ģʽ; <5:4>�������; <3>�������λ; <2>�������; <1>����/ռ�ձ�; <0>P/Aƥ��
	
	_mfi0	= 0B00000000;		//<1>STMA0E; <0>STMP0E
	_integ	= 0B00000000;		//<1:0>�ⲿ�жϱ���ѡ��λ
	_intc1	= 0B00000000;		//<2>ADE; <1>DEE; <0>MF0E
	_intc0	= 0B00000101;		//<3>TB1E; <2>TB0E; <1>INTE; <0>EMI
	
	_wdtc	= 0B01010111;		//<7-3>WE; <2-0>WS
	/////////////////////////////////////////////////////////////
	u8_tm_idle_timeout = IDLE_CNT;
	
	u8_loader_mode = 0;
	u8_loader_mode_last = 0;
	b_ld_out1 = 0;
	b_ld_out2 = 0;
	b_ld_out3 = 0;
	u8_hw_pwm_tel = 0;
	u8_hw_pwm_teh = 0;
	u16_sw_pwm_tel_app = 0;
	b_ee_rd_en = 1;
	
	u8_key_value = 0;
	u8_key_value_old = 0;
	
	b_adc_bat_level = 1;
	b_adc_bat_level_new = 1;
	b_ld_flash_flag = 0;
	u8_ld_flash_cnt = 0;
	u16_adc_vdd_avg = 0xFFFF;
	while(1)
	{
		_clrwdt();				//���1������һ��
		/////////////////////////////////////////////////////////////
		if(b_tm_1ms_trig == 1)
		{
			b_tm_1ms_trig = 0;

			b_tm_1ms_flag2 = 1;
			
			u8_key_down_cnt++;
			u8_key_up_cnt++;
			
			u8_tm_20ms_cnt++;
			if(u8_tm_20ms_cnt > 19)				//20ms
			{
				u8_tm_20ms_cnt = 0;
				
				u8_tm_500ms_cnt++;
				if(u8_tm_500ms_cnt > 24)		//500ms
				{
					u8_tm_500ms_cnt = 0;
					
					if(u8_tm_5s_timeout)
						u8_tm_5s_timeout--;
									
					if(u8_tm_idle_timeout)
						u8_tm_idle_timeout--;

					if(b_adc_bat_level == 0)
					{
						//��ѹ��
						u8_ld_mode_change_cnt++;

						if((u8_ld_mode_change_cnt > 3) && (u8_ld_mode_change_cnt < 6))
						{
							u8_ld_mode_change_cnt = 10;

							u8_loader_mode_eeprom = 0;
							b_ee_wt_en = 1;
						}
						else if(u8_ld_mode_change_cnt > 10)
						{
							u8_ld_mode_change_cnt = 10;
						}
					}
					else
					{
						u8_ld_mode_change_cnt = 0;
					}
					
					//��ص�������������
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
				}
			}
			
			if(b_ld_flash_flag == 0)
			{
				if(b_ld_out1 == 1)
				{
					LD1_EN = 0;
				}
				else
				{
					LD1_EN = 1;
				}
				if(b_ld_out3 == 1)
				{
					LD3_EN = 0;
				}
				else
				{
					LD3_EN = 1;
				}
				
				if(b_ld_out2 == 1)
				{
					LD_OUT2=1;
				}
				else
				{
					LD_OUT2=0;
				}
				if((b_ld_out1 == 1) || (b_ld_out3 == 1))
				{
					LD_OUT1 = 1;
				}
				else
				{
					LD_OUT1 = 0;
				}
			}
			else
			{
				LD_OUT1 = 0;
				LD_OUT2 = 0;
				LD1_EN = 1;
				LD3_EN = 1;
			}
		}
		/////////////////////////////////////////////////////////////
#if(ADC_CONVERT_EN == 1)
		
		_sadc0 = 0B01110000;	//<7>START; <6>ADBZ; <5>ENADC; <4>ADRFS; <1:0>CH
		_sadc1 = 0B01100010;	//<7-5>�����ź�ѡ��; <2-0>ʱ��Դ
		_sadc2 = 0B11001010;	//<7>ENOPA; <6>VBGEN; <3-0>�ο���ѹ
		_start = 0;
		_start = 1;
		_start = 0;
		while(_adbz);
		u16_adc_vdd_avg = ((uint16)_sadoh<<8)+(_sadol);
		
		if(u16_adc_vdd_avg > 1860)
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
		else if(u16_adc_vdd_avg > 1560)
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
		
#endif

		/////////////////////////////////////////////////////////////
		//��������
		if(KEY_IN == KEY_DOWN)
		{
			if(u8_key_down_cnt >= KEY_LONG_PRESS_DOWN)
			{
				u8_key_down_cnt = KEY_SHORT_PRESS_UP+1;
			}
			else if((u8_key_down_cnt >= KEY_SHORT_PRESS_DOWN) && (u8_key_down_cnt < KEY_SHORT_PRESS_UP))
			{
				u8_key_down_cnt = KEY_SHORT_PRESS_UP+1;
				if(b_key_up_flag)
				{
					b_key_up_flag = 0;
					u8_key_up_cnt = 0;
					u8_key_value = 0x01;
					u8_key_value_old = 0x10;
				}
			}
		}
		else
		{
			u8_key_down_cnt = 0;
			u8_key_value = 0x00;
			if(u8_key_up_cnt >= KEY_SHORT_PRESS_UP)
			{
				b_key_up_flag = 1;
				u8_key_up_cnt = KEY_SHORT_PRESS_UP;
			}
		}
		
		if(u8_key_value_old != u8_key_value)
		{
			u8_key_value_old = u8_key_value;
			if(u8_key_value == 0x01)
			{
				u8_key_value = 0;
				//��������һ��
				if(u8_loader_mode == 1)
				{
					if(u8_tm_5s_timeout > 0)
					{
						u8_loader_mode = 2; 	//����ǿ
						u8_tm_5s_timeout = 10;
					}
					else
					{
						u8_loader_mode_last = u8_loader_mode;
						u8_loader_mode = 0; 	//��
						b_adc_bat_fail = 0;
					}
				}
				else if(u8_loader_mode == 2)
				{
					if(u8_tm_5s_timeout > 0)
					{
						u8_loader_mode = 3;		//����ǿ
						u8_tm_5s_timeout = 10;
					}
					else
					{
						u8_loader_mode_last = u8_loader_mode;
						u8_loader_mode = 0;		//��
						b_adc_bat_fail = 0;
					}
				}
				else if(u8_loader_mode == 3)
				{
					if(u8_tm_5s_timeout > 0)
					{
						u8_loader_mode = 4;		//UV
						u8_tm_5s_timeout = 10;
					}
					else
					{
						u8_loader_mode_last = u8_loader_mode;
						u8_loader_mode = 0;		//��
						b_adc_bat_fail = 0;
					}
				}
				else if(u8_loader_mode == 4)
				{
					if(u8_tm_5s_timeout > 0)
					{
						u8_loader_mode = 0; 	//��
						u8_loader_mode_last = u8_loader_mode;
						u8_tm_5s_timeout = 10;
						b_adc_bat_fail = 0;
					}
					else
					{
						u8_loader_mode_last = u8_loader_mode;
						u8_loader_mode = 0; 	//��
						b_adc_bat_fail = 0;
					}
				}
				else
				{
					if(u8_loader_mode_last == 0)
					{
						u8_loader_mode = 1; 	//����
					}
					else
					{
						u8_loader_mode = u8_loader_mode_last;
					}
					u8_tm_5s_timeout = 10;
				}
				
				u8_loader_mode_eeprom = u8_loader_mode;
				b_ee_wt_en = 1;
			}
		}
		
		////////////////////////////////////////////////////////////*/
		//���⹦��
		if(u8_loader_mode == 1)					//����
		{
			b_ld_out1 = 1;
			b_ld_out2 = 0;
			b_ld_out3 = 0;
			u8_tm_idle_timeout = IDLE_CNT;
		}
		else if(u8_loader_mode == 2)			//������
		{
			b_ld_out1 = 0;
			b_ld_out2 = 1;	
			_pac7	  =	1;
			b_ld_out3 = 0;
			u8_tm_idle_timeout = IDLE_CNT;
		}
		else if(u8_loader_mode == 3)			//����ǿ
		{
			b_ld_out1 = 0;
			b_ld_out2 = 1;
			_pac7	  = 0;
			_pa7	  = 0;
			b_ld_out3 = 0;
			u8_tm_idle_timeout = IDLE_CNT;
		}
		else if(u8_loader_mode == 4)			//����UV
		{
			b_ld_out1 = 0;
			b_ld_out2 = 0;
			b_ld_out3 = 1;
			u8_tm_idle_timeout = IDLE_CNT;
		}
		else if(u8_loader_mode == 0)			//��
		{
			b_ld_out1 = 0;
			b_ld_out2 = 0;
			u16_sw_pwm_tel_app = 0;
			b_ld_out3 = 0;
		}

		/////////////////////////////////////////////////////////////
		//���߻���
		if(u8_tm_idle_timeout == 0)
		{
			//*
			//׼������
			_intc0	= 0B00000000;
			_intc1	= 0B00000000;
			
			_pa 	= 0B10000011;		//��ƽ		1:��;	0:��
			_pac	= 0B00000010;		//�������	1:����; 0:���
			_pawu	= 0B00000010;		//����		1:ʹ��; 0:��ֹ
			_papu	= 0B00000010;		//����		1:ʹ��; 0:��ֹ
			_pa 	= 0B10000011;		//��ƽ		1:��;	0:��
			_sadc0	= 0B00000000;
			_sadc1	= 0B00000000;
			_sadc2	= 0B00000000;
			
			_clrwdt();
			_wdtc	= 0B10101111;		//<7-3>WE; <2-0>WS
			
			_nop();
			_nop();
			_halt();
			_nop();
			_nop();
			
			_wdtc	= 0B01010111;
			_clrwdt();
			
			_mfi0	= 0B00000000;		//<1>STMA0E; <0>STMP0E
			_integ	= 0B00000000;		//<1:0>�ⲿ�жϱ���ѡ��λ
			_intc1	= 0B00000000;		//<2>ADE; <1>DEE; <0>MF0E
			_intc0	= 0B00000101;		//<3>TB1E; <2>TB0E; <1>INTE; <0>EMI
			
			u8_tm_idle_timeout = IDLE_CNT;
			b_key_up_flag = 1;
			//*/
		}
		/////////////////////////////////////////////////////////////
#if (ROM_Rd_Wt_EN == 1)
		//��EEPROM����
		if(b_ee_rd_en)
		{
			b_ee_rd_en = 0;

			_eea = 0;
			_mp1 = 0x40;
			_bp=1;
			_iar1 |= 0x02;
			_iar1 |= 0x01;
			while((_iar1&0x01) == 0x01)
			{
				_clrwdt();
			}
			_iar1=0;
			_bp=0;
			u8_loader_mode = _eed;
			
			u8_tm_idle_timeout = IDLE_CNT;
		}
		/////////////////////////////////////////////////////////////
		//дEEPROM����
		if(b_ee_wt_en)
		{
			b_ee_wt_en = 0;
			_eea   = 0;
			_eed   = u8_loader_mode_eeprom;
			_mp1 = 0x40;
			_bp=1;
			_emi = 0;			//д����ִ��ǰ���ж�λ����
			_iar1 |= 0x08;
			_iar1 |= 0x04;
			_emi = 1;			//д����ִ�к����ж�λʹ��
			while((_iar1&0x04) == 0x04)
			{
				_clrwdt();
			}
			_iar1=0;
			_bp=0;
			u8_tm_idle_timeout = IDLE_CNT;
		}
#endif
		/////////////////////////////////////////////////////////////
	}
}


