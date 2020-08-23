/*****************
*file: boot_task.c
*Author: dingfojin
*date: 2005.12.2
****************************/

#include "l3BootTask.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sysBtsConfigData.h"

#define LONGSWAP_D(value)  (value)

//#if !defined(__WIN32_SIM__)&&!defined(__NUCLEUS__)
    #include <ioLib.h>
    #include <stat.h>
    #include <netDrv.h>
    #include <fiolib.h>
    #include <ramDrv.h>
    #include <dosFsLib.h>
    #include <remLib.h>
    #include <taskLib.h>
    #include <inetLib.h>
    #include <vmLib.h>
//#endif


#include "Message.h"
#include "ComMessage.h"
#include "Timer.h"
#include "log.h"
#ifdef WBBU_CODE
#define	CF2ST0_DELAY			10	/* 800ns - nCONFIG low to nSTATUS low */
#define	CF2CK_DELAY			2000	/* 40us - nCONFIG high to first rising edge on DCLK */
#define	DSU_DELAY			1		/* 10ns - Data setup time before rising edge on DCLK */
#define	CH_DELAY				1 /* 10ns - DCLK high time */
#define	CL_DELAY				4	/* 10ns - DCLK low time */
#define	CD2UM_DELAY_COUNT	100	/* 20us - CONF_DONE high to user mode */
extern unsigned char  L2_BOOT_FLAG;
#endif
extern "C" int  bootFlag;    /***0x55 is disable, other enable************/
extern "C" int i2c_read(unsigned int device_addr, unsigned int offset_addr,unsigned char* val, unsigned int len);

void OAM_LOGSTR(LOGLEVEL level, UINT32 errcode, const char* text);
void OAM_LOGSTR1(LOGLEVEL level, UINT32 errcode, const char* text, int arg1);
#ifndef WBBU_CODE
/*从909项目流中移植过来 jiaying20100805*/
#define MV64360_GPP_VAL (0xf104)      
#define MV64360_BASE_ADRS  (0x14000000)
#undef  EIEIO
#define EIEIO              WRS_ASM (" eieio")

/* Read, write macros for system MV64360 controller */

#define MV64360_REG_RD(x,val)	\
{ \
		UINT32 tmpVar ; \
		tmpVar = *((volatile UINT32*)(MV64360_BASE_ADRS + (x))); \
		EIEIO ; \
		(*val) = LONGSWAP(tmpVar); \
		EIEIO ; \
}
    
#define MV64360_REG_WR(x,val)	\
{	\
		UINT32 tmpVar ; \
                tmpVar = LONGSWAP(val); \
		EIEIO ; \
		*((volatile UINT32*)(MV64360_BASE_ADRS + (x))) = tmpVar ; \
		EIEIO ; \
}
#endif
#ifdef WBBU_CODE
extern "C" {
typedef void (*pfBOOTRxCallBack)(char *, UINT16);
void Drv_RegisterBOOT(pfBOOTRxCallBack BOOTFunc);
 void reset_dsp(unsigned short index)  ;
void ResetDspNew(unsigned char index);
unsigned int Read_Fpga_Version_Soft();
void Set_Fpga_Clk(unsigned char type);
}
#endif
#ifndef M_TGT_WANIF
const char L2_BOOTROM_FILENAME[] = "L2_Image.bin";
#else
const char L2_BOOTROM_FILENAME[] = "L2_Image.bin";
#endif
const char L2_APP_CODE_FILENAME[] = "L2_app";
const char MCP_APP_CODE_FILENAME[] = "app_mcp.out";
const char AUX_APP_CODE_FILENAME[] = "app_aux.out";
const char FEP_APP_CODE_FILENAME[] = "app_fep.out";
const char FPGA_L2_FILENAME[] = "fpga_L2.out";
const char FPGA_L1_FILENAME[] = "fpga_L1.out";
const char FPGA_FEP0_FILENAME[] = "fpga_FEP0.out";
const char FPGA_FEP1_FILENAME[] = "fpga_FEP1.out";
#ifdef WBBU_CODE
const char FPGA_WBBU_FILENAME[] = "wbbu_fpga.bin";//wangwenhua add 20090522
const char DSP_Name_No5_L2[] = "dsp_image_L2_5";
const char DSP_Name_No4_L1[] = "dsp_image_L1_4";
const char DSP_Name_No3_L1[] = "dsp_image_L1_3";
const char DSP_Name_No2_L1[] = "dsp_image_L1_2";
const char DSP_Name_No1_L1[] = "dsp_image_L1_1";
const char WRRU_Mcu[] = "wrru_mcu";
const char WRRU_Fpga[] = "wrru_fpga";
char g_dsp1_exception_data[3][200]={0};
#endif
#ifndef WBBU_CODE
extern "C"		int timesIn5mins_AUX ;
extern "C"		int timesIn5mins_FEP0 ;
extern "C"		int timesIn5mins_FEP1 ;
#endif

L3BootTask* L3BootTask::instance=NULL;
#ifdef WBBU_CODE
static unsigned char Reset_DSP_FLAG = 0;/***0x11-dsp1,0x22-dsp 2,0x33,dsp 3,0x44,0x55****/
extern "C" unsigned char Reset_Dsp(unsigned char index,unsigned char flag);
extern "C"  void   Reset_All_DSP();

extern  "C" unsigned int    bspGetBootSource();
extern "C" void SetLocalFepRxInt(unsigned char flag);//wangwenhua add 20111026
extern "C" void PowerControl(unsigned char flag);
extern "C" unsigned char  RestFPGA();
extern "C" void    ResetAifSerdies();
#define   M_BOOT_ONE_Minter_Timer_M   0x350e  
#endif
L3BootTask::L3BootTask ()
{
    ::strcpy(m_szName, "tL3Boot");
    m_uPriority = M_TP_L3BM;//M_TP_L3BM
    m_uOptions = 0;
    m_uStackSize = 20000;
    m_iMsgQMax = 1000;

    /***state init***/
    for ( int i = 0; i<8; i++ )
    {
        state_MCP[i] = BOOTING;
        timer_MCP[i] = NULL;
    }   

    state_AUX = BOOTING; 
    timer_AUX = NULL;

    state_FEP[0] = BOOTING; 
    state_FEP[1] = BOOTING;
    timer_FEP[0] = NULL;
    timer_FEP[1] = NULL;

    state_L2 = BOOTING;
    state_System = BOOTING;
    timer_L2 = NULL;
    timer_L2Preloader = NULL;
#ifdef WBBU_CODE
    Vertimer = NULL;
    ActiveUsertime = NULL;
#endif
	#ifndef WBBU_CODE
   /*lijinan 20090202*/
   timesIn5mins_AUX = 0;
   timesIn5mins_FEP0 = 0;
   timesIn5mins_FEP1 = 0;

#endif
}



bool L3BootTask::IsNeedTransaction() const
{
    return false;
}

TID L3BootTask::GetEntityId() const
{
    return M_TID_BM;//L3Boot id;
}

L3BootTask::~L3BootTask ()
{
}


L3BootTask* L3BootTask::GetInstance()
{
    if ( instance == NULL )
    {
        instance = new L3BootTask;
    }
    return instance;
}
#ifdef WBBU_CODE
unsigned char dsp_addr[6];
unsigned short dsp_addr_H = 0;
unsigned short dsp_addr_M = 0;
unsigned short dsp_addr_L = 0;
extern "C"
{
	void bootDSPImage (unsigned char index,const char* filename,unsigned short  dsp_addr_H,unsigned short dsp_addr_M ,unsigned short dsp_addr_L);
}

