/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_ushell_elf.c 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
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

/**************************** 私用头文件* **********************************/
#include "bsp_ushell.h"
/******************************* 局部宏定义 *********************************/

#define S_IRUSR 00400

static ULONG symGetPosByValue(WORDPTR value);
#define B2L(a, size) toLE((UCHAR*)&a, size)
#define B2L2(a) (B2L(a, 2))
#define B2L4(a) (B2L(a, 4))
#define MAX_SEC_NUM     100
#define MAX_SEC_NAME_LEN    30

/*********************** 全局变量定义/初始化 **************************/
typedef struct 
{
    UCHAR *name;
    UCHAR *addr;
    ULONG size;
    ULONG  type;
}SYMBOL_ENT;

static SYMBOL_ENT* taSymTbl = NULL;
static ULONG ulSymTblSize;
static ULONG g_iEndianType;
static UCHAR s_aSecNames[MAX_SEC_NUM][MAX_SEC_NAME_LEN] = {{0}};
static BOOLEAN bSymTabInitFlag = FALSE;
extern char * readline (char * cmd);

/************************** 局部常数和类型定义 ************************/


/*************************** 局部函数原型声明 **************************/
ULONG  BspSymFindByName(UCHAR *name, WORDPTR *pValue, ULONG *size, ULONG *pType);
static ULONG  BspOpenCurrentELF(UCHAR *filename);
static UINT32 BspVerifyELFHeader(UINT32 fd, Elf32_Ehdr *pHdr);
static UINT32 BspBuildSymTable(Elf32_Ehdr  *elfHdr, UINT32 fd);
static BOOLEAN isOurConcernedSection (ULONG section_index);
static ULONG BspSortSymbolTable(SYMBOL_ENT *buff, ULONG symbolnum);
static ULONG toLE(UCHAR *p, ULONG size);

/************************************ 函数实现 ************************* ****/
/**********************************************************************
* 函数名称：BspStrcpy
* 功能描述：
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013-07-10    V1.0    刘刚         创建
************************************************************************/
CHAR *BspStrcpy( CHAR *pcDst,const CHAR *pcSrc )
{
    CHAR   *pcResult = pcDst;
    if ( ( pcDst == NULL ) || ( pcSrc == NULL ) )
    {
        return pcResult;
    }
    while ( '\0' != ( *pcDst++ = *pcSrc++ ) );
    return pcResult;
}

/**********************************************************************
* 函数名称：BspStrcpy
* 功能描述：
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013-07-10    V1.0    刘刚         创建
************************************************************************/
ULONG BspInitSymbolTable(UCHAR *filename)
{
    if(taSymTbl == NULL)
    {
        return BspOpenCurrentELF(filename);
    }
    else
    {
        return ERROR;
    }
}

/**********************************************************************
* 函数名称：BspStrcpy
* 功能描述：
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013-07-10    V1.0    刘刚         创建
************************************************************************/
static ULONG BspOpenCurrentELF(UCHAR *filename)
{
	ULONG fd;
	Elf32_Ehdr elfHdr;
	
	fd = open((char *)filename, S_IRUSR);
	if (fd < 0)
	{
		printf("\nBspOpenCurrentELF open file error: %s\n",filename);
		return ERROR;
	}
	
	if (OK != BspVerifyELFHeader (fd, &elfHdr))
	{
		printf("error at line %d\n", __LINE__);
		close(fd);
		return ERROR;
	}
	
	B2L2 (elfHdr.e_type);
	B2L2 (elfHdr.e_machine);
	B2L4 (elfHdr.e_shoff);
	B2L2 (elfHdr.e_shentsize);
	B2L2 (elfHdr.e_shnum); 
	B2L2 (elfHdr.e_shstrndx);
	
	if (OK != BspBuildSymTable(&elfHdr, fd))
	{
		close(fd);
		return ERROR;
	}    
	
	close(fd);
	
	return OK;
	
}


