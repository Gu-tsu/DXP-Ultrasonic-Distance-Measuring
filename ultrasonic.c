#include <reg52.h>	         //���õ�Ƭ��ͷ�ļ�
#define uchar unsigned char  //�޷����ַ��� �궨��	������Χ0~255
#define uint  unsigned int	 //�޷������� �궨��	������Χ0~65535
#include <intrins.h>


//����ܶ�ѡ����      0     1    2    3    4    5	6	  7	  8	   9	
uchar code smg_du[]={0x28,0xee,0x32,0xa2,0xe4,0xa1,0x21,0xea,0x20,0xa0};	 //����


uchar dis_smg[4]   ={0xff,0xff,0xff,0xff};

//�����λѡ����
sbit smg_we1 = P3^4;	    //�����λѡ����
sbit smg_we2 = P3^5;
sbit smg_we3 = P3^6;
sbit smg_we4 = P3^7;

sbit c_send   = P3^2;		//����������
sbit c_recive = P3^3;		//����������

sbit beep = P2^3;   //������IO�ڶ���
 uint flag_300ms = 1;

long distance;	        //����
uint set_d=50;	            //����
uchar flag_csb_juli;    //��������������
uint  flag_time0;       //�������涨ʱ��0��ʱ���

uchar menu_1;           //�˵���Ƶı���
 
/***********************1ms��ʱ����*****************************/
void delay_1ms(uint q)
{
	uint i,j;
	for(i=0;i<q;i++)
		for(j=0;j<120;j++);
}


/********************������������*****************/
uchar key_can;	 //����ֵ

void key()	    //������������
{
	static uchar key_new;  //key_new  ��������Ĺ��������������ּ���  
	key_can = 0;                   //����ֵ��ԭ
	P2 |= 0x07;
	if((P2 & 0x07) != 0x07)		//��������
	{
		delay_1ms(1);	     	//����������
		if(((P2 & 0x07) != 0x07) && (key_new == 1))
		{						//ȷ���ǰ�������
			key_new = 0;		//key_new = 0   ˵�������Ѱ���
			switch(P2 & 0x07)
			{
				case 0x06: key_can = 3; break;	   //�õ�k3��ֵ
				case 0x05: key_can = 2; break;	   //�õ�k2��ֵ
				case 0x03: key_can = 1; break;	   //�õ�k1��ֵ
			}
		}			
	}
	else 
		key_new = 1;	//key_new = 1   ˵�������Ѿ��ɿ���
}

/****************����������ʾ����***************/
void key_with()
{
	if(key_can == 1)		//���ü�
	{
		menu_1 ++;
		if(menu_1 >= 2)
		{
			menu_1 = 0;
 		}
 	}
	if(menu_1 == 1)			//���ñ���
	{
		if(key_can == 2)
		{
			set_d ++ ;		//��1
			if(set_d > 500)
				set_d = 500;
		}
		if(key_can == 3)
		{
			set_d -- ;		//��1
			if(set_d <= 1)
				set_d = 1;
		}
		dis_smg[0] = smg_du[set_d % 10];	           //ȡС����ʾ
		dis_smg[1] = smg_du[set_d / 10 % 10] ;         //ȡ��λ��ʾ
		dis_smg[2] = smg_du[set_d / 100 % 10] & 0xdf ; //ȡʮλ��ʾ
		dis_smg[3] = 0x60;	        //a
	}	
}  

/****************��������***************/
void clock_h_l()
{
 	if(distance <= set_d)      //�������õľ���ͻᱨ��
	{
 		beep = ~beep; 	  //����������	
 	}
	else 
	{
 		beep = 1;		//ȡ������
	}	
}

