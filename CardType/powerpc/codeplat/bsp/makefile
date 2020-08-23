#*******************************************************************************
#*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
#********************************************************************************
#* 源文件名:   makefile for root dsp
#* 功能:       根据编译选项编译不同的目标程序  
#* 版本:       v0.1                                                          
#* 编制日期:   201307096                           
#* 作者:       syl                                      
#*******************************************************************************/ 
ifeq ($(SIM_DSP), y)
SUBDIRS = sim
else
SUBDIRS = board modules
endif

ifeq ($(PRODUCT), BSP)
OBJPATH = project/obj
endif

all: 
	for dir in $(SUBDIRS); \
	do $(MAKE) -C $$dir all || exit 1; \
	done
	
	for dir in $(OBJPATH); \
	do $(MAKE) -C $$dir all || exit 1; \
	done
	
clean:

	@for dir in $(SUBDIRS) $(OBJPATH); do make -C $$dir clean|| exit 1; done

	$(RM) *.bak 

.PHONY: all clean