/**********************************************************************
* 函数名称：UINT32 BspBuildSymTable(Elf32_Ehdr *elfHdr, UINT32 fd)
* 功能描述：构造符号表
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013-07-10    V1.0    刘刚         创建
************************************************************************/
UINT32 BspBuildSymTable(Elf32_Ehdr *elfHdr, UINT32 fd)
{
    ULONG i;
    LONG ret;
    ULONG iSymStrTabOffset = 0;  
    ULONG iSymStrTabSize = 0;  
    ULONG iSymTabOffset = 0;  
    ULONG iSymTabSize = 0;  
    ULONG iSymTabEntSize = 0; 
    Elf32_Shdr secHdr; 
    Elf32_Shdr shSecNameStrTab;  
    UCHAR *pSecNameStrTab = NULL; 
    UCHAR *pSymStrTab = NULL; 
    UCHAR *pSymTab = NULL; 
	
    ret=lseek(fd, elfHdr->e_shoff + sizeof(Elf32_Shdr)*(elfHdr->e_shstrndx),SEEK_SET);
    if(-1==ret)
    {
        printf("BspBuildSymTable lseek failed!errno:%d\n",ERROR);
	      return ERROR;
    }
	
    ret=read(fd, &shSecNameStrTab, sizeof(Elf32_Shdr));
    if ( -1 == ret||0 == ret)
    {
        printf("BspBuildSymTable read failed!errno:%d\n",ERROR);
        return ERROR;
    }    
	
    B2L4 (shSecNameStrTab.sh_name);
    B2L4 (shSecNameStrTab.sh_type);
    B2L4 (shSecNameStrTab.sh_offset);
    B2L4 (shSecNameStrTab.sh_addr);
    B2L4 (shSecNameStrTab.sh_addralign);
    B2L4 (shSecNameStrTab.sh_size);
    B2L4 (shSecNameStrTab.sh_entsize);
	
    pSecNameStrTab = (UCHAR*)malloc(shSecNameStrTab.sh_size);
    if (NULL == pSecNameStrTab)
    {
        return ERROR;
    }
	
    ret=lseek(fd, shSecNameStrTab.sh_offset, SEEK_SET);
    if(-1==ret)
    {
        printf("BspBuildSymTable lseek failed!errno:%d\n",ERROR);
	      return ERROR;
    }
	
    ret=read(fd, pSecNameStrTab, shSecNameStrTab.sh_size);
	  if ( -1 == ret||0 == ret)
    {
        printf("BspBuildSymTable read failed!errno:%d\n",ERROR);
        return ERROR;
    } 
	  
    ret=lseek(fd, elfHdr->e_shoff, SEEK_SET);
    if(-1==ret)
    {
        printf("BspBuildSymTable lseek failed!errno:%d\n",ERROR);
	      return ERROR;
    }
	
    for (i = 0; i < elfHdr->e_shnum; i++)
    {
        ret=read(fd, &secHdr, sizeof(secHdr));
	    if ( -1 == ret||0 == ret)
        {
            printf("BspBuildSymTable read failed!\n");
            return ERROR;
        } 
        B2L4 (secHdr.sh_name);
        B2L4 (secHdr.sh_type);
        B2L4 (secHdr.sh_offset);
        B2L4 (secHdr.sh_addr);
        B2L4 (secHdr.sh_addralign);
        B2L4 (secHdr.sh_size);
        B2L4 (secHdr.sh_entsize);   
		strncpy ((char *)&s_aSecNames[i][0], (char *)&pSecNameStrTab[secHdr.sh_name],MAX_SEC_NAME_LEN);
		
		if ((SHT_STRTAB == secHdr.sh_type) && (0 == strncmp(".strtab", (char *)&pSecNameStrTab[secHdr.sh_name], 7)))
        {
            iSymStrTabOffset = secHdr.sh_offset; 
            iSymStrTabSize = secHdr.sh_size; 
            continue;
        }
		if ((SHT_SYMTAB == secHdr.sh_type) && (0 == strncmp(".symtab", (char *)&pSecNameStrTab[secHdr.sh_name], 7)))
        {
            iSymTabOffset = secHdr.sh_offset; 
            iSymTabSize = secHdr.sh_size;  
            iSymTabEntSize = secHdr.sh_entsize;  
            continue;
        }
    }
	
    pSymStrTab = (UCHAR*)malloc(iSymStrTabSize);
    if (NULL == pSymStrTab)
    {
        free((UCHAR *)pSecNameStrTab);
        return ERROR;
    }
	
    ret=lseek(fd, iSymStrTabOffset, SEEK_SET);
    if(-1==ret)
    {
        printf("BspBuildSymTable lseek failed!errno:%d\n",ERROR);
	      return ERROR;
    }
	
    ret=read(fd, pSymStrTab, iSymStrTabSize);
	  if ( -1 == ret||0 == ret)
    {
        printf("BspBuildSymTable read failed!errno:%d\n",ERROR);
        return ERROR;
    }
    pSymTab = (UCHAR*)malloc(iSymTabSize);
    if (NULL == pSymTab)
    {
        free((UCHAR *)pSecNameStrTab);
        free((UCHAR *)pSymStrTab);
        return ERROR;
    }
	
    ret=lseek(fd, iSymTabOffset, SEEK_SET);
    if(-1==ret)
    {
        printf("BspBuildSymTable lseek failed!errno:%d\n",ERROR);
	      return ERROR;
    }
	
    ret=read(fd, pSymTab, iSymTabSize);
	  if ( -1 == ret||0 == ret)
    {
        printf("BspBuildSymTable read failed!errno:%d\n",ERROR);
        return ERROR;
    }
	  
    ULONG iSymTabEntryNum = iSymTabSize / iSymTabEntSize;   
    Elf32_Sym *pSymEnt = (Elf32_Sym*)pSymTab;             
    ulSymTblSize = 0;  
	
    for (i = 0; i < iSymTabEntryNum; i++)
    {
        if  ((STT_FUNC == ELF32_ST_TYPE(pSymEnt->st_info) || (STT_OBJECT == ELF32_ST_TYPE(pSymEnt->st_info))) &&
            isOurConcernedSection(pSymEnt->st_shndx) && 0 != pSymEnt->st_shndx)
        {
            ulSymTblSize++;
        }
        pSymEnt++;
    }
	
    taSymTbl = (SYMBOL_ENT*)malloc(ulSymTblSize *sizeof(SYMBOL_ENT));
    if (NULL == taSymTbl)
    {
        free((UCHAR *)pSecNameStrTab);
        free((UCHAR *)pSymStrTab);
        free((UCHAR *)pSymTab);
        return ERROR;
    }
	
    SYMBOL_ENT *pCurMemForSym = taSymTbl; 
    pSymEnt = (Elf32_Sym*)pSymTab; 
    for (i = 0; i < iSymTabEntryNum; i++)
    {
		if  ((STT_FUNC == ELF32_ST_TYPE(pSymEnt->st_info) || (STT_OBJECT == ELF32_ST_TYPE(pSymEnt->st_info))) 
			&&isOurConcernedSection(pSymEnt->st_shndx))

        {
            pCurMemForSym->name = &pSymStrTab[pSymEnt->st_name];
            pCurMemForSym->addr = (UCHAR*)pSymEnt->st_value;
            pCurMemForSym->type = ELF32_ST_TYPE(pSymEnt->st_info);
            pCurMemForSym->size = pSymEnt->st_size;
            pCurMemForSym++;
        }
        pSymEnt++;
    } 
	
    BspSortSymbolTable(taSymTbl, ulSymTblSize);
    if(pSecNameStrTab)
    {
        free((UCHAR *)pSecNameStrTab);  
    }
	
    if(pSymTab)
    {
        free((UCHAR *)pSymTab);       
    }    
	
    bSymTabInitFlag = TRUE;
    return OK;
	
}

