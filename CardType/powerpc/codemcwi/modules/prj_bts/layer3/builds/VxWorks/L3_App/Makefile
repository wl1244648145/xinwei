# Makefile generated by the project manager
#


# GENERATED: Tue Jan 29 13:46:57 中国标准时间 2008
# DO NOT EDIT - file is regenerated whenever the project changes


## core information

ifeq ($(wildcard Makefile),)
PRJ_DIR        = ..
else
PRJ_DIR        = .
endif
PRJ_FILE       = L3_App.wpj
PRJ_TYPE       = vxWorks
PRJ_OBJS       = sysALib.o mv64360Sio.o mv643xxEnd.o sysLib.o usrAppInit.o prjConfig.o linkSyms.o PciEnd.o spyLib.o VlanEnd.o
BOOT_OBJS      = romInit.o romStart.o $(BOOT_EXTRA)
BUILD_SPEC     = default
BSP_DIR        = $(WIND_BASE)/target/config/McWill_L3
TGT_DIR        = $(WIND_BASE)/target



## set searching directories for dependencies

vpath %.c $(BSP_DIR)
vpath %.cpp $(BSP_DIR)
vpath %.cxx $(BSP_DIR)


## build-configuration info

ifeq ($(BUILD_SPEC),default)
CPU            = PPC604
TOOL           = gnu
TOOL_FAMILY    = gnu
DEFAULT_RULE   = vxWorks
endif

ifeq ($(BUILD_SPEC),default_rom)
CPU            = PPC604
TOOL           = gnu
TOOL_FAMILY    = gnu
DEFAULT_RULE   = vxWorks_rom
endif

ifeq ($(BUILD_SPEC),default_romCompress)
CPU            = PPC604
TOOL           = gnu
TOOL_FAMILY    = gnu
DEFAULT_RULE   = vxWorks_romCompress
endif

ifeq ($(BUILD_SPEC),default_romResident)
CPU            = PPC604
TOOL           = gnu
TOOL_FAMILY    = gnu
DEFAULT_RULE   = vxWorks_romResident
endif


## component-configuration info

