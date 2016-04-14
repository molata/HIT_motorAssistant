#include"iodefine.h"
#include"stdint.h"
#define key_in  PORT2.PORT.BYTE
#define key_out PORT3.DR.BYTE
#include<machine.h>

static uint8_t key_prs[3][3];//*************以下定义变量
unsigned short keycnt[3][3];
static uint8_t key_down[3][3];
static uint8_t key_long[3][3];
static uint8_t key_once[3][3];
static uint8_t key_longrising[3][3];


extern unsigned short parabit;

extern unsigned short para1;//*********************************以下定义变量************************
extern unsigned short para1_compare;
extern unsigned short parabit;
static uint8_t ledbit=1;
static short j;
static short t;
static uint8_t countLED;
static short n=0;

unsigned char HIT_judge;
unsigned short HIT_tran_start;


extern unsigned short HIT_PNnum[152];
extern unsigned short HIT_addressnum[152];
extern unsigned short HIT_Fn_Sn_f_s[152];

static short e=5;
static short p;
unsigned short HIT_table_d;
extern unsigned short para[5];
extern unsigned short para1;
extern unsigned short parae[5];


unsigned short rcvbuff_t;
unsigned short rcvbuff_short;

unsigned short rcvbuff_char_1;
unsigned short rcvbuff_char_2;
unsigned short rcvbuff_char_3;
unsigned short rcvbuff_char_4;
unsigned char rcvbuff_char;
extern unsigned char trcbuff[4];
extern unsigned char rcvbuff[4];
//extern unsigned short rcvbuff[8];
extern int HIT_current_filter_initial;
extern int HIT_current_filter_depth;
extern int para1_filter[20];
extern int para1_sum;
extern int HIT_u_sensor_sum;
//extern unsigned char g[4];
extern unsigned short g[4];
unsigned short HIT_encord_need = 0;
unsigned char HIT_senddata[8];
unsigned short HIT_change_num;
unsigned short HIT_change_num_title;
unsigned char HIT_send_data;
unsigned char HIT_send_data_buff[4];
unsigned char HIT_ee_dis;
unsigned short HIT_send_adrr;
unsigned short HIT_rec_adrr_buff_short;
unsigned int HIT_encorder_reset_int;
unsigned char HIT_encorder_reset_buff[8];

void Mtu0IcCmAIntFunc();//******************以下定义函数
void UpdateKeyval();
void Smoothing();


void ChangeNum();

//void Mtu0IcCmBIntFunc();//*******************以下定义函数************************
void DispNum(uint8_t Num);
void SON(uint8_t i);
void disp(void);
void Flashing();

void SendData(void);
void ReceiveData(void);


void Sci0TrFunc()
{
	
}


void Sci0ReFunc()
{

}





void Mtu0IcCmAIntFunc()//*******************************中断函数开始****************************
{
	MTU0.TSR.BIT.TGFA=0;   //软件的方式将计数器置零
	
	UpdateKeyval();   // 更新按键的数值
	ChangeNum();//20120930wl
	
/*		
	if((parae[0] == 15)&&(parae[1] == 16)&&(parae[2] == 0)&&(parae[3] == 0)&&(parae[4] == 5))
	{
	//	if((para[0] == 4)&&(para[1] == 8)&&(para[2] == 0)&&(para[3] == 7)&&(para[4] == 0))	
		if(HIT_rec_adrr_buff_short == 7084)
		{
			para[0] = 5; 
			para[1] = 14; 
			para[2] = 0; 
			para[3] = 0; 
			para[4] = 16; 		
		}
	//	if((para[0] == 3)&&(para[1] == 8)&&(para[2] == 0)&&(para[3] == 7)&&(para[4] == 0))	
		if(HIT_rec_adrr_buff_short == 7083)
		{
			para[0] = 5; 
			para[1] = 14; 
			para[2] = 0; 
			para[3] = 0; 
			para[4] = 15; 		
		}
	}*/
	disp();  // 显示数值

}

