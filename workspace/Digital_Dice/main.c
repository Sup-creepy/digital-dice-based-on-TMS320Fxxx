/*
 * 功能描述：
 *     使用GPIO驱动LCD1602和LED,用LCD模拟骰子，通过按键实现不同的功能
 *     使用串口通信实现作弊功能。
 */


#include "DSP28x_Project.h"
#include "DSP2833x_Device.h"
#include "DSP2833x_examples.h"
#include "stdlib.h"


/****************子程序声明*****************/
void delay_loop_slow(void);	//慢延时
void delay_loop_fast(void);	//快延时
void Gpio_select(void);		//GPIO初始化
void Gpio_ShowState(void);
//SCI串口
void scib_echoback_init(void);//SCI串口
void scib_fifo_init(void);
void scib_xmit(int a);
void scib_msg(char *msg);
int getnum(Uint16 a);	//读取串口数据
int RECV(void);		//接收串口数据
//LCD显示屏
void LCD_init(void);                          		//初始化函数
void LCD_write_command(unsigned char command);   //写指令函数
void LCD_write_data(unsigned char dat);          //写数据函数
void show(char *cont);                       		//显示子函数
void Init_Port(void);   							//端口初始化

/****************全局变量定义*****************/
volatile Uint32 Xint1Count; //发生外部中断1的次数；
volatile Uint32 Xint2Count; //发生外部中断2的次数；
volatile Uint32 Xint2Loop; //给外部中断2的for循环变量
Uint16 RC1;
Uint16 RC2;

/****************端口宏定义*****************/
#define KEY1 GpioDataRegs.GPADAT.bit.GPIO24		//定义按键
#define KEY2 GpioDataRegs.GPADAT.bit.GPIO25
#define KEY3 GpioDataRegs.GPADAT.bit.GPIO26
#define LCD_RS GpioDataRegs.GPBDAT.bit.GPIO34
#define LCD_RW GpioDataRegs.GPADAT.bit.GPIO8
#define LCD_EN GpioDataRegs.GPADAT.bit.GPIO9
#define LCD_DB GpioDataRegs.GPCDAT.all
#define DELAY 35.700L

int PR[6] = {16,17,17,17,17,16};	//正常概率，骰子的六个面概率相等
int P[6];	//设置概率
int result;	//设置结果
int flag = 0;		//
int result_flag=0;