/***********************����λѡ����*****************************/
void smg_we_switch(uchar i)
{
	switch(i)
	{
		case 0: smg_we1 = 0;  smg_we2 = 1; smg_we3 = 1;  smg_we4 = 1; break;
		case 1: smg_we1 = 1;  smg_we2 = 0; smg_we3 = 1;  smg_we4 = 1; break;
		case 2: smg_we1 = 1;  smg_we2 = 1; smg_we3 = 0;  smg_we4 = 1; break;
		case 3: smg_we1 = 1;  smg_we2 = 1; smg_we3 = 1;  smg_we4 = 0; break;
	}	
}

/***********************������ʾ����*****************************/
void display()
{
	static uchar i;   
	i++;
	if(i >= 4)
		i = 0;	
	smg_we_switch(i);		 //λѡ
	P1 = dis_smg[i];		 //��ѡ	        
}

/******************С��ʱ����*****************/
void delay()
{
	_nop_(); 		           //ִ��һ��_nop_()ָ�����1us
	_nop_(); 
	_nop_(); 
	_nop_(); 
	_nop_(); 
	_nop_(); 
	_nop_(); 
	_nop_(); 
	_nop_();  
	_nop_(); 
}


/*********************������������*****************************/
void send_wave()
{
	c_send = 1;		           //10us�ĸߵ�ƽ���� 
	delay();
	c_send = 0;	 
	TH0 = 0;		          //����ʱ��0����
	TL0 = 0;
	TR0 = 0;				  //�ض�ʱ��0��ʱ
	while(!c_recive);		  //��c_reciveΪ��ʱ�ȴ�
	TR0=1;
	while(c_recive)		      //��c_reciveΪ1�������ȴ�
	{
		flag_time0 = TH0 * 256 + TL0;
		if((flag_time0 > 40000))      //������������������Χʱ����ʾ3��888
		{
			TR0 = 0;
			flag_csb_juli = 2;
			distance = 888;
			break ;		
		}
		else 
		{
			flag_csb_juli = 1;	
		}
	}
	if(flag_csb_juli == 1)
	{	
		TR0=0;							 //�ض�ʱ��0��ʱ
		distance =flag_time0;			 //������ʱ��0��ʱ��
		distance *= 0.017;               // 0.017 = 340M / 2 = 170M = 0.017M ���������
		if((distance > 600))				 //���� = �ٶ� * ʱ��
		{	
			distance = 888;				 //�������6m�ͳ��������������� 
		}
	}  
}


/*********************��ʱ��0����ʱ��1��ʼ��******************/
void time_init()	  
{
	EA  = 1;	 	  //�����ж�
	TMOD = 0X11;	  //��ʱ��0����ʱ��1������ʽ1
	ET0 = 0;		  //�ض�ʱ��0�ж� 
	TR0 = 1;		  //����ʱ��0��ʱ
	ET1 = 1;		  //����ʱ��1�ж� 
	TR1 = 1;		  //����ʱ��1��ʱ	
}



/***************������*****************/
void main()
{
	beep = 0;		 //������һ��   
	delay_1ms(150);
	P0 = P1 = P2 = P3 = 0xff;	   //��ʼ����Ƭ��IO��Ϊ�ߵ�ƽ
	time_init();	//��ʱ����ʼ������
	while(1)
	{	
		flag_300ms ++;	  //��1 
		if(flag_300ms >= 300)
		{		
			flag_300ms = 0;
			send_wave();	//����뺯��
			if(menu_1 == 0)
			{
				dis_smg[0] = smg_du[distance % 10];		 //��ʾ����
				dis_smg[1] = smg_du[distance / 10 % 10];
				dis_smg[2] = smg_du[distance / 100 % 10] & 0xdf; ;	
				dis_smg[3] = 0xff;	        //����ʾ
			}
			clock_h_l();     //��������
		}
		key();			     //��������
 		key_with();			 //����������
 		delay_1ms(1);		 //��ʱ1����
	}
}

/*********************��ʱ��1�жϷ������************************/
void time1_int() interrupt 3
{	
 	TH1 = 0xf8;
	TL1 = 0x30;     //2ms
	display();		//�������ʾ����
 }