void L3BootTask::BootPCallBack(char *pdata, UINT16 len)
{
  #if 1
    int i;
    unsigned short index =0xff;
    unsigned short msgid = 0x00;
    if(Reset_DSP_FLAG==0x11)
    	{
    		printf("DSP 1 bootp data\n");
    	}
    else if(Reset_DSP_FLAG==0x22)
    	{
    		printf("DSP 2 bootp data\n");
    	}
    else    if(Reset_DSP_FLAG==0x33)
    	{
    		printf("DSP 3 bootp data\n");
    	}
    else     if(Reset_DSP_FLAG==0x44)
    	{
    		printf("DSP 4 bootp data\n");
    	}
    else     if(Reset_DSP_FLAG==0x55)
    	{
    		printf("DSP 5 bootp data\n");
    	}
    else
    	{
    	     //  printf("Other Packet data \n");
    	}
    for(i = 0; i < 16;i++)
    	{
    	            if((i>5) &&(i<12))
    	            	{
    	            	     dsp_addr[i-6] = pdata[i];
    	            	}
    	             printf("%x,",pdata[i]);

    	}
	printf("\n");
   // ff,ff,ff,ff,ff,ff,8,0,28,16,b0,72,8,0,45,0,1,48,0,1,0,0,10,11,a9,a5,0,0,0,0,0,0,0,
//0,0,44,0,43
//    dsp_addr_H = dsp_addr[0]*0x100+dsp_addr[1];
//     dsp_addr_M = dsp_addr[2]*0x100+dsp_addr[3];
 //     dsp_addr_L = dsp_addr[4]*0x100+dsp_addr[5];
     // bootDSPImage(5,"/RAMD:0/load/dsp_image");
  #endif
  if((pdata[35] == 0x44)&&(pdata[37] == 0x43))/***boot packet***/
  {
     msgid = DSP_BOOT;
  }
  else if((pdata[12] == 0xaa)&&(pdata[13] == 0xaa))
  {
       //return;
           for(i = 0; i < len;i++)
    	{
    	        
    	     printf("%x,",pdata[i]);

    	}
	printf("\n");
       msgid = L2_RUNING;
	L2_BOOT_FLAG=1;
  }
  else if((pdata[12] == 0x13)&&(pdata[13] == 0x13))
    {
          if(pdata[14]==0x12)//aux
            {
                 memcpy(g_dsp1_exception_data[0],pdata,200);
            }
          else if(pdata[14]==0)//fep 0
            {
                 memcpy(g_dsp1_exception_data[1],pdata,200);
            }
          else if(pdata[14]==0x10)//fep1
            {
                 memcpy(g_dsp1_exception_data[2],pdata,200);
            }
    }
  else
  {
         if((pdata[6] == 0x30)&&(pdata[11] == 0x35)) //L1 DSP 要求将DSP boot success 给打印出来
         {
              printf("DSP1 boot Success\n");
         }
         else   if((pdata[6] == 0x30)&&(pdata[11] == 0x36))
         {
              printf("DSP2 boot Success\n");
         }
         else   if((pdata[6] == 0x30)&&(pdata[11] == 0x37))
        {
              printf("DSP3 boot Success\n");
         }
        else   if((pdata[6] == 0x30)&&(pdata[11] == 0x38))
         {
              printf("DSP4 boot Success\n");
         }
  	   return;
  	}
 
     static L3BootTask *pBootnstance = NULL;
    pBootnstance = L3BootTask::GetInstance();
   
   
    CComMessage *pComMsg = new( L3BootTask::GetInstance(), 8 ) CComMessage;
     unsigned char * p ;
     p=(unsigned char*) pComMsg->GetDataPtr();
    pComMsg->SetDataLength(8);
    pComMsg->SetMessageId(msgid/*DSP_BOOT*/);
    pComMsg->SetDstTid(M_TID_BM);
    pComMsg->SetSrcTid(M_TID_BM);
    memcpy(p+2,&dsp_addr[0],6);//MAC地址
    if(msgid==DSP_BOOT)
    	{
   
   
      //判断是bootp协议
      if(Reset_DSP_FLAG==0x11)
      	{
      	//      printf("boot dsp 1\n");
        //     bootDSPImage(5,"/RAMD:0/load/dsp_image_L1_1");
          index = 1;
      	}
      else if(Reset_DSP_FLAG == 0x22)
      	{
      	    //    printf("boot dsp 2\n");
      	      //    bootDSPImage(5,"/RAMD:0/load/dsp_image_L1_2");
      	      index = 2;
      	}
        else if(Reset_DSP_FLAG == 0x33)
      	{
      	  //    printf("boot dsp 3\n");
      	     //  bootDSPImage(5,"/RAMD:0/load/dsp_image_L1_3");
      	     index = 3;
      	}
          else if(Reset_DSP_FLAG == 0x44)
      	{
      	 //       printf("boot dsp 4\n");
      	      //  bootDSPImage(5,"/RAMD:0/load/dsp_image_L1_4");
      	      index= 4;
      	}
         else if(Reset_DSP_FLAG == 0x55)
      	{
      	 //      printf("boot dsp 5\n");
      	      //  bootDSPImage(5,"/RAMD:0/load/dsp_image_L2_5");
      	      index = 5;
      	}  
         Reset_DSP_FLAG = 0;

         memcpy(p,(unsigned char*)&index,2);
    	}
         CComEntity::PostEntityMessage(pComMsg);

}
#endif
void L3BootTask::down_fpga()
{
    //100331 lijinan
    if ( bootFlag == REBOOT_DISABLE )
    {
        OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> down_fpga is disabled, please execute rebootEnable()");        
        return;
    }
    
    int i, j, count;
    unsigned int  temp;
    char filename_L2[40];
    char filename_L1[40];
    char filename_FEP0[50], filename_FEP1[50];
    FILE *file_L2,* file_L1, *file_FEP0, *file_FEP1;

    j = 5;
    while ( j-- )
    {
        unsigned int cpld_cmd;

        /****fpga state******/
        CPLD_READ_REG(FPGA_STATE_REG, &temp);   
        while ( !(temp & (0x1<<8))/***bit8***/ )
        {
            CPLD_READ_REG(FPGA_STATE_REG, &temp);   
        }
        /***config data state******/
        CPLD_WRITE_REG(CONFIG_DATASTATE_REG, 0x0);


        /*start config fpga***/
        /*CPLD_WRITE_REG(SYSTEM_CMD_REG, (1<<0) | (1<<1));*/   /**bit 1 watchdog*/
        CPLD_READ_REG(SYSTEM_CMD_REG, &cpld_cmd);   
        cpld_cmd |= 1<<0;
        CPLD_WRITE_REG(SYSTEM_CMD_REG, cpld_cmd);

        /***download fpga******/
        strcpy(filename_L2, DEVICE_RAMDISK);
        strcat(filename_L2, RAMDISK_DOWNLOAD_PATH);
        strcat(filename_L2, FPGA_L2_FILENAME);
        file_L2 = fopen(filename_L2, "rb");
        if ( file_L2 == NULL )
        {
            LOG(LOG_CRITICAL, 0, "L2 fpga file open fail!! ");      
            break;
        }

        strcpy(filename_L1, DEVICE_RAMDISK);
        strcat(filename_L1, RAMDISK_DOWNLOAD_PATH);
        strcat(filename_L1, FPGA_L1_FILENAME);
        file_L1 = fopen(filename_L1, "rb");
        if ( file_L1 == NULL )
        {
            LOG(LOG_CRITICAL, 0, "L1 fpga file open fail!!");  
            break;
        }

        strcpy(filename_FEP1, DEVICE_RAMDISK);
        strcat(filename_FEP1, RAMDISK_DOWNLOAD_PATH);
		strcpy(filename_FEP0, DEVICE_RAMDISK);
		strcat(filename_FEP0, RAMDISK_DOWNLOAD_PATH);

		strcat(filename_FEP1, FPGA_FEP1_FILENAME);
		strcat(filename_FEP0, FPGA_FEP0_FILENAME);

		file_FEP1 = fopen(filename_FEP1, "rb");
        if ( file_FEP1 == NULL )
        {
            LOG(LOG_CRITICAL, 0, "fep1 fpga file open fail!! "); 
            break;
        }

		file_FEP0 = fopen(filename_FEP0, "rb");
        if ( file_FEP0 == NULL )
        {
            LOG(LOG_CRITICAL, 0, "fep0 fpga file open fail!! ");    
            break;
        }

        while ( (!feof(file_L2)) || (!feof(file_L1))  || ( !feof(file_FEP0))  ||  (!feof(file_FEP1)) )
        {

            unsigned char data_L2,data_L1,data_FEP1,data_FEP0;
            unsigned int data;

            /****fpga state******/
            CPLD_READ_REG(FPGA_STATE_REG, &temp);       
            /******************/
            if ( !(temp & (1<<7)) /***bit 7***/ )
            {  /***CONFIG ERROR BIT, 0 for error********/
                break;
            }

            if ( temp & (1<<4)/*BIT4*/ )
            {  /***0 -- write enable******/
                taskDelay(5);
                LOG1(LOG_CRITICAL, 0, "write disable, FPGA_STATE_REG value is %x !!", temp);
                continue;
            }

            /**buffer bits 7-0 is L2fpga, 15-8 is L1fpga, 23-16 is fep0fpga, 31-24 is fep1fpga*****/
            if ( !feof(file_L2) )
            {
                count = fread(&data_L2, 1/*size*/, 1, file_L2);     
            }
            else
            {
                data_L2 = 0xff;
            }

            if ( !feof(file_L1) )
            {
                count = fread(&data_L1, 1/*size*/, 1, file_L1); 
            }
            else
            {
                data_L1 = 0xff;
            }

            if ( !feof(file_FEP1) )
            {
                count = fread(&data_FEP1, 1/*size*/, 1, file_FEP1); 
            }
            else
            {
                data_FEP1 = 0xff;
            }

            if ( !feof(file_FEP0) )
            {
                count = fread(&data_FEP0, 1/*size*/, 1, file_FEP0);     
            }
            else
            {
                data_FEP0 = 0xff;
            }
            /*
                    count = fread(&data_L2, 1, 1, file_L2); 
                    count = fread(&data_L1, 1, 1, file_L1); 
                    count = fread(&data_FEP1, 1, 1, file_FEP1); 
                    count = fread(&data_FEP0, 1, 1, file_FEP0); 	
            */      
            *(((unsigned char *)&data)+0) = data_FEP1;   *(((unsigned char *)&data)+1) = data_FEP0;
            *(((unsigned char *)&data)+2) = data_L1;   *(((unsigned char *)&data)+3) = data_L2;
            /* printf("data :%x   ", data); */
            CPLD_WRITE_REG(FPGA_REG, data);     
            while ( *(unsigned int *)(CPLD_BASE + FPGA_REG) != data )
            {
                LOG2(LOG_CRITICAL, 0, "write cpld error: fpga write is %x, read is %x, repeat write!!!!",data, *(unsigned int *)(CPLD_BASE + FPGA_REG));
                taskDelay(5);
                CPLD_WRITE_REG(FPGA_REG, data);     
            }

            /* CPLD_WRITE_REG(FPGA_STATE_REG, (1<<4)); */   /****write state , bit4 ******/

        }/***while*******/

        /***config data state******/
        if ( feof(file_L2) )
        {
            CPLD_WRITE_REG(CONFIG_DATASTATE_REG, (0x1<<0));
        }

        fclose(file_L2);
        fclose(file_L1);
        fclose(file_FEP0);
        fclose(file_FEP1);

        /* printf("fpga download finish!!!\n"); */
        /***waiting ***/
        /* i = 32;     while(i--) CPLD_WRITE_REG(FPGA_REG, 0xffffffff);   */
        i= 50000; while ( i-- );

        CPLD_READ_REG(FPGA_STATE_REG, &temp);       
        if ( temp & (0xf | (0x1<<6)) == (0xf | (0x1<<6)) /*(1<<0)*/ /* BIT0*/ )
        {  /*********/
            OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> FPGA download succeed");
            return;
        }
        else
        {
            LOG1(LOG_CRITICAL, 0, "fpga config error, FPGA_STATE_REG value is %x !!!\n", temp);                 
        }

    }  /***while(j--) 5times***/

    ////////////////////////////
    // failed after 5th trial
    OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> FPGA DOWNLOAD FAILED !!!!!!!!!!!!!!!!!!!!!");
    bspSetBtsResetReason(RESET_REASON_SW_INIT_DOWNFPGA_FAIL/*RESET_REASON_SW_INIT_FAIL*/);
    taskDelay(500);
    rebootBTS(BOOT_CLEAR );


}
#ifdef WBBU_CODE
void L3BootTask::down_fpga_wbbu()
{
    int i, j, count;
    unsigned int  temp;
    char filename_L2[40];
  // char filename_L1[40];
  //  char filename_FEP0[50], filename_FEP1[50];
  unsigned int ver;
    FILE *file_L2;//,* file_L1, *file_FEP0, *file_FEP1;
    i = 0;
    j = 5;
    unsigned char flag = 0;
   while ( j-- )
    {
        /***download fpga******/
        flag = 0;
        strcpy(filename_L2, "/RAMD:0/load/wbbu_fpga.bin");
        //strcat(filename_L2, RAMDISK_DOWNLOAD_PATH);
        //strcat(filename_L2, FPGA_L2_FILENAME);
       
        file_L2 = fopen(filename_L2, "rb");
        if ( file_L2 == NULL )
        {
            LOG(LOG_CRITICAL, 0, "L2 fpga file open fail!! ");  
            
            break;
        }
		   unsigned short  delayCount = 0;

		    set_nPROGRAM(0);/**program =0****/
	//	  set_nINT(0);/*****/
			//  set_DCLK(1);	/* DCLK = 0 */
			for (delayCount = 0; delayCount < 100; delayCount++);	/* tCF2ST0 : max 800 ns */

		   set_nPROGRAM(1);/**program =1****/

		     printf("read ok\n");
		   //    return;
		//   set_nINT(1);/*****/
		for (delayCount = 0; delayCount < 10000; delayCount++);
       while (!read_nINT())
       	{
       		    i++;
       	        if(i>200000)
       		 	{
       		  	     printf("read_nINT error\n");
       		   	  //   return;
       		   	  flag  = 1;
       		   	  break;
       		 	}
       		  //  taskDelay(1000);
       	}
    if(flag==0)
    	{
    	printf("flag:%d\n",flag);
    while ( (!feof(file_L2)) )  
				{
						 unsigned short  data_L2;
						 unsigned char data_high,data_low;
						 unsigned int count;
							count = fread(&data_L2, 2/*size*/, 1, file_L2);  
							data_high = data_L2>>8;
							data_low  = data_L2&0xff;
							set_DCLK(0);
							for (i = 0; i < 8; i++)
							{
								/* DCLK = 0 时, 在DATA0 上放置数据*/
								set_DATA0(data_high & 0x80);		/* The least significant bit (MSB) of each data byte must be presented first */

								/* 延时(tDSU : min 7 ns) */
								for (delayCount = 0; delayCount < DSU_DELAY; delayCount++);	/* tDSU : min 6 ns */

								/* DCLK -> 1 , 使FPGA 读入数据*/
								set_DCLK(1);

								/* 延时(tCH : min 4 ns) */
								for (delayCount = 0; delayCount < CH_DELAY; delayCount++);	/* tCH : min 4 ns */

								set_DCLK(0);
								
								/* 准备下一位数据*/
								data_high <<= 1;

							
							}
							for (i = 0; i < 8; i++)
							{
								/* DCLK = 0 时, 在DATA0 上放置数据*/
								set_DATA0(data_low & 0x80);		/* The least significant bit (MSB) of each data byte must be presented first */

								/* 延时(tDSU : min 7 ns) */
								for (delayCount = 0; delayCount < DSU_DELAY; delayCount++);	/* tDSU : min 6 ns */

								/* DCLK -> 1 , 使FPGA 读入数据*/
								set_DCLK(1);

								/* 延时(tCH : min 4 ns) */
								for (delayCount = 0; delayCount < CH_DELAY; delayCount++);	/* tCH : min 4 ns */

								set_DCLK(0);
								
								/* 准备下一位数据*/
								data_low <<= 1;

							
							}
				
				}
				}
       fclose(file_L2);
         	for (delayCount = 0; delayCount < CF2ST0_DELAY; delayCount++);
	          if(read_nINT() && read_CONF_DONE())
		      {

			         OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> FPGA download succeed");
			               ver = Read_Fpga_Version_Soft();
            	OAM_LOGSTR1(LOG_CRITICAL, 0, "[BTS boot] ==> FPGA Version:%08x\n",ver);
            	taskDelay(3);
				RestFPGA();//wangwenhua 2012-3-20
            	taskDelay(10);
            	 SetLocalFepRxInt(0);
	            return;
	        }
	        else
	        {
	            OAM_LOGSTR(LOG_CRITICAL, 0, "fpga config error, FPGA_STATE_REG value is  !!!\n" );                 
	        }
	      }


    ////////////////////////////
    // failed after 5th trial
   #ifndef WBBU_NVRAM
   OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> FPGA DOWNLOAD FAILED !!!!!!!!!!!!!!!!!!!!!");
   bspSetBtsResetReason(RESET_REASON_SW_INIT_DOWNFPGA_FAIL/*RESET_REASON_SW_INIT_FAIL*/);
   taskDelay(1000);
   rebootBTS(BOOT_CLEAR );
   #endif
}
#endif
CTimer * L3BootTask::CreateBootTimer(int cpuType, int index, int timerLen)
{
    CComMessage* pmessage_expires;
    char * message_expires;

    pmessage_expires = new (this, 2) CComMessage;
    message_expires = (char *)(pmessage_expires->GetDataPtr());

    message_expires[0] = cpuType;
    message_expires[1] = index;

    pmessage_expires->SetDataLength(2);
    pmessage_expires->SetMessageId(BOOT_TIMER_EXPIRES);
    pmessage_expires->SetDstTid(M_TID_BM/*****/);
    pmessage_expires->SetSrcTid(M_TID_BM/*****/);

    CTimer * timer = new CTimer(1, timerLen, pmessage_expires); /***type len message***/
    if ( NULL == timer)
    {
        pmessage_expires->Destroy();
    }
    return timer;

}