/**********************************************************************
* 函数名称：BspSymFindByName
* 功能描述：
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013-07-10    V1.0    刘刚         创建
************************************************************************/

ULONG symFindByValue(WORDPTR value, UCHAR *name, WORDPTR *pValue, ULONG *pType)
{
    ULONG wPosition;
    if (name == NULL || taSymTbl == NULL)
    {
        return  ERROR;
    }
	
    wPosition = symGetPosByValue(value);
    if (ERROR != wPosition)
    {
        BspStrcpy((CHAR *)name,(const CHAR *)taSymTbl[wPosition].name);
        if (pValue)
        {
            *pValue = (WORDPTR)(taSymTbl[wPosition].addr);
        }
		
        if (pType)
        {
            *pType = (ULONG)(taSymTbl[wPosition].type);
        }
		
        return OK;
    }
	
    return ERROR;
	
}

/**********************************************************************
* 函数名称：BspSymFindByName
* 功能描述：
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013-07-10    V1.0    刘刚         创建
************************************************************************/
ULONG BspSymFindByName(UCHAR *name, WORDPTR *pValue, ULONG *size, ULONG *pType)
{
    ULONG num;
    ULONG i;
    num = ulSymTblSize;
	
    if(taSymTbl == NULL)
    {
        return  ERROR;
    }
	
    for (i = 0; i < num; i++)
    {
        if (strcmp((const CHAR *)name,(const CHAR *)taSymTbl[i].name) == 0)
        {
            break;
        }
    }
	
    if (i >= num)
    {
        return  ERROR;
    }
	
    if (pValue)
    {
        *pValue = (WORDPTR)(taSymTbl[i].addr);
    }
	
    if (pType)
    {
        *pType = (ULONG)(taSymTbl[i].type);
    }
	
    if (size)
    {
        *size = (ULONG)(taSymTbl[i].size);
    }
	
    return OK;
}
/**********************************************************************
* 函数名称：BspSymFindByName
* 功能描述：
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013-07-10    V1.0    刘刚         创建
************************************************************************/
ULONG symFindByPartName(UCHAR *name)
{
    ULONG num;
    ULONG i,j;
    ULONG cnt=0;
    UCHAR *FuncName[10]={NULL};
    UCHAR *buf;
    UCHAR ucFuncNameLen = 0;
    UCHAR ucIsFind = 0;
    UCHAR TableName[100]={0};
    num = ulSymTblSize;
	
    if(taSymTbl == NULL)
    {
        return  ERROR;
    }
	
    printf("name size is %d\n",strlen((const CHAR *)name));
    for (i = 0; i < num; i++)
    {
        ucFuncNameLen = strlen((const char *)taSymTbl[i].name);
        BspStrcpy((CHAR *)TableName,(CHAR *)taSymTbl[i].name);
		
        for(j=0; j<ucFuncNameLen; j++)
        {
            /*对比taSymTbl[i].name中是否存在name的字段*/
            if (strncmp((const CHAR *)name, (const CHAR *)&(TableName[j]),strlen((const CHAR *)name)) == 0)
            {
                ucIsFind = 1;
                break;
            }
        }
		
        /*存在name字段则打印结果*/
        if (ucIsFind)
        {
            ucIsFind = 0;
            FuncName[cnt]=taSymTbl[i].name;
            cnt ++ ;
            if(cnt >= 10)
            {
                cnt=0;
				
                /*每次打印输出10个函数*/
                for(j=0;j<10;j++)
                {
                    printf("%s\n",FuncName[j]);
                }
                printf("Press \"q\"or\"Q\" Exit Function Print:");
                fflush(stdout);
                //buf = (UCHAR *)readline("");
                if((0 == strcmp("q",(const CHAR *)buf))||(0 == strcmp("Q",(const CHAR *)buf)))
                {
                    break;
                }
            }
        }
    }
    return OK;
}

