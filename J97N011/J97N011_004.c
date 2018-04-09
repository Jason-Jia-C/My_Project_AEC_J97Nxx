/*
��Ŀ����:ATDͷ��(��·)
	�༭����:2018.4.6
	����	:JasonJia
	PCB���� :��Ϊ
	оƬ��װ:HT66F004 NSOP_16
	���ʱ��:1ms
	��������:��-ǿ-��+��Ӧ(����2S�򿪸�Ӧ����Ӧֻ�л���ǰ��λ�Ŀ��أ����漰��λ�л�)
	�޸ļ�¼:
			2018��4��6�ձ༭��һ��



��λ����
	         ________________________
            |VSS                  VDD|
    IFR_LED |PC0         PB0/INT0/AN0| LD1_SW
    LD_HALF |PC1         PB1/INT1/AN1| LD2_SW
IFR_RECEIVE |PC2        PB2/PTCK0/AN2| 
   IFR_SEND |PA0/PTP0   PA4/PTCK1/AN3| 
            |PA1/PTP0I   PA5/AN4/VREF| KEY
            |PA2        PA6/AN5/VREFO| 
            |PA3/PTP1I   PA7/PTP1/AN6| LD_EN
             ------------------------

*/	
#include "HT66F004.h"
#define bool	bit
#define uint8 	unsigned char 
#define uint16	unsigned int 

#define IFR_LED					_pc0
#define LD_HALF					_pc1
#define IFR_RECEIVE				_pc2
#define IFR_SEND				_pa0
#define KEY						_pa5
#define LD_EN					_pa7
#define LD1_SW					_pb0
#define LD2_SW					_pb1

#define LD1_HALF				{_pcc1=1;_pcpu1=0;LD_EN=1;LD1_SW=1;LD2_SW=0;}
#define LD1_FULL				{_pcpu1=0;_pcc1=0;LD_EN=1;LD1_SW=1;LD2_SW=0;}
#define LD2_HALF				{_pcc1=1;_pcpu1=0;LD_EN=1;LD1_SW=0;LD2_SW=1;} 
#define LD2_FULL				{_pcpu1=0;_pcc1=0;LD_EN=1;LD1_SW=0;LD2_SW=1;}
#define LD_OFF					{_pcc1=1;_pcpu1=0;LD_EN=0;LD1_SW=0;LD2_SW=0;}
#define IFR_ON					{_pasr=0x01;_ptm0rpl=0xD3;_ptm0rph=0x00;_ptm0al=0x69;_ptm0ah=0x00;_ptm0c0=0b00011000;_ptm0c1=0b10101000;IFR_LED=1;}		//�����Ե���38K�ķ�������LEDָʾ��
#define IFR_OFF					{_pasr=0x00;_ptm0rpl=0x00;_ptm0rph=0x00;_ptm0al=0x00;_ptm0ah=0x00;_ptm0c0=0b00000000;_ptm0c1=0b00000000;IFR_LED=0;}		//�رշ�����LEDָʾ��

#define HALT_START_TM			5000			//���߼�ʱ5S
#define KEY_PRESS_DOWN_TM		2000			//��������2S

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

uint8	u8_loader_mode;
uint8	u8_loader_mode1;

//IFR	
bool	b_tm_ifr_1ms_flag;
uint8	u8_ifr_mode;
uint8	u8_ifr_step;
uint16	u16_tm_ifr_1ms;



#pragma norambank

enum loader_mode_t
{
	ld1_half_t,
	ld1_full_t,
	ld2_half_t,
	ld2_full_t,
	ld_off_t,
};
enum ifr_status_t
{
	ifr_on_t,
	ifr_off_t,
};