void UpdateKeyval()//**********************************键盘扫描****包含滤波*****************************************
{
	key_out=0x01;
		if((key_in&0x04)==0){key_prs[0][0]=0;}
		if((key_in&0x04)!=0){key_prs[0][0]=1;}//对[0][0]对应16端口做出判断***************display // 将当前数值修改
	
		if((key_in&0x10)==0){key_prs[0][2]=0;}
		if((key_in&0x10)!=0){key_prs[0][2]=1;}//对[0][2]对应14端口做出判断***************reset   // 退出数据    
		
	key_out=0x02;
		if((key_in&0x04)==0){key_prs[1][0]=0;}
		if((key_in&0x04)!=0){key_prs[1][0]=1;}//对[1][0]对应26端口做出判断***************DATA    // 显示数是多少
	
		if((key_in&0x08)==0){key_prs[1][1]=0;}
		if((key_in&0x08)!=0){key_prs[1][1]=1;}//对[1][1]对应25端口做出判断**************up
		
		if((key_in&0x10)==0){key_prs[1][2]=0;}
		if((key_in&0x10)!=0){key_prs[1][2]=1;}//对[1][2]对应24端口做出判断***************JOG   // 按键扫描
		
	key_out=0x04;
		if((key_in&0x04)==0){key_prs[2][0]=0;}
		if((key_in&0x04)!=0){key_prs[2][0]=1;}//对[2][0]对应36端口做出判断***************right
	
		if((key_in&0x08)==0){key_prs[2][1]=0;}
		if((key_in&0x08)!=0){key_prs[2][1]=1;}//对[2][1]对应35端口做出判断***************down
		
		if((key_in&0x10)==0){key_prs[2][2]=0;}
		if((key_in&0x10)!=0){key_prs[2][2]=1;}//对[2][2]对应34端口做出判断***************left
		
		Smoothing();
			
}

void Smoothing()//*****************************************滤波*********************************************
{
	uint8_t i,j;
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			if(key_prs[i][j]==1){keycnt[i][j]++;}
			if(key_prs[i][j]==0){keycnt[i][j]=0;}
			
			if(keycnt[i][j]>100) {key_down[i][j]=1;}//一个奇怪的现象：如果用key_down=1来作为灯亮的条件，一直按住按键**
			if(keycnt[i][j]<=100){key_down[i][j]=0;}//灯是闪的，因为keycnt没有清零，不停的累加，溢出后重0开始计数*****
			
			if(keycnt[i][j]>1000) {key_long[i][j]=1;}//keycnt的类型很重要，如果定义的不对，很可能加不到1000.
			if(keycnt[i][j]<=1000){key_long[i][j]=0;}//一定要定义可以大于1000的变量
			
			if(keycnt[i][j]==101) {key_once[i][j]=1;}         // 区别长按和短按的区别
			if(keycnt[i][j]!=101){key_once[i][j]=0;}
			
			if(keycnt[i][j]==1001) {key_longrising[i][j]=1;}
			if(keycnt[i][j]!=1000){key_longrising[i][j]=0;}
			
			if((keycnt[i][j]>1001)&&(keycnt[i][j]%500==0)){key_longrising[i][j]=1;}
		}
	
	}
}

unsigned char HIT_send_adrr_buff[8];

unsigned char HIT_rec_adrr_buff[8];
unsigned int HIT_send_adrr_int;
unsigned char HIT_data_enter = 0;
unsigned char HIT_send_enc_ok = 0;
unsigned char HIT_unable_flag = 0;
unsigned char HIT_rec_adrr_buff_j = 0;
 unsigned short HIT_filter_initial = 1;
 unsigned short HIT_filter_depth;
 unsigned short HIT_filter_i;
 unsigned short HIT_a_phase_filter[32];
 int HIT_a_phase_sum;

