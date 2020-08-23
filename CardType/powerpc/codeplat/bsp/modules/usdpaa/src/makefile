#*******************************************************************************
#*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
#********************************************************************************
#* 源文件名:   makefile for root dir
#* 功能:       根据编译选项编译不同的目标程序  
#* 版本:       v0.1                                                          
#* 编制日期:   201307096                           
#* 作者:       syl                                      
#*******************************************************************************/ 
ifeq ($(PRODUCT), BSP)
OUTPUTPATH = $(BSPOUTPUTPATH)
endif

ifeq ($(PRODUCT), PLATFORM)
OUTPUTPATH =  $(PLATFORMOUTPUTPATH)
endif

ifeq ($(PRODUCT), MCWILL)
OUTPUTPATH =  $(MCWILLOUTPUTPATH)
endif

ifeq ($(PRODUCT), LTE)
OUTPUTPATH =  $(LTEOUTPUTPATH)
endif

SRCC = $(shell ls *.c)
OBJS := $(patsubst %.c, %.o,$(SRCC))

all: $(OBJS)
	$(shell cp $(OBJS) $(OUTPUTPATH))


clean:
	$(RM) *.o *.bak 
	
.PHONY: all clean