/////////////////////////////////////////////////////////////
#pragma 	vector		ext0_int	@ 0x04		//�ⲿ�ж�0
#pragma 	vector		tb0_int		@ 0x08		//Time Base 0
#pragma 	vector		tb1_int		@ 0x0C		//Time Base 1
#pragma 	vector		mf0_int		@ 0x10 		//�๦��0 stm�ж�
#pragma		vector		eeprom_int	@ 0x14		//EEPROM
#pragma 	vector		adc_int		@ 0x18		//AD
#pragma 	vector		ext1_int	@ 0x1C		//�ⲿ�ж�1
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
void ifr_deal()
{
	if(u8_ifr_mode==ifr_on_t)
	{
		if((u8_loader_mode!=ld_off_t)||(u8_loader_mode1!=ld_off_t))
		{
			if(b_tm_ifr_1ms_flag==1)
			{
				b_tm_ifr_1ms_flag=0;
				if(u8_ifr_step==1)
				{
					if(IFR_RECEIVE==1)
					{
						u16_tm_ifr_1ms++;	
					}
					else
					{
						if(u16_tm_ifr_1ms>20)						//�������źŴ���20ms
						{
							u8_ifr_step=2;						//��״̬�����ı�������һ��
						}
						u16_tm_ifr_1ms=0;	
					}
					
				}
				else if(u8_ifr_step==2)
				{
					if(IFR_RECEIVE==0)
					{
						u16_tm_ifr_1ms+=5;		
					}
					else
					{
						if(u16_tm_ifr_1ms>0)					//�и����ź�ʱ���ص�һ��
						{
							u16_tm_ifr_1ms--;	
						}
						else
						{
							u16_tm_ifr_1ms=0;
							u8_ifr_step=1;
							
						}
					}
					if(u16_tm_ifr_1ms>30)						//���źŴ���6ms
					{
//						u16_tm_ifr_1ms=0;	
						u8_ifr_step=3;
					}
				}
				else if(u8_ifr_step==3)
				{
					if(IFR_RECEIVE==0)
					{
						u16_tm_ifr_1ms++;		
					}
					else
					{
						if(u16_tm_ifr_1ms>0)
						{
							u16_tm_ifr_1ms--;
						}
						else
						{
							u8_ifr_step=4;
						}
					}
					if(u16_tm_ifr_1ms>200)						//�����Ԥ��
					{
						u16_tm_ifr_1ms=0;
						u8_ifr_step=1;
					}
				}
				else if(u8_ifr_step==4)
				{
					u8_ifr_step=1;
					if(u8_loader_mode1==ld_off_t)
					{
						u8_loader_mode1=u8_loader_mode;
						u8_loader_mode=ld_off_t;
					}
					else
					{
						u8_loader_mode=u8_loader_mode1;
						u8_loader_mode1=ld_off_t;
					}
				}
			}
		}
	}
}
void main()
{
	_bp=0;
	for(_mp0=0x40;_mp0<=0x9F;_mp0++)
	{
		_iar0=0;
	}
	u8_loader_mode=ld_off_t;
	u8_loader_mode1=ld_off_t;
	u8_ifr_mode=ifr_off_t;
	
	_rstc	= 0B01010101;		//��λѡ��Ĵ���
	_pasr	= 0B00000000;		//<7:6>PA7;<5:4>PA6;<3:2>PA5;<1>PA4;<0>PA0
	_pbsr	= 0B00000000;		//<5>PB5;<4>PB4;<3>PB3;<2>PB2;<1>PB1;<0>PB0
	_scomc	= 0B00000000;		//<6:5>ƫѹ����;<4>SCOMEN;<3>COM3EN;<2>COM2EN;<1>COM1EN;<0>COM0EN	
	
	_pa		= 0B00100000;		//��ƽ		1:��;   0:��
	_pac	= 0B00100000;		//�������	1:����; 0:���
	_pawu	= 0B00100000;		//����		1:ʹ��; 0:��ֹ
	_papu	= 0B00100000;		//����		1:ʹ��; 0:��ֹ
	_pa		= 0B00100000;		//��ƽ		1:��;   0:��
	_pb		= 0B00000000;		//��ƽ		1:��;   0:��
	_pbc	= 0B00000000;		//�������	1:����; 0:���
	_pbpu	= 0B00000000;		//����		1:ʹ��; 0:��ֹ
	_pb		= 0B00000000;		//��ƽ		1:��;   0:��
	_pc		= 0B00000000;		//��ƽ		1:��;   0:��
	_pcc	= 0B00000110;		//�������	1:����; 0:���
	_pcpu	= 0B00000000;		//����		1:ʹ��; 0:��ֹ
	_pc		= 0B00000000;		//��ƽ		1:��;   0:��
	
	_sadc0	= 0B00000000;    	//<7>START; <6>ADBZ; <5>ENADC; <4>ADRFS; <2:0>CH
	_sadc1	= 0B00000000;		//<7-5>�����ź�ѡ��; <2-0>ʱ��Դ
	_sadc2	= 0B00000000;		//<7>ENOPA; <6>VBGEN; <3-0>�ο���ѹ
	_smod	= 0B00000001;		//<7-5>��Ƶѡ��; <3>LTO; <2>HTO; <1>IDLEN; <0>HLCLK
	_smod1	= 0B00000000;		//<7>FSYSON; <3>RSTF; <2>LVRF; <0>WRF
	_ptm0c0	= 0B00000000;		//<7>PT0PAU; <6-4>PT0CK; <3>PT0ON;
	_ptm0c1	= 0B00000000;		//<7:6>����ģʽ; <5:4>�������; <3>PT0OC; <2>�������; <1>��׽����Դ; <0>P/Aƥ��
	_ptm1c0	= 0B00000000;		//<7>PT1PAU; <6-4>PT1CK; <3>PT1ON;
	_ptm1c1	= 0B00000000;		//<7:6>����ģʽ; <5:4>�������; <3>PT1OC; <2>�������; <1>��׽����Դ; <0>P/Aƥ��
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
			b_tm_ifr_1ms_flag=1;
			u8_tm_key1_1ms++;
			if(b_key_flag1==1)						//�������¼�ʱ
			{
				u16_tm_key_press_down_1ms++;	
			}
			u16_tm_halt_1ms++;
			if(u16_tm_halt_1ms>HALT_START_TM)						//���߼�ʱ
			{
				u16_tm_halt_1ms=0;
				b_halt_flag=1;
			}		
		}
		if(b_tm_100ms_flag==1)
		{
			b_tm_100ms_flag=0;
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
			u16_tm_key_press_down_1ms=0;
			if(b_key_flag1==1)
			{
				b_key_flag1=0;
				if(u8_loader_mode==ld1_half_t)
				{	
					u8_loader_mode=ld1_full_t;
				}
				else if(u8_loader_mode==ld1_full_t)
				{
					u8_loader_mode=ld_off_t;
				}
				else
				{
					if(u8_loader_mode1==ld_off_t)
					{
						u8_loader_mode=ld1_half_t;		
					}
					else
					{
						if(u8_loader_mode1==ld1_half_t)
						{
							u8_loader_mode=ld1_full_t;
						}
						else
						{
							u8_loader_mode=ld1_half_t;
						}
						u8_loader_mode1=ld_off_t;
					}
				}
			}	
		}
		if(u16_tm_key_press_down_1ms>KEY_PRESS_DOWN_TM)					//������������2S
		{
			b_key_flag1=0;
			u16_tm_key_press_down_1ms=0;			
			u8_ifr_step=1;
			u16_tm_ifr_1ms=0;
			if(u8_ifr_mode==ifr_off_t)
			{
				u8_ifr_mode=ifr_on_t;
				IFR_ON
			}
			else
			{
				u8_ifr_mode=ifr_off_t;
				IFR_OFF
			}	
		}
		ifr_deal();
		if(u8_loader_mode==ld1_full_t)
		{
			LD1_FULL
		}
		else if(u8_loader_mode==ld1_half_t)
		{
			LD1_HALF
		}
		else if(u8_loader_mode==ld2_full_t)
		{
			LD2_FULL
		}
		else if(u8_loader_mode==ld2_half_t)
		{
			LD2_HALF
		}
		else
		{
			LD_OFF
		}
