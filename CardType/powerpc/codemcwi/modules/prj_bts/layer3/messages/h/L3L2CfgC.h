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
	// 0 - ��������   1 - �澯  2--  ����λ
	// Ind  �� 1 �D�D 8Ϊ��Ƶ��    Ind  �� 9 �D�D Ƶ�ٰ�
};
#pragma pack()
#endif