void ChangeNum()//*********************************修改para1 的值*************************
{
	if(para[e]>19)     //一共有0，1，... U, N , F 19个数
	     {para[e]=0;}
	
	if(para[e]<0)
	     {para[e]=19;}
		
		 
	if(e>5)         // 一共有5个选项
	      {e=0;}
	  
	if(e<0)
	       {e=5;}
	  
	if(key_once[1][1]==1)//up键按下
		{para[e]=para[e]+parabit;}//add para1 
		
	if(key_once[2][1]==1)//down键按下
		{para[e]=para[e]-parabit;}//add para1 
		
	if(key_once[2][2]==1)//left键按下
		{e=e+1;}//add para1 
		
	if(key_once[2][0]==1)//right键按下
		{e=e-1;}//add para1 
		

//		 g[n]=para[0]+para[1]*10+para[2]*100+para[3]*1000+para[4]*10000;
		
		
//		para1=para[0]+para[1]*17+para[2]*18*17+para[3]*1000+para[4]*10000;
	SCI0.SSR.BIT.RDRF = 0;

	if(HIT_send_enc_ok == 1)/////enable receive encord angle //原来的程序中有接收编码器数据的功能
	{
		SCI0.SSR.BIT.RDRF = 0;
		R_PG_SCI_StartReceiving_C0(HIT_rec_adrr_buff,8);     // 这个接收的是什么数据？？？
		
	//	HIT_rec_adrr_buff_short = (((unsigned short)HIT_rec_adrr_buff[6])<<8) + (unsigned short)(HIT_rec_adrr_buff[7]);
		
		for(HIT_rec_adrr_buff_j=0; HIT_rec_adrr_buff_j<8; HIT_rec_adrr_buff_j++)  // 解析这8个数
		{
			HIT_judge = (HIT_rec_adrr_buff[HIT_rec_adrr_buff_j]&0xf0)>>4;       // 将接收的数据右移4位，进行判断
			if(HIT_judge == 1)
			{
				HIT_tran_start = 1;	     // ？？？？这个是什么东西
			}
			if(HIT_tran_start == 1)
			{
				if(HIT_judge == 1)
				{
					rcvbuff_char_1 = (unsigned short)(HIT_rec_adrr_buff[HIT_rec_adrr_buff_j]&0x0f)<<12;	// 将数左移12位，移到最开始端
				}	
				if(HIT_judge == 2)
				{
					rcvbuff_char_2 = (unsigned short)(HIT_rec_adrr_buff[HIT_rec_adrr_buff_j]&0x0f)<<8;	
				}
				if(HIT_judge == 3)
				{
					rcvbuff_char_3 = (unsigned short)(HIT_rec_adrr_buff[HIT_rec_adrr_buff_j]&0x0f)<<4;	
				}
				if(HIT_judge == 4)
				{
					rcvbuff_char_4 = (unsigned short)(HIT_rec_adrr_buff[HIT_rec_adrr_buff_j]&0x0f);	
				}
				HIT_rec_adrr_buff_short = rcvbuff_char_1 + rcvbuff_char_2 + rcvbuff_char_3 + rcvbuff_char_4;// 将数值组合起来
			}
		
		}    // 接收陀螺的数据结束
		
		
		
	if(HIT_filter_initial == 1)
	{
		HIT_filter_initial = 0;
		HIT_filter_depth = 0;
		for (HIT_filter_i=0;HIT_filter_i<32;HIT_filter_i++)
		{
			HIT_a_phase_filter[HIT_filter_i] = HIT_rec_adrr_buff_short;
			HIT_a_phase_sum = ((int)HIT_rec_adrr_buff_short) << 5;
		}		
	}
	
	HIT_a_phase_sum -= (int)HIT_a_phase_filter[HIT_filter_depth];
	
	HIT_a_phase_filter[HIT_filter_depth] = HIT_rec_adrr_buff_short;
	
	HIT_a_phase_sum += (int)HIT_rec_adrr_buff_short;
	
	++HIT_filter_depth;
	if(HIT_filter_depth == 32)
	{
		HIT_filter_depth = 0;
	}
		
	HIT_rec_adrr_buff_short = (unsigned short)(HIT_a_phase_sum/32);
		
		
		
		para[0]=HIT_rec_adrr_buff_short%10;		//	*********显示参数
		para[1]=(HIT_rec_adrr_buff_short/10)%10;
		para[2]=(HIT_rec_adrr_buff_short/100)%10;
		para[3]=(HIT_rec_adrr_buff_short/1000)%10;
		para[4]=(HIT_rec_adrr_buff_short/10000)%10;

				if((parae[0] == 5)&&(parae[1] == 0)&&(parae[2] == 0)&&(parae[3] == 16)&&(parae[4] == 15)&&(HIT_rec_adrr_buff_short == 7083))
				{
				   para[0]=15;		//	*********显示参数
				   para[1]= 0;
				   para[2]= 0;
				   para[3]= 14;
				   para[4]= 5;	
				}
				if((parae[0] == 5)&&(parae[1] == 0)&&(parae[2] == 0)&&(parae[3] == 16)&&(parae[4] == 15)&&(HIT_rec_adrr_buff_short == 7084))
				{
				   para[0]=16;		//	*********显示参数
				   para[1]= 0;
				   para[2]= 0;
				   para[3]= 14;
				   para[4]= 5;	
				}
				if((parae[0] == 5)&&(parae[1] == 0)&&(parae[2] == 0)&&(parae[3] == 16)&&(parae[4] == 15)&&(HIT_rec_adrr_buff_short == 5720))
				{
				   para[0]= 0;		//	*********显示参数
				   para[1]= 0;
				   para[2]= 10;
				   para[3]= 20;
				   para[4]= 10;	
				}
	}	
	
para1_compare = para[0]+para[1]*16+para[2]*16*17+para[3]*100+para[4]*100;	
if(HIT_unable_flag == 0)
{	
	if(key_prs[0][0]==1)  			//****根本就不是什么disp，明明就是发送的指令
//	if(key_once[0][0]==1) 
	{
	//	para1_compare=para[0]+para[1]*17+para[2]*18*17+para[3]*1000+para[4]*10000;
	//   HIT_ee_dis = 0x55;
	//   R_PG_SCI_StartSending_C0(&HIT_ee_dis,1);
		
	   for(HIT_table_d=0;HIT_table_d<152;HIT_table_d++)   // 找到当前数值和对应的属性
	   {
		  if((para1_compare == 0x0c26)||(para1_compare == 0x0c21)||(para1_compare == 0x0c25))//encord send enable
		  {
			  HIT_send_enc_ok = 1;	  
		  }
		  if(HIT_PNnum[HIT_table_d]==para1_compare)    // 找到一个对应数值和当前数值相同
	      {
			  parae[0]=para[0];
			  parae[1]=para[1];
			  parae[2]=para[2];
			  parae[3]=para[3];
			  parae[4]=para[4];     // 这个也不知道干什么的
///////////////////////////////////////send fn_pn data address	///////////////////////		  
			  HIT_send_adrr = HIT_PNnum[HIT_table_d];
			  
			  HIT_send_adrr_int = (int)HIT_send_adrr;
			  
			  HIT_send_adrr_buff[7] = (unsigned char)(HIT_send_adrr_int&0x000f) + 128;//////////data	
			  HIT_send_adrr_buff[6] = (unsigned char)((HIT_send_adrr_int&0x00f0)>>4) +112;
			  HIT_send_adrr_buff[5] = (unsigned char)((HIT_send_adrr_int&0x0f00)>>8) + 96;
			  HIT_send_adrr_buff[4] = (unsigned char)((HIT_send_adrr_int&0xf000)>>12) + 80;
			  
			  HIT_send_adrr_buff[3] = (unsigned char)(HIT_send_adrr_int&0x000f) + 64;///////////address	
			  HIT_send_adrr_buff[2] = (unsigned char)((HIT_send_adrr_int&0x00f0)>>4) +48;
			  HIT_send_adrr_buff[1] = (unsigned char)((HIT_send_adrr_int&0x0f00)>>8) + 32;
			  HIT_send_adrr_buff[0] = (unsigned char)((HIT_send_adrr_int&0xf000)>>12) + 16;
			  if(SCI0.SSR.BIT.RDRF == 0)
			  {
			  	R_PG_SCI_StartSending_C0(HIT_send_adrr_buff,8);
			  }		  
			  SCI0.SSR.BIT.RDRF = 0;

			  R_PG_SCI_StartReceiving_C0(HIT_rec_adrr_buff,8);   // 发送数据以后，会确认一下数对不对
			   
			   HIT_rec_adrr_buff_short = (((unsigned short)HIT_rec_adrr_buff[6])<<8) + (unsigned short)(HIT_rec_adrr_buff[7]);
			   
			   para[0]=HIT_rec_adrr_buff_short%10;		//	*********显示参数
			   para[1]=(HIT_rec_adrr_buff_short/10)%10;
			   para[2]=(HIT_rec_adrr_buff_short/100)%10;
			   para[3]=(HIT_rec_adrr_buff_short/1000)%10;
			   para[4]=(HIT_rec_adrr_buff_short/10000)%10;
///////////////////////////////////////send fn_pn data address END	///////////////////////	

		  }
		}
	
	
	}
}    // 判断是不是要全部显示
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//状态显示
/*	  if(para1_compare == 3105)
	  {
		  if(HIT_rec_adrr_buff_short == 7084)
		  {
			  para[0]=16;
			  para[1]=0;
			  para[2]=0;
			  para[3]=14;
			  para[4]=5;		 
		  } 
		  if(HIT_rec_adrr_buff_short == 7083)
		  {
			  para[0]=15;
			  para[1]=0;
			  para[2]=0;
			  para[3]=14;
			  para[4]=5;		 
		  } 
	  }
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(key_once[0][2]==1)
      {
		  PORTB.DR.BIT.B7=1;
		  e=5;
		  para[0]= parae[0];
		  para[1]= parae[1];
		  para[2]= parae[2];
		  para[3]= parae[3];
		  para[4]= parae[4];
		  HIT_encorder_reset_int = 0x5555;
			  
		  HIT_encorder_reset_buff[7] = (unsigned char)(HIT_encorder_reset_int&0x000f) + 128;//////////data	
		  HIT_encorder_reset_buff[6] = (unsigned char)((HIT_encorder_reset_int&0x00f0)>>4) +112;
		  HIT_encorder_reset_buff[5] = (unsigned char)((HIT_encorder_reset_int&0x0f00)>>8) + 96;
		  HIT_encorder_reset_buff[4] = (unsigned char)((HIT_encorder_reset_int&0xf000)>>12) + 80;
		  HIT_encorder_reset_buff[3] = (unsigned char)(HIT_encorder_reset_int&0x000f) + 64;///////////address	
		  HIT_encorder_reset_buff[2] = (unsigned char)((HIT_encorder_reset_int&0x00f0)>>4) +48;
		  HIT_encorder_reset_buff[1] = (unsigned char)((HIT_encorder_reset_int&0x0f00)>>8) + 32;
		  HIT_encorder_reset_buff[0] = (unsigned char)((HIT_encorder_reset_int&0xf000)>>12) + 16;
			  
		  SCI0.SSR.BIT.RDRF = 0;
		  R_PG_SCI_StartSending_C0(HIT_encorder_reset_buff,8);
		  HIT_send_enc_ok = 0;
	  }
	if(key_prs[1][0]==0)
	{
		HIT_data_enter = 0;	
	}
	if(key_prs[1][0]==1)				//	*******************DATA
		{
			PORTB.DR.BIT.B7=0;
/////////////////////////////////////////////////////////////////////////////////////////////////			
			e=5;
			HIT_change_num = para[0]+para[1]*10+para[2]*100+para[3]*1000+para[4]*10000;
			HIT_change_num_title = parae[0]+parae[1]*16+parae[2]*16*17+parae[3]*100+parae[4]*100;
			PORTB.DR.BIT.B7 = 1;
			
/////////////////////////////////////////////////////////////////////////////////////////////////			
			if(HIT_data_enter < 150)
			{
				SendData();
			}
			HIT_data_enter++;
			if(HIT_data_enter > 151)
			{
				HIT_data_enter = 151;	
			}

		}
		
	if(key_prs[1][2]==1)//JOG servo on
	{
/*		e=5;	2012102816_35
		HIT_change_num = para[0]+para[1]*10+para[2]*100+para[3]*1000+para[4]*10000;
		HIT_change_num_title = parae[0]+parae[1]*16+parae[2]*16*17+parae[3]*100+parae[4]*100;
		PORTB.DR.BIT.B7 = 1;*/	
		  parae[0] = 10;
		  parae[1] = 0;
		  parae[2] = 0;
		  parae[3] = 16;
		  parae[4] = 15;	  
		  HIT_send_adrr_int = 0x0c30;
		  
		  HIT_send_adrr_buff[7] = (unsigned char)(HIT_send_adrr_int&0x000f) + 128;//////////data	
		  HIT_send_adrr_buff[6] = (unsigned char)((HIT_send_adrr_int&0x00f0)>>4) +112;
		  HIT_send_adrr_buff[5] = (unsigned char)((HIT_send_adrr_int&0x0f00)>>8) + 96;
		  HIT_send_adrr_buff[4] = (unsigned char)((HIT_send_adrr_int&0xf000)>>12) + 80;
		  
		  HIT_send_adrr_buff[3] = (unsigned char)(HIT_send_adrr_int&0x000f) + 64;///////////address	
		  HIT_send_adrr_buff[2] = (unsigned char)((HIT_send_adrr_int&0x00f0)>>4) +48;
		  HIT_send_adrr_buff[1] = (unsigned char)((HIT_send_adrr_int&0x0f00)>>8) + 32;
		  HIT_send_adrr_buff[0] = (unsigned char)((HIT_send_adrr_int&0xf000)>>12) + 16;
		  
		  R_PG_SCI_StartSending_C0(HIT_send_adrr_buff,8);
	}
	

}




