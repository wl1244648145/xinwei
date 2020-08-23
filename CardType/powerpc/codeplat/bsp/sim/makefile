#*******************************************************************************
#*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
#********************************************************************************
#* 源文件名:   makefile for root dsp
#* 功能:       根据编译选项编译不同的目标程序  
#* 版本:       v0.1                                                          
#* 编制日期:   201307096                           
#* 作者:       syl                                      
#*******************************************************************************/ 
SRCC = $(shell ls *.c)
OBJS = $(patsubst %.c,%.o,$(SRCC))

all:$(OBJS)
	#@echo 1 $(SRCC)
	#@echo 2 $(OBJS)
	$(shell cp $(OBJS) $(PLATFORMOUTPUTPATH))
	#PLATFORMOUTPUTPATH$(shell rm $(OBJS)) 
clean:
	$(RM) *.o *.bak 

.PHONY: all clean