//*******************主程序**********************//
void main(void)
{
	/**************初始化****************/
// 初始化系统控制
   InitSysCtrl();

// 清除所有中断,初始化中断向量表:
// 禁止CPU全局中断
   DINT;

// 禁止PIE中断及清除所有PIE中断标志
   InitPieCtrl();

// 禁止CPU中断和清除所有CPU中断标志
   IER = 0x0000;
   IFR = 0x0000;

//初始化PIE中断向量表，并使其指向中断服务子程序（ISR）
   InitPieVectTable();

//初始化SCI
   scib_fifo_init();
   scib_echoback_init();
   SCIStdioInit();

//初始化LCD
   Init_Port();
   LCD_init();

   LCD_write_command(0x01);	//清屏
   LCD_write_command(0x80);	//LCD第一行显示
   show("  LOADING...  ");
   DELAY_US(1000000);
   int MENU = 1;

   SCIPuts("\r\n ============DIGITIAL DICE=========== \r\n", -1);
   SCIPuts("\rWelcome to cheat interface! Enter # to change probability!\r\n", -1);
   while(1)
   {
	   if(MENU == 1)	//Start界面
	   {
		   GpioDataRegs.GPBDAT.all = 0x00040000;
		   LCD_write_command(0x80);	//LCD第一行显示
		   show("  DIGITAL DICE   ");

		   LCD_write_command(0xc0);	//LCD第二行显示
		   show("   Waiting  1    ");
		   delay_loop_slow();	//低速延时

		   LCD_write_command(0xc0);	//LCD第二行显示
		   show("   Waiting  2    ");
		   delay_loop_slow();	//低速延时

		   LCD_write_command(0xc0);	//LCD第二行显示
		   show("   Waiting  3    ");
		   delay_loop_slow();	//低速延时

		   LCD_write_command(0xc0);	//LCD第二行显示
		   show("   Waiting  4    ");
		   delay_loop_slow();	//低速延时

		   LCD_write_command(0xc0);	//LCD第二行显示
		   show("   Waiting  5    ");
		   delay_loop_slow();	//低速延时

		   LCD_write_command(0xc0);	//LCD第二行显示
		   show("   Waiting  6    ");
		   delay_loop_slow();	//低速延时

		   if(!KEY2)
		   {
			   MENU = 2;//按键2被按下跳到Rolling界面
//			   while(!KEY2);
			   LCD_write_command(0x01);	//清屏
		   }
		   if(flag == 1)	//串口接收到#进入作弊界面
			   MENU = 4;//进入作弊界面
	   }

	   if(MENU == 2)	//Rolling界面
	   {
		   GpioDataRegs.GPBDAT.all = 0x00080000;
//		   LCD_write_command(0x01);	//清屏
		   LCD_write_command(0x80);	//LCD第一行显示
		   show("  Dice Rolling! ");

		   delay_loop_fast();	//快速延时
		   LCD_write_command(0xc0);	//LCD第二行显示
		   show("   Rolling: 1  ");

		   delay_loop_fast();	//快速延时
		   LCD_write_command(0xc0);	//LCD第二行显示
		   show("   Rolling: 2  ");

		   delay_loop_fast();	//快速延时
		   LCD_write_command(0xc0);	//LCD第二行显示
		   show("   Rolling: 3  ");

		   delay_loop_fast();	//快速延时
		   LCD_write_command(0xc0);	//LCD第二行显示
		   show("   Rolling: 4  ");

		   delay_loop_fast();	//快速延时
		   LCD_write_command(0xc0);	//LCD第二行显示
		   show("   Rolling: 5  ");

		   delay_loop_fast();	//快速延时
		   LCD_write_command(0xc0);	//LCD第二行显示
		   show("   Rolling: 6  ");

		   if(!KEY1)	//按键1被按下跳到Start界面
		   {
			   MENU = 1;
//			   while(!KEY1);
			   LCD_write_command(0x01);	//清屏
		   }
		   if(!KEY3)	//按键3被按下跳到Stop界面
		   {
			   MENU = 3;
//			   while(!KEY3);
			   LCD_write_command(0x01);	//清屏
		   }
		   if(flag == 1)	//串口接收到#进入作弊界面
			   MENU = 4;
	   }

	   if(MENU == 3)	//Stop界面
	   {
		   GpioDataRegs.GPBDAT.all = 0x00100000;
		   LCD_write_command(0x80);	//LCD第一行显示
		   show("      STOP      ");
			   result = rand()%100;
			   P[0] = PR[0];
			   P[1] = P[0]+PR[1];
			   P[2] = P[1]+PR[2];
			   P[3] = P[2]+PR[3];
			   P[4] = P[3]+PR[4];
			   P[5] = P[4]+PR[5];
		   if(0<=result && result<P[0])		//默认概率下的结果显示
		   {
		   		LCD_write_command(0xc0);	//LCD第二行显示
		   		show("   COUNT:  1   ");

		   }
		   else if(P[0]<=result && result<P[1])
		   {
		   		LCD_write_command(0xc0);	//LCD第二行显示
		   		show("   COUNT:  2   ");

		   }
		   	else if(P[1]<=result && result<P[2])
		   	{
		   		LCD_write_command(0xc0);	//LCD第二行显示
		   		show("   COUNT:  3   ");

		   	}
		   	else if(P[2]<=result && result<P[3])
		   	{
		   		LCD_write_command(0xc0);	//LCD第二行显示
		   		show("   COUNT:   4   ");

		   	}
		   	else if(P[3]<=result && result<P[4])
		   	{
		   		LCD_write_command(0xc0);	//LCD第二行显示
		   		show("   COUNT:  5   ");

		   	}
		   	else if(P[4]<=result && result<P[5])
		   	{
		   		LCD_write_command(0xc0);	//LCD第二行显示
		   		show("   COUNT:  6   ");

		   	}
		   	else
		   	{
		   		LCD_write_command(0xc0);	//LCD第二行显示
		   		show("     ERROR     ");

		   	}
		   while(1)		//等待按键
		   	{
		   		if (!KEY1)	//按键1被按下跳到Start界面
		   		{
		   			MENU = 1;
		   			LCD_write_command(0x01);	//清屏
		   			break;
		   		}
		   		if (KEY2==0)//按键2被按下跳到Rolling界面
		   		{
		   			MENU = 2;
		   			LCD_write_command(0x01);	//清屏
		   			break;
		   		}
		   	}
	   }

	   if(MENU == 4)	//串口作弊界面
	   {
		   GpioDataRegs.GPBDAT.all = 0x00200000;
			while(1)
			{

				SCIPuts("\r Please Enter the probability of each number(unit:1%)\r\n", -1);
				SCIPuts("\r Enter the probability of 1,range 0-99 \r\n", -1);
				PR[0]=RECV();
				SCIPuts("\r Enter the probability of 2,range 0-99 \r\n", -1);
				PR[1]=RECV();
				SCIPuts("\r Enter the probability of 3,range 0-99 \r\n", -1);
				PR[2]=RECV();
				SCIPuts("\r Enter the probability of 4,range 0-99 \r\n", -1);
				PR[3]=RECV();
				SCIPuts("\r Enter the probability of 5,range 0-99 \r\n", -1);
				PR[4]=RECV();
				SCIPuts("\r Enter the probability of 6,range 0-99 \r\n", -1);
				PR[5]=RECV();
				if((PR[0]+PR[1]+PR[2]+PR[3]+PR[4]+PR[5])!=100)
				{
					SCIPuts("\r\n ERROR: The probabilities must add up to 100% \r\n", -1);
					flag = 0;
					//result_flag = 0;
					MENU = 1;
					init_pos();
					break;
				}
					else
				{
					SCIPuts("\r Change Succeeded! \r\n", -1);
					flag = 0;
					//result_flag = 1;
					MENU = 2;
					break;
				}
			}

	   }

   }



}




