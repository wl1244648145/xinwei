#ifndef _INC_L3L2CFGCOMMON
#define _INC_L3L2CFGCOMMON

#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif

#pragma pack(1)
#define BTS_BOARDS_CNT (9)
struct T_BTSBoardsState
{
    UINT8 BoardState[BTS_BOARDS_CNT];   
	// 0 - 正常运行   1 - 告警  2--  不在位
	// Ind  ＝ 1 ―― 8为射频板    Ind  ＝ 9 ―― 频踪板
};
#pragma pack()
#endif

