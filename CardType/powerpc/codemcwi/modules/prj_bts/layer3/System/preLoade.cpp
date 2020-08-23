#include "taskLib.h"
#include "stat.h"
#include "fcntlcom.h"
#include "string.h"
#include "iolib.h"
#include "stdio.h"
//#include "blkIo.h"
#include "dosFsLib.h"
#include "bootLib.h"
#include "fcntl.h"
#include "config.h"
#include "ramDrv.h"
#include "remLib.h"
#include "netDrv.h"

/***l3 boot task****/

#define TRAN_READY   0xAAAAAAAA
#define TRAN_FINISH  0x11111111


//#define FTP_SERVER			"192.168.2.154" 
#define RAMDISK_NAME		"/RAMDISK"		/** /download/ **/
#define FILE_PATH			"/download/"/** /download/ **/

int copy_file(char * filename){

	char buffer[100]; 
	int fd0, fd1;
	char filename0[40], filename1[40];
	struct stat statData;	/* used to get size */
	int state;
	int i;


	strcpy(filename0, "devFTP:");
/*	strcat(filename0, "");	  */
	
	strcat(filename0, filename);
	fd0 = open(filename0, O_RDONLY, 0664);
	if(fd0 == ERROR ){
		printf("L3bootTask open  error!!!!\n");
	}
	printf("fd0 is %d        ", fd0);

	strcpy(filename1, RAMDISK_NAME);
	strcat(filename1, FILE_PATH);
	strcat(filename1, filename);
	fd1 = open(filename1, O_WRONLY | O_CREAT | O_TRUNC, 0664);
	if(fd1 == ERROR ){
		printf("L3bootTask open  error!!!!\n");
	}
	printf("fd1 is %d\n", fd1);
	
	state = ioctl (fd0, FIOFSTATGET, (int)&statData);
	if(state == ERROR){
		printf("L3bootTask ioctl error!!!!\n");
		close(fd0);
		close(fd1);
		return ERROR;
	}
	
	printf("%s size:%d        ", filename, statData.st_size);
	for(i=0; i<statData.st_size; ){
		int count = read(fd0, buffer, 100);   
		write(fd1, buffer, count);	
		i = i + count;
	}
	printf("actual write size:%d \n", i);
	close(fd0);
	close(fd1);
	
	return 1;

}

#define CPLD_BASE 0x1C000000
#define CPLD_WRITE_REG(reg, temp)  {unsigned int  val;  val = /*LONGSWAP*/ (temp);	*(unsigned int *)(CPLD_BASE + reg) = val ; }
#define CPLD_READ_REG(reg, temp)	{unsigned int  val; val = *(unsigned int *)(CPLD_BASE + reg);  *temp = /*LONGSWAP */(val); }

#define RESET_REG		0x0C   
#define FPGA_REG		0x08   
#define FPGA_STATE_REG  0x04   
#define MANUFA_DATE_REG 0x14   /**manufacture date****/
#define CONFIG_DATASTATE_REG 0x18   /**manufacture date****/
#define CPLD_REV_REG	0x10   
#define SYSTEM_CMD_REG	0x00   

#define  PCI_REMAP_MEM1		0xf2000000
/* #define	 L2ENTRYADDR		0xf000000	*//***jump addr****/
#define	 L2ENTRYADDR		0x0000000/*0x10000*/


void down_file(){
	
	/**copy file from ftp server** *****/
	BLK_DEV               *pBlkDev; 
	DOS_VOL_DESC          *pVolDesc; 
	char path[40];
	
	BOOT_PARAMS       bootParams;
	(void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);

	pBlkDev = ramDevCreate (0, 512, 1024, 1024*32, 0); /**512* (1024*32) = 16M*********/
	pVolDesc = dosFsMkfs(RAMDISK_NAME/*"RAMDISK:"*/, pBlkDev);

	strcpy(path, RAMDISK_NAME);
	strcat(path, FILE_PATH);
	mkdir(path);

    /**use to oam****/
    mkdir("/RAMDISK/WORK/");
    mkdir("/RAMDISK/BACKUP/");
	
	/*iam("vxworks","vxworks");*/
	/*netDevCreate("devFTP:", FTP_SERVER, 1); */ /**0 is rsh, 1 is ftp*********/
	iam(bootParams.usr,bootParams.passwd);
	netDevCreate("devFTP:", bootParams.had, 1);  /**0 is rsh, 1 is ftp*********/

	copy_file("bootrom_L2.bin");
	copy_file("fpga_L2.out");
	copy_file("fpga_L1.out");
	copy_file("fpga_FEP0.out");
	copy_file("fpga_FEP1.out");

//	copy_file("dsp_aux.out");
	/* copy_file("vxWorks_L2.st"); */
}