extern "C" int bspGetLoadVersion_A();
extern "C" int bspGetLoadVersion_B();

#ifdef WBBU_CODE
void   testBootline()
{
     BOOT_PARAMS       bootParams;
     (void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);

}
#endif
#ifndef WBBU_CODE
bool L3BootTask::Initialize()
{
    FILE *file_L2, *file_L1, *file_FEP0, *file_FEP1;
    int i, count;
    unsigned int temp;
    /*unsigned char data[4]; */
    char filename_L2[40], filename_L1[40], filename_FEP0[40],filename_FEP1[40];
    BOOT_PARAMS       bootParams;
    bool  isL2DownloadImage = true;


#if !defined(__WIN32_SIM__)&&!defined(__NUCLEUS__)  /****to debug*********/
    /***ram disk*********/
    BLK_DEV               *pBlkDev; 
    DOS_VOL_DESC          *pVolDesc; 
    char path[40];

    /***init******/
    if ( !CBizTask::Initialize() )
    {
        LOG(LOG_CRITICAL,0,"L3BootTask Initialize failed.");
        return false;
    }

    m_uFlags |= M_TF_RUNNING;
    m_uFlags |= M_TF_INITED;
    RegisterEntity(false);

    /********************/
    strcpy(path, DEVICE_RAMDISK);
    strcat(path, RAMDISK_DOWNLOAD_PATH);
    mkdir(path);
    mkdir(RAMDISK_CPE_DIR_NAME);
	//calibration directory.--by xiaoweifang.
    mkdir(RAMDISK_CALIBRATION_DIR);
    mkdir(RAMDISK_TEMP);   


    (void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);

    /****boot data source***************/
    //source = bspGetBootupSource();
    if ( (strncmp(bootParams.bootDev,"MV", 2) == 0) || (strncmp(bootParams.bootDev,"mv", 2) == 0) )
    {
        BootSource = DATA_SOURCE_MV;
    }
    else
    {
        BootSource = DATA_SOURCE_CF;
        T_BootLoadState * loaderParam = (T_BootLoadState * )NVRAM_BASE_ADDR_BOOT_STATE;

        if (loaderParam->nvramSafe != NVRAM_VALID_PATTERN 
            || ((loaderParam->bootPlane!=BOOT_PLANE_A) &&(loaderParam->bootPlane!=BOOT_PLANE_B)) )
        {   /* if the pattern is wrong, reinitializa */
            if(bspEnableNvRamWrite((char *)loaderParam, sizeof(T_BootLoadState))==TRUE)
    	     {
                loaderParam->workFlag = LOAD_STATUS_VERIFIED;
                loaderParam->bootPlane = BOOT_PLANE_A;
                loaderParam->LoadVersion_A = bspGetLoadVersion_A();
                loaderParam->LoadVersion_B = bspGetLoadVersion_B();
                loaderParam->nvramSafe = NVRAM_VALID_PATTERN;
                bspDisableNvRamWrite((char *)loaderParam, sizeof(T_BootLoadState));
    	     }
        }
    }

    if ( BootSource != DATA_SOURCE_CF )
    {
        /**copy all boot file to ramdisk***/        
        OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> BTS booting from FTP server");
        netDevCreate(FTP_DEVICE_NAME, bootParams.had, 1);  /**0 is rsh, 1 is ftp*********/

        iam(bootParams.usr, bootParams.passwd);

        BOOT_PARAMS       bootParams2;
        (void)bootStringToStruct((char *)(BOOT_LINE_ADRS_L2), &bootParams2);
        if ( strlen(bootParams2.bootFile) > 0 )
        {
            strcpy(L2_apppName, bootParams2.bootFile);
        }
        else
        {
            strcpy(L2_apppName, L2_BOOTROM_FILENAME);   
        }
        if ( 0 != strcmp(L2_apppName,L2_BOOTROM_FILENAME ))
        {
            isL2DownloadImage = false;   //download image, not in debug mode
        }
    }
    else
    {
        strcpy(L2_apppName, L2_BOOTROM_FILENAME);

        OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> BTS boot from Compact Flash card");
    }

    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(L2_apppName);

    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(MCP_APP_CODE_FILENAME);

    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(AUX_APP_CODE_FILENAME);

    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(FEP_APP_CODE_FILENAME);

    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(FPGA_L2_FILENAME);

    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(FPGA_L1_FILENAME);


    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }

    copy_file_to_ramdisk(FPGA_FEP0_FILENAME);
    copy_file_to_ramdisk(FPGA_FEP1_FILENAME);
	
    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }

    // create bootup timers
    for (int mcpIndex = 0; mcpIndex<M_NumberOfMCP; mcpIndex++)
    {
        timer_MCP[mcpIndex] = CreateBootTimer(MCP, mcpIndex, MCP_BOOT_EXPIRES);
    }
    timer_AUX = CreateBootTimer(AUX, 0, AUX_BOOT_EXPIRES); /***type len message***/

    //lijinan for (int fepIndex=0; fepIndex++; fepIndex<M_NumberOfFEP)
     for (int fepIndex=0; fepIndex<M_NumberOfFEP;fepIndex++)
    {
        timer_FEP[fepIndex] = CreateBootTimer(FEP, fepIndex, FEP_BOOT_EXPIRES);
    }

    if ( isL2DownloadImage)
    {
        timer_L2 = CreateBootTimer(L2, 0, L2_BOOT_IMAGE_TIMER_LEN);
    }
    else
    {
        timer_L2 = CreateBootTimer(L2, 0, L2_BOOT_EXPIRES);
    }
    timer_L2Preloader = CreateBootTimer(L2_PRELOADER, 0, L2_PRELOADER_BOOT_TIMER_LEN);


    OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> FPGA download started ");
    taskDelay(10);      /***waiting for print out************/
    down_fpga();

/*cpld reset register ,  bit0 reset bts, bit1 reset l2, bit2 reset L2fpga and MCP, bit3 reset L1fpga and aux, bit4 reset Fep0, bit5 reset fep1***/
 
    /***waiting*****/
    taskDelay(10);
    /***reset all fpga*****/
    system_reset(FPGA, 0);

    taskDelay(100);
    system_reset(AUX, 0);

    taskDelay(10);
    /**reset L2 or bts***/
    system_reset(L2, 0);  

   /*lijinan 20090202*/
    CComMessage *tmoMsg = new(this, 0)CComMessage;
    if (tmoMsg)
    {
        tmoMsg->SetDstTid(M_TID_BM);
        tmoMsg->SetSrcTid(M_TID_BM);
        tmoMsg->SetMessageId(MSGID_DIAG_5MIN_TIME_OUT);
         CTimer * timer = new CTimer(1, 300000, tmoMsg);
        if (timer)
        {
            timer->Start();
        }
	 else
	 {
	     tmoMsg->Destroy();	     
	 }
    }



#endif /*** !defined(__WIN32_SIM__)&&!defined(__NUCLEUS__)  to debug*****/


    return true;
}
#else
bool L3BootTask::Initialize()
{
    FILE *file_L2, *file_L1, *file_FEP0, *file_FEP1;
    int i, count;
    unsigned int bsource;
    unsigned int temp;
    /*unsigned char data[4]; */
    char filename_L2[40], filename_L1[40], filename_FEP0[40],filename_FEP1[40];
    BOOT_PARAMS       bootParams;
    bool  isL2DownloadImage = true;


#if !defined(__WIN32_SIM__)&&!defined(__NUCLEUS__)  /****to debug*********/
    /***ram disk*********/
 //   BLK_DEV               *pBlkDev; 
 //   DOS_VOL_DESC          *pVolDesc; 
    char path[40];

    /***init******/
    if ( !CBizTask::Initialize() )
    {
        LOG(LOG_CRITICAL,0,"L3BootTask Initialize failed.");
        return false;
    }

    m_uFlags |= M_TF_RUNNING;
    m_uFlags |= M_TF_INITED;
    RegisterEntity(false);

    /********************/
    strcpy(path, "/RAMD:0/"/*DEVICE_RAMDISK*/);
    strcat(path, RAMDISK_DOWNLOAD_PATH);
    mkdir(path);
    mkdir(RAMDISK_CPE_DIR_NAME);
	//calibration directory.--by xiaoweifang.
    mkdir(RAMDISK_CALIBRATION_DIR);
    mkdir(RAMDISK_TEMP);  


    (void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);

    /****boot data source***************/
    //source = bspGetBootupSource();
    if ( (strncmp(bootParams.hostName,"hos", 3) == 0) || (strncmp(bootParams.hostName,"HOS", 3) == 0) )
    {
        BootSource = DATA_SOURCE_MV;//ftp
       
    }
    else
    {
        BootSource = DATA_SOURCE_CF;
        T_BootLoadState * loaderParam = (T_BootLoadState * )NVRAM_BASE_ADDR_BOOT_STATE;

        if (loaderParam->nvramSafe != NVRAM_VALID_PATTERN 
            || ((loaderParam->bootPlane!=BOOT_PLANE_A) &&(loaderParam->bootPlane!=BOOT_PLANE_B)) )
        {   /* if the pattern is wrong, reinitializa */
            if(bspEnableNvRamWrite((char *)loaderParam, sizeof(T_BootLoadState))==OK)
    	     {
                loaderParam->workFlag = LOAD_STATUS_VERIFIED;
                loaderParam->bootPlane = BOOT_PLANE_A;
                loaderParam->LoadVersion_A = bspGetLoadVersion_A();
                loaderParam->LoadVersion_B = bspGetLoadVersion_B();
                loaderParam->nvramSafe = NVRAM_VALID_PATTERN;
                bspDisableNvRamWrite((char *)loaderParam, sizeof(T_BootLoadState));
    	     }
        }
    }

    if ( BootSource != DATA_SOURCE_CF )
    {
        /**copy all boot file to ramdisk***/        
        OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> BTS booting from FTP server");
 
        netDevCreate(FTP_DEVICE_NAME, bootParams.had, 1);  /**0 is rsh, 1 is ftp*********/
      
        iam(bootParams.usr, bootParams.passwd);
 #ifndef WBBU_CODE   
        BOOT_PARAMS       bootParams2;
        (void)bootStringToStruct((char *)(BOOT_LINE_ADRS_L2), &bootParams2);
        if ( strlen(bootParams2.bootFile) > 0 )
        {
            strcpy(L2_apppName, bootParams2.bootFile);
        }
        else
        {
            strcpy(L2_apppName, L2_BOOTROM_FILENAME);   
        }
        if ( 0 != strcmp(L2_apppName,L2_BOOTROM_FILENAME ))
        {
            isL2DownloadImage = false;   //download image, not in debug mode
        }
   #endif
    }
    else
    {
     //   strcpy(L2_apppName, L2_BOOTROM_FILENAME);

        OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> BTS boot from Compact Flash card");
    }

    taskDelay(10);
#ifndef WBBU_CODE
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(L2_apppName);

    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(MCP_APP_CODE_FILENAME);

    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(AUX_APP_CODE_FILENAME);

    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(FEP_APP_CODE_FILENAME);

    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(FPGA_L2_FILENAME);

    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(FPGA_L1_FILENAME);


    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }

    copy_file_to_ramdisk(FPGA_FEP0_FILENAME);
    copy_file_to_ramdisk(FPGA_FEP1_FILENAME);
	
    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }

    // create bootup timers
    for (int mcpIndex = 0; mcpIndex<M_NumberOfMCP; mcpIndex++)
    {
        timer_MCP[mcpIndex] = CreateBootTimer(MCP, mcpIndex, MCP_BOOT_EXPIRES);
    }
    timer_AUX = CreateBootTimer(AUX, 0, AUX_BOOT_EXPIRES); /***type len message***/

    //lijinan for (int fepIndex=0; fepIndex++; fepIndex<M_NumberOfFEP)
     for (int fepIndex=0; fepIndex<M_NumberOfFEP;fepIndex++)
    {
        timer_FEP[fepIndex] = CreateBootTimer(FEP, fepIndex, FEP_BOOT_EXPIRES);
    }

    if ( isL2DownloadImage)
    {
        timer_L2 = CreateBootTimer(L2, 0, L2_BOOT_IMAGE_TIMER_LEN);
    }
    else
    {
        timer_L2 = CreateBootTimer(L2, 0, L2_BOOT_EXPIRES);
    }
    timer_L2Preloader = CreateBootTimer(L2_PRELOADER, 0, L2_PRELOADER_BOOT_TIMER_LEN);


    OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> FPGA download started ");
    taskDelay(10);      /***waiting for print out************/
    down_fpga();

/*cpld reset register ,  bit0 reset bts, bit1 reset l2, bit2 reset L2fpga and MCP, bit3 reset L1fpga and aux, bit4 reset Fep0, bit5 reset fep1***/
 
    /***waiting*****/
    taskDelay(10);
    /***reset all fpga*****/
    system_reset(FPGA, 0);

    taskDelay(100);
    system_reset(AUX, 0);

    taskDelay(10);
    /**reset L2 or bts***/
    system_reset(L2, 0);  

   /*lijinan 20090202*/
    CComMessage *tmoMsg = new(this, 0)CComMessage;
    if (tmoMsg)
    {
        tmoMsg->SetDstTid(M_TID_BM);
        tmoMsg->SetSrcTid(M_TID_BM);
        tmoMsg->SetMessageId(MSGID_DIAG_5MIN_TIME_OUT);
         CTimer * timer = new CTimer(1, 300000, tmoMsg);
        if (timer)
        {
            timer->Start();
        }
	 else
	 {
	     tmoMsg->Destroy();	     
	 }
    }
#else
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(DSP_Name_No5_L2);

    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(DSP_Name_No4_L1);

    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(DSP_Name_No3_L1);

    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(DSP_Name_No2_L1);

    taskDelay(10);
        if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(DSP_Name_No1_L1);
    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(FPGA_WBBU_FILENAME);

    taskDelay(10);
        if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
     //  iam("vxworks", "vxworks");
    }
    copy_file_to_ramdisk(WRRU_Mcu);

    taskDelay(10);
        if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
     //  iam("vxworks", "vxworks");
    }
    copy_file_to_ramdisk(WRRU_Fpga);

    taskDelay(10);
       if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
      //  iam("vxworks", "vxworks");
    }
    copy_file_to_ramdisk(DSP_Name_No5_L2);

    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
      //  iam("vxworks", "vxworks");
    }
    copy_file_to_ramdisk("bootrom.bin");
    #if 0
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }
    copy_file_to_ramdisk(FPGA_L1_FILENAME);


    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
    }

    copy_file_to_ramdisk(FPGA_FEP0_FILENAME);
    copy_file_to_ramdisk(FPGA_FEP1_FILENAME);
	#endif
    taskDelay(10);
    if ( BootSource != DATA_SOURCE_CF )
    {
        iam(bootParams.usr, bootParams.passwd);
   //    iam("vxworks", "vxworks");
    }

    // create bootup timers
    for (int mcpIndex = 0; mcpIndex<M_NumberOfMCP; mcpIndex++)
    {
        timer_MCP[mcpIndex] = CreateBootTimer(MCP, mcpIndex, MCP_BOOT_EXPIRES);
    }
    timer_AUX = CreateBootTimer(AUX, 0, AUX_BOOT_EXPIRES); /***type len message***/

    for (int fepIndex=0; fepIndex++; fepIndex<M_NumberOfFEP)
    {
        timer_FEP[fepIndex] = CreateBootTimer(FEP, fepIndex, FEP_BOOT_EXPIRES);
    }

    if ( isL2DownloadImage)
    {
        timer_L2 = CreateBootTimer(L2, 0, L2_BOOT_IMAGE_TIMER_LEN);
    }
    else
    {
        timer_L2 = CreateBootTimer(L2, 0, L2_BOOT_EXPIRES);
    }
    timer_L2Preloader = CreateBootTimer(L2_PRELOADER, 0, L2_PRELOADER_BOOT_TIMER_LEN);


    OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> FPGA download started ");
    taskDelay(10);      /***waiting for print out************/
  

   down_fpga_wbbu();
/*cpld reset register ,  bit0 reset bts, bit1 reset l2, bit2 reset L2fpga and MCP, bit3 reset L1fpga and aux, bit4 reset Fep0, bit5 reset fep1***/
// Set_Fpga_Clk(2);//wangwenhua add 20091024
    /***waiting*****/
    taskDelay(100);

    Drv_RegisterBOOT(L3BootTask::BootPCallBack);

   taskDelay(100);
#if 1
      ResetDspNew(1);//l2
    Reset_DSP_FLAG =0x22;
    taskDelay(100);
    


 
   ResetDspNew(2);
   Reset_DSP_FLAG =0x33;
    taskDelay(100);
    
     ResetDspNew(3);
     Reset_DSP_FLAG =0x44;
    taskDelay(100);
    ResetDspNew(4);
    Reset_DSP_FLAG =0x55;
    taskDelay(200);
     ResetDspNew(0);
     Reset_DSP_FLAG =0x11;
    taskDelay(100);

    #endif
 //   copy_file_to_ramdisk(FPGA_FEP1_FILENAME);
#endif //ifdef WBBU_NVRAM
#endif /*** !defined(__WIN32_SIM__)&&!defined(__NUCLEUS__)  to debug*****/
 

    return true;
}
#endif
bool L3BootTask::ProcessComMessage(CComMessage* pComMsg)
{

    UINT16 usMsgId = pComMsg->GetMessageId();
#ifdef WBBU_CODE
       unsigned short dsp_addr_H ;
        unsigned short dsp_addr_M ;
        unsigned short dsp_addr_L ;
            unsigned short index = 0xff;
            unsigned char *p ;
#endif
    switch ( usMsgId )
    {
        case L2_PRELOADER_RUNING:

            L3BOOT_L2PreLoaderRuning();
            break;

        case L2_RUNING:

            L3BOOT_L2AppRuning();
            break;
        case L2_REQUEST_RESET:
            L3BOOT_L2RequestReboot();
            break;

        case AUX_RESET:
            L3BOOT_AUX_Rest();
            break;

        case AUX_RUNING:

            L3BOOT_AUX_Runing();
            break;

        case MCP_RESET:
            L3BOOT_MCP_Rest(pComMsg);
            break;

        case MCP_RUNING:
            L3BOOT_MCP_Runing(pComMsg);
            break;

        case FEP_RESET:
            L3BOOT_FEP_Rest(pComMsg);
            break;

        case FEP_RUNING:
            L3BOOT_FEP_Runing(pComMsg);
            break;

        case BOOT_SYSTEM_RESET:    /***from oam***/
            L3BOOT_System_Rest(pComMsg);
            break;

        case BOOT_TIMER_EXPIRES:
            L3BOOT_TIMER_Expires(pComMsg);
            break;
#ifdef WBBU_CODE
         case DSP_BOOT://wangwenhua add 20090825
      //         BOOT_PARAMS       bootParams;
   //       (void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
            p = (unsigned char*)pComMsg->GetDataPtr();
          dsp_addr_H = p[2]*0x100+p[3];
          dsp_addr_M = p[4]*0x100+p[5];
          dsp_addr_L = p[6]*0x100+p[7];
         index = p[0]*0x100+p[1];
        if(index==1)
      	{
      //	      printf("boot dsp 1\n");
           bootDSPImage(1,"/RAMD:0/load/dsp_image_L1_1",dsp_addr_H,dsp_addr_M,dsp_addr_L);
         
      	}
      else if(index == 2)
      	{
      	   //     printf("boot dsp 2\n");
      	        bootDSPImage(2,"/RAMD:0/load/dsp_image_L1_2",dsp_addr_H,dsp_addr_M,dsp_addr_L);
      	  
      	}
        else if(index == 3)
      	{
      	  //    printf("boot dsp 3\n");
      	       bootDSPImage(3,"/RAMD:0/load/dsp_image_L1_3",dsp_addr_H,dsp_addr_M,dsp_addr_L);
      	    
      	}
          else if(index == 4)
      	{
      	  //      printf("boot dsp 4\n");
      	      bootDSPImage(4,"/RAMD:0/load/dsp_image_L1_4",dsp_addr_H,dsp_addr_M,dsp_addr_L);
      	   
      	}
         else if(index == 5)
      	{
      	 //      printf("boot dsp 5\n");
      	        bootDSPImage(5,"/RAMD:0/load/dsp_image_L2_5",dsp_addr_H,dsp_addr_M,dsp_addr_L);
      	 
      	}  
         else
         {
         //	   printf("boot dsp index:%x error\n",index);
         }
	     break;
#endif
	/*lijinan 20090202*/
	case MSGID_DIAG_5MIN_TIME_OUT:
	     L3BOOT_5MinTIMER_Expires();
	     break;

        default:
            LOG2(LOG_CRITICAL, 0, "invalid message: 0x%x, src task id is %d!!!!\n", usMsgId, pComMsg->GetSrcTid());
            break;  
    }
    pComMsg->Destroy();

    return true;
}

void L3BootTask::SendCpuResetAlarm(int cpuType, int index)
{
    CComMessage* pmessage_alarm;

    struct alarm_message * message_alarm;

    pmessage_alarm = new (this, sizeof(struct alarm_message)) CComMessage;
    message_alarm = (struct alarm_message *)(pmessage_alarm->GetDataPtr());

    message_alarm->tranId = 0;
    message_alarm->element = cpuType;
    message_alarm->index = index;
    message_alarm->type = ALARM_RESET;

    pmessage_alarm->SetDataLength(6);

    pmessage_alarm->SetMessageId(BOOT_ALARM/**message id***/);
    pmessage_alarm->SetDstTid(M_TID_SYS/*****/);
    pmessage_alarm->SetSrcTid(M_TID_BM/*****/);

    CComEntity::PostEntityMessage(pmessage_alarm);

}