COMPONENTS = INCLUDE_ANSI_ASSERT \
	INCLUDE_ANSI_CTYPE \
	INCLUDE_ANSI_LOCALE \
	INCLUDE_ANSI_MATH \
	INCLUDE_ANSI_STDIO \
	INCLUDE_ANSI_STDIO_EXTRA \
	INCLUDE_ANSI_STDLIB \
	INCLUDE_ANSI_STRING \
	INCLUDE_ANSI_TIME \
	INCLUDE_ARP_API \
	INCLUDE_ATA \
	INCLUDE_ATA_SHOW \
	INCLUDE_AUX_CLK \
	INCLUDE_BOOT_LINE_INIT \
	INCLUDE_BSD_SOCKET \
	INCLUDE_BUF_MGR \
	INCLUDE_CACHE_ENABLE \
	INCLUDE_CACHE_SUPPORT \
	INCLUDE_CBIO \
	INCLUDE_CPLUS \
	INCLUDE_CPLUS_DEMANGLER \
	INCLUDE_CPLUS_IOSTREAMS \
	INCLUDE_CPLUS_IOSTREAMS_FULL \
	INCLUDE_CPLUS_LANG \
	INCLUDE_CPLUS_STL \
	INCLUDE_CPLUS_STRING \
	INCLUDE_CPLUS_STRING_IO \
	INCLUDE_CTORS_DTORS \
	INCLUDE_DEBUG \
	INCLUDE_DHCPC_LEASE_CLEAN \
	INCLUDE_DISK_CACHE \
	INCLUDE_DISK_PART \
	INCLUDE_DISK_UTIL \
	INCLUDE_DLL \
	INCLUDE_DOSFS \
	INCLUDE_DOSFS_CHKDSK \
	INCLUDE_DOSFS_DIR_FIXED \
	INCLUDE_DOSFS_DIR_VFAT \
	INCLUDE_DOSFS_FAT \
	INCLUDE_DOSFS_FMT \
	INCLUDE_DOSFS_MAIN \
	INCLUDE_END \
	INCLUDE_END_BOOT \
	INCLUDE_ENV_VARS \
	INCLUDE_EXC_HANDLING \
	INCLUDE_EXC_SHOW \
	INCLUDE_EXC_TASK \
	INCLUDE_FLOATING_POINT \
	INCLUDE_FORMATTED_IO \
	INCLUDE_FTP \
	INCLUDE_FTPD_SECURITY \
	INCLUDE_FTP_SERVER \
	INCLUDE_GNU_INTRINSICS \
	INCLUDE_HASH \
	INCLUDE_HOST_TBL \
	INCLUDE_HW_FP \
	INCLUDE_HW_FP_SHOW \
	INCLUDE_ICMP \
	INCLUDE_ICMP_SHOW \
	INCLUDE_IO_SYSTEM \
	INCLUDE_IP \
	INCLUDE_KERNEL \
	INCLUDE_LOADER \
	INCLUDE_LOGGING \
	INCLUDE_LOOPBACK \
	INCLUDE_LSTLIB \
	INCLUDE_MEMORY_CONFIG \
	INCLUDE_MEM_MGR_BASIC \
	INCLUDE_MEM_MGR_FULL \
	INCLUDE_MEM_SHOW \
	INCLUDE_MMU_BASIC \
	INCLUDE_MODULE_MANAGER \
	INCLUDE_MSG_Q \
	INCLUDE_MSG_Q_SHOW \
	INCLUDE_MUX \
	INCLUDE_NETDEV_CONFIG \
	INCLUDE_NETDEV_NAMEGET \
	INCLUDE_NETMASK_GET \
	INCLUDE_NETWORK \
	INCLUDE_NET_HOST_SETUP \
	INCLUDE_NET_INIT \
	INCLUDE_NET_LIB \
	INCLUDE_NET_REM_IO \
	INCLUDE_NET_SETUP \
	INCLUDE_NET_SHOW \
	INCLUDE_PING \
	INCLUDE_PIPES \
	INCLUDE_POSIX_CLOCKS \
	INCLUDE_RAMDRV \
	INCLUDE_RBUFF \
	INCLUDE_RNG_BUF \
	INCLUDE_RPC \
	INCLUDE_SECURITY \
	INCLUDE_SELECT \
	INCLUDE_SELECT_SUPPORT \
	INCLUDE_SEM_BINARY \
	INCLUDE_SEM_COUNTING \
	INCLUDE_SEM_MUTEX \
	INCLUDE_SEM_SHOW \
	INCLUDE_SHELL \
	INCLUDE_SHELL_BANNER \
	INCLUDE_SIGNALS \
	INCLUDE_SIO \
	INCLUDE_STANDALONE_SYM_TBL \
	INCLUDE_STAT_SYM_TBL \
	INCLUDE_STDIO \
	INCLUDE_SYM_TBL \
	INCLUDE_SYM_TBL_INIT \
	INCLUDE_SYM_TBL_SHOW \
	INCLUDE_SYSCLK_INIT \
	INCLUDE_SYSHW_INIT \
	INCLUDE_SYS_START \
	INCLUDE_SYS_TIMESTAMP \
	INCLUDE_TASK_HOOKS \
	INCLUDE_TASK_SHOW \
	INCLUDE_TASK_VARS \
	INCLUDE_TCP \
	INCLUDE_TCP_SHOW \
	INCLUDE_TELNET \
	INCLUDE_TIMESTAMP \
	INCLUDE_TIMEX \
	INCLUDE_TTY_DEV \
	INCLUDE_UDP \
	INCLUDE_UDP_SHOW \
	INCLUDE_USER_APPL \
	INCLUDE_VXEVENTS \
	INCLUDE_WATCHDOGS \
	INCLUDE_WDB \
	INCLUDE_WDB_BANNER \
	INCLUDE_WDB_BP \
	INCLUDE_WDB_COMM_END \
	INCLUDE_WDB_CTXT \
	INCLUDE_WDB_DIRECT_CALL \
	INCLUDE_WDB_EVENTPOINTS \
	INCLUDE_WDB_EVENTS \
	INCLUDE_WDB_EXC_NOTIFY \
	INCLUDE_WDB_EXIT_NOTIFY \
	INCLUDE_WDB_FUNC_CALL \
	INCLUDE_WDB_GOPHER \
	INCLUDE_WDB_HW_FP \
	INCLUDE_WDB_MEM \
	INCLUDE_WDB_REG \
	INCLUDE_WDB_START_NOTIFY \
	INCLUDE_WDB_SYS \
	INCLUDE_WDB_SYS_HW_FP \
	INCLUDE_WDB_TASK \
	INCLUDE_WDB_TASK_BP \
	INCLUDE_WDB_TASK_HW_FP \
	INCLUDE_WDB_TSFS \
	INCLUDE_WDB_USER_EVENT \
	INCLUDE_WDB_VIO \
	INCLUDE_WDB_VIO_LIB \
	INCLUDE_WINDVIEW \
	INCLUDE_WINDVIEW_CLASS \
	INCLUDE_WVUPLOAD_FILE \
	INCLUDE_WVUPLOAD_TSFSSOCK