void delay1ms(int x)			// 延迟函数开始 
{	int i,j;					// 声明整数变数i,j 
	for (i=0;i<x;i++)			// 计数x次,延迟x 1ms 
		for (j=0;j<120;j++);	// 计数120次，延迟1ms 
}	









void disp(void)//*****************************************显示para1的值***************************
{
	
	switch(ledbit)
	{
		case 1:
			{
				SON(ledbit);
				DispNum(para[0]);//para[0]
			if(e==1)
			  	{delay1ms(480);}
				ledbit++;break;}
		case 2:
			{
				SON(ledbit);
				DispNum(para[1]);//para[1]
			if(e==2)
			  	{delay1ms(480);}
				ledbit++;break;}
		case 3:
			{
				SON(ledbit);
				DispNum(para[2]);//para[2]
			if(e==3)
			  	{delay1ms(480);}
				ledbit++;break;}
		case 4:
			{
				SON(ledbit);
			DispNum(para[3]);//para[3]
			if(e==4)
			  	{delay1ms(480);}
				ledbit++;break;}
		case 5:
			{
				SON(ledbit);
				DispNum(para[4]);//para[4]
			if(e==0)
			  	{delay1ms(480);}
			ledbit=1;break;}
		default:
			break;
	}
}

void SON(uint8_t i)//**************************************数码管位选************************************
{
	switch(i)
	{
		case 5:
			PORT1.DR.BIT.B1=1;//             
			PORTA.DR.BYTE=0;break;	//一定要所有的数码管选中端口同时赋值，否则
		case 4:						//break退出之后赋的值保持，导致所有数码管显示同样的数字
			PORT1.DR.BIT.B1=0;
			PORTA.DR.BYTE=0x04;break;
		case 3:
			PORT1.DR.BIT.B1=0;//**********************百位************右数第三个
			PORTA.DR.BYTE=0x08;break;
		case 2:
			PORT1.DR.BIT.B1=0;//**********************十位************右数第二个
			PORTA.DR.BYTE=0x10;break;
		case 1:
			PORT1.DR.BIT.B1=0;// *********************个位************右数第一个数码管选定
			PORTA.DR.BYTE=0x20;break;
		default:
			break;
	}
}