/***message process function*****/
bool L3BootTask::L3BOOT_L2PreLoaderRuning()
{
#ifndef WBBU_CODE
    int i, j;
    FILE * file;
    int count, data;
    int temp,tempold;
    CComMessage* pmessage;
    char * message;
    char filename[40];
    /* unsigned char buffer[5*1024]; */
    unsigned char buffer;
    unsigned char mac_L2[6];
    BOOT_PARAMS       bootParams_L2;
    BOOT_PARAMS       bootParams_L3;
    unsigned int badip_L2;
    struct in_addr ip32; 

    if (timer_L2Preloader)
    {
        timer_L2Preloader->Stop();
    }

    SendCpuResetAlarm(L2,0);

    /****trans bootrom*****/
    strcpy(filename, DEVICE_RAMDISK);
    strcat(filename, RAMDISK_DOWNLOAD_PATH);

    //strcat(filename, L2_BOOTROM_FILENAME);
    strcat(filename, L2_apppName);

    file = fopen(filename, "rb");
    if ( file == NULL )
    {
        LOG(LOG_CRITICAL, 0, "L2 bootrom file open fail!! \n");     
        return false;
    }

    OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> L2 PreLoader is ready to accept L2 Image ");

    /**trans L2 bootline***/
    T_I2C_TABLE i2cData;
    /***L2 mac****/
    i2c_read(I2C_E2PROM_DEV_ADDR, 0, (UCHAR *)&i2cData, sizeof(i2cData));
    struct in_addr l2MacLast3B;
    l2MacLast3B.s_addr = i2cData.L2_mac[3]*0x10000 + i2cData.L2_mac[4]*0x100 + i2cData.L2_mac[5];

    // force the bootdev of L2 to be "mv" and backplane IP address to be (1+L3backplane IP address)
    (void)bootStringToStruct((char *)BOOT_LINE_ADRS_L2, &bootParams_L2);
    (void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams_L3);
    bool needToChange=false;
    if (  0 != strcmp(bootParams_L2.bootDev, "mv"))
    {
        strcpy(bootParams_L2.bootDev, "mv");
        needToChange = true;
    }
    bootParams_L2.unitNum = 0;
    struct in_addr curL2Addr;
    ip32.s_addr = inet_addr(bootParams_L2.bad);
    if (  inet_addr(bootParams_L2.bad) != inet_addr(bootParams_L3.bad)+1 )
    {
        needToChange = true;
        strcpy(bootParams_L2.bad,  bootParams_L3.bad);
        ip32.s_addr = inet_addr(bootParams_L2.bad) + 1;
        inet_ntoa_b(ip32, bootParams_L2.bad);
    }
	if (l2MacLast3B.s_addr != inet_addr(bootParams_L2.other))
	{
        needToChange = true;
	    inet_ntoa_b(l2MacLast3B, bootParams_L2.other);
	}
    if (needToChange)
    {
        taskLock();
        if(bspEnableNvRamWrite( (char*)NVRAM_BASE_ADDR, vmBasePageSizeGet())==TRUE)
    	{
            (void) bootStructToString ((char *)BOOT_LINE_ADRS_L2, &bootParams_L2);
            bspDisableNvRamWrite( (char *)NVRAM_BASE_ADDR, vmBasePageSizeGet());
    	}
        taskUnlock();
    }

    UINT32 *srcAddr = (UINT32 *)BOOT_LINE_ADRS_L2;
    UINT32 *tgtAddr = (UINT32 *)(PCI0_MEM2_BASE + L2_SRAM_BOOT_LINE_OFFSET);
    // copy L2 bootline Parameters to L2 SRAM 
    for ( i = 0; i<L2_SRAM_BOOT_LINE_SIZE/sizeof(int); i++ )
    {
        *(tgtAddr + i) = *(srcAddr  + i);    
    }

    // copy L2 image to L2 Memory 
    j = 0;
    do
    {

        count = fread(&buffer, 1, 1 /*5*1024*/, file);
        *((unsigned char  *)(PCI0_MEM3_BASE + L2ENTRYADDR /*0xf6010000*/  + j)) = buffer;  
        while ( *((unsigned char  *)(PCI0_MEM3_BASE + L2ENTRYADDR /*0xf6010000*/  + j)) != buffer )
        {
            *((unsigned char  *)(PCI0_MEM3_BASE + L2ENTRYADDR /*0xf2010000*/  + j)) = buffer;  

            taskDelay(5);
            printf("write sdram error %x: write is %x, read is %x, repeat write!!!!",(L2ENTRYADDR + j), buffer, (*(unsigned char  *)(PCI0_MEM3_BASE + L2ENTRYADDR + j)));

        }
        j++;  i++;
    }while ( !feof(file) );

    fclose(file);

    // copy L2 boot time to L2 SRAM 
    T_TimeDate l2BootTime = bspGetDateTime();
    srcAddr = (UINT32*)&l2BootTime;
    tgtAddr = (UINT32 *)(PCI0_MEM2_BASE + L2_SRAM_BOOT_TIME_OFFSET);
    for ( i = 0; i<L2_SRAM_BOOT_TIME_SIZE/sizeof(int); i++ )
    {
        *(tgtAddr + i) = *(srcAddr + i);    
    }


    /**send finish message**/       
    data = LONGSWAP_D (BOOTROM_SEND_FINISH);
    *((int  *)(PCI0_MEM2_BASE + 0x1c10 /*0x12001c10*/)) = data;  /***0x1c10 is inbound message register****/
    while ( *((int  *)(0xf4000000)) != BOOTROM_SEND_FINISH )
    {
        *((int  *)(0xf4000000)) = BOOTROM_SEND_FINISH;  /** sram , send finish message**/
        *((int  *)(0xf4000004)) = L2ENTRYADDR;  /** sram , send finish message**/
    }   

    OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> L2 PPC image download finished");
	
    /*lijinan 20090224 
内存映射地址  
	 消息类型	附加参数（可选） ...
L2	ox42000100	0x42000104	...
L3	0xf4000100	0xf4000104	...

L2_PRE_FINISH	  		0xf0000001	preloader执行完毕，准备跳转

L2_APP_START			0xf0000002

L2_PCI_BUFFER_ERR		0xf0001000
L2_PCI_NETPOOL_ERR		0xf0001001
L2_PCI_END_ERR		0xf0001002
L2_PCISIO_PTY_ERR		0xf0001003
L2_PCISIO_TASK_ERR		0xf0001004
L2_BOOT_FTP_CREATE_ERR	0xf0001005
L2_BIZ_TASK_ERR		0xf0001006

L2_BOOT_END			0xf0002000*/

 #if 0
    tempold = 0;
    temp = 0;
    while(1)
    {
    	temp = *((int  *)(0xf4000100));
	if(temp!=tempold)
	{
		printf("\n[BTS boot] ==> L2 boot state:%x\n",temp);
	}
	tempold = temp;
	if((temp==L2_BOOT_END)||(temp==L2_BIZ_TASK_ERR)||(temp==L2_BOOT_FTP_CREATE_ERR)||
		(temp==L2_PCISIO_TASK_ERR)||(temp==L2_PCISIO_PTY_ERR)||(temp==L2_PCI_END_ERR)||
		(temp==L2_PCI_NETPOOL_ERR)||(temp==L2_PCI_BUFFER_ERR))
	{
		break;
	}
	
	taskDelay(2);

    }
#endif
    

    /**start timer****/
    if ( timer_L2 != NULL )
    {
        timer_L2->Stop();
        timer_L2->Start();  
    }
#endif
    return true;
}



int  displayL2RunTrace()
{

	int *addr = (int *)0xf4000100;
 

		
	printf("\n 0xf4000100 --0xf4000128:%x,%x,%x,%x,%x,%x,%x,%x\n",*addr,*(addr+1),*(addr+2),*(addr+3),\
		*(addr+4),*(addr+5),*(addr+6),*(addr+7));

	if(*(addr+1)==L2_BOOT_FTP_CREATE_ERR)
		printf("\nnetDevCreate(FTP_DEVICE_NAME, PeerName, 1) error!!!!!!!\n");
	if(*(addr+2)==L2_BIZ_TASK_ERR)
		printf("\nL2_BIZ_TASK_ERR......\n");
	if(*(addr+3)==L2_PCISIO_PTY_ERR)
		printf("\nL2_PCISIO_PTY_ERR......\n");
	if(*(addr+4)==L2_PCISIO_TASK_ERR)
		printf("\nERROR!!! tSioOut taskSpawn failed.\n");
	if(*(addr+5)==L2_PCI_BUFFER_ERR)
		printf("\nReserved Pci Buffer space is not enough\n");
	if(*(addr+6)==L2_PCI_NETPOOL_ERR)
		printf("\nPCI interface netbuf Pool initialization failure\n");
	if(*(addr+7)==L2_PCI_END_ERR)
		printf("\nstart pci end err...\n");
	


	
	
}

bool L3BootTask::L3BOOT_L2AppRuning()
{
    #ifndef WBBU_CODE
    UINT32 gppRegVal;
    #endif
    state_L2 = RUNNING;
    #ifdef WBBU_CODE
    state_System = RUNNING;//wangwenhua add 2012-4-23 for test same syn 
    #endif
    OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> L2 APP Runing!!!");    
#ifndef WBBU_CODE
/*电源控制，从909项目流移植过来 jiaying20100805*/
    /*GPIO m909, set pin5,0*/
    MV64360_REG_RD(MV64360_GPP_VAL, &gppRegVal);    
    gppRegVal &= ~(1 << 5); /***write 0***/
    gppRegVal &= ~(1 << 4); /***write 0***/
    MV64360_REG_WR(MV64360_GPP_VAL, gppRegVal);
#endif
    /****DeleteTimer timer*****/
    if ( timer_L2 != NULL )
    {
        timer_L2->Stop();
    }
    taskDelay(100);
    send_working_message(L2, 0);
#ifdef WBBU_CODE
    		 send_system_runing();
#endif
#ifndef WBBU_CODE//wangwenhua mark 20091024
    if ( RUNNING != state_AUX )
    {
        timer_AUX->Stop();
        timer_AUX->Start();
    }
    for (int mcpIndex=0; mcpIndex<M_NumberOfMCP; mcpIndex++)
    {
        if ( RUNNING != state_MCP[mcpIndex])
        {
            timer_MCP[mcpIndex]->Stop();
            timer_MCP[mcpIndex]->Start();
        }
    }
#endif
	//按照 与FPGA等的讨论，此时增加一个复位aif serdies的接口
#ifdef WBBU_CODE
   ResetAifSerdies();
#endif
    return true;
}

#ifdef WBBU_CODE


extern "C" void rebootL2()
{
      Reset_Dsp(5,2);
}

extern "C" void rebootFEP()
{
         Reset_Dsp(1,2);
}

extern "C" void rebootAUX()
{
          Reset_Dsp(1,2);
}

#else


extern "C" void rebootL2();
#endif

bool L3BootTask::L3BOOT_L2RequestReboot()
{
    rebootL2();
    return true;
}

bool L3BootTask::L3BOOT_AUX_Rest()
{
    CComMessage* pmessage;
    struct down_message  * message;

    /****send AUX down message**/
    pmessage = new (this, sizeof(struct down_message)) CComMessage;

    message = (struct down_message *)(pmessage->GetDataPtr());
    message->element = AUX;
    message->index = 0;
    strcpy(message->filename, AUX_APP_CODE_FILENAME);

    pmessage->SetDataLength(sizeof(struct down_message));
    pmessage->SetMessageId(AUX_DOWN/**message id***/);
    pmessage->SetDstTid(M_TID_L2BOOT/**L2 boot task***/);
    pmessage->SetSrcTid(M_TID_BM/*****/);

    CComEntity::PostEntityMessage(pmessage);
    OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> AUX Reset, start AUX code download ");

    if ( state_AUX == RUNNING )
    {
        SendCpuResetAlarm(AUX,0);
    }

    state_AUX = RESET;

    /***start timer expires********/
    if ( timer_AUX != NULL )
    {
        timer_AUX->Stop();
        timer_AUX->Start(); 
    }

    return true;
}

bool L3BootTask::L3BOOT_AUX_Runing()
{

    state_AUX = RUNNING;
    /****DeleteTimer timer*****/
    if ( timer_AUX != NULL )
    {
        timer_AUX->Stop();
    }

    send_working_message(AUX, 0);

    if ( state_FEP[0] != RUNNING )
    {
        system_reset(FEP, FEP0);
    }
    if ( state_FEP[1] != RUNNING )
    {
        system_reset(FEP, FEP1);
    }

    return true;
}

bool L3BootTask::L3BOOT_MCP_Rest(CComMessage* pComMsg)
{
    CComMessage* pmessage;
    CComMessage* pmessage_expires;
    struct down_message * message;
    char * message_expires;

    char index = *(char *)(pComMsg->GetDataPtr());

    /****send mcp down message**/
    pmessage = new (this, sizeof(struct down_message)) CComMessage;

    message = (struct down_message *)(pmessage->GetDataPtr());
    message->element = MCP;
    message->index = index;
    strcpy(message->filename, MCP_APP_CODE_FILENAME );

    pmessage->SetDataLength(sizeof(struct down_message));
    pmessage->SetMessageId(MCP_DOWN/**message id***/);
    pmessage->SetDstTid(M_TID_L2BOOT/**L2 boot task***/);
    pmessage->SetSrcTid(M_TID_BM/*****/);

    CComEntity::PostEntityMessage(pmessage);
    OAM_LOGSTR1(LOG_CRITICAL, 0, "[BTS boot] ==> MCP %d reset, start code download", index);

    if ( state_MCP[index] == RUNNING )
    {
        SendCpuResetAlarm(MCP,index );
    }
    state_MCP[index] = RESET;

    /***start timer expires********/
    timer_MCP[index]->Start();  

    return true;
}

