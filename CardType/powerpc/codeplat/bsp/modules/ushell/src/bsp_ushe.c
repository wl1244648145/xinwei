/********************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
*********************************************************************************
* 源文件名:           bsp_ushell.c 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:        
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------

*********************************************************************************/

/************************** 包含文件声明 **********************************/
/**************************** 共用头文件* **********************************/
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <sys/types.h>
#include "sys/select.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <elf.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/unistd.h>
#include <mqueue.h>

#include "bsp_ushell.h"
#define IPC_MSG_LENTH 512
#define ARG_NUM 10
#define ARG_LENGTH 100
/*********************** 全局变量定义/初始化 **************************/
extern UINT32 BspInitSymbolTable(CHAR *filename);
extern void PrintSymTable();
extern UINT32 symFindByName(CHAR *name, void *pValue, UINT32 *size, UINT32 *pType);
extern char * readline (char * cmd);
extern CHAR *BspStrcpy( CHAR *pcDst,const CHAR *pcSrc );
extern ULONG BspSymFindByName(UCHAR *name, WORDPTR *pValue, ULONG *size, ULONG *pType);
//extern  int add_history(char *); 
extern ULONG symFindByPartName(UCHAR *name);
/************************** 局部常数和类型定义 ************************/
UINT32 BspSymbolInit(CHAR *);
struct mq_attr attr;
CHAR msg[IPC_MSG_LENTH];
UINT32 g_ushell_debug = 0;
UINT32 (*BspRunFUNC)(UINT32, UINT32, UINT32, UINT32, UINT32, UINT32, UINT32, UINT32, UINT32, UINT32);


/*************************** 局部函数原型声明 **************************/
UINT32 BspSymbolInit(CHAR *);
/************************************ 函数实现 ************************* ****/