COMPONENT_LIBS = 


include $(TGT_DIR)/h/make/defs.project


## build-configuration info

ifeq ($(BUILD_SPEC),default)
ADDED_CFLAGS     = -g
AR               = arppc
AS               = ccppc
BOOT_EXTRA       = 
CC               = ccppc
CC_ARCH_SPEC     = -mcpu=604 -mstrict-align
CFLAGS           = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -I$(WIND_BASE)/target/config/all -I$(PRJ_DIR) -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -DNDEBUG -DBOOT_L2 -DM_TGT_L3 -DL2_TX -DPRJ_BUILD_MCWILL_L3 -DM_TGT_WANIF
CFLAGS_AS        = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -P -xassembler-with-cpp -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g
CFLAGS_AS_PROJECT = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I/h -I. -I$(WIND_BASE)\target\config\all -I$(WIND_BASE)\target/h -I$(WIND_BASE)\target/src/config -I$(WIND_BASE)\target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DAP7447 -g -P -xassembler-with-cpp
CFLAGS_PROJECT   = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I/h -I. -I$(WIND_BASE)\target\config\all -I$(WIND_BASE)\target/h -I$(WIND_BASE)\target/src/config -I$(WIND_BASE)\target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DAP7447
CPP              = ccppc -E -P
DOC_FILES        = sysLib
EXTRA_MODULES    = $(WIND_BASE)/target/config/McWill_L3/cacheALib.obj $(WIND_BASE)/target/config/McWill_L3/cacheArchLib.obj $(WIND_BASE)/target/config/McWill_L3/mmuPpcLib.obj
LD               = ldppc
LDFLAGS          = -X -N -Map vxWorks.map
LD_LINK_PATH     = -L$(WIND_BASE)/target/lib/ppc/PPC604/prignu -L$(WIND_BASE)/target/lib/ppc/PPC604/common
LD_PARTIAL       = ccppc -r -nostdlib -Wl,-X
LD_PARTIAL_FLAGS = -X -r
LIBS             = $(VX_OS_LIBS) $(PRJ_DIR)/../L2If/L3_lib/L2If.a $(PRJ_DIR)/../l3data/l3_lib/l3data.a $(PRJ_DIR)/../tVoice/L3_lib/Voice.a $(PRJ_DIR)/../Messages/l3_lib/Messages.a $(PRJ_DIR)/../L3_boot/l3_lib/L3_boot.a $(PRJ_DIR)/../Diag/L3_Lib/Diag.a $(PRJ_DIR)/../OAM/l3_lib/l3oam.a $(PRJ_DIR)/../APFramework/L3_lib/ApFrmWk.a $(PRJ_DIR)/../telnet/L3_Lib/telnet.a $(PRJ_DIR)/../L2TxInL3/L2Lib/L2.a $(VX_OS_LIBS)
LOCAL_MEM_SIZE   = 10000000  # default 256 MB DDR configure for vxWorks  ###
NM               = nmppc
OPTION_DEFINE_MACRO = -D
OPTION_DEPEND    = -M -w
OPTION_GENERATE_DEPENDENCY_FILE = -MD
OPTION_INCLUDE_DIR = -I
OPTION_LANG_C    = -xc
OPTION_UNDEFINE_MACRO = -U
POST_BUILD_RULE  = $(PRJ_DIR)\l3_App_end.bat
RAM_HIGH_ADRS    = 00800000  # RAM text/data address for bootrom
RAM_LOW_ADRS     = 00010000  # RAM text/data address for vxWorks     00010000
ROM_WARM_ADRS    = 00010008  # warm address
SIZE             = sizeppc
TOOL_FAMILY      = gnu
USR_ENTRY        = usrInit
endif

