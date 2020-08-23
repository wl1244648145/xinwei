/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: L3OamDiag.h
 *
 * DESCRIPTION:
 *     Define the class of the receiving task of the Diag modulel.
 *
 * HISTORY:
 * Date        Author       Description
 * ----------  ----------   ----------------------------------------------------
 * 07/04/2006  Tian JingWei Initial file creation.
 * 01/08/2006  Xin Wang     Nat.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3OAMDIAG
#define _INC_L3OAMDIAG

#ifndef _INC_TASK
#include "BizTask.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#ifndef _INC_LOGAREA
#include "LogArea.h"
#endif

#define SOCKET int
#define MAX_DIAGTOOL_REGCNT   20
#define DIAG_TOOLREG_REC_NOTUSEED  0
#define DIAG_TOOLREG_REC_USEED     1
struct BtsDiagHeader 
{   
    UINT32  FN;
    UINT32  SeqNum;
    UINT16  Version;    
    UINT16  type;
    UINT16  Len;    
    UINT16  Rsv;    
};

typedef struct
{   
    BtsDiagHeader header;
    UINT32      IP;
    UINT16      Port;
    UINT8       Username[40]; //temp
    UINT8       Password[40]; //temp    
} T_DiagToolReg;

#pragma pack(2)
typedef struct
{   
    BtsDiagHeader header;
	UINT32      EID;
	UINT16      Flag;
	UINT16      BSGlobal;
	UINT16      Period;
    UINT32      IP;
    UINT16      Port;
    UINT8       Username[40]; //temp
    UINT8       Password[40]; //temp    
} T_DiagToolUtMonitorReq;
#pragma pack()
struct T_DiagToolRec
{   
    UINT16  Flag;       //表项是否正在使用  0--未使用         1--已使用
    UINT32  TTL;
    UINT32  IP;
    UINT16  Port;

	T_DiagToolRec(): Flag(DIAG_TOOLREG_REC_NOTUSEED), TTL(3), IP(0),Port(0)
	{ };
};

struct T_DiagToolRecs
{
	UINT32   CurrentCnt;
	T_DiagToolRec DiagToolRec[MAX_DIAGTOOL_REGCNT];
	T_DiagToolRecs() : CurrentCnt(0) {} 
};

//class CL3OamDiagEMSL3L2:public CTask
//{
//private:
//    SOCKET m_sfdEmsL3, m_sfdL3L2;
//public:
//    CL3OamDiagEMSL3L2();
//    ~CL3OamDiagEMSL3L2();
//private:
//    virtual TID GetEntityId() const;
//    virtual bool Initialize();
//    virtual void MainLoop();
//    bool CreateSocket();
//    void CheckValid();
//};

class CL3OamDiagEMSL3L2:public CBizTask
{
private:
    SOCKET m_sfdL3L2;
	static CL3OamDiagEMSL3L2* s_ptaskDiagL3;
public:
	CL3OamDiagEMSL3L2();
	~CL3OamDiagEMSL3L2();
	static CL3OamDiagEMSL3L2* GetInstance();
private:
	virtual TID GetEntityId() const;
	virtual bool Initialize();
	virtual bool ProcessComMessage(CComMessage*);
	bool CreateSocket();
	void CloseSocket();
	void CheckValid();
    bool IsMonitoredForDeadlock()  { return false; };
    int  GetMaxBlockedTime() { return WAIT_FOREVER ;};
};

class CL3OamDiagL2L3EMS:public CTask
{
/*private:*/
public:
    SOCKET m_sfdL3Ems, m_sfdL2L3;
public:
    CL3OamDiagL2L3EMS();
    ~CL3OamDiagL2L3EMS();
#ifdef WBBU_CODE
void  send_2_diagtool(unsigned char *pBuf,unsigned int Datalen);
 static CL3OamDiagL2L3EMS* GetInstance();

    static CL3OamDiagL2L3EMS* s_ptaskDiag;
#endif
private:
    virtual TID GetEntityId() const;
    virtual bool Initialize();
    virtual void MainLoop();
    bool CreateSocket();
    bool IsMonitoredForDeadlock()  { return false; }
    int  GetMaxBlockedTime() { return WAIT_FOREVER;}

};

#endif
