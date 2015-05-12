#include <string.h>
#include "VS10XX.h"	
#include "wm_hostspi.h" 	   	
#include "wm_gpio.h"
#include "wm_config.h"

 #if CODEC_VS1053
extern void delay_us(unsigned int time);

//VS10XX���ò���
//0,�������ƣ�1�����Ƶ�ޣ�2���������ƣ�3�����Ƶ��
//0,henh.1,hfreq.2,lenh.3,lfreq 5,������
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
	
	   	   
//��λVS10XX
//ע��,�������������,ϵͳSPIʱ�ӱ��޸ĳ�9M����
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
	VS10XX_ID=VS_RD_Reg(SPI_STATUS);//��ȡ״̬�Ĵ���
	VS10XX_ID>>=4;					//�õ�VS10XX��оƬ�ź� 
	printf("\nVS10XX_ID===%d\n",VS10XX_ID);
	
	if(VS10XX_ID==VS1053)VS_WR_Cmd(SPI_MODE,0x0816);//�����λ,��ģʽ
	else VS_WR_Cmd(SPI_MODE,0x0804);				//�����λ,��ģʽ

	while(tls_gpio_read(VS_DQ) ==0 && retry<200)			//�ȴ�DREQΪ��
	{
		retry++;
		delay_us(50);
	}; 	
	retry=0;
	while(VS_RD_Reg(SPI_CLOCKF)!=0x9800)//�ȴ����óɹ�0x9800
	{
		VS_WR_Cmd(SPI_CLOCKF,0x9800);  	//����VS10XX��ʱ��,3.5��Ƶ ,2xADD  
		if(retry++>100)break; 
	}		   				
 	VS_Rst_DecodeTime();				//��λ����ʱ��	    
    //��VS10XX����4���ֽ���Ч���ݣ���������SPI����
 	tls_spi_setup(TLS_SPI_MODE_0, TLS_SPI_CS_HIGH, 5000000);	//5M
	codec_data_cs_low();
	tls_spi_write(temp, 4);
	codec_data_cs_high();
} 
//Ӳ��λMP3
//����1:��λʧ��
//����0:��λ�ɹ�
u8 VS_HD_Reset(void)
{
	u8 retry=0;
	tls_gpio_write(VS_XDCS, 1);	//ȡ�����ݴ���
	tls_gpio_write(VS_XCS, 1);
	tls_gpio_write(VS_RST, 0);
	tls_os_time_delay(10);
	tls_gpio_write(VS_RST, 1);
	while(tls_gpio_read(VS_DQ)==0&&retry<200)//�ȴ�DREQΪ��
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
//���Ҳ��� 
void VS_Sine_Test(void)
{					
	u8 temp[10];
	
	VS_HD_Reset();	 
	VS_WR_Cmd(0x0b,0X2020);	  //��������	 
 	VS_WR_Cmd(SPI_MODE,0x0820);//����VS10XX�Ĳ���ģʽ     
	while(tls_gpio_read(VS_DQ)== 0);     //�ȴ�DREQΪ��
	//printf("mode sin:%x\n",VS_RD_Reg(SPI_MODE));
 	//��VS10XX�������Ҳ������0x53 0xef 0x6e n 0x00 0x00 0x00 0x00
 	//����n = 0x24, �趨VS10XX�����������Ҳ���Ƶ��ֵ��������㷽����VS10XX��datasheet
  	tls_spi_setup(TLS_SPI_MODE_0, TLS_SPI_CS_LOW, 1000000);	// 1M
	tls_gpio_write(VS_XDCS, 0);	//ѡ�����ݴ���
	memset(temp, 0, sizeof(temp));
	temp[0] = 0x53;
	temp[1] = 0xef;
	temp[2] = 0x6e;
	temp[3] = 0x24;
	tls_spi_write(temp, 8);
	tls_os_time_delay(20);
	tls_gpio_write(VS_XDCS, 1);
    //�˳����Ҳ���
    tls_gpio_write(VS_XDCS, 0);//ѡ�����ݴ���
    	temp[0] = 0x45;
	temp[1] = 0x78;
	temp[2] = 0x69;
	temp[3] = 0x74;
	tls_spi_write(temp, 8);
	tls_gpio_write(VS_XDCS, 1);
    //�ٴν������Ҳ��Բ�����nֵΪ0x44���������Ҳ���Ƶ������Ϊ�����ֵ
    	tls_gpio_write(VS_XDCS, 0);
	temp[0] = 0x53;
	temp[1] = 0xef;
	temp[2] = 0x6e;
	temp[3] = 0x44;
	tls_spi_write(temp, 8);
	tls_os_time_delay(20);
	tls_gpio_write(VS_XDCS, 1);
    //�˳����Ҳ���
   	tls_gpio_write(VS_XDCS, 0);
	temp[0] = 0x45;
	temp[1] = 0x78;
	temp[2] = 0x69;
	temp[3] = 0x74;
	tls_spi_write(temp, 8);
	tls_gpio_write(VS_XDCS, 1);
} 	  
//VS10XX ram ���� 	
//����ֵ:
//��1053:0X83FF,����ȫ������OK
//��1003:0x807F,����ȫ������OK																			 
u16 VS_Ram_Test(void)
{		
	u16 RAM_Read;			
	u8 temp[10];
	
	VS_HD_Reset();     		   
 	VS_WR_Cmd(SPI_MODE,0x0820);		// ����VS10XX��RAM����ģʽ
	while(tls_gpio_read(VS_DQ) == 0);	// �ȴ�DREQΪ��	   
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
	// ����õ���ֵΪ0x807F���������á�			 
	printf("SPI_HDAT0:%x\n",RAM_Read);//������ 
	
	return RAM_Read;
}		
#endif
//��VS10XXд����
//address:�����ַ
//data:��������
void VS_WR_Cmd(u8 address,u16 data)
{  
	u16 retry=0;
	u8 temp[5];
	
	while(tls_gpio_read(VS_DQ)==0&&(retry++)<0xfffe)retry++;	//�ȴ�����    
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
//��VS10XXд����
void VS_WR_Data(u8 data)
{
	u8 temp[2];
	memset(temp, 0, sizeof(temp));
	temp[0] = data;
	codec_data_cs_low();
	tls_spi_write(temp, 1);
	codec_data_cs_high();
}         
//��VS10XX�ļĴ���     
//ע�ⲻҪ�ñ��ٶ�ȡ,�����
u16 VS_RD_Reg(u8 address)
{ 
	u16 temp=0; 
	u8 retry=0;
	u8 txbuf[4];
	u8 rxbuf[4];
	
	while(tls_gpio_read(VS_DQ)==0&&(retry++)<0XFE);	//�ȴ�����	  
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
//��ȡVS10XX��RAM
u16 VS_RD_Wram(u16 addr) 
{ 			   	  
	VS_WR_Cmd(SPI_WRAMADDR, addr); 
	return VS_RD_Reg(SPI_WRAM);  
} 
  
//FOR WAV HEAD0 :0X7761 HEAD1:0X7665    
//FOR MIDI HEAD0 :other info HEAD1:0X4D54
//FOR WMA HEAD0 :data speed HEAD1:0X574D
//FOR MP3 HEAD0 :data speed HEAD1:ID
//������Ԥ��ֵ
const u16 bitrate[2][16]=
{ 
{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0}, 
{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0}
};
//����Kbps�Ĵ�С
//�õ�mp3&wma�Ĳ�����
u16 VS_Get_HeadInfo(void)
{
	unsigned int HEAD0;
	unsigned int HEAD1;            
    HEAD0=VS_RD_Reg(SPI_HDAT0); 
    HEAD1=VS_RD_Reg(SPI_HDAT1);
    switch(HEAD1)
    {        
        case 0x7665:return 0;	//WAV��ʽ
        case 0X4D54:			//MIDI��ʽ 
		case 0X4154:			//AAC_ADTS
		case 0X4144:			//AAC_ADIF
		case 0X4D34:			//AAC_MP4/M4A
		case 0X4F67:			//OGG
        case 0X574D:			//WMA��ʽ
        {
			////printf("HEAD0:%d\n",HEAD0);
            HEAD1=HEAD0*2/25;					//�൱��*8/100
            if((HEAD1%10)>5)return HEAD1/10+1;	//��С�����һλ��������
            else return HEAD1/10;
        }
        default://MP3��ʽ
        {
            HEAD1>>=3;
            HEAD1=HEAD1&0x03; 
            if(HEAD1==3)HEAD1=1;
            else HEAD1=0;
            return bitrate[HEAD1][HEAD0>>12];
        }
    } 
}  
//�������ʱ��                          
void VS_Rst_DecodeTime(void)
{
	VS_WR_Cmd(SPI_DECODE_TIME,0x0000);
	VS_WR_Cmd(SPI_DECODE_TIME,0x0000);//��������
}
//�õ�mp3�Ĳ���ʱ��n sec
u16 VS_Get_DecodeTime(void)
{ 
    return VS_RD_Reg(SPI_DECODE_TIME);   
} 	 

//vs10xxװ��patch.
//patch��patch�׵�ַ
//len��patch����
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
		VS_WR_Cmd(SPI_VOL,volt); //������ 	
	}
}

void vs_mute(void)
{
	VS_WR_Cmd(SPI_VOL,0xfefe); //������ 	
}

//�趨VS10XX���ŵ������͸ߵ��� 
void set10XX(void)
{
    u8 t;
    u16 bass=0; //�ݴ������Ĵ���ֵ
    u16 volt=0; //�ݴ�����ֵ
    u8 vset=0;  //�ݴ�����ֵ 	 
    vset=255-VS10XXram[4];//ȡ��һ��,�õ����ֵ,��ʾ���ı�ʾ 
    volt=vset;
    volt<<=8;
    volt+=vset;//�õ��������ú��С
     //0,henh.1,hfreq.2,lenh.3,lfreq        
    for(t=0;t<4;t++)
    {
        bass<<=4;
        bass+=VS10XXram[t]; 
    }     
	VS_WR_Cmd(SPI_BASS,bass);//BASS   
    VS_WR_Cmd(SPI_VOL,volt); //������ 
} 



//��ʼ��VS10XX��IO��	 
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
