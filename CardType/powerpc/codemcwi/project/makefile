#*******************************************************************************
#*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
#********************************************************************************
#* 源文件名:   makefile for root dsp
#* 功能:       根据编译选项编译不同的目标程序  
#* 版本:       v0.1                                                          
#* 编制日期:   201307096                           
#* 作者:       syl                                      
#*******************************************************************************/ 
DIR_TEMP = $(shell ls -d */)
SUBDIRS_TEMP = $(subst /, ,$(DIR_TEMP))
PARTERNAL = %inc
SUBDIRS = $(filter-out $(PARTERNAL),$(SUBDIRS_TEMP))

all: 

	for dir in $(OBJPATH); \
	do $(MAKE) -C $$dir all || exit 1; \
	done
	$(shell cp $(CODEBTSPATH)/codemcwill/modules/prj_common/APFramework/SRC/DemoTask2.o $(MCWILLOUTPUTPATH))
  
	
clean:

	@for dir in $(SUBDIRS) $(OBJPATH); do make -C $$dir clean|| exit 1; done

	$(RM) *.bak 

.PHONY: all clean
