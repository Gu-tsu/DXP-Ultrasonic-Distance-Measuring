#include <reg52.h>	         //调用单片机头文件
#define uchar unsigned char  //无符号字符型 宏定义	变量范围0~255
#define uint  unsigned int	 //无符号整型 宏定义	变量范围0~65535
#include <intrins.h>


//数码管段选定义      0     1    2    3    4    5	6	  7	  8	   9	
uchar code smg_du[]={0x28,0xee,0x32,0xa2,0xe4,0xa1,0x21,0xea,0x20,0xa0};	 //断码


uchar dis_smg[4]   ={0xff,0xff,0xff,0xff};

//数码管位选定义
sbit smg_we1 = P3^4;	    //数码管位选定义
sbit smg_we2 = P3^5;
sbit smg_we3 = P3^6;
sbit smg_we4 = P3^7;

sbit c_send   = P3^2;		//超声波发射
sbit c_recive = P3^3;		//超声波接收

sbit beep = P2^3;   //蜂鸣器IO口定义
 uint flag_300ms = 1;

long distance;	        //距离
uint set_d=50;	            //距离
uchar flag_csb_juli;    //超声波超出量程
uint  flag_time0;       //用来保存定时器0的时候的

uchar menu_1;           //菜单设计的变量
 
/***********************1ms延时函数*****************************/
void delay_1ms(uint q)
{
	uint i,j;
	for(i=0;i<q;i++)
		for(j=0;j<120;j++);
}


/********************独立按键程序*****************/
uchar key_can;	 //按键值

void key()	    //独立按键程序
{
	static uchar key_new;  //key_new  这个变量的功能是做按键松手检测的  
	key_can = 0;                   //按键值还原
	P2 |= 0x07;
	if((P2 & 0x07) != 0x07)		//按键按下
	{
		delay_1ms(1);	     	//按键消抖动
		if(((P2 & 0x07) != 0x07) && (key_new == 1))
		{						//确认是按键按下
			key_new = 0;		//key_new = 0   说明按键已按下
			switch(P2 & 0x07)
			{
				case 0x06: key_can = 3; break;	   //得到k3键值
				case 0x05: key_can = 2; break;	   //得到k2键值
				case 0x03: key_can = 1; break;	   //得到k1键值
			}
		}			
	}
	else 
		key_new = 1;	//key_new = 1   说明按键已经松开了
}

/****************按键处理显示函数***************/
void key_with()
{
	if(key_can == 1)		//设置键
	{
		menu_1 ++;
		if(menu_1 >= 2)
		{
			menu_1 = 0;
 		}
 	}
	if(menu_1 == 1)			//设置报警
	{
		if(key_can == 2)
		{
			set_d ++ ;		//加1
			if(set_d > 500)
				set_d = 500;
		}
		if(key_can == 3)
		{
			set_d -- ;		//减1
			if(set_d <= 1)
				set_d = 1;
		}
		dis_smg[0] = smg_du[set_d % 10];	           //取小数显示
		dis_smg[1] = smg_du[set_d / 10 % 10] ;         //取个位显示
		dis_smg[2] = smg_du[set_d / 100 % 10] & 0xdf ; //取十位显示
		dis_smg[3] = 0x60;	        //a
	}	
}  

/****************报警函数***************/
void clock_h_l()
{
 	if(distance <= set_d)      //低于设置的距离就会报警
	{
 		beep = ~beep; 	  //蜂鸣器报警	
 	}
	else 
	{
 		beep = 1;		//取消报警
	}	
}

/***********************数码位选函数*****************************/
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

/***********************数码显示函数*****************************/
void display()
{
	static uchar i;   
	i++;
	if(i >= 4)
		i = 0;	
	smg_we_switch(i);		 //位选
	P1 = dis_smg[i];		 //段选	        
}

/******************小延时函数*****************/
void delay()
{
	_nop_(); 		           //执行一条_nop_()指令就是1us
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


/*********************超声波测距程序*****************************/
void send_wave()
{
	c_send = 1;		           //10us的高电平触发 
	delay();
	c_send = 0;	 
	TH0 = 0;		          //给定时器0清零
	TL0 = 0;
	TR0 = 0;				  //关定时器0定时
	while(!c_recive);		  //当c_recive为零时等待
	TR0=1;
	while(c_recive)		      //当c_recive为1计数并等待
	{
		flag_time0 = TH0 * 256 + TL0;
		if((flag_time0 > 40000))      //当超声波超过测量范围时，显示3个888
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
		TR0=0;							 //关定时器0定时
		distance =flag_time0;			 //读出定时器0的时间
		distance *= 0.017;               // 0.017 = 340M / 2 = 170M = 0.017M 算出来是米
		if((distance > 600))				 //距离 = 速度 * 时间
		{	
			distance = 888;				 //如果大于6m就超出超声波的量程 
		}
	}  
}


/*********************定时器0、定时器1初始化******************/
void time_init()	  
{
	EA  = 1;	 	  //开总中断
	TMOD = 0X11;	  //定时器0、定时器1工作方式1
	ET0 = 0;		  //关定时器0中断 
	TR0 = 1;		  //允许定时器0定时
	ET1 = 1;		  //开定时器1中断 
	TR1 = 1;		  //允许定时器1定时	
}



/***************主函数*****************/
void main()
{
	beep = 0;		 //开机叫一声   
	delay_1ms(150);
	P0 = P1 = P2 = P3 = 0xff;	   //初始化单片机IO口为高电平
	time_init();	//定时器初始化程序
	while(1)
	{	
		flag_300ms ++;	  //加1 
		if(flag_300ms >= 300)
		{		
			flag_300ms = 0;
			send_wave();	//测距离函数
			if(menu_1 == 0)
			{
				dis_smg[0] = smg_du[distance % 10];		 //显示距离
				dis_smg[1] = smg_du[distance / 10 % 10];
				dis_smg[2] = smg_du[distance / 100 % 10] & 0xdf; ;	
				dis_smg[3] = 0xff;	        //不显示
			}
			clock_h_l();     //报警函数
		}
		key();			     //按键函数
 		key_with();			 //按键处理函数
 		delay_1ms(1);		 //延时1毫秒
	}
}

/*********************定时器1中断服务程序************************/
void time1_int() interrupt 3
{	
 	TH1 = 0xf8;
	TL1 = 0x30;     //2ms
	display();		//数码管显示函数
 }