ifeq ($(BUILD_SPEC),default_rom)
ADDED_CFLAGS     = -g
AR               = arppc
AS               = ccppc
BOOT_EXTRA       = 
CC               = ccppc
CC_ARCH_SPEC     = -mcpu=604 -mstrict-align
CFLAGS           = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g
CFLAGS_AS        = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -P -xassembler-with-cpp -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g
CFLAGS_AS_PROJECT = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I/h -I. -I$(WIND_BASE)\target\config\all -I$(WIND_BASE)\target/h -I$(WIND_BASE)\target/src/config -I$(WIND_BASE)\target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DAP7447 -g -P -xassembler-with-cpp
CFLAGS_PROJECT   = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I/h -I. -I$(WIND_BASE)\target\config\all -I$(WIND_BASE)\target/h -I$(WIND_BASE)\target/src/config -I$(WIND_BASE)\target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DAP7447
CPP              = ccppc -E -P
DOC_FILES        = sysLib
EXTRA_MODULES    = $(WIND_BASE)/target/config/McWill_L3/cacheALib.obj $(WIND_BASE)/target/config/McWill_L3/cacheArchLib.obj $(WIND_BASE)/target/config/McWill_L3/mmuPpcLib.obj
HEX_FLAGS        = 
LD               = ldppc
LDFLAGS          = -X -N
LD_LINK_PATH     = -L$(WIND_BASE)/target/lib/ppc/PPC604/gnu -L$(WIND_BASE)/target/lib/ppc/PPC604/common
LD_PARTIAL       = ccppc -r -nostdlib -Wl,-X
LD_PARTIAL_FLAGS = -X -r
LIBS             = $(VX_OS_LIBS)
LOCAL_MEM_SIZE   = 10000000  # default 256 MB DDR configure for vxWorks  ###
NM               = nmppc
OPTION_DEFINE_MACRO = -D
OPTION_DEPEND    = -M -w
OPTION_GENERATE_DEPENDENCY_FILE = -MD
OPTION_INCLUDE_DIR = -I
OPTION_LANG_C    = -xc
OPTION_UNDEFINE_MACRO = -U
RAM_HIGH_ADRS    = 00800000  # RAM text/data address for bootrom
RAM_LOW_ADRS     = 00010000  # RAM text/data address for vxWorks     00010000
ROM_SIZE         = 00080000  # number of bytes of ROM space 512k
ROM_TEXT_ADRS    = 00010000  # ROM entry address, PPC boot vector
ROM_WARM_ADRS    = 00010008  # warm address
SIZE             = sizeppc
TOOL_FAMILY      = gnu
USR_ENTRY        = usrInit
POST_BUILD_RULE  = 
endif

ifeq ($(BUILD_SPEC),default_romCompress)
ADDED_CFLAGS     = -g
AR               = arppc
AS               = ccppc
BOOT_EXTRA       = 
CC               = ccppc
CC_ARCH_SPEC     = -mcpu=604 -mstrict-align
CFLAGS           = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g
CFLAGS_AS        = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -P -xassembler-with-cpp -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g
CFLAGS_AS_PROJECT = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I/h -I. -I$(WIND_BASE)\target\config\all -I$(WIND_BASE)\target/h -I$(WIND_BASE)\target/src/config -I$(WIND_BASE)\target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DAP7447 -g -P -xassembler-with-cpp
CFLAGS_PROJECT   = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I/h -I. -I$(WIND_BASE)\target\config\all -I$(WIND_BASE)\target/h -I$(WIND_BASE)\target/src/config -I$(WIND_BASE)\target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DAP7447
CPP              = ccppc -E -P
DOC_FILES        = sysLib
EXTRA_MODULES    = $(WIND_BASE)/target/config/McWill_L3/cacheALib.obj $(WIND_BASE)/target/config/McWill_L3/cacheArchLib.obj $(WIND_BASE)/target/config/McWill_L3/mmuPpcLib.obj
HEX_FLAGS        = 
LD               = ldppc
LDFLAGS          = -X -N
LD_LINK_PATH     = -L$(WIND_BASE)/target/lib/ppc/PPC604/gnu -L$(WIND_BASE)/target/lib/ppc/PPC604/common
LD_PARTIAL       = ccppc -r -nostdlib -Wl,-X
LD_PARTIAL_FLAGS = -X -r
LIBS             = $(VX_OS_LIBS)
LOCAL_MEM_SIZE   = 10000000  # default 256 MB DDR configure for vxWorks  ###
NM               = nmppc
OPTION_DEFINE_MACRO = -D
OPTION_DEPEND    = -M -w
OPTION_GENERATE_DEPENDENCY_FILE = -MD
OPTION_INCLUDE_DIR = -I
OPTION_LANG_C    = -xc
OPTION_UNDEFINE_MACRO = -U
RAM_HIGH_ADRS    = 00800000  # RAM text/data address for bootrom
RAM_LOW_ADRS     = 00010000  # RAM text/data address for vxWorks     00010000
ROM_SIZE         = 00080000  # number of bytes of ROM space 512k
ROM_TEXT_ADRS    = 00010000  # ROM entry address, PPC boot vector
ROM_WARM_ADRS    = 00010008  # warm address
SIZE             = sizeppc
TOOL_FAMILY      = gnu
USR_ENTRY        = usrInit
POST_BUILD_RULE  = 
endif