////////////////////////////Halt//////////////////////////////////////////////////////////////////////////
		if((u8_loader_mode!=ld_off_t)||(u8_ifr_mode!=ifr_off_t))
		{
			u16_tm_halt_1ms=0;
		}
		if(b_halt_flag==1)
		{
			_intc0	= 0B00000000;		//<3>TB1E;<2>TB0E;<1>INT0E;<0>EMI
			_tbc	= 0B00000000;		//<7>TBON; <6>TBCK=fsys/4; <5:4>TB11-TB10; <2-0>TB02-TB00(fTB/2048 1ms)
			_pa 	= 0B00100000;		//��ƽ		1:��;	0:��
			_pac	= 0B00100000;		//�������	1:����; 0:���
			_pawu	= 0B00100000;		//����		1:ʹ��; 0:��ֹ
			_papu	= 0B00100000;		//����		1:ʹ��; 0:��ֹ
			_pa 	= 0B00100000;		//��ƽ		1:��;	0:��
			_pb 	= 0B00000000;		//��ƽ		1:��;	0:��
			_pbc	= 0B00000000;		//�������	1:����; 0:���
			_pbpu	= 0B00000000;		//����		1:ʹ��; 0:��ֹ
			_pb 	= 0B00000000;		//��ƽ		1:��;	0:��
			_pc 	= 0B00000000;		//��ƽ		1:��;	0:��
			_pcc	= 0B00000110;		//�������	1:����; 0:���
			_pcpu	= 0B00000000;		//����		1:ʹ��; 0:��ֹ
			_pc 	= 0B00000000;		//��ƽ		1:��;	0:��
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