//****************函数调用***************//

int init_pos(void)//初始化概率
{
	PR[0]=16;
	PR[1]=17;
	PR[2]=17;
	PR[3]=17;
	PR[4]=17;
	PR[5]=16;
}

int RECV(void)
{
	int RCI;
	RC1=0x20;
	RC2=0x20;
	while(ScibRegs.SCIFFRX.bit.RXFFST!=1) {} // wait for XRDY =1 for empty state
	RC1=ScibRegs.SCIRXBUF.all;
	RCI=getnum(RC1);
	DELAY_US(500);
	if(ScibRegs.SCIFFRX.bit.RXFFST==1)
	{
		RC2 = ScibRegs.SCIRXBUF.all;
	    RCI=10*RCI+getnum(RC2);
	}
	SCIPuts("\r You entered:", -1);
	scib_xmit(RC1);
	scib_xmit(RC2);
	return RCI;
}

//串口接收函数
int SCI_RECV(void){
	int recv_flag=0;
	int recv_nums[3];
	int i=0;
	while(recv_flag==0){
		for(i=0;i<3;i++){
			while(ScibRegs.SCIFFRX.bit.RXFFST!=1) {}
			recv_nums[i]=ScibRegs.SCIRXBUF.all;
		}
		if(recv_nums[0]==0xAA){
			int count=0;
			for(i=0;i<8;i++){
				int temp=recv_nums[1]>>i;
				if(temp&0x0001){
					count++;
				}
			}
			//奇校验
			if(count%2){
				if(recv_nums[2]==0)
					recv_flag=1;
				else if(recv_nums[2]==1)
					recv_flag=0;
			}else{
				if(recv_nums[2]==0)
					recv_flag=0;
				else if(recv_nums[2]==1)
					recv_flag=1;
			}
		}
		if(recv_flag){
			SCIPuts("\r\n successfully received",-1);
		}else{
			SCIPuts("\r\n something went wrong! please transmit again",-1);
		}
	}
	return recv_nums[1];
}

void Init_Port(void)
{
	EALLOW;
	GpioCtrlRegs.GPCMUX1.all = 0;
	GpioCtrlRegs.GPCDIR.all = 0XFFFFFFFF;
	// RW
	GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 0;  // GPIO8
	GpioCtrlRegs.GPADIR.bit.GPIO8 = 1;   // output
	GpioDataRegs.GPACLEAR.bit.GPIO8 = 1;
	// EN
	GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;  // GPIO9
	GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;   // output
	GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;
	// RS
	GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;  // GPIO34
	GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;   // output
	GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
	EDIS;
}