void DispNum(uint8_t Num)//*******************************显示内容***********************************
{
	switch(Num)
	{
		case 0:
 			PORT7.DR.BYTE = 0x01;//***********0-9的显示**********
 			PORT9.DR.BIT.B1 = 0X01;
			break;
		case 1:
 			PORT7.DR.BYTE = 0x73;
 			PORT9.DR.BIT.B1 = 0X01;
			break;
		case 2:
 			PORT7.DR.BYTE = 0x49;
 			PORT9.DR.BIT.B1 = 0X00; 
			break;
		case 3:
 			PORT7.DR.BYTE = 0x61;
 			PORT9.DR.BIT.B1 = 0X00;
			break;
		case 4:
 			PORT7.DR.BYTE = 0x33;
 			PORT9.DR.BIT.B1 = 0X00;
			break;
		case 5:
 			PORT7.DR.BYTE = 0x25;
 			PORT9.DR.BIT.B1 = 0X00;
			break;
		case 6:
 			PORT7.DR.BYTE = 0x05;
 			PORT9.DR.BIT.B1 = 0X00;
			break;
		case 7:
 			PORT7.DR.BYTE = 0x71;
 			PORT9.DR.BIT.B1 = 0X01;
			break;
		case 8:
 			PORT7.DR.BYTE = 0x01;
 			PORT9.DR.BIT.B1 = 0X00;
			break;
		case 9:
 			PORT7.DR.BYTE = 0x21;
 			PORT9.DR.BIT.B1 = 0X00;
			break;
		case 10:
			PORT7.DR.BYTE = 16;
			PORT9.DR.BIT.B1 = 0X00;//A
			break;
		case 11:	
			PORT7.DR.BYTE = 6;
			PORT9.DR.BIT.B1 = 0X00;//b
			break;
		case 12:
			PORT7.DR.BYTE = 12;
			PORT9.DR.BIT.B1 = 0X01;//c
			break;	
		case 13:
			PORT7.DR.BYTE = 66;
			PORT9.DR.BIT.B1 = 0X00;//d
			break;
		case 14:
			PORT7.DR.BYTE = 12;
			PORT9.DR.BIT.B1 = 0X00;//e
			break;
		case 15:
			PORT7.DR.BYTE = 28;
			PORT9.DR.BIT.B1 = 0X00;//f
			break;
		case 16:
 			PORT7.DR.BYTE = 0x57;//*******************显示 n
 			PORT9.DR.BIT.B1 = 0X00;
			break;
		case 17:
 			PORT7.DR.BYTE = 0x0f;//*******************显示 t
 			PORT9.DR.BIT.B1 = 0X00;
			break;		
		case 18:
 			PORT7.DR.BYTE = 0x19;//*******************显示 P
 			PORT9.DR.BIT.B1 = 0X00;
			break;
		case 19:
 			PORT7.DR.BYTE = 0x03;//*******************显示 U
 			PORT9.DR.BIT.B1 = 0X01;
			break;
		case 20:
			PORT7.DR.BYTE = 0x0f;//*******************显示 l
 			PORT9.DR.BIT.B1 = 0X01;
			break;
				
	}
}
void Flashing()
{
	j++;

	if(j>400) 
	{
		
		t=parabit;
		countLED=0;

		while(t!=0)
		{
			t=t/10;
			countLED++;
		}
	
		if(ledbit==countLED++&&ledbit!=5){ledbit=ledbit+1;};
	}
	if(j==801){j=1;}
}

