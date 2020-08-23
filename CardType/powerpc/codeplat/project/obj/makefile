#*******************************************************************************
#*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
#********************************************************************************
#* 源文件名:   makefile for root dsp
#* 功能:       根据编译选项编译不同的目标程序  
#* 版本:       v0.1                                                          
#* 编制日期:   201307096                           
#* 作者:       syl                                      
#*******************************************************************************/ 
target = $(PLATFORM_APP_NAME)
all: $(target)

$(target): *.o
	$(CC) $(CFLAGS) $^ -o $(APP_OUTPUT_PATH)/$@ *.a

clean:
	$(RM) *.o *.bak $(APP_OUTPUT_PATH)/$(PLATFORM_APP_NAME) 

.PHONY: all clean
