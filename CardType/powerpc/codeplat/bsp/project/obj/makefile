#*******************************************************************************
#*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
#********************************************************************************
#* 源文件名:   makefile for root dsp
#* 功能:       根据编译选项编译不同的目标程序  
#* 版本:       v0.1                                                          
#* 编制日期:   201307096                           
#* 作者:       syl                                      
#*******************************************************************************/ 
target = $(BSP_APP_NAME)

OBJS = *.o

all: $(target)

$(target): $(OBJS) 
	$(CC) $(CFLAGS) $^ *.a -o $(APP_OUTPUT_PATH)/$@ 

clean:
	$(RM) $(OBJS) *.bak $(APP_OUTPUT_PATH)/$(BSP_APP_NAME) 

.PHONY: all clean