/**********************************************************************
* 函数名称：toLE
* 功能描述：
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013-07-10    V1.0    刘刚         创建
************************************************************************/
static ULONG toLE(UCHAR *p, ULONG size)
{
    ULONG i;
    WORD16 s;
    if (g_iEndianType == ELFDATA2MSB)
    {
        if (4 == size)
        {
            i = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
            *(ULONG*)p = i;
        }
        else if (2 == size)
        {
            s = (p[0] << 8) + p[1];
            *(WORD16*)p = s;
        }
    }
    return 0;
}

/**********************************************************************
* 函数名称：BspVerifyELFHeader
* 功能描述：
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013-07-10    V1.0    刘刚         创建
************************************************************************/
static UINT32 BspVerifyELFHeader(UINT32 fd, Elf32_Ehdr *pHdr)
{
    ULONG iReadRv;
    ULONG iHdrSize;
    iHdrSize = sizeof(*pHdr);
	
    iReadRv = read(fd, (UCHAR*)pHdr, iHdrSize);
    if (iReadRv != iHdrSize)
    {
        printf("iReadRv = %d\n",(int)iReadRv);
        printf("Erroneous header read\n");
        return (ERROR);
    }
	
    if (strncmp((const CHAR *)pHdr->e_ident,(const CHAR *)ELFMAG, SELFMAG) != 0)
    {
        return (ERROR);
    }
	
    if (ELFDATA2MSB == pHdr->e_ident[EI_DATA])
    {
        g_iEndianType = ELFDATA2MSB;
    }
    else if (ELFDATA2LSB == pHdr->e_ident[EI_DATA])
    {
        g_iEndianType = ELFDATA2LSB;
    }
    else
    {
        printf("error at line %d\n", __LINE__);
        printf("unknown enidan type\n");
        return (ERROR);
    }
    return OK;
}

