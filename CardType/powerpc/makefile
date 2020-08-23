#*******************************************************************************
#*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
#********************************************************************************
#* 源文件名:   makefile for root dir
#* 功能:       根据编译选项编译不同的目标程序  
#* 版本:       v0.1                                                          
#* 编制日期:   201307096                           
#* 作者:       syl                                      
#*******************************************************************************/ 
BOARD = BBU20M
PRODUCT = BSP
CROSS_COMPILE = /opt/fsl/1.2/sysroots/i686-fslsdk-linux/usr/bin/ppce500mc-fsl-linux/powerpc-fsl-linux-

CC  = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++  
AR  = $(CROSS_COMPILE)ar
LD  = $(CROSS_COMPILE)ld 
ARFLAGS = cr
RM = -rm -rf

CODEBTSPATH =$(shell pwd)

PLATFORMOUTPUTPATH = $(CODEBTSPATH)/codeplatform/project/obj
BSPOUTPUTPATH = $(CODEBTSPATH)/codeplatform/bsp/project/obj
MCWILLOUTPUTPATH = $(CODEBTSPATH)/codemcwill/project/obj
LTEOUTPUTPATH = $(CODEBTSPATH)/codelte/project/obj
APP_OUTPUT_PATH = $(CODEBTSPATH)/output

BSP_APP_NAME = bsp_app
PLATFORM_APP_NAME = pfm_app
MCWILL_APP_NAME = mcw_app
LTE_APP_NAME = lte_app

ifeq ($(BOARD), BBU20M)
CPU_TYPE = FSL_P2041
endif

CFLAGS = -Wall 

ifeq ($(DEBUG), y)
CFLAGS += -g 
else
CFLAGS += 
endif

CFLAGS += -D__CPU_LTE_CARDTYPE__ -lpthread -lm -lrt

ifeq ($(PRODUCT), BSP)
SUBDIRS = codeplatform
CFLAGS += -I$(CODEBTSPATH)/codeplatform/bsp/com_inc -I./../inc  -DBSP_DEBUG
endif

ifeq ($(PRODUCT), PLATFORM)
SUBDIRS = codeplatform
CFLAGS += -I$(CODEBTSPATH)/codeplatform/bsp/com_inc -I./../inc 
endif

ifeq ($(PRODUCT), MCWILL)
SUBDIRS = codeplatform codemcwill
CFLAGS += -I$(CODEBTSPATH)/codeplatform/bsp/com_inc -I./../inc 
CPPFLAGS = -lpthread -D M_TGT_L3 -D WBBU_CODE -D DIAG_PRINTFS -D NDEBUG 
CPPFLAGS += -I$(CODEBTSPATH)/bsp/com_inc
CPPFLAGS += -I$(CODEBTSPATH)/codemcwill/modules/prj_common/include 
CPPFLAGS += -I$(CODEBTSPATH)/codemcwill/modules/prj_common/APFramework/H
CPPFLAGS += -I$(CODEBTSPATH)/codemcwill/modules/prj_common/APFramework/SRC 
endif

ifeq ($(PRODUCT), LTE)
SUBDIRS = codeplatform codelte
CFLAGS += -I$(CODEBTSPATH)/bsp/com_inc
CFLAGS += -I$(CODEBTSPATH)/codelte/com_inc
endif


export CC CPP CFLAGS CPPFLAGS AR ARFLAGS LD RM 
export BOARD PRODUCT TARGET CPU_TYPE BSP_APP_NAME MCWILL_APP_NAME LTE_APP_NAME  PLATFORM_APP_NAME
export BSPOUTPUTPATH PLATFORMOUTPUTPATH MCWILLOUTPUTPATH CODEBTSPATH  APP_OUTPUT_PATH

all: 
	for dir in $(SUBDIRS); \
	do $(MAKE) -C $$dir all || exit 1; \
	done


clean:

	@for dir in $(SUBDIRS); do make -C $$dir clean|| exit 1; done

.PHONY: all clean
