#include <string.h>
#include "VS10XX.h"	
#include "wm_hostspi.h" 	   	
#include "wm_gpio.h"
#include "wm_config.h"

 #if CODEC_VS1053
extern void delay_us(unsigned int time);

//VS10XX设置参数
//0,高音控制；1，最低频限；2，低音控制；3，最低频限
//0,henh.1,hfreq.2,lenh.3,lfreq 5,主音量
u8 VS10XX_ID=0;
//u8 VS10XXram[5]={0,0,0,0,230};	
u8 VS10XXram[5]={15,6,7,10,230};	

void codec_data_cs_low(void)
{
	while(0 == tls_gpio_read(SPI_CS_PIN))
	{	
		printf("\nflash is busy,codec data cs wait...\n");
		tls_os_time_delay(1);
	}
	tls_gpio_write(VS_XDCS, 0);
}

void codec_data_cs_high(void)
{
	tls_gpio_write(VS_XDCS, 1);
}

void codec_cmd_cs_low(void)
{
	while(0 == tls_gpio_read(SPI_CS_PIN))
	{	
		printf("\nflash is busy,codec cmd cs wait...\n");
		tls_os_time_delay(1);
	}
	tls_gpio_write(VS_XCS, 0);
}

void codec_cmd_cs_high(void)
{
	tls_gpio_write(VS_XCS, 1);
}
	
	   	   
//软复位VS10XX
//注意,在这里设置完后,系统SPI时钟被修改成9M左右
void VS_Soft_Reset(void)
{	 
	u8 retry; 
	u8 temp[6];
	tls_spi_setup(TLS_SPI_MODE_0, TLS_SPI_CS_LOW, 250000);	//250K
	while(tls_gpio_read(VS_DQ) == 0);
	memset(temp,0,sizeof(temp));
	codec_data_cs_low();
	tls_spi_write(temp, 1);
	codec_data_cs_high();
	retry=0; 					
	VS10XX_ID=VS_RD_Reg(SPI_STATUS);//读取状态寄存器
	VS10XX_ID>>=4;					//得到VS10XX的芯片信号 
	printf("\nVS10XX_ID===%d\n",VS10XX_ID);
	
	if(VS10XX_ID==VS1053)VS_WR_Cmd(SPI_MODE,0x0816);//软件复位,新模式
	else VS_WR_Cmd(SPI_MODE,0x0804);				//软件复位,新模式

	while(tls_gpio_read(VS_DQ) ==0 && retry<200)			//等待DREQ为高
	{
		retry++;
		delay_us(50);
	}; 	
	retry=0;
	while(VS_RD_Reg(SPI_CLOCKF)!=0x9800)//等待设置成功0x9800
	{
		VS_WR_Cmd(SPI_CLOCKF,0x9800);  	//设置VS10XX的时钟,3.5倍频 ,2xADD  
		if(retry++>100)break; 
	}		   				
 	VS_Rst_DecodeTime();				//复位解码时间	    
    //向VS10XX发送4个字节无效数据，用以启动SPI发送
 	tls_spi_setup(TLS_SPI_MODE_0, TLS_SPI_CS_HIGH, 5000000);	//5M
	codec_data_cs_low();
	tls_spi_write(temp, 4);
	codec_data_cs_high();
} 
//硬复位MP3
//返回1:复位失败
//返回0:复位成功
u8 VS_HD_Reset(void)
{
	u8 retry=0;
	tls_gpio_write(VS_XDCS, 1);	//取消数据传输
	tls_gpio_write(VS_XCS, 1);
	tls_gpio_write(VS_RST, 0);
	tls_os_time_delay(10);
	tls_gpio_write(VS_RST, 1);
	while(tls_gpio_read(VS_DQ)==0&&retry<200)//等待DREQ为高
	{
		retry++;
		delay_us(50);
	}; 	   
	tls_os_time_delay(2);
	if(retry>=200)
	{
		printf("\nvs hd reset fail\n");
		return 1;
	}
	else 
		return 0;	    		 
}
#if 0
//正弦测试 
void VS_Sine_Test(void)
{					
	u8 temp[10];
	
	VS_HD_Reset();	 
	VS_WR_Cmd(0x0b,0X2020);	  //设置音量	 
 	VS_WR_Cmd(SPI_MODE,0x0820);//进入VS10XX的测试模式     
	while(tls_gpio_read(VS_DQ)== 0);     //等待DREQ为高
	//printf("mode sin:%x\n",VS_RD_Reg(SPI_MODE));
 	//向VS10XX发送正弦测试命令：0x53 0xef 0x6e n 0x00 0x00 0x00 0x00
 	//其中n = 0x24, 设定VS10XX所产生的正弦波的频率值，具体计算方法见VS10XX的datasheet
  	tls_spi_setup(TLS_SPI_MODE_0, TLS_SPI_CS_LOW, 1000000);	// 1M
	tls_gpio_write(VS_XDCS, 0);	//选中数据传输
	memset(temp, 0, sizeof(temp));
	temp[0] = 0x53;
	temp[1] = 0xef;
	temp[2] = 0x6e;
	temp[3] = 0x24;
	tls_spi_write(temp, 8);
	tls_os_time_delay(20);
	tls_gpio_write(VS_XDCS, 1);
    //退出正弦测试
    tls_gpio_write(VS_XDCS, 0);//选中数据传输
    	temp[0] = 0x45;
	temp[1] = 0x78;
	temp[2] = 0x69;
	temp[3] = 0x74;
	tls_spi_write(temp, 8);
	tls_gpio_write(VS_XDCS, 1);
    //再次进入正弦测试并设置n值为0x44，即将正弦波的频率设置为另外的值
    	tls_gpio_write(VS_XDCS, 0);
	temp[0] = 0x53;
	temp[1] = 0xef;
	temp[2] = 0x6e;
	temp[3] = 0x44;
	tls_spi_write(temp, 8);
	tls_os_time_delay(20);
	tls_gpio_write(VS_XDCS, 1);
    //退出正弦测试
   	tls_gpio_write(VS_XDCS, 0);
	temp[0] = 0x45;
	temp[1] = 0x78;
	temp[2] = 0x69;
	temp[3] = 0x74;
	tls_spi_write(temp, 8);
	tls_gpio_write(VS_XDCS, 1);
} 	  
//VS10XX ram 测试 	
//返回值:
//对1053:0X83FF,表明全部测试OK
//对1003:0x807F,表明全部测试OK																			 
u16 VS_Ram_Test(void)
{		
	u16 RAM_Read;			
	u8 temp[10];
	
	VS_HD_Reset();     		   
 	VS_WR_Cmd(SPI_MODE,0x0820);		// 进入VS10XX的RAM测试模式
	while(tls_gpio_read(VS_DQ) == 0);	// 等待DREQ为高	   
	tls_spi_setup(TLS_SPI_MODE_0, TLS_SPI_CS_LOW, 250000);	//250K
	memset(temp, 0, sizeof(temp));
	tls_gpio_write(VS_XDCS, 0);
	temp[0] = 0x4d;
	temp[1] = 0xea;
	temp[2] = 0x6d;
	temp[3] = 0x54;
	tls_spi_write(temp, 8);
	tls_gpio_write(VS_XDCS, 1);
	tls_os_time_delay(20);
	RAM_Read=VS_RD_Reg(SPI_HDAT0);
	// 如果得到的值为0x807F，则表明完好。			 
	printf("SPI_HDAT0:%x\n",RAM_Read);//输出结果 
	
	return RAM_Read;
}		
#endif
//向VS10XX写命令
//address:命令地址
//data:命令数据
void VS_WR_Cmd(u8 address,u16 data)
{  
	u16 retry=0;
	u8 temp[5];
	
	while(tls_gpio_read(VS_DQ)==0&&(retry++)<0xfffe)retry++;	//等待空闲    
	tls_spi_setup(TLS_SPI_MODE_0, TLS_SPI_CS_LOW, 1000000);	// 1M
	codec_data_cs_high();
	memset(temp, 0, sizeof(temp));
	
	codec_cmd_cs_low();
	temp[0] = VS_WRITE_COMMAND;
	temp[1] = address;
	temp[2] = data>>8;
	temp[3] = data&0xff;
	tls_spi_write(temp, 4);
	codec_cmd_cs_high();
	tls_spi_setup(TLS_SPI_MODE_0, TLS_SPI_CS_LOW, 5000000);	// 5M
} 
//向VS10XX写数据
void VS_WR_Data(u8 data)
{
	u8 temp[2];
	memset(temp, 0, sizeof(temp));
	temp[0] = data;
	codec_data_cs_low();
	tls_spi_write(temp, 1);
	codec_data_cs_high();
}         
//读VS10XX的寄存器     
//注意不要用倍速读取,会出错
u16 VS_RD_Reg(u8 address)
{ 
	u16 temp=0; 
	u8 retry=0;
	u8 txbuf[4];
	u8 rxbuf[4];
	
	while(tls_gpio_read(VS_DQ)==0&&(retry++)<0XFE);	//等待空闲	  
	if(retry>=0XFE)return 0;	 						    
	tls_spi_setup(TLS_SPI_MODE_0, TLS_SPI_CS_LOW, 1000000);	// 1M
	codec_data_cs_high();
	memset(txbuf, 0, sizeof(txbuf));
	memset(rxbuf, 0, sizeof(rxbuf));
	codec_cmd_cs_low();
	txbuf[0] = VS_READ_COMMAND;
	txbuf[1] = address;
	tls_spi_write_then_read(txbuf, 2, rxbuf, 2);
	codec_cmd_cs_high();
	temp = rxbuf[0] << 8;
	temp |= rxbuf[1];
	tls_spi_setup(TLS_SPI_MODE_0, TLS_SPI_CS_LOW, 5000000);	// 5M
	
    return temp; 
}  
//读取VS10XX的RAM
u16 VS_RD_Wram(u16 addr) 
{ 			   	  
	VS_WR_Cmd(SPI_WRAMADDR, addr); 
	return VS_RD_Reg(SPI_WRAM);  
} 
  