ifeq ($(BUILD_SPEC),default_romResident)
ADDED_CFLAGS     = -g
AR               = arppc
AS               = ccppc
BOOT_EXTRA       = 
CC               = ccppc
CC_ARCH_SPEC     = -mcpu=604 -mstrict-align
CFLAGS           = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g
CFLAGS_AS        = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -P -xassembler-with-cpp -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g
CFLAGS_AS_PROJECT = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I/h -I. -I$(WIND_BASE)\target\config\all -I$(WIND_BASE)\target/h -I$(WIND_BASE)\target/src/config -I$(WIND_BASE)\target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DAP7447 -g -P -xassembler-with-cpp
CFLAGS_PROJECT   = -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I/h -I. -I$(WIND_BASE)\target\config\all -I$(WIND_BASE)\target/h -I$(WIND_BASE)\target/src/config -I$(WIND_BASE)\target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DAP7447
CPP              = ccppc -E -P
DOC_FILES        = sysLib
EXTRA_MODULES    = $(WIND_BASE)/target/config/McWill_L3/cacheALib.obj $(WIND_BASE)/target/config/McWill_L3/cacheArchLib.obj $(WIND_BASE)/target/config/McWill_L3/mmuPpcLib.obj
HEX_FLAGS        = 
LD               = ldppc
LDFLAGS          = -X -N
LD_LINK_PATH     = -L$(WIND_BASE)/target/lib/ppc/PPC604/gnu -L$(WIND_BASE)/target/lib/ppc/PPC604/common
LD_PARTIAL       = ccppc -r -nostdlib -Wl,-X
LD_PARTIAL_FLAGS = -X -r
LIBS             = $(VX_OS_LIBS)
LOCAL_MEM_SIZE   = 10000000  # default 256 MB DDR configure for vxWorks  ###
NM               = nmppc
OPTION_DEFINE_MACRO = -D
OPTION_DEPEND    = -M -w
OPTION_GENERATE_DEPENDENCY_FILE = -MD
OPTION_INCLUDE_DIR = -I
OPTION_LANG_C    = -xc
OPTION_UNDEFINE_MACRO = -U
RAM_HIGH_ADRS    = 00800000  # RAM text/data address for bootrom
RAM_LOW_ADRS     = 00010000  # RAM text/data address for vxWorks     00010000
ROM_SIZE         = 00080000  # number of bytes of ROM space 512k
ROM_TEXT_ADRS    = 00010000  # ROM entry address, PPC boot vector
ROM_WARM_ADRS    = 00010008  # warm address
SIZE             = sizeppc
TOOL_FAMILY      = gnu
USR_ENTRY        = usrInit
POST_BUILD_RULE  = 
endif

# override make definitions only below this line


# override make definitions only above this line

include $(TGT_DIR)/h/make/rules.project


## build-configuration info

sysALib.o: $(PRJ_DIR)/prjParams.h

ifeq ($(BUILD_SPEC),default)

