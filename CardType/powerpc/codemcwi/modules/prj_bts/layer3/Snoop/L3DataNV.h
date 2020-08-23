/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataNVRAM.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   03/30/06   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __SNOOP_NVRAM_H__
#define __SNOOP_NVRAM_H__

#include <stdio.h>

#include <map>
using namespace std;
#include <list>

#include "L3DataTypes.h"
#include "L3DataMacAddress.h"
#include "McWill_bts.h"
#include "L3DataAssert.h"


/********************************************************
 *SNOOP_NVRAM_CCB_NUM:
 *M_NVRAM_INITIALIZED:NVRAM第一次初始话后在DATA头上写的值
 ********************************************************/
#define SNOOP_NVRAM_CCB_NUM         (M_MAX_USER_PER_BTS)
#define M_NVRAM_INITIALIZED         (0x20070723)
#define M_NVRAM_VERSION             (0x20070723)


/****************************
 *stDataNVRAMhdr:NVRAM的
 *CCB前存储的上次启动前
 *系统的有关配置
 */
struct stDataNVRAMhdr
{
    UINT32 ulInitialized;
    UINT32 ulRAID;
    UINT32 ulLocalBts;
};


/****************************
 *stDataNVRAMhdr:NVRAM的
 *CCB前存储的上次启动前
 *系统的有关配置
 */
struct stDataNVRAMTail
{
    UINT32 version;
};


/****************************
 *NVRamCCB结构: 
 ****************************/
#pragma pack (1)
struct stNVRamCCB
{
    UINT8   ucIsOccupied:1;
    UINT8   ucIsAnchor:1;
    UINT8   ucIsServing:1;
    UINT8   ucIpType:5;
    UINT32  ulEid;
    UINT32  ulServingBts;
    DATA    Data;
    UINT16  usGroupID;  //to recover the EB entry correctly when BTS reboots.
    UINT8   aucMac[ M_MAC_ADDRLEN ];
    UINT8 ucIsRcpeFlag;//0:no,1:yes
};
#pragma pack ()


/****************************
 *CNVRamCCBTable类: 
 ****************************/
class CNVRamCCBTable
{
public:
    CNVRamCCBTable()
        {
        m_pCCBTable = (stNVRamCCB*)( (UINT8*)( NVRAM_BASE_ADDR_DATA ) + sizeof( stDataNVRAMhdr ) );
        }
    ~CNVRamCCBTable(){}

    void   showStatus(UINT32);

    void   SetCurrRAID(const UINT32 &ulRAID)  
        {
        ( (stDataNVRAMhdr*)( NVRAM_BASE_ADDR_DATA ) )->ulRAID = ulRAID;
//bspNvRamWrite( (char*)&( ( (stDataNVRAMhdr*)( NVRAM_BASE_ADDR_DATA ) )->ulRAID ), (char*)&ulRAID, sizeof( ulRAID ) );
        }
    void   SetLocalBts(const UINT32 &ulIp)    
        {
        ( (stDataNVRAMhdr*)( NVRAM_BASE_ADDR_DATA ) )->ulLocalBts = ulIp;
//bspNvRamWrite( (char*)&( ( (stDataNVRAMhdr*)( NVRAM_BASE_ADDR_DATA ) )->ulLocalBts ), (char*)&ulIp, sizeof( ulIp ) );
        }
    void   SetInitialized()    
        {
        ( (stDataNVRAMhdr*)( NVRAM_BASE_ADDR_DATA ) )->ulInitialized = M_NVRAM_INITIALIZED;
//UINT32 ulInitialized = M_NVRAM_INITIALIZED;
//bspNvRamWrite( (char*)&( ( (stDataNVRAMhdr*)( NVRAM_BASE_ADDR_DATA ) )->ulInitialized ), (char*)&ulInitialized, sizeof( ulInitialized ) );
        }

    UINT32 GetLastRAID()    const   { return ( (stDataNVRAMhdr*)( NVRAM_BASE_ADDR_DATA ) )->ulRAID; }
    UINT32 GetLocalBts()    const   { return ( (stDataNVRAMhdr*)( NVRAM_BASE_ADDR_DATA ) )->ulLocalBts; }
    UINT32 GetInitialized() const   { return ( (stDataNVRAMhdr*)( NVRAM_BASE_ADDR_DATA ) )->ulInitialized; }

    UINT32 getVersion()     const   { return ( (stDataNVRAMTail*)(m_pCCBTable+SNOOP_NVRAM_CCB_NUM) )->version; }
    void   setVersion()     const   { ( (stDataNVRAMTail*)(m_pCCBTable+SNOOP_NVRAM_CCB_NUM ))->version = M_NVRAM_VERSION; }

    stNVRamCCB *GetCCBTableBasePtr() const { return m_pCCBTable; }

////Free CCB list methods
    void   InsertFreeCCB(UINT16);
    UINT16 GetFreeCCBIdxFromList();

////BPtree methods
    bool   BPtreeAdd(CMac&, UINT16);
    bool   BPtreeDel(CMac&);
    UINT16 BPtreeFind(CMac&);

    //---------------------------------------------
    //返回指定下标的CCB
    //---------------------------------------------
    stNVRamCCB* GetCCBByIdx(UINT16);

    //打印指定下标的CCB
    void showCCB(UINT16);
    //打印指定Mac地址的CCB
    void showCCB(CMac &Mac){ showCCB( BPtreeFind( Mac ) ); }

    void clearCCB();
    void init();

    void groupModify(UINT32, UINT16);

private:
    //members.
    list<UINT16>      m_listFreeCCB;
    stNVRamCCB        *m_pCCBTable;
    map<CMac, UINT16> m_CCBtree;    //CCB的索引树
};

#endif /*__SNOOP_NVRAM_H__*/