void down_fpga(){
	int i, j, count;
	unsigned int  temp;
	char filename_L2[40];
	char filename_L1[40];
	char filename_FEP0[40], filename_FEP1[40];
	FILE *file_L2,* file_L1, *file_FEP0, *file_FEP1;


	j = 5;
	while(j--){

		/****fpga state******/
		CPLD_READ_REG(FPGA_STATE_REG, &temp);	
		while(!(temp & (0x1<<8))/***bit8***/){
			CPLD_READ_REG(FPGA_STATE_REG, &temp);	
		}
		/***config data state******/
		CPLD_WRITE_REG(CONFIG_DATASTATE_REG, 0x0);


		/*start config fpga***/
		CPLD_WRITE_REG(SYSTEM_CMD_REG, 1<<0/*0x1*/);
		
		/***download fpga******/
		strcpy(filename_L2, RAMDISK_NAME);
		strcat(filename_L2, FILE_PATH);
		strcat(filename_L2, "fpga_L2.out");
		file_L2 = fopen(filename_L2, "rb");
		if(file_L2 == NULL){
			printf("L2 fpga file open fail!! \n");		
		}

		strcpy(filename_L1, RAMDISK_NAME);
		strcat(filename_L1, FILE_PATH);
		strcat(filename_L1, "fpga_L1.out");
		file_L1 = fopen(filename_L1, "rb");
		if(file_L1 == NULL){
			printf("L1 fpga file open fail!! \n");		
		}

		strcpy(filename_FEP1, RAMDISK_NAME);
		strcat(filename_FEP1, FILE_PATH);
		strcat(filename_FEP1, "fpga_FEP1.out");
		file_FEP1 = fopen(filename_FEP1, "rb");
		if(file_FEP1 == NULL){
			printf("fep1 fpga file open fail!! \n");		
		}

		strcpy(filename_FEP0, RAMDISK_NAME);
		strcat(filename_FEP0, FILE_PATH);
		strcat(filename_FEP0, "fpga_FEP0.out");
		file_FEP0 = fopen(filename_FEP0, "rb");
		if(file_FEP0 == NULL){
			printf("fep0 fpga file open fail!! \n");		
		}
		
		while((!feof(file_L2)) || (!feof(file_L1))  || ( !feof(file_FEP0))  ||  (!feof(file_FEP1)) ){
			
			unsigned char data_L2,data_L1,data_FEP1,data_FEP0;
			unsigned int data;
				
			/****fpga state******/
			CPLD_READ_REG(FPGA_STATE_REG, &temp);		

	#if 0	/**************/
			if(temp & 0xf == 0xf/*BIT4*/){  /***0 -- write enable******/
				printf("Warning: fpga config finish, but file not read finish!!!!!\n");
				break;
			}
	#endif

	#if 1
			/******************/
			if(!(temp & (1<<7)) /***bit 7***/){  /***CONFIG ERROR BIT, 0 for error********/
				break;
			}
	#endif
		
			if(temp & (1<<4)/*BIT4*/){  /***0 -- write enable******/
				printf("write disable, FPGA_STATE_REG value is %x !!\n", temp);
				continue;
			}
			
			/**buffer bits 7-0 is L2fpga, 15-8 is L1fpga, 23-16 is fep0fpga, 31-24 is fep1fpga*****/
			if(!feof(file_L2)){
				count = fread(&data_L2, 1/*size*/, 1, file_L2); 	
			}else{
				data_L2 = 0xff;
			}

			if(!feof(file_L1)){
				count = fread(&data_L1, 1/*size*/, 1, file_L1); 
			}else{
				data_L1 = 0xff;
			}

			if(!feof(file_FEP1)){
				count = fread(&data_FEP1, 1/*size*/, 1, file_FEP1); 
			}else{
				data_FEP1 = 0xff;
			}

			if(!feof(file_FEP0)){
				count = fread(&data_FEP0, 1/*size*/, 1, file_FEP0); 	
			}else{
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
			while(*(unsigned int *)(CPLD_BASE + FPGA_REG) != data){
				printf("write cpld error: fpga write is %x, read is %x, repeat write!!!!",data, *(unsigned int *)(CPLD_BASE + FPGA_REG));
				CPLD_WRITE_REG(FPGA_REG, data);		
			}
					
			/* CPLD_WRITE_REG(FPGA_STATE_REG, (1<<4)); */	/****write state , bit4 ******/

		}/***while*******/
		
		/***config data state******/
		if(feof(file_L2)){		
			CPLD_WRITE_REG(CONFIG_DATASTATE_REG, (0x1<<0));
		}

		fclose(file_L2);
		fclose(file_L1);
		fclose(file_FEP0);
		fclose(file_FEP1);

		/* printf("fpga download finish!!!\n"); */
		/***waiting ***/
		/* i = 32;     while(i--) CPLD_WRITE_REG(FPGA_REG, 0xffffffff);   */
		i= 50000; while(i--);

		CPLD_READ_REG(FPGA_STATE_REG, &temp);		
		if(temp & (0xf | (0x1<<6)) == (0xf | (0x1<<6)) /*(1<<0)*/ /* BIT0*/){  /*********/
			printf("fpga config ok!!!\n");
			return;
		}else{
			printf("fpga config error, FPGA_STATE_REG value is %x !!!\n", temp);					
		}
	
	}  /***while(j--) 5times***/

}


void rebootL2(){
	int i, j,data;
	char filename[40];
	FILE * file;

    *((int  *)(0x14001c10)) = 0;   // set inbound message 0 
    *((int  *)(0x14001c24)) =  0xffffffff;   // clear interrupt
    
	/*reset ,  bit0 reset bts, bit1 reset l2, bit2 reset L2fpga and MCP, bit3 reset L1fpga and aux, bit4 reset Fep0, bit5 reset fep1************/
	CPLD_WRITE_REG(RESET_REG, (0x1<<1) );		
	/* i= 0x50000000; */
	/* while(i)i--; */

	
	printf("wait for L2 message!!!!\n");
	/****message*********/
	while(1){
		data = *((int  *)(0x14001c24));  
		data = LONGSWAP (data);
			
		if(data & (0x1<<0)){
			
			*((int  *)(0x14001c24)) = data & (0x1<<0);   /**clear interruptr**/
			//MV64360_I2O_IB_INT_CAUSE_CPU0
            
			data = *((int  *)(0x14001c10));  //MV64360_I2O_IB_MSG0_CPU0
			data = LONGSWAP (data);	
			
			printf("preloader interrupt occur, message is %x !!!!\n", data);
			if(data == TRAN_READY){
				break;
			}			
		}
	}


	/**download bootrom********/
	strcpy(filename, RAMDISK_NAME);
	strcat(filename, FILE_PATH);
	strcat(filename, "bootrom_L2.bin"); 
	/*strcat(filename, "vxWorks_L2.st"); */
	
	file = fopen(filename, "rb"); /**binary file**/
	i = j = 0;
	do{
		int count;
		/* char buffer[5*1024];*/
		unsigned char buffer;
		
		count = fread(&buffer, 1, 1 /*5*1024*/, file);
		*(unsigned char  *)(PCI_REMAP_MEM1 + L2ENTRYADDR /*0xf2010000*/  + j) = buffer;
        int errorCount=0;
		while(*(unsigned char  *)(PCI_REMAP_MEM1 + L2ENTRYADDR /*0xf2010000*/  + j) != buffer){			
			*(unsigned char  *)(PCI_REMAP_MEM1 + L2ENTRYADDR /*0xf2010000*/  + j) = buffer;  
            errorCount++;
            //if (errorCount==100000)
            {
                errorCount = 0;
                printf("write sdram error %x: write is %x, read is %x, repeat write!!!!",(L2ENTRYADDR + j), buffer, (*(unsigned char  *)(PCI_REMAP_MEM1 + L2ENTRYADDR + j)));
            }
		}
		j = j + 1;  i = i + 1;
		
#if 0		
		for(i = 0; i<count; i++,j++){

			*(char  *)(PCI_REMAP_MEM1 + L2ENTRYADDR /*0xf2010000*/  + j) = buffer[i];  
		}	
#endif
			
	}while(!feof(file));
	fclose(file);
	
	/***bootline *********/
	for(i = 0; i<64; i++){
		*(int  *)(0xf4000000  + BOOT_LINE_ADRS_L2_OFFSET + i*4) = *(int  *)(BOOT_LINE_ADRS_L2  + i*4);    
	}  

#if 1
	     
	while(*((int  *)(0xf4000000)) != TRAN_FINISH){	     
		/*  *((int  *)(0x12001c10)) = TRAN_FINISH;   */  /**send finish message**/
		*((int  *)(0xf4000000)) = TRAN_FINISH;  /** sram , send finish message**/
	}	
#endif


	printf("\nL2 bootrom trans finish!!!!\n");

}

void pre_loader()
{
    	/**copy file from ftp server** *****/
	down_file();

	/**fpga***/
	down_fpga();
    /*reset ,  bit0 reset bts, bit1 reset l2, bit2 reset L2fpga and MCP, bit3 reset L1fpga and aux, bit4 reset Fep0, bit5 reset fep1************/
    CPLD_WRITE_REG(RESET_REG, (0x1<<1) | (0x1<<2) | (0x1<<3) | (0x1<<4 ) | (0x1<<5 ));		

#ifdef BOOT_L2
    rebootL2();
#endif
}





void mem_test(int size){
	unsigned int *addr, *start, *end;
	unsigned int val;
	unsigned int readback;

	unsigned int incr = 1;
	unsigned int pattern = 0;
	
	/* size = ((size-1) & 0x4) + 0x4; */
	start = (unsigned int *)malloc(size);
    if (start == NULL)
    {
        printf("size too big, try with a smaller value\n");
        return;
    }
	end = start + size/4;

	for (;;) { /*************/

		printf ("\rPattern %08lX  Writing... \n", pattern);

		for (addr=start,val=pattern; addr<end; addr++) {
			*addr = val;
			val  += incr;
		}

		printf("Reading...\n");

		for (addr=start,val=pattern; addr<end; addr++) {
			readback = *addr;
			if (readback != val) {
				printf ("\nMem error @ 0x%08X: "
					"found %08lX, expected %08lX\n",
					(unsigned int)addr, readback, val);
			}
			val += incr;
		}

		incr = -incr;
		pattern++;
		pattern &= 0xffff;
		
	}	
}