usrAppInit.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -I$(WIND_BASE)/target/config/all -I$(PRJ_DIR) -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -DNDEBUG -DBOOT_L2 -DM_TGT_L3 -DL2_TX -DPRJ_BUILD_MCWILL_L3 -DM_TGT_WANIF -c $(PRJ_DIR)/usrAppInit.c

PciEnd.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -I$(WIND_BASE)/target/config/all -I$(PRJ_DIR) -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -DNDEBUG -DBOOT_L2 -DM_TGT_L3 -DL2_TX -DPRJ_BUILD_MCWILL_L3 -DM_TGT_WANIF -c $(PRJ_DIR)/../../../../../prj_bts_split/common/PciIf/PciEnd.c

spyLib.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -I$(WIND_BASE)/target/config/all -I$(PRJ_DIR) -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -DNDEBUG -DBOOT_L2 -DM_TGT_L3 -DL2_TX -DPRJ_BUILD_MCWILL_L3 -DM_TGT_WANIF -c $(PRJ_DIR)/spyLib.c
VlanEnd.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -I$(WIND_BASE)/target/config/all -I$(PRJ_DIR) -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -DNDEBUG -DBOOT_L2 -DM_TGT_L3 -DL2_TX -DPRJ_BUILD_MCWILL_L3 -DM_TGT_WANIF -c $(PRJ_DIR)/../../../../../prj_bts_split/bts_bsp/McWill_L3/VlanEnd.c
endif

ifeq ($(BUILD_SPEC),default_rom)

usrAppInit.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g -c $(PRJ_DIR)/usrAppInit.c

PciEnd.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g -c $(PRJ_DIR)/../../../../../prj_bts_split/common/PciIf/PciEnd.c

spyLib.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g -c $(PRJ_DIR)/spyLib.c
VlanEnd.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g -c $(PRJ_DIR)/../../../../../prj_bts_split/bts_bsp/McWill_L3/VlanEnd.c
endif

ifeq ($(BUILD_SPEC),default_romCompress)

usrAppInit.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g -c $(PRJ_DIR)/usrAppInit.c

PciEnd.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g -c $(PRJ_DIR)/../../../../../prj_bts_split/common/PciIf/PciEnd.c

spyLib.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g -c $(PRJ_DIR)/spyLib.c
VlanEnd.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g -c $(PRJ_DIR)/../../../../../prj_bts_split/bts_bsp/McWill_L3/VlanEnd.c
endif

ifeq ($(BUILD_SPEC),default_romResident)

usrAppInit.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g -c $(PRJ_DIR)/usrAppInit.c

PciEnd.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g -c $(PRJ_DIR)/../../../../../prj_bts_split/common/PciIf/PciEnd.c

spyLib.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g -c $(PRJ_DIR)/spyLib.c
VlanEnd.o:
	$(CC) -g -mcpu=604 -mstrict-align -ansi -fno-builtin -I$(PRJ_DIR) -I$(WIND_BASE)/target/config/McWill_L3 -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv -DCPU=PPC604 -DTOOL_FAMILY=gnu -DTOOL=gnu -DPRJ_BUILD -DAP7447 -g -c $(PRJ_DIR)/../../../../../prj_bts_split/bts_bsp/McWill_L3/VlanEnd.c
endif


## dependencies


sysALib.o: $(WIND_BASE)/target/config/McWill_L3/sysALib.s \
	$(PRJ_DIR)/prjComps.h \
	$(PRJ_DIR)/prjParams.h


mv64360Sio.o: $(WIND_BASE)/target/config/McWill_L3/mv64360Sio.c \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stddef.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stdarg.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/va-ppc.h


mv643xxEnd.o: $(WIND_BASE)/target/config/McWill_L3/mv643xxEnd.c \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stddef.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stdarg.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/va-ppc.h


sysLib.o: $(WIND_BASE)/target/config/McWill_L3/sysLib.c \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stddef.h \
	$(PRJ_DIR)/prjComps.h \
	$(PRJ_DIR)/prjParams.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stdarg.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/va-ppc.h


romInit.o: $(WIND_BASE)/target/config/McWill_L3/romInit.s \
	$(PRJ_DIR)/prjComps.h \
	$(PRJ_DIR)/prjParams.h


