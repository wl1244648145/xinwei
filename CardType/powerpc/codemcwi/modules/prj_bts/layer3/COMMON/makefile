#*******************************************************************************
#*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
#********************************************************************************
#* 源文件名:   makefile for root dir
#* 功能:       根据编译选项编译不同的目标程序  
#* 版本:       v0.1                                                          
#* 编制日期:   201307096                           
#* 作者:       syl                                      
#*******************************************************************************/ 
SRCC = $(shell ls *.c)
OBJS := $(patsubst %.c, %.o,$(SRCC))

SRCPP = $(shell ls *.cpp)
CPPOBJS := $(patsubst %.cpp, %.o,$(SRCPP))

all: $(OBJS) $(CPPOBJS)

$(CPPOBJS): %.o:%.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $^
	
	$(shell cp *.o $(MCWILLOUTPUTPATH))
	

clean:
	$(RM) *.o *.bak 
	
.PHONY: all clean