//慢延时
void delay_loop_slow()
{
    volatile long i;
    for (i = 0; i < 7500000; i++)
    {
    	if (KEY2==0)//key2被按下，跳出循环
    		break;
    	if(ScibRegs.SCIFFRX.bit.RXFFST==1)	//串口接收到数据
    	{
    		RC1 = ScibRegs.SCIRXBUF.all;
    		if(RC1==0x23)	//接收到#  flag置1
    		{
    			flag = 1;
    			SCIPuts("\r\n Enter Cheat Mode!", -1);
    			break;
    		}
    	}
    }
}

//快延时，
void delay_loop_fast()
{
    volatile long i;
    for (i = 0; i < 250000; i++)
    {
    	if (!KEY1)	//key1被按下，跳出循环
    		break;
    	if (!KEY3)	//key3被按下，跳出循环
    		break;
    	if(ScibRegs.SCIFFRX.bit.RXFFST==1)	//串口接收到数据
    	{
    		RC1 = ScibRegs.SCIRXBUF.all;
    		if(RC1==0x23)	//接收到#
    		{
    			flag = 1;
    			break;
    		}
    	}
    }
}

int getnum(Uint16 a)
{
	int b=0;
	switch (a)
	{
		case 0x0030:{b=0;break;}
		case 0x0031:{b=1;break;}
		case 0x0032:{b=2;break;}
		case 0x0033:{b=3;break;}
		case 0x0034:{b=4;break;}
		case 0x0035:{b=5;break;}
		case 0x0036:{b=6;break;}
		case 0x0037:{b=7;break;}
		case 0x0038:{b=8;break;}
		case 0x0039:{b=9;break;}
		default:{break;}
	}
	return b;
}
/*--------------------------------------
;模块名称:show(char *cont)
;功    能:LCD1602显示子函数
;参数说明:cont为显示字符串指针参数
;-------------------------------------*/
void show(char *cont)
{
	while(*cont!=0)
	{
		LCD_write_data(*cont);
		DELAY_US(500);  //延时500us
		cont++;
	}
}

/*--------------------------------------
;模块名称:LCD_write_command();
;功    能:LCD1602写指令函数
;参数说明:command为写命令参数
;-------------------------------------*/
void LCD_write_command(unsigned char command)
{
	unsigned char i;
	Uint32 temp = 0;

	for(i=0;i<8;i++)
	{
		if(command & 0x01)
			temp |= 0x01;

		if(i == 7)
			break;

		temp = temp << 1;
		command >>= 1;
	}
	temp = temp << 8;

	LCD_EN=0;
	LCD_RS=0;         //指令
	LCD_RW=0;         //写入
	LCD_DB=temp;//赋值给D0-D7对应的GPIO79-GPIO72引脚
	DELAY_US(500);
	LCD_EN=1;          //允许
	DELAY_US(1000);  //
	LCD_EN=0;
}

/*--------------------------------------
;模块名称:LCD_write_data();
;功    能:LCD1602写数据函数
;参数说明:dat为写数据参数
;-------------------------------------*/
void LCD_write_data(unsigned char dat)
{
	unsigned char i;
	Uint32 temp = 0;

	for(i=0;i<8;i++)
	{
		if(dat & 0x01)
			temp |= 0x01;

		if(i == 7)
			break;

		temp = temp << 1;
		dat >>= 1;
	}

	temp = temp << 8;
	LCD_EN=0;
	LCD_RS=1;          //数据
	LCD_RW=0;          //写入
	LCD_DB=temp;//赋值给D0-D7对应的GPIO79-GPIO72引脚
	DELAY_US(500);
	LCD_EN=1;           //允许
	DELAY_US(1000);
	LCD_EN=0;
}
/*--------------------------------------
;模块名称:LCD_init();
;功    能:初始化LCD1602
;占用资源:--
;参数说明:--
;-------------------------------------*/
void LCD_init(void)
{
	DELAY_US(15000);//延迟15ms
	LCD_write_command(0x38);//设置8位格式，2行，5x7
	DELAY_US(5000);//延迟5ms
	LCD_write_command(0x38);//设置8位格式，2行，5x7
	DELAY_US(5000);
	LCD_write_command(0x38);//设置8位格式，2行，5x7
	LCD_write_command(0x38);//设置8位格式，2行，5x7
	LCD_write_command(0x08);//关显示，不显示光标，光标不闪烁；
	LCD_write_command(0x01);//清除屏幕显示：数据指针清零，所有显示清零；
	LCD_write_command(0x06);//设定输入方式，增量不移位
	LCD_write_command(0x0c);//整体显示，关光标，不闪烁
}