/**********************************************************************
* 函数名称：isOurConcernedSection
* 功能描述：
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013-07-10    V1.0    刘刚         创建
************************************************************************/
static BOOLEAN isOurConcernedSection (ULONG section_index)
{
    if(section_index >= MAX_SEC_NUM || section_index < 0)
    {
        return FALSE;
    }
	
    if(strcmp ((char *)s_aSecNames[section_index],".text") == 0 
	    ||strcmp ((char *)s_aSecNames[section_index],".data") == 0
	    ||strcmp((char *)s_aSecNames[section_index],".bss") == 0 
	    ||strcmp((char *)s_aSecNames[section_index],".sbss") == 0 
	    ||strcmp((char *)s_aSecNames[section_index],".sdata") == 0 
	    ||strcmp((char *)s_aSecNames[section_index],".tbss") == 0 
	    ||strcmp((char *)s_aSecNames[section_index],".tdata") == 0)
    {
        return TRUE;
    }
	
    return FALSE;
}

/**********************************************************************
* 函数名称：BspPartition
* 功能描述：
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013-07-10    V1.0    刘刚         创建
************************************************************************/
static ULONG BspPartition(SYMBOL_ENT arr[],ULONG low,ULONG high)
{
    SYMBOL_ENT pivot = arr[low];
	
    while (low < high)
    {
        while ((low<high) && (arr[high].addr >= pivot.addr))
        {
            high--;
        }
        arr[low] = arr[high];
        while ((low<high) && (arr[low].addr <= pivot.addr))
        {
            low++;
        }
        arr[high] = arr[low];
    }
	
    arr[low] = pivot;
    return low;
	
}

/**********************************************************************
* 函数名称：BspQuickSort
* 功能描述：
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013-07-10    V1.0    刘刚         创建
************************************************************************/
static VOID BspQuickSort(SYMBOL_ENT arr[],ULONG low,ULONG high)
{
    ULONG pivotindex;
	
    if (low < high)
    {
        pivotindex = BspPartition(arr,low,high);
        BspQuickSort(arr,low,pivotindex-1);
        BspQuickSort(arr,pivotindex+1,high);
    }
}

/**********************************************************************
* 函数名称：BspSortSymbolTable
* 功能描述：
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013-07-10    V1.0    刘刚         创建
************************************************************************/
static ULONG BspSortSymbolTable(SYMBOL_ENT *buff, ULONG symbolnum)
{
	BspQuickSort(buff,0,symbolnum - 1);
	return OK;
}

/**********************************************************************
* 函数名称：symGetPosByValue
* 功能描述：
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013-07-10    V1.0    刘刚         创建
************************************************************************/
static ULONG symGetPosByValue(WORDPTR value)
{
    ULONG length;
    WORDPTR addr;
    ULONG low;
    ULONG high;
    ULONG mid;
	
    length = ulSymTblSize;
    low = 1;
    high = length;
	
    while (low <= high)
    {
        mid = (low + high) / 2;
        addr = (WORDPTR)taSymTbl[mid].addr;
        if ((value >= addr) && (value < (addr + taSymTbl[mid].size)))
        {
            return mid;
        }
        else if (value < addr)
        {
            high = mid - 1;
        }
        else
        {
            low = mid + 1;
        }
    }
    return ERROR;
}