unsigned short HIT_send_ser_num = 0;
unsigned short HIT_Re_change_num_title;
void SendData(void)
{
	HIT_senddata[7] = (unsigned char)(HIT_change_num&0x000f) + 128;	
	HIT_senddata[6] = (unsigned char)((HIT_change_num&0x00f0)>>4) +112;
	HIT_senddata[5] = (unsigned char)((HIT_change_num&0x0f00)>>8) + 96;
	HIT_senddata[4] = (unsigned char)((HIT_change_num&0xf000)>>12) + 80;
	
	HIT_Re_change_num_title = 0;
	
	HIT_senddata[3] = (unsigned char)(HIT_Re_change_num_title&0x000f) + 64;//(unsigned char)(HIT_change_num_title&0x000f) + 64;	
	HIT_senddata[2] = (unsigned char)((HIT_Re_change_num_title&0x00f0)>>4) + 48;//(unsigned char)((HIT_change_num_title&0x00f0)>>4) +48;
	HIT_senddata[1] = (unsigned char)((HIT_Re_change_num_title&0x0f00)>>8) + 32;//(unsigned char)((HIT_change_num_title&0x0f00)>>8) + 32;
	HIT_senddata[0] = (unsigned char)((HIT_Re_change_num_title&0xf000)>>12) + 16;//(unsigned char)((HIT_change_num_title&0xf000)>>12) + 16;
	
		R_PG_SCI_StartSending_C0(HIT_senddata,8);

	if (HIT_send_ser_num == 9)
	{
		HIT_send_ser_num = 0;	
	}
	
	
	
	PORTB.DR.BIT.B7=0;
	
	nop();nop();nop();nop();
}

void ReceiveData(void)
{
	R_PG_SCI_StartReceiving_C0(rcvbuff,4);

	para1=rcvbuff[0]*10+rcvbuff[1];
	
}