// Test 1,SCIA  DLB, 8-bit word, baud rate 0x000F, default, 1 STOP bit, no parity
void scib_echoback_init()
{
    // Note: Clocks were turned on to the Scib peripheral
    // in the InitSysCtrl() function

 	ScibRegs.SCICCR.all =0x0007;   // 1 stop bit,  No loopback
                                   // No parity,8 char bits,
                                   // async mode, idle-line protocol
	ScibRegs.SCICTL1.all =0x0003;  // enable TX, RX, internal SCICLK,
                                   // Disable RX ERR, SLEEP, TXWAKE
	ScibRegs.SCICTL2.all =0x0003;
	ScibRegs.SCICTL2.bit.TXINTENA =1;
	ScibRegs.SCICTL2.bit.RXBKINTENA =1;
	#if (CPU_FRQ_150MHZ)
	      ScibRegs.SCIHBAUD    =0x0000;  // 115200 baud @LSPCLK = 37.5MHz.
	      ScibRegs.SCILBAUD    =0x0028;
	#endif
	#if (CPU_FRQ_100MHZ)
      ScibRegs.SCIHBAUD    =0x0000;  // 115200 baud @LSPCLK = 20MHz.
      ScibRegs.SCILBAUD    =0x0015;
	#endif
	ScibRegs.SCICTL1.all =0x0023;  // Relinquish SCI from Reset
}

// Transmit a character from the SCI
void scib_xmit(int a)
{
    while (ScibRegs.SCIFFTX.bit.TXFFST != 0) {}
    ScibRegs.SCITXBUF=a;

}

void scib_msg(char * msg)
{
    int i;
    i = 0;
    while(msg[i] != '\0')
    {
        scib_xmit(msg[i]);
        i++;
    }
}

// Initalize the SCI FIFO
void scib_fifo_init()
{
    ScibRegs.SCIFFTX.all=0xE040;
    ScibRegs.SCIFFRX.all=0x204f;
    ScibRegs.SCIFFCT.all=0x0;

}


void Gpio_ShowState(void)
{
   // Example 1:
   // Toggle I/Os using DATA registers

   for(;;)
   {
	   // GPIO12 = 1; GPIO13 = 0;
	   GpioDataRegs.GPADAT.all = 0x00001000;
	   // GPIO50 = 1; GPIO51 = 0; GPIO52 = 0; GPIO53 = 0;
	   GpioDataRegs.GPBDAT.all = 0x00040000;
	   delay_loop_slow();

	   // GPIO50 = 0; GPIO51 = 1; GPIO52 = 0; GPIO53 = 0;
	   GpioDataRegs.GPBDAT.all = 0x00080000;
	   delay_loop_slow();

	   // GPIO12 = 0; GPIO13 = 1;
	   GpioDataRegs.GPADAT.all = 0x00002000;
	   // GPIO50 = 0; GPIO51 = 0; GPIO52 = 1; GPIO53 = 0;
	   GpioDataRegs.GPBDAT.all = 0x00100000;
	   delay_loop_slow();

	   // GPIO50 = 0; GPIO51 = 0; GPIO52 = 0; GPIO53 = 1;
	   GpioDataRegs.GPBDAT.all = 0x00200000;
	   delay_loop_slow();
    }
}

void Gpio_select(void)
{
	EALLOW;
	GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 0;  // GPIO12
	GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 0;  // GPIO13

	GpioCtrlRegs.GPADIR.bit.GPIO12 = 1;   // output
	GpioCtrlRegs.GPADIR.bit.GPIO13 = 1;   // output

	GpioCtrlRegs.GPBMUX2.bit.GPIO50 = 0;  // GPIO50
	GpioCtrlRegs.GPBMUX2.bit.GPIO51 = 0;  // GPIO51
	GpioCtrlRegs.GPBMUX2.bit.GPIO52 = 0;  // GPIO52
	GpioCtrlRegs.GPBMUX2.bit.GPIO53 = 0;  // GPIO53

    GpioCtrlRegs.GPBDIR.bit.GPIO50 = 1;   // output
    GpioCtrlRegs.GPBDIR.bit.GPIO51 = 1;   // output
    GpioCtrlRegs.GPBDIR.bit.GPIO52 = 1;   // output
    GpioCtrlRegs.GPBDIR.bit.GPIO53 = 1;   // output
    EDIS;
}