bool L3BootTask::L3BOOT_MCP_Runing(CComMessage* pComMsg)
{
    char index = *(char*)(pComMsg->GetDataPtr());

    state_MCP[index] = RUNNING;
    /****DeleteTimer timer*****/
    if ( timer_MCP[index] != NULL )
    {
        timer_MCP[index]->Stop();
    }

    send_working_message(MCP, index);

    OAM_LOGSTR1(LOG_CRITICAL, 0, "[BTS boot] ==> MCP %d is running", index);

    if ( state_System != RUNNING )
    {
        if ( (state_MCP[0] == RUNNING) && (state_MCP[1] == RUNNING) && (state_MCP[2] == RUNNING) && (state_MCP[3] == RUNNING) && 
             (state_MCP[4] == RUNNING) && (state_MCP[5] == RUNNING) && (state_MCP[6] == RUNNING) && (state_MCP[7] == RUNNING) &&
             (state_FEP[0] == RUNNING) && (state_FEP[1] == RUNNING) )
        {
            /****send system runing message**********/
            state_System = RUNNING;
            send_system_runing();
        }
    }


    return true;
}

bool L3BootTask::L3BOOT_FEP_Rest(CComMessage* pComMsg)
{
    CComMessage* pmessage;
    struct down_message  * message;

    char index = *(char*)(pComMsg->GetDataPtr());

    if ( RUNNING != state_AUX)
    {
        OAM_LOGSTR1(LOG_CRITICAL, 0, "[BTS boot] ==> FEP %d reset before AUX running, ignored", 
                    index);
        return true;
    }
    /****send fep down message**/
    pmessage = new (this, sizeof(struct down_message)) CComMessage;

    message = (struct down_message *)(pmessage->GetDataPtr());
    message->element = FEP;
    message->index = index;
    strcpy(message->filename, FEP_APP_CODE_FILENAME );

    pmessage->SetDataLength(sizeof(struct down_message));
    pmessage->SetMessageId(FEP_DOWN/**message id***/);
    pmessage->SetDstTid(M_TID_L2BOOT/**L2 boot task***/);
    pmessage->SetSrcTid(M_TID_BM/*****/);

    CComEntity::PostEntityMessage(pmessage);
    OAM_LOGSTR1(LOG_CRITICAL, 0, "[BTS boot] ==> FEP %d reset, start code download", index);

    if ( state_FEP[index] == RUNNING )
    {
        /**send alarm message****/
        SendCpuResetAlarm(FEP, index);
    }
    state_FEP[index] = RESET;

    /***start timer expires********/
    if ( timer_FEP[index] != NULL )
    {
        timer_FEP[index]->Stop();
        timer_FEP[index]->Start();  
    }
    return true;
}

bool L3BootTask::L3BOOT_FEP_Runing(CComMessage* pComMsg)
{
    char index = *(char*)(pComMsg->GetDataPtr());

    state_FEP[index] = RUNNING;
    /****DeleteTimer timer*****/
    if ( timer_FEP[index] != NULL )
    {
        timer_FEP[index]->Stop();
    }

    send_working_message(FEP, index);
    if ( state_System != RUNNING )
    {
        if ( (state_MCP[0] == RUNNING) && (state_MCP[1] == RUNNING) && (state_MCP[2] == RUNNING) && (state_MCP[3] == RUNNING) && 
             (state_MCP[4] == RUNNING) && (state_MCP[5] == RUNNING) && (state_MCP[6] == RUNNING) && (state_MCP[7] == RUNNING) &&
             (state_FEP[0] == RUNNING) && (state_FEP[1] == RUNNING) )
        {
            OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> Whole BTS boot finished");
            /****send system runing message**********/
            state_System = RUNNING;
            send_system_runing();
        }
    }


    return true;
}


bool L3BootTask::L3BOOT_TIMER_Expires(CComMessage* pComMsg)
{
    char *data;
    char element, index;

    data = (char *)(pComMsg->GetDataPtr()); 
    element = *data;
    index = *(data + 1);

    /***send alarm message*********/
    if (element != L2_PRELOADER)
    {
        CComMessage* pmessage_alarm;
        struct alarm_message * message_alarm;

        pmessage_alarm = new (this, 6) CComMessage;
        message_alarm = (struct alarm_message *)(pmessage_alarm->GetDataPtr());

        message_alarm->tranId = 0;
        message_alarm->element = element;
        message_alarm->index = index;
        message_alarm->type = ALARM_EXPIRES;

        pmessage_alarm->SetDataLength(6);

        pmessage_alarm->SetMessageId(BOOT_ALARM/**message id***/);
        pmessage_alarm->SetDstTid(M_TID_SYS/*****/);
        pmessage_alarm->SetSrcTid(M_TID_BM/*****/);

        CComEntity::PostEntityMessage(pmessage_alarm);
        /***reset element**/
        LOG2(LOG_CRITICAL, 0, "L3BOOT_TIMER_Expires, element= %d, index = %d \n", element, index);
#ifndef WBBU_CODE
        if(element==MCP&&index!=ELEMENT_ALL)
        {
                state_MCP[index] =  TIME_OUT_RESET;
        }
#endif
        system_reset(element, index);
    }
    else
    {
        LOG(LOG_CRITICAL, 0, "L3BOOT_TIMER_Expires, L2 Preloader didn't boot up \n");
        system_reset(L2, 0);    
    }

    return true;
}

int L3Boot5MinTestFlag = 0;
int L3Boot5MinTest()
{

    L3Boot5MinTestFlag = 1;
    L3BootTask::GetInstance()->L3BOOT_5MinTIMER_Expires();

    return 0;

}

void resetExcepL3()
{
	 L3BootTask::GetInstance()->resetBtsExcepL3();
}

void L3BootTask::resetBtsExcepL3()
{
#ifndef WBBU_CODE
		    state_FEP[0] = BOOTING; 
		    state_FEP[1] = BOOTING;
		    state_AUX = BOOTING; 
		    state_AUX = BOOTING; 
		    state_L2 = BOOTING;
		/***state init***/
		    for ( int i = 0; i<8; i++ )
		    {
		        state_MCP[i] = BOOTING;
		    }  
			
		   down_fpga();

		/*cpld reset register ,  bit0 reset bts, bit1 reset l2, bit2 reset L2fpga and MCP, bit3 reset L1fpga and aux, bit4 reset Fep0, bit5 reset fep1***/
		 
		    /***waiting*****/
		    taskDelay(10);
		    /***reset all fpga*****/
		    system_reset(FPGA, 0);

		    taskDelay(100);
		    system_reset(AUX, 0);

		    taskDelay(10);
		    /**reset L2 or bts***/
		    system_reset(L2, 0);  
#endif			
}

int L3ResetL2Flag = 0;

void L3BootTask::L3BOOT_5MinTIMER_Expires()
{
#ifndef WBBU_CODE
	if((timesIn5mins_AUX>10)||(timesIn5mins_FEP0>10)&&(timesIn5mins_FEP1>10)||(L3Boot5MinTestFlag==1))
	{
		#if 1
		if(L3ResetL2Flag)
		{
		    state_FEP[0] = BOOTING; 
		    state_FEP[1] = BOOTING;
		    state_AUX = BOOTING; 
		    state_AUX = BOOTING; 
		    state_L2 = BOOTING;
		/***state init***/
		    for ( int i = 0; i<8; i++ )
		    {
		        state_MCP[i] = BOOTING;
		    }  
			
		   down_fpga();

		/*cpld reset register ,  bit0 reset bts, bit1 reset l2, bit2 reset L2fpga and MCP, bit3 reset L1fpga and aux, bit4 reset Fep0, bit5 reset fep1***/
		 
		    /***waiting*****/
		    taskDelay(10);
		    /***reset all fpga*****/
		    system_reset(FPGA, 0);

		    taskDelay(100);
		    system_reset(AUX, 0);

		    taskDelay(10);
		    /**reset L2 or bts***/
		    system_reset(L2, 0);  
			
		    L3ResetL2Flag = 0;
		}
		else
		{
			down_fpga();

		/*cpld reset register ,  bit0 reset bts, bit1 reset l2, bit2 reset L2fpga and MCP, bit3 reset L1fpga and aux, bit4 reset Fep0, bit5 reset fep1***/
		 
		    /***waiting*****/
		    taskDelay(10);
		    /***reset all fpga*****/
		    system_reset(FPGA, 0);

		    //taskDelay(100);
		   // system_reset(AUX, 0);

		    taskDelay(10);
		    /**reset L2 or bts***/
		    system_reset(L2, 0);  
			L3ResetL2Flag = 1;

		}

	          
		    L3Boot5MinTestFlag = 0;
		#endif
		
		    
		   printf("\nL3BOOT_5MinTIMER_Expires,reboot bts %d,%d,%d\n",timesIn5mins_AUX,timesIn5mins_FEP0,timesIn5mins_FEP1);
		/*
		bspSetBtsResetReason(RESET_REASON_DSPERR_IN5MIN);
   	       taskDelay(50);
   	       rebootBTS(BOOT_CLEAR );*/
	}
	//printf("\nL3BOOT_5MinTIMER_Expires,%d,%d,%d\n",timesIn5mins_AUX,timesIn5mins_FEP0,timesIn5mins_FEP1);
	   timesIn5mins_AUX = 0;
	   timesIn5mins_FEP0 = 0;
	   timesIn5mins_FEP1 = 0;
#endif
}

/*************************************************************************************************************************/
/*reset ,  bit0 reset bts, bit1 reset l2, bit2 reset L2fpga and MCP, bit3 reset L1fpga and aux, bit4 reset Fep0, bit5 reset fep1***/
void L3BootTask::system_reset(char element, char index)
{
    unsigned int value;

    switch(element)
    {
        case BTS:
            OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> reset BTS");   
            break;
        case L2:
            OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> Reset L2 PowerPC system"); 
            break;
        case MCP:
            OAM_LOGSTR1(LOG_CRITICAL, 0, "[BTS boot] ==> Reset MCP %d", index);    
            break;
        case AUX:
            OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> Reset AUX");  
            break;
        case FEP:
            OAM_LOGSTR1(LOG_CRITICAL, 0, "[BTS boot] ==> Reset FEP %d", index);    
            break;
        case FPGA:
            OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> Reset FPGA");  
            break;
        default:
            OAM_LOGSTR1(LOG_CRITICAL, 0, "[BTS boot] ==> Invalid CPU type %d", element);  
            break;
    }


    /************/
    if ( bootFlag == REBOOT_DISABLE )
    {
        OAM_LOGSTR(LOG_CRITICAL, 0, "[BTS boot] ==> Reset is disabled, please execute rebootEnable()");        
        return;
    }

    switch ( element )
    {
        case BTS:
#ifndef WBBU_CODE
            value = BIT0;   
#endif
            rebootBTS(0); 

            /*CPLD_WRITE_REG(RESET_REG, value); */
            break;

        case L2:
#ifndef WBBU_CODE
            value = BIT1;
            CPLD_WRITE_REG(RESET_REG, value);
            state_L2 = RESET;
            if(timer_L2Preloader)
            {
                timer_L2Preloader->Start();
            }
#else
       Reset_Dsp (5,0);
#endif
            break;

        case MCP:
#ifdef WBBU_CODE
        	Reset_All_DSP();
#else
            /***send reset message to L2bootTask*********/
            {
                if(index != ELEMENT_ALL )
                {
                        if(state_MCP[index] ==  RESET)
                            break;
                }
                CComMessage* pmessage;
                char * message;

                pmessage = new (this, 1) CComMessage;
                message = (char *)(pmessage->GetDataPtr());
                message[0] = index;

                pmessage->SetDataLength(1);
                pmessage->SetMessageId(L3TOMCP_RESET/**message id***/);
                pmessage->SetDstTid(M_TID_L2BOOT/*****/);
                pmessage->SetSrcTid(M_TID_BM/*****/);

                CComEntity::PostEntityMessage(pmessage);
                if ( index == ELEMENT_ALL )
                {
                    for ( int i=0;i<8;i++ )
                    {
                        state_MCP[i] =  RESET;
                        timer_MCP[i]->Stop();
                        timer_MCP[i]->Start();
                    }
                }
                else
                {
                    state_MCP[index] = RESET;   
                    timer_MCP[index]->Stop();
                    timer_MCP[index]->Start();
                }
            }
            /*
            value = BIT2;
            CPLD_WRITE_REG(RESET_REG, value);
            */
            #endif
            break;

        case AUX:  
        	#ifndef WBBU_CODE
            value = BIT3;    /******reset L1fpga********/
            CPLD_WRITE_REG(RESET_REG, value);
            state_AUX = RESET;
            if ( RUNNING == state_L2)
            {
                timer_AUX->Stop();
                timer_AUX->Start();
            }
	     /*lijinan 20090202 */
	     timesIn5mins_AUX++;
#else
            Reset_Dsp (1,2);
#endif
            break;

        case FEP:
        	#ifndef WBBU_CODE
            switch ( index )
            {
                case FEP0:
                    value = BIT4;
                    state_FEP[0] = RESET;
                    timer_FEP[0]->Stop();
                    timer_FEP[0]->Start();
					
		     /*lijinan 20090202 */
	             timesIn5mins_FEP0++;
                    break;
                case FEP1:
                    value = BIT5;
                    state_FEP[1] = RESET;
                    timer_FEP[1]->Stop();
                    timer_FEP[1]->Start();
					
		    /*lijinan 20090202 */
	             timesIn5mins_FEP1++;
                    break;
                case ELEMENT_ALL:
                    value = BIT4 | BIT5;
                    state_FEP[0] = RESET; state_FEP[1] = RESET;
                    timer_FEP[0]->Stop();
                    timer_FEP[0]->Start();
                    timer_FEP[1]->Stop();
                    timer_FEP[1]->Start();

		 /*lijinan 20090202 */
	             timesIn5mins_FEP0++;
		      timesIn5mins_FEP1++;
                    break;
            }
            CPLD_WRITE_REG(RESET_REG, value);
            #else

             Reset_Dsp (1,2);
#endif
            break;

        case FPGA:
#ifndef WBBU_CODE
            value = BIT2 | BIT3 | BIT4 | BIT5;
            CPLD_WRITE_REG(RESET_REG, value);
#endif
            break;

        default:
            LOG1(LOG_CRITICAL, 0, "system_reset: cpu element error, element=%d!!!!\n",element);
            break;

    }/***element switch****/    

}