romStart.o: $(WIND_BASE)/target/config/comps/src/romStart.c \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stddef.h \
	$(PRJ_DIR)/prjComps.h \
	$(PRJ_DIR)/prjParams.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stdarg.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/va-ppc.h


usrAppInit.o: $(PRJ_DIR)/usrAppInit.c \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stddef.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stdarg.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/va-ppc.h \
	$(PRJ_DIR)/prjComps.h \
	$(PRJ_DIR)/prjParams.h


prjConfig.o: $(PRJ_DIR)/prjConfig.c \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stddef.h \
	$(PRJ_DIR)/prjComps.h \
	$(PRJ_DIR)/prjParams.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stdarg.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/va-ppc.h


linkSyms.o: $(PRJ_DIR)/linkSyms.c


PciEnd.o: $(PRJ_DIR)/../../../../../prj_bts_split/common/PciIf/PciEnd.c \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stddef.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stdarg.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/va-ppc.h \
	$(PRJ_DIR)/prjComps.h \
	$(PRJ_DIR)/prjParams.h


spyLib.o: $(PRJ_DIR)/spyLib.c \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stddef.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stdarg.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/va-ppc.h


VlanEnd.o: $(PRJ_DIR)/../../../../../prj_bts_split/bts_bsp/McWill_L3/VlanEnd.c \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stddef.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/stdarg.h \
	$(WIND_BASE)/host/$(WIND_HOST_TYPE)/lib/gcc-lib/powerpc-wrs-vxworks/gcc-2.96/include/va-ppc.h \
	$(PRJ_DIR)/../../../../../prj_bts_split/bts_bsp/McWill_L3/mcWill_bts.h \
	$(PRJ_DIR)/../../../../../prj_bts_split/bts_bsp/McWill_L3/config.h \
	$(PRJ_DIR)/prjComps.h \
	$(PRJ_DIR)/prjParams.h \
	$(PRJ_DIR)/../../../../../prj_bts_split/bts_bsp/McWill_L3/mv64360.h \
	$(PRJ_DIR)/../../../../../prj_bts_split/bts_bsp/McWill_L3/sysBtsConfigData.h \
	$(PRJ_DIR)/../../../../../prj_bts_split/bts_bsp/McWill_L3/mv643xxEnd.h


## user defined rules

romStart.o : 
	$(CC) $(OPTION_OBJECT_ONLY) $(CFLAGS) $(PROJECT_BSP_FLAGS_EXTRA) $(ROM_FLAGS_EXTRA) $< -o $@

mv643xxEnd.o : 
	$(CC) $(OPTION_OBJECT_ONLY) $(CFLAGS) $(PROJECT_BSP_FLAGS_EXTRA) $(WIND_BASE)/target/config/McWill_L3/mv643xxEnd.c -o $@

mv64360Sio.o : 
	$(CC) $(OPTION_OBJECT_ONLY) $(CFLAGS) $(PROJECT_BSP_FLAGS_EXTRA) $(WIND_BASE)/target/config/McWill_L3/mv64360Sio.c -o $@

romInit.o : 
	$(CC) $(OPTION_OBJECT_ONLY) $(CFLAGS_AS) $(PROJECT_BSP_FLAGS_EXTRA) $(ROM_FLAGS_EXTRA) $< -o $@

linkSyms.o : 
	$(CC) $(OPTION_OBJECT_ONLY) $(CFLAGS) $(PROJECT_BSP_FLAGS_EXTRA) $(PRJ_DIR)/linkSyms.c -o $@

sysLib.o : 
	$(CC) $(OPTION_OBJECT_ONLY) $(CFLAGS) $(PROJECT_BSP_FLAGS_EXTRA) $(WIND_BASE)/target/config/McWill_L3/sysLib.c -o $@

sysALib.o : 
	$(AS) $(OPTION_OBJECT_ONLY) $(CFLAGS_AS) $(PROJECT_BSP_FLAGS_EXTRA) $(WIND_BASE)/target/config/McWill_L3/sysALib.s -o $@

prjConfig.o : 
	$(CC) $(OPTION_OBJECT_ONLY) $(CFLAGS) $(PROJECT_BSP_FLAGS_EXTRA) $(PRJ_DIR)/prjConfig.c -o $@