/**********************************************************************
* 函数名称：void BspFunction(UCHAR * ucpsymbname, UCHAR * ucpboardname)
* 功能描述：xshell的入口函数
* 输入参数：执行程序名称   单板名称
* 输出参数：无
* 返 回 值：
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013-07-10    V1.0    刘刚         创建
************************************************************************/
void cddir(unsigned char *pname)
{
    chdir(pname);
}
CHAR *msg_cd;
void bsp_ushell_init(UCHAR * ucpsymbname, UCHAR * ucpboardname)
{
    UINT32 i,j,k;
    CHAR command_buf[ARG_LENGTH],argstr[ARG_NUM][ARG_LENGTH],argtemp[ARG_LENGTH];
    CHAR historyCMD[ARG_NUM][ARG_LENGTH]={0};
    CHAR *phistoryCMD = NULL;
    UINT32 CmdCnt=0;
    UINT32 argint[ARG_NUM],argflag[ARG_NUM];
    UINT32 value;
    UINT32 SetValue=0;
    static UINT32 ShellFlag=0;
    CHAR * buf;
    UINT32 fd, fd_old; 
    UINT32  symtype,symsize; 
    int iStrRet=0;
    UINT32 rtnflag;
    if(ShellFlag)
    {
        return;
    }
    rtnflag = BspSymbolInit((char *)ucpsymbname);
    if(rtnflag==ERROR)
    {
        printf("BspSymbolInit, return err! can't use shell\n");
        return;
    }
    //UINT32 btsid = bspGetBtsID(0);
    //printf("BBU(0x%x)->",btsid);
    //fflush(stdout);
	
    while(1) 
    {
        memset(msg,0,IPC_MSG_LENTH);
        buf = (char*)readline((char*)"");
        if(buf==NULL)
            continue;
        if(strlen(buf)==0)
        {
            printf("%s->",ucpboardname);
                continue;
        }
        if(strlen(buf)>IPC_MSG_LENTH)
            memcpy(msg,buf,IPC_MSG_LENTH);
		else
	 memcpy(msg,buf,strlen(buf));
        
        printf("%s->",ucpboardname);
        
        fflush(stdout);
 
        memset(command_buf,'\0',ARG_LENGTH);
        memset(argstr[0],'\0',sizeof(argstr));
        for(i=0;i<ARG_NUM;i++)
        {
          argflag[i]=0;
        }
 
        i=0;j=0;k=0;
        for(i = 0;(i<IPC_MSG_LENTH);i++)
        { 
            if(j == 0)
            {
                if((msg[i]!=' ')&&(msg[i]!='\0')&&(msg[i]!='='))
                {    
                    command_buf[k] = msg[i];
                    k++;
                }
                else if(msg[i] =='=')
                {
                    SetValue=1;
                    j++;
                    k = 0; 
                }
                else   
                {          
                    command_buf[k] = '\0';      
                    j++;
                    k = 0;                    
                }
            }
            else
            { 
                if((msg[i]!=',')&&(msg[i]!='\0')&&(msg[i]!='='))
                {    
 
                    if(k<ARG_LENGTH -1) 
                    {
                        argstr[j-1][k] = msg[i];
                        k++;
                    }
                    else
                    {
                        printf(" para %d is too long \n",j);
		                    fflush(stdout);
                        goto USHELL_PARA_ERROR;
                    }
                }
                else if(msg[i] =='=')
                {
                        SetValue=1;
                }
                else   
                {       
                    argstr[j-1][k] = '\0';         
                    j++;
                    k = 0;
                    if(msg[i]=='\0')
                    {
                        break;
                    }
                }
            }
        }
#if 1        
        //将空格去掉argstr[ARG_NUM][ARG_LENGTH]
        int t = 0;
        for(i = 0 ; i< ARG_NUM; i++)
        {
        	if(argstr[i][0]==32)
        	{
        		t = 1;
	        	for(j = 1; j < ARG_LENGTH; j++)
	        	{
	        		if(argstr[i][j]!=32)
	        		{
	        			t = j;
	        			break;
	        	    }
	        	}
	        	for(j= 0;j<(ARG_LENGTH - t );j++)
	        	{
	        		argstr[i][j] = argstr[i][j+t];
	        	}
	        	
            }
        }
        if(SetValue==1)
        {
        	
        	//printf("argstr:%d,%d,%d,%d,%d,%d\n",argstr[0][0],argstr[0][1],argstr[0][2],argstr[0][3],argstr[0][4],t);
        }
#endif         
        for(i = 0; i < 10; i++)
        {
 
            if(((argstr[i][0] == '0')&&(argstr[i][1] == 'x'))
                ||((argstr[i][0] == '0')&&(argstr[i][1] == 'X')))
            {
                sscanf(argstr[i],"0x%x",&argint[i]);
                if(g_ushell_debug == 1)
                {
                    printf("line :%d argstr[%d] :%s argint[%d]:0x%x \n",__LINE__,i,argstr[i],i,argint[i]);
					          fflush(stdout);
                }
            }
            else if(argstr[i][0] == '\"')
            {
                argflag[i]=1;
                //if(ARG_LENGTH>=strlen(&argstr[i][1]))    
                {
                    memcpy(argtemp,&argstr[i][1],strlen(&argstr[i][1]));
                    argtemp[strlen(&argstr[i][1])-1]='\0';
                    //CHAR *BspStrcpy( CHAR *pcDst,const CHAR *pcSrc );
                    BspStrcpy((char *)argstr[i],(char *)argtemp);
                }
                
                if(g_ushell_debug == 1)
                {
                    printf("line2 :%d argstr[%d] :%s  \n",__LINE__,i,argstr[i]);
					          fflush(stdout);
                }
            }
            else
            {
            
            	if((argstr[i][0]>='0')&&(argstr[i][0]<='9'))
            	{
                	argint[i] = atoi(argstr[i]);
                }
#if 1
                else
                {
                	//argint[i] = argstr[i];
                	//在符号表中查找此变量，并且得到此符号的地址和值；
                	//BspSymFindByName((UCHAR*)argstr[i],)
                	if(BspSymFindByName((UCHAR*)argstr[i],(WORDPTR*)&BspRunFUNC, (ULONG*)&symsize,(ULONG*)&symtype) == 0) 
                	{
                		if(STT_OBJECT == symtype )
                        {
				                switch(symsize)
				                {
				                    case 1:
				                    {
					                 
				                      //  printf("4%s = 0x%x value=0x%x=%d\n",argstr,(int)BspRunFUNC,*(char *)BspRunFUNC,*(char *)BspRunFUNC);
				                        argint[i] = *(char *)BspRunFUNC;
				                        break;
				                    }
				                    case 2:
				                    {
					                                       
				                       // printf("5%s = 0x%x value=0x%x=%d\n",argstr,(int)BspRunFUNC,*(short *)BspRunFUNC,*(short *)BspRunFUNC);
				                           argint[i] = *(short *)BspRunFUNC;
				                        break;
				                    }
				                    case 4:
				                    {
					                                        
				                       // printf("6:%s = 0x%x value=0x%x=%d\n",argstr,(int)BspRunFUNC,*(long *)BspRunFUNC,*(long *)BspRunFUNC);
				                           argint[i] = *(long *)BspRunFUNC;
				                        break;
				                    }
				                    default:
				                    {
				                        break;
				                     }
                                 } 
                            }  
		                	else
		                	{
		                		argint[i] = 0;
		                	}
		           
		       }
		    }     
#endif //wangwenhua               //  printf("BspSymFindByName:%d,%d\n",i,argflag[i]);
                if(g_ushell_debug == 1)
                {
                    printf("line :%d argstr[%d] :%s argint[%d]:0x%x \n",__LINE__,i,argstr[i],i,argint[i]);
					          fflush(stdout);
                }
            }
        }
        if((!strcmp("ps",command_buf))||(!strcmp("h",command_buf))||(!strcmp("lkup",command_buf)))
        {
            goto  PROCESS_SHOW;
        }	
        if ( (!strcmp("cmdOpen",command_buf))||(!strcmp("cmdClose",command_buf)) )
        {
            printf("shell is not support this commond!\n");
            fflush(stdout);
            continue;
        }
        if(BspSymFindByName((UCHAR *)command_buf,(WORDPTR *)&BspRunFUNC,(ULONG *)&symsize,(ULONG *)&symtype) == 0) 
        {      
            printf("\r    \r");
            fflush(stdout);	
 
            if(STT_FUNC == symtype )
            {
                value = BspRunFUNC((UINT32)((argflag[0]== 0)?argint[0]:(UINT32)argstr[0]),
                                 (UINT32)((argflag[1]== 0)?argint[1]:(UINT32)argstr[1]),
                                 (UINT32)((argflag[2]== 0)?argint[2]:(UINT32)argstr[2]),
                                 (UINT32)((argflag[3]== 0)?argint[3]:(UINT32)argstr[3]),
                                 (UINT32)((argflag[4]== 0)?argint[4]:(UINT32)argstr[4]),
                                 (UINT32)((argflag[5]== 0)?argint[5]:(UINT32)argstr[5]),
                                 (UINT32)((argflag[6]== 0)?argint[6]:(UINT32)argstr[6]),
                                 (UINT32)((argflag[7]== 0)?argint[7]:(UINT32)argstr[7]),
                                 (UINT32)((argflag[8]== 0)?argint[8]:(UINT32)argstr[8]),
                                 (UINT32)((argflag[9]== 0)?argint[9]:(UINT32)argstr[9]));
                printf("value = %d=0x%x\n",value,value);
				        fflush(stdout);
            }
 
            else if(STT_OBJECT == symtype )
            {
                switch(symsize)
                {
                    case 1:
                    {
	                      if(SetValue)
	                      {
                            *(char *)BspRunFUNC = (char)argint[0];
                        }
                        printf("%s = 0x%x value=0x%x=%d\n",command_buf,(int)BspRunFUNC,*(char *)BspRunFUNC,*(char *)BspRunFUNC);
                        break;
                    }
                    case 2:
                    {
	                      if(SetValue)
	                      {
                            *(short *)BspRunFUNC = (short)argint[0];
	                      }                        
                        printf("%s = 0x%x value=0x%x=%d\n",command_buf,(int)BspRunFUNC,*(short *)BspRunFUNC,*(short *)BspRunFUNC);
                        break;
                    }
                    case 4:
                    {
	                      if(SetValue)
	                      {
                            *(long *)BspRunFUNC = (long)argint[0];
	                      }                        
						printf("%s = 0x%x value=0x%x=%d\n",command_buf,(int)BspRunFUNC,*(unsigned int *)BspRunFUNC,(int)*(long *)BspRunFUNC);
                        break;
                    }
                   default:
                    {
                        int i;
                        char * ucptemp; 
                        ucptemp = (char *)BspRunFUNC;
						printf("%s= addr:0x%x\n",command_buf,(unsigned int)BspRunFUNC);
                        for(i=0;i<symsize;i++)
                        {
                            if((i%4==0)&&(i>0))
                            {
                                printf("  ");
                                fflush(stdout);
                            }
                            if((i%16==0)&&(i>0))
                            {
                                printf("\n");
                            }
                            printf("%02x",*(ucptemp+i));
				                    fflush(stdout);
                        }
                        printf("\n");
                    }                    
                }
            }
            else
            {
               printf("Parameter:%d error \n",symtype);
            }
			
            printf("%s->",ucpboardname);
            fflush(stdout);
            SetValue = 0; 
            add_history((char *)msg); 
        }
        else 
        {
            PROCESS_SHOW:            
            iStrRet=strcmp("",command_buf);
            if(0 == iStrRet)
            {
                fflush(stdout);
            } 
            else if(strcmp("h",command_buf)&&strcmp("lkup",command_buf)&&strcmp("reboot",command_buf))
            {
                printf("\r    \r");
                fflush(stdout);	
                system(buf);
                printf("%s->",ucpboardname);
                fflush(stdout);
                add_history(msg);  
                fflush(stdout);            
            }    
            iStrRet=strcmp("h",command_buf);/*显示历史操作命令*/
	          if(0 == iStrRet)
            {
                printf("\r    \r");
                fflush(stdout);	
		            for(i=0;i<ARG_NUM;i++)
                printf("%d:    %s\n",i,historyCMD[(CmdCnt+i)%ARG_NUM]);
                printf("%s->",ucpboardname);
                fflush(stdout);
                add_history((char *)msg); 
            }
            
            if ( !strcmp("reboot",command_buf) )
            {
                system("reboot");
                break;
            }
            
            iStrRet=strcmp("lkup",command_buf);/*显示历史操作命令*/
            if(0 == iStrRet)
            {
                printf("\r    \r");
                fflush(stdout);	
                symFindByPartName(argstr[0]);
                printf("%s->",ucpboardname);
                fflush(stdout);
                add_history((char *)msg); 
            }
            SetValue = 0; 
        }
        phistoryCMD = historyCMD[CmdCnt];
        memcpy(phistoryCMD,msg,ARG_LENGTH);
        CmdCnt = (CmdCnt +1)%ARG_NUM;
	USHELL_PARA_ERROR:
       	continue;
    }
}

/********************************************************************************
* 函数名称: BspSymbolInit							
* 功    能:         xshell的入口函数                            
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
UINT32 BspSymbolInit(CHAR * filename)
{
    return BspInitSymbolTable(filename);
}