bool L3BootTask::L3BOOT_System_Rest(CComMessage* pComMsg)
{
    char *data;
    char element, index;

    data = (char *)(pComMsg->GetDataPtr()) + 2 /**tran id****/; 
    element = *data;
    index = *(data + 1);

    system_reset(element, index);

    return true;
}

void L3BootTask::send_working_message(char element, char index)
{
    CComMessage* pmessage;
    char * message;

    pmessage = new (this, 4) CComMessage;
    message = (char *)(pmessage->GetDataPtr());
    message[0] = 0;
    message[1] = 0;
    message[2] = element;
    message[3] = index;

    pmessage->SetDataLength(2);
    pmessage->SetMessageId(BOOT_WORKING/**message id***/);
    pmessage->SetDstTid(M_TID_SYS/*****/);
    pmessage->SetSrcTid(M_TID_BM/*****/);

    CComEntity::PostEntityMessage(pmessage);
}

#define MSGID_DIAG_1MIN_TIME_OUT         0x3902  //lijinan 20081013
void L3BootTask::send_system_runing()
{
    CComMessage* pmessage;

    pmessage = new (this, 0) CComMessage;
    pmessage->SetDataLength(0);
    pmessage->SetMessageId(BOOT_SYSTEM_RUNING/**message id***/);
    pmessage->SetDstTid(M_TID_SYS/*****/);
    pmessage->SetSrcTid(M_TID_BM/*****/);

    CComEntity::PostEntityMessage(pmessage);    

    //启动定时版本查询定时器
    CComMessage *tmoMsg = new(this, 0)CComMessage;
    if (tmoMsg)
    {
        tmoMsg->SetDstTid(M_TID_DIAGM);
        tmoMsg->SetSrcTid(M_TID_BM);
        tmoMsg->SetMessageId(MSGID_DIAG_1MIN_TIME_OUT);
#ifdef WBBU_CODE
        if(Vertimer==NULL)
        {
         Vertimer = new CTimer(1, 5000, tmoMsg);//5s比较合理
        if (Vertimer)
        {
            Vertimer->Start();
        }
       }
	
#else
         CTimer * timer = new CTimer(1, 10000, tmoMsg);
        if (timer)
        {
            timer->Start();
        }
#endif
    }
    //lijinan 20081013
    #ifdef WBBU_CODE

	//wangwenhua add 2012-7-23


	/****************************************************
     启动1min定时器给PM任务，开始去查询L2层active Session 


	****************************************************/
       CComMessage *tmoMsg_New = new(this, 2)CComMessage;
       if (tmoMsg_New)
	    {
			 tmoMsg_New->SetDstTid(M_TID_PM);
			 tmoMsg_New->SetSrcTid(M_TID_BM);
			 tmoMsg_New->SetMessageId(M_BOOT_ONE_Minter_Timer_M);
              unsigned char *ptr =(unsigned char *)tmoMsg_New->GetDataPtr();
			 ptr[0] =0xff;
			 ptr[1] =0xff;
			  if(ActiveUsertime==NULL)
			   {
				      ActiveUsertime = new CTimer(1, 1000*60, tmoMsg_New);//60s
				      if (ActiveUsertime)
				      {
				            ActiveUsertime->Start();
				      }
			   }
		
	     }

   #endif


}
int L3BootTask::copy_file_to_ramdisk(const char *filename)
{

#if !defined(__WIN32_SIM__)&&!defined(__NUCLEUS__)  /****to debug********/	

    char buffer[100]; 
    int fd0, fd1;
    char filename0[200], filename1[200];
    struct stat statData;   /* used to get size */
    int state;

/*	if(COPY_FILE_FROM_FTPSERVER){ */ 
    if ( BootSource != DATA_SOURCE_CF )
    {
        strcpy(filename0, FTP_DEVICE_NAME);
        strcat(filename0, FTP_SERVER_PATH); 
    }
    else
    {          /****from cf card********/
        if ( bspGetBootPlane() != BOOT_PLANE_B )
        {
            strcpy(filename0, FILE_PATH_CF_A);  
        }
        else
        {
            strcpy(filename0, FILE_PATH_CF_B);
        }

    }

    strcat(filename0, filename);

    int openCount=0;
    do
    {
        fd0 = open(filename0, O_RDONLY, 0664);
        if ( fd0 == ERROR )
        {
            LOG1(LOG_CRITICAL, 0, "Open file %s on boot device error! \n",(int)filename);
            taskDelay(10);
        }
        else
        {
            break;
        }
        openCount ++;
    } while ( openCount < 3 );
    if (ERROR == fd0)
    {
        LOG1(LOG_CRITICAL, 0, "Give up on opening file %s on boot device! BOOT UP FAILED \n",(int)filename);
        return ERROR;
    }

    /**DEBUG**/
    //printf("filename0 is :%s!!!\n",filename0);
#ifndef WBBU_CODE
    strcpy(filename1, DEVICE_RAMDISK);
#else
       strcpy(filename1, "/RAMD:0/");
#endif
    strcat(filename1, RAMDISK_DOWNLOAD_PATH);
    strcat(filename1, filename);
    fd1 = open(filename1, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if ( fd1 == ERROR )
    {
        LOG1(LOG_CRITICAL, 0, "Open file %s on RamDisk error! \n",(int)filename);
        return ERROR;
    }
    /**DEBUG**/
    //printf("filename1 is :%s!!!\n",filename1);

    state = ioctl (fd0, FIOFSTATGET, (int)&statData);
    if ( state == ERROR )
    {
        LOG(LOG_CRITICAL, 0, "L3bootTask ioctl error!!!!\n");
        close(fd0);
        close(fd1);
        return ERROR;
    }
    for ( int i=0; i<statData.st_size; )
    {
        int count = read(fd0, buffer, 100);   
        write(fd1, buffer, count);  
        i = i + count;
    }

    OAM_LOGSTR1(LOG_CRITICAL, 0, "[BTS boot] ==> Download file :%s finished",(int)filename);

    close(fd0);
    close(fd1);

#endif
    return true;
}

int rebootMCP(char index)
{

    if ( index >= 8 )
    {
        index = ELEMENT_ALL; /***all****/
    }

    L3BootTask::GetInstance()->system_reset(MCP, index);

    return 0;

}

void reloadL1()
{
	BOOT_PARAMS       bootParams;

	(void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
	 iam(bootParams.usr, bootParams.passwd);

	
    L3BootTask::GetInstance()->copy_file_to_ramdisk(MCP_APP_CODE_FILENAME);
    L3BootTask::GetInstance()->copy_file_to_ramdisk(AUX_APP_CODE_FILENAME);
    L3BootTask::GetInstance()->copy_file_to_ramdisk(FEP_APP_CODE_FILENAME);
    taskDelay(10);

}

void reloadL2()
{
	BOOT_PARAMS       bootParams;

	(void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
	 iam(bootParams.usr, bootParams.passwd);


    L3BootTask::GetInstance()->copy_file_to_ramdisk(L3BootTask::GetInstance()->L2_apppName);
    taskDelay(10);
}

void reloadAll()
{
	BOOT_PARAMS       bootParams;

	(void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
	 iam(bootParams.usr, bootParams.passwd);


    L3BootTask::GetInstance()->copy_file_to_ramdisk(L3BootTask::GetInstance()->L2_apppName);

    L3BootTask::GetInstance()->copy_file_to_ramdisk(MCP_APP_CODE_FILENAME);
    L3BootTask::GetInstance()->copy_file_to_ramdisk(AUX_APP_CODE_FILENAME);
    L3BootTask::GetInstance()->copy_file_to_ramdisk(FEP_APP_CODE_FILENAME);

    L3BootTask::GetInstance()->copy_file_to_ramdisk(FPGA_L2_FILENAME);
    L3BootTask::GetInstance()->copy_file_to_ramdisk(FPGA_L1_FILENAME);
    L3BootTask::GetInstance()->copy_file_to_ramdisk(FPGA_FEP0_FILENAME);
    L3BootTask::GetInstance()->copy_file_to_ramdisk(FPGA_FEP1_FILENAME);
    taskDelay(10);
}






/*******update bootrom*****************************/

#define Addr1 0xff815554   /*******15554 = 0x5555 << 2*****/
#define Addr2 0xff80aaa8   /*******0aaa8 = 0x2aaa << 2 *****/

int flashChipErase()
{
    printf("Erase the whole flash chip .........");
    *(volatile long*)Addr1 = 0x00AA00AA;
    *(volatile long*)Addr2 = 0x00550055;
    *(volatile long*)Addr1 = 0x00800080;
    *(volatile long*)Addr1 = 0x00AA00AA;
    *(volatile long*)Addr2 = 0x00550055;
    *(volatile long*)Addr1 = 0x00100010;
    taskDelay(10);
    if ( (*(volatile long*)Addr1 & 0x00800080) != 0x00800080 )
    {
        printf("flash chip erase error!\n");
        return -1;
    }
    printf("Done\n");
    return 0;
}


int wordProgram(long addr, unsigned long v)
{
    long temp1,temp2;
    int i;
    *(volatile long*)Addr1 = 0x00AA00AA;
    *(volatile long*)Addr2 = 0x00550055;
    *(volatile long*)Addr1 = 0x00A000A0;
    *(volatile long*)addr = v;
    for (i=0; i<4000; i++)
        i++;

    for ( i = 0; i<100; i++)
    {  // polling to see if the flashing has finished
        temp1 = *(volatile long*)addr & 0x00400040;
        temp2 = *(volatile long*)addr & 0x00400040;
        if (temp1 == temp2)
        {
            break;
        }
    }
    if ( temp1 != temp2 )
    {
        printf("word program error at address %x! \n", (UINT32)addr);
        return -1;
    }
    if ( *(volatile long*)addr != v) 
    {
        printf("word program error at address %x! \n", (UINT32)addr);
        return -1;
    }
    return 0;
}
#ifdef WBBU_CODE
#include "tffs/flflash.h"
#include "tffs/backgrnd.h"

extern "C" FLStatus TE28F160Write    (
    int     				offsetAddress,
    const void  		*buffer,
    int             length
    );
extern "C"  FLStatus TE28F160Erase
    (
/*    FLFlash vol,*/
    int     firstErasableBlock,
    int     numOfErasableBlocks
    );

extern "C" FLStatus TE28F160Program (
    UINT16 *    addr,
    UINT16      value
    );
void updateBootrom()
{

    FILE * file;
    int count;
    char filename[40];
    unsigned short buffer;
    unsigned int addr;
    unsigned char *pTemp;
    BOOT_PARAMS       bootParams;
    (void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
    iam(bootParams.usr, bootParams.passwd);
#if 0
    if ( ERROR == L3BootTask::GetInstance()->copy_file_to_ramdisk("bootrom.bin"))
    {
        LOG(LOG_CRITICAL, 0, "error L3 bootrom file open fail!! \n"); 
        return;
    }
#endif
    strcpy(filename, DEVICE_RAMDISK);
    strcat(filename, RAMDISK_DOWNLOAD_PATH);
    strcat(filename, "bootrom.bin");

    file = fopen(/*filename*/"/RAMD:0/load/bootrom.bin", "rb");
    if ( file == NULL )
    {
        LOG(LOG_CRITICAL, 0, "error L3 bootrom file open fail!! \n");       
        return;
    }
    #if 0
    pTemp =(unsigned char*) malloc(1024*1024);

    if(pTemp==NULL)
    	{
    	      fclose(file);
    	  LOG(LOG_CRITICAL, 0, "no enough memory for bootrom update \n");
    	}


    #endif
  #if 1
   /* flashChipErase();
   */

    TE28F160Erase(0,39);
   addr = (0xfff00000);
    int wordCount=0;
    do
    {

        count = fread(&buffer, 2, 1, file);
        if ( count > 0 )
        {
           #if 1
            if ( TE28F160Program((UINT16*)addr, buffer) )
            {  //failed writing to flash
                printf("!!!  Wring L3 Bootrom failed, need to do it again. DO NOT REBOOT or POWERCYCLE \n");
                break;
            }
            #endif
          #if 0
            if(TE28F160Write(4*wordCount, (UINT16*)buffer,count))
            	{
            	       printf("!!!  Wring L3 Bootrom failed, need to do it again. DO NOT REBOOT or POWERCYCLE \n");
                     break;
            	}
            #endif
            addr = addr+count*2;
            wordCount++;
        }
        if ( 0 == (wordCount%1024) )
        {
            printf("%d word written to FLASH\n",wordCount);
        }
    }while ( !feof(file) );
    printf("Update L3 Bootrom finished, with %d B written\n", (addr-0xfff00100)*4);

    #endif
    fclose(file);

}
#else
void updateBootrom()
{

    FILE * file;
    int count;
    char filename[40];
    unsigned int buffer;
    unsigned int addr;

    BOOT_PARAMS       bootParams;
    (void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
    iam(bootParams.usr, bootParams.passwd);

    if ( ERROR == L3BootTask::GetInstance()->copy_file_to_ramdisk("bootrom_L3.bin"))
    {
        LOG(LOG_CRITICAL, 0, "error L3 bootrom file open fail!! \n"); 
        return;
    }

    strcpy(filename, DEVICE_RAMDISK);
    strcat(filename, RAMDISK_DOWNLOAD_PATH);
    strcat(filename, "bootrom_L3.bin");

    file = fopen(filename, "rb");
    if ( file == NULL )
    {
        LOG(LOG_CRITICAL, 0, "error L3 bootrom file open fail!! \n");       
        return;
    }

    flashChipErase();
    addr = (0xfff00100);
    int wordCount=0;
    do
    {

        count = fread(&buffer, 1, 4/*5*1024*/, file);
        if ( count > 0 )
        {
            if ( wordProgram(addr, buffer) )
            {  //failed writing to flash
                printf("!!!  Wring L3 Bootrom failed, need to do it again. DO NOT REBOOT or POWERCYCLE \n");
                break;
            }
            addr = addr+4;
            wordCount ++;
        }
        if ( 0 == (wordCount%1024) )
        {
            printf("%d word written to FLASH\n",wordCount);
        }
    }while ( !feof(file) );
    printf("Update L3 Bootrom finished, with %d B written\n", (addr-0xfff00100)*4);
    fclose(file);

}

#endif
void L3BootTask::ShowBootState()
{
    printf("L2 Power PC is : %s\n", (RUNNING == state_L2)? "RUNNING": "BOOTING");
    printf("AUX         is : %s\n", (RUNNING == state_AUX)? "RUNNING": "BOOTING");
    for (int mcpIndex=0; mcpIndex<M_NumberOfMCP; mcpIndex++)
    {
        printf("MCP %d is : %s\n", mcpIndex,(RUNNING == state_MCP[mcpIndex])? "RUNNING": "BOOTING");
    }
    for (int fepIndex=0; fepIndex<M_NumberOfFEP; fepIndex++)
    {
        printf("FEP %d is : %s\n", fepIndex,(RUNNING == state_FEP[fepIndex])? "RUNNING": "BOOTING");
    }
}

extern "C" STATUS cpuBootStateShow()
{
    L3BootTask::GetInstance()->ShowBootState();
    return OK;
}


 bool sysStateIsRun()
{
	 if(L3BootTask::GetInstance()->getState_System()==1)
	 	return 1;
	 else
	 	return 0;
	 
}
#ifdef WBBU_CODE
 /******************************************************************************
*
*	FUNCTION NAME: fpga_download
*	___________________________________________________________________________
*
*	DESCRIPTION:	Download data to FPGA according to the FPGA configuration timing
*
*	INPUTS:			N/A
*
*	RETURNS:		OK, or ERROR
*
*******************************************************************************/
STATUS  FPGA_download()
{
  L3BootTask::GetInstance()->down_fpga_wbbu();
	return (0);
}
extern  unsigned char Need_Load_Cfg;
extern int bootFlag;
extern "C"  void ResetDspNew1(unsigned char index);
 extern "C" void SetFpga_Para();
 unsigned char  Reset_Dsp(unsigned char index,unsigned char flag)
 {
       
      if((index>5)||(index==0))
      	{
      	     printf("dsp index(1-5)error:%d\n",index);
      	     return 0;
      	     	
      	}
        index--;
        if(bootFlag == REBOOT_DISABLE)
        {
            return 0;
        }
         switch (index)
         {
               case 0:
               	Reset_DSP_FLAG = 0x11;
               	//taskDelay(1000);
               	break;
               case 1:
               	Reset_DSP_FLAG = 0x22;
               	
               //	taskDelay(1000);
               	break;
               case 2:
               	Reset_DSP_FLAG = 0x33;
               	//taskDelay(1000);
               	break;
               case 3:
               	Reset_DSP_FLAG = 0x44;
               	//taskDelay(1000);
               	break;
               case 4:
               	Reset_DSP_FLAG = 0x55;
               	//taskDelay(1000);
               	break;
               default:
               	break;
         }
         if(flag==1)
           {
               Need_Load_Cfg  = 1;
            }
         else if(flag ==2)
         	{
         	  ResetDspNew1( index);
         	  if(index==0)//手动复位AUX时，也主动加载校准数据wangwenhua modify 20111221
         	  {
         	  	     Need_Load_Cfg = 1;
         	  }
         	  return  1;
         	}
           ResetDspNew(index);
            return  1;
 }
  void   Reset_All_DSP()
 {
     unsigned char  i ;
   //   for(;;)
       unsigned int base_addr = 0xd1000012;
      if(bootFlag == REBOOT_DISABLE)
        {
            return;
        }
		
	*(unsigned short*)base_addr = 0x0000;/**first reset all dsp***/
       	   Reset_Dsp(2,0);
      	      	   
      	      	   taskDelay(50);
		 Reset_Dsp(3,0);
      	      	   
      	      	   taskDelay(50);
			Reset_Dsp(4,0);
      	      	   
      	      	   taskDelay(50);
		Reset_Dsp(5,0);
      	      	   
      	      	   taskDelay(100);
		Reset_Dsp(1,0);
      	      	   
      	      	   taskDelay(10);
 }
 void reloadDSP1()
{
	BOOT_PARAMS       bootParams;

	(void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
	 iam(bootParams.usr, bootParams.passwd);

	
    L3BootTask::GetInstance()->copy_file_to_ramdisk(DSP_Name_No1_L1);

    taskDelay(10);

}
  void reloadDSP2()
{
	BOOT_PARAMS       bootParams;

	(void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
	 iam(bootParams.usr, bootParams.passwd);

	
    L3BootTask::GetInstance()->copy_file_to_ramdisk(DSP_Name_No2_L1);
  
    taskDelay(10);

}
   void reloadDSP3()
{
	BOOT_PARAMS       bootParams;

	(void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
	 iam(bootParams.usr, bootParams.passwd);

	
    L3BootTask::GetInstance()->copy_file_to_ramdisk(DSP_Name_No3_L1);

    taskDelay(10);

}
    void reloadDSP4()
{
	BOOT_PARAMS       bootParams;

	(void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
	 iam(bootParams.usr, bootParams.passwd);

	
    L3BootTask::GetInstance()->copy_file_to_ramdisk(DSP_Name_No4_L1);

    taskDelay(10);

}
     void reloadDSP5()
{
	BOOT_PARAMS       bootParams;

	(void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
	 iam(bootParams.usr, bootParams.passwd);

	
    L3BootTask::GetInstance()->copy_file_to_ramdisk(DSP_Name_No5_L2);

    taskDelay(100);

}
 void reloadFPGA()
{
	BOOT_PARAMS       bootParams;

	(void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
	 iam(bootParams.usr, bootParams.passwd);

	
    L3BootTask::GetInstance()->copy_file_to_ramdisk(FPGA_WBBU_FILENAME);

    taskDelay(100);

}
void reloadAllDSP()
{
        reloadDSP5();
        reloadDSP4();
        reloadDSP3();
        reloadDSP2();
        reloadDSP1();
}
void reloadAllL1()
{
         reloadDSP4();
        reloadDSP3();
        reloadDSP2();
        reloadDSP1();
}

void reloadWRRU()
{
	BOOT_PARAMS       bootParams;

	(void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
	 iam(bootParams.usr, bootParams.passwd);

	
      L3BootTask::GetInstance()->copy_file_to_ramdisk(WRRU_Mcu);
       L3BootTask::GetInstance()->copy_file_to_ramdisk(WRRU_Fpga);

    taskDelay(100);

}


extern "C" void  Reset_ALL()
{
          if(bootFlag == REBOOT_DISABLE)
        {
            return ;
        }
      FPGA_download();
     //  RestFPGA();
      taskDelay(100);

      Reset_Dsp(2,2);
      taskDelay(100);
      Reset_Dsp(3,2);
      taskDelay(100);
      Reset_Dsp(4,2);
      taskDelay(100);
      Reset_Dsp(5,2);
      taskDelay(100);
      Reset_Dsp(1,2);
      taskDelay(100);

       SetFpga_Para();
      SetLocalFepRxInt(1);
      
      
     
}


extern "C" unsigned char  RestFPGA()
{
        if(bootFlag == REBOOT_DISABLE)
        {
            return 0;
        }
        *(unsigned short*)0xd100005a= 0x000a;
        for(int i=0; i<10000; i++)
        {
        }
         *(unsigned short*)0xd100005a = 0x0000;
	 return 0;
}

extern "C" unsigned char   R_FPGA()
{
     
        if(bootFlag == REBOOT_DISABLE)
        {
            return 0;
        }
      RestFPGA();
      taskDelay(100);
      SetFpga_Para();
      SetLocalFepRxInt(1);
	return 0;
}

//新添加的对AIF模块逻辑复位的寄存器地址为0xd100005e,1=复位，0=复位释放；

//建议老王在boot DSP成功后，1s之后对AIF模块逻辑复位一下，以便保护；
extern "C" void    ResetAifSerdies()
{
       
        if(bootFlag == REBOOT_DISABLE)
        {
            return ;
        }
        *(unsigned short*)0xd100005e= 0x0001;
        for(int i=0; i<10000; i++)
        {
        }
         *(unsigned short*)0xd100005e = 0x0000;
}

extern "C" unsigned char  IfResetDSP();

unsigned int    g_ResetDSP1_NOUser = 0;
extern "C" void    ResetDSP1_NOUSER()
{
         if(bootFlag == REBOOT_DISABLE)
        {
            return ;
        }
	if(IfResetDSP())
	{
	      printf("Reset dsp1 due to no active user for over 15 min\n");
	      
	      Reset_Dsp(1,2);
		 g_ResetDSP1_NOUser++;
	}
}

void print_dsp1_ex_data(unsigned char index)
{
    if(0 == index)
    {
        printf("aux report errormsg:\n");
    }
    else if(1 == index)
    {
        printf("fep0 report errormsg:\n");
    }
    else if(2 == index)
    {
        printf("fep1 report errormsg:\n");
    }
    else
    {
        printf("[index=0:aux errormsg,1:fep0 errormsg,2:fep1 errormsg],other value is error.\n");
        return;
    }
    
    for(int i=0;i<200;i++)
    {
        printf("%2x,",g_dsp1_exception_data[index][i]);
        if(0 == ((i+1)%20))
        {
            printf("\n");
        }
    }
}
#endif