//FOR WAV HEAD0 :0X7761 HEAD1:0X7665    
//FOR MIDI HEAD0 :other info HEAD1:0X4D54
//FOR WMA HEAD0 :data speed HEAD1:0X574D
//FOR MP3 HEAD0 :data speed HEAD1:ID
//比特率预定值
const u16 bitrate[2][16]=
{ 
{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0}, 
{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0}
};
//返回Kbps的大小
//得到mp3&wma的波特率
u16 VS_Get_HeadInfo(void)
{
	unsigned int HEAD0;
	unsigned int HEAD1;            
    HEAD0=VS_RD_Reg(SPI_HDAT0); 
    HEAD1=VS_RD_Reg(SPI_HDAT1);
    switch(HEAD1)
    {        
        case 0x7665:return 0;	//WAV格式
        case 0X4D54:			//MIDI格式 
		case 0X4154:			//AAC_ADTS
		case 0X4144:			//AAC_ADIF
		case 0X4D34:			//AAC_MP4/M4A
		case 0X4F67:			//OGG
        case 0X574D:			//WMA格式
        {
			////printf("HEAD0:%d\n",HEAD0);
            HEAD1=HEAD0*2/25;					//相当于*8/100
            if((HEAD1%10)>5)return HEAD1/10+1;	//对小数点第一位四舍五入
            else return HEAD1/10;
        }
        default://MP3格式
        {
            HEAD1>>=3;
            HEAD1=HEAD1&0x03; 
            if(HEAD1==3)HEAD1=1;
            else HEAD1=0;
            return bitrate[HEAD1][HEAD0>>12];
        }
    } 
}  
//重设解码时间                          
void VS_Rst_DecodeTime(void)
{
	VS_WR_Cmd(SPI_DECODE_TIME,0x0000);
	VS_WR_Cmd(SPI_DECODE_TIME,0x0000);//操作两次
}
//得到mp3的播放时间n sec
u16 VS_Get_DecodeTime(void)
{ 
    return VS_RD_Reg(SPI_DECODE_TIME);   
} 	 

