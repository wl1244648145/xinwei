#*******************************************************************************
#*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
#********************************************************************************
#* 源文件名:   makefile for root dsp
#* 功能:       根据编译选项编译不同的目标程序  
#* 版本:       v0.1                                                          
#* 编制日期:   201307096                           
#* 作者:       syl                                      
#*******************************************************************************/ 
target = $(MCWILL_APP_NAME)

all: $(target)
$(target): *.o
	$(CPP) $(CPPFLAGS) $^ -o $(APP_OUTPUT_PATH)/$@  *.a
		
		
clean:
	$(RM) *.o *.bak $(APP_OUTPUT_PATH)/$(MCWILL_APP_NAME) 

.PHONY: all clean