//vs10xx装载patch.
//patch：patch首地址
//len：patch长度
void VS_Load_Patch(u16 *patch,u16 len) 
{
	u16 i; 
	u16 addr, n, val; 	  			   
	for(i=0;i<len;) 
	{ 
		addr = patch[i++]; 
		n    = patch[i++]; 
		if(n & 0x8000U) //RLE run, replicate n samples 
		{ 
			n  &= 0x7FFF; 
			val = patch[i++]; 
			while(n--)VS_WR_Cmd(addr, val);  
		}else //copy run, copy n sample 
		{ 
			while(n--) 
			{ 
				val = patch[i++]; 
				VS_WR_Cmd(addr, val); 
			} 
		} 
	} 	
} 	     		   

void vs_set_vol(int vol)
{
	int tmp;
	u16 volt = 0;
	
	tmp = vol;
	printf("\nvol==%d,tmp=%d\n",vol,tmp);
	if(vol >= 0 && vol <= 100)
	{
		volt = tmp<<8;
		volt += tmp;
		printf("\nvolt==%x\n",volt);
		VS_WR_Cmd(SPI_VOL,volt); //设音量 	
	}
}

void vs_mute(void)
{
	VS_WR_Cmd(SPI_VOL,0xfefe); //设音量 	
}

//设定VS10XX播放的音量和高低音 
void set10XX(void)
{
    u8 t;
    u16 bass=0; //暂存音调寄存器值
    u16 volt=0; //暂存音量值
    u8 vset=0;  //暂存音量值 	 
    vset=255-VS10XXram[4];//取反一下,得到最大值,表示最大的表示 
    volt=vset;
    volt<<=8;
    volt+=vset;//得到音量设置后大小
     //0,henh.1,hfreq.2,lenh.3,lfreq        
    for(t=0;t<4;t++)
    {
        bass<<=4;
        bass+=VS10XXram[t]; 
    }     
	VS_WR_Cmd(SPI_BASS,bass);//BASS   
    VS_WR_Cmd(SPI_VOL,volt); //设音量 
} 



//初始化VS10XX的IO口	 
void VS_Init(void)
{
	tls_gpio_cfg(VS_RST, TLS_GPIO_DIR_OUTPUT, TLS_GPIO_ATTR_FLOATING);
	tls_gpio_cfg(VS_XCS, TLS_GPIO_DIR_OUTPUT, TLS_GPIO_ATTR_FLOATING);
	tls_gpio_write(VS_XCS, 1);
	tls_gpio_cfg(VS_DQ, TLS_GPIO_DIR_INPUT, TLS_GPIO_ATTR_PULLHIGH);
	tls_gpio_cfg(VS_XDCS, TLS_GPIO_DIR_OUTPUT, TLS_GPIO_ATTR_FLOATING);
	tls_gpio_write(VS_XDCS, 1);
} 

#endif
