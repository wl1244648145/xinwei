#ifndef _INC_L3OAMRPTHEADER
#define _INC_L3OAMRPTHEADER

#ifndef _INC_STDHDR
#include "stdhdr.h"
#endif

#ifndef _INC_COMENTITY
#include "ComEntity.h"
#endif

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#include <string.h>

#define CPEM_RPT_RFONOFF_TIMELENGTH  1*60*1000 // minute
#define CPEM_RPT_RFONOFF_INTEVALCNT  3
#define CPEM_RPT_CFG_TIMELENGTH  1*60*1000 // minute
#define CPEM_RPT_CFG_INTEVALCNT  3
#define CPEM_RPT_GET_TIMELENGTH  3*60*1000 // minute
#define CPEM_RPT_GET_INTEVALCNT  3

enum 
{
    RPTOK = 0,
	RFOnOffTimeout,
	CFGTimeout,
	GETTimeout
};
enum 
{
	RPT_RF_On = 0,
	RPT_RF_Of
};

#pragma pack(1)
typedef struct T_RPT_RF_CFG_RSP
	{
		UINT16 usTransID;
		UINT16 usRst1;
		UINT32 ulPID;
		UINT16 usRst2;
	}tRptRfCfgRsp;
typedef struct T_RPT_GET_RSP
	{		
		UINT16 usTransID;	      //
		UINT16 usRslt;
		UINT32 ulPID;
		UINT16 usPwr;        // 转发功率
		UINT16 usRcvFreq;    // 接收频点
		UINT16 usTransFreq;  // 转发频点
		UINT16 usDownGain;   // 上行增益
		UINT16 usUpGain;     // 下行增益
		SINT16 usHemperature;// 温度
		UINT16 usHumidity;   // 湿度
		UINT16 usHeartBeat;  // 心跳时间
		UINT32 usVerData;    // Ver_data
		UINT16 usRFAlrm;     // 射频告警
		UINT16 usEnvAlarm;   // 环境告警
		UINT16 usCtrlAlrm;   // 监控告警
		UINT16 usRsvAlrm;    // 保留告警
		UINT16 usBBStatus;
	}tRptGetRsp;
typedef struct T_RPT_ALARM_NOTIFY
	{		
		UINT16 usTransID;	      //
		//UINT16 usRslt;
		UINT32 ulPID;
		UINT16 usPwr;        // 转发功率
		UINT16 usRcvFreq;    // 接收频点
		UINT16 usTransFreq;  // 转发频点
		UINT16 usDownGain;   // 上行增益
		UINT16 usUpGain;     // 下行增益
		SINT16 usHemperature;// 温度
		UINT16 usHumidity;   // 湿度
		UINT16 usHeartBeat;  // 心跳时间
		UINT32 usVerData;    // Ver_data
		UINT16 usRFAlrm;     // 射频告警
		UINT16 usEnvAlarm;   // 环境告警
		UINT16 usCtrlAlrm;   // 监控告警
		UINT16 usRsvAlrm;    // 保留告警
//		UINT16 usConnStatus;
		UINT16 usBBStatus;
	}tRptAlrmNtfy;
typedef struct T_RPT_RF_REQ
	{
		UINT16	usTransID;	      //
		UINT32  ulPID;
		UINT16  usOnOff;
	}tRptRfReq;
typedef struct T_RPT_CFG_REQ
	{
		UINT16	usTransID;	      //
		UINT32  ulPID;
		UINT16	usPower;    //转发功率值
		UINT16	usRcevFreq; //接收频点
		UINT16	usTransFreq;//转发频点
		UINT16	usDownGain; //下行增益
		UINT16	usUpGain;   //上行增益
		UINT16	usHeartBeat;//心跳时间
	}tRptCfgReq;
typedef struct T_RPT_GET_REQ
	{
		UINT16 usTransID;
		UINT32 ulPID;
	}tRptGetReq;
#pragma pack()

class CL3_EMS2L3_GetReq: public CMessage
{
	#pragma pack(1)
	struct T_RPTGETREQ
	{
		UINT16 usTransID;
		UINT32 ulPID;
	};
#pragma pack()
public:
    CL3_EMS2L3_GetReq(CMessage &rMsg):CMessage(rMsg){  };
    CL3_EMS2L3_GetReq(){};
    bool CreateMessage(CComEntity* Entity)
    {
        CMessage :: CreateMessage(*Entity);
        //SetMessageId(M_L3_CPE_UPGRADE_SW_REQ);
        return true;
    };
    ~CL3_EMS2L3_GetReq(){};
    UINT32  GetDefaultDataLen() { return sizeof(T_RPTGETREQ); };
    UINT32  GetRptPID(){ return ((T_RPTGETREQ *)GetDataPtr())->ulPID; };
    BOOL    SetData(UINT8* pd) 
    {
    	if( ! pd )
    		return false;
    	memcpy( (UINT8*)GetDataPtr(), pd, GetDefaultDataLen());
    };

/*

public: 
    CL3_EMS2L3_GetReq(CMessage &rMsg);
    CL3_EMS2L3_GetReq();
    bool CreateMessage(CComEntity* Entity);
    ~CL3_EMS2L3_GetReq();
    UINT32  GetDefaultDataLen();
	UINT32  GetRptPID();
	BOOL    SetData(UINT8* pd) ;
	*/
};







#if 0


class CL3_L32EMS_CommonRsp : public CMessage
{
public:
    CL3_L32EMS_CommonRsp(CMessage &rMsg);
    CL3_L32EMS_CommonRsp();
    bool CreateMessage(CComEntity* Entity);
    ~CL3_L32EMS_CommonRsp();
    UINT32  GetRptPID();
    UINT32  GetDefaultDataLen();
    UINT16  GetResult();
    BOOL    SetData(UINT8* pd);
    BOOL    SetResult(UINT16 us);
#pragma pack(1)
	struct T_RPTCOMMONRSP
	{
		UINT16 usTransID;
		UINT16 usRst;
		UINT32 ulPID;
	};
#pragma pack()
};

class CL3_RPTTIMEOUT: public CMessage
{
#pragma pack(1)
	struct T_RPTTIMEOUT
	{
		UINT16 usTransID;
		UINT16 usRslt;
		UINT32 ulPID;
	};
#pragma pack()
public: 
    CL3_RPTTIMEOUT(CMessage &rMsg);
    CL3_RPTTIMEOUT();
    bool CreateMessage(CComEntity* Entity);
    ~CL3_RPTTIMEOUT();
    UINT32  GetDefaultDataLen();
    UINT32  GetRptPID();
	void    SetResult( UINT16 rslt );
	UINT32    GetResult();
	BOOL    SetData(UINT8* pd) ;
};




class CL3_L32RPT_OnOffReq : public CMessage
{
#pragma pack(1)
    struct T_RPTONOFF
	{
		UINT16	usTransID;	      //
		UINT32  ulPID;
		UINT16  usOnOff;
	};
#pragma pack()
public: 
    CL3_L32RPT_OnOffReq(CMessage &rMsg);
    CL3_L32RPT_OnOffReq();
    bool CreateMessage(CComEntity* Entity);
    ~CL3_L32RPT_OnOffReq();
    UINT32  GetDefaultDataLen();
	UINT32  GetRptPID();
	BOOL    SetData(UINT8* pd) ;
};

class CL3_L32RPT_ConfigReq : public CMessage
{
#pragma pack(1)
    struct T_RPTCFG
	{
		UINT16	usTransID;	      //
		UINT32  ulPID;
		UINT16	usPower;    //转发功率值
		UINT16	usRcevFreq; //接收频点
		UINT16	usTransFreq;//转发频点
		UINT16	usDownGain; //下行增益
		UINT16	usUpGain;   //上行增益
		UINT16	usHeartBeat;//心跳时间
	};
#pragma pack()
public: 
    CL3_L32RPT_ConfigReq(CMessage &rMsg);
    CL3_L32RPT_ConfigReq();
    bool CreateMessage(CComEntity* Entity);
    ~CL3_L32RPT_ConfigReq();
    UINT32  GetDefaultDataLen();
	UINT32  GetRptPID();
	BOOL    SetData(UINT8* pd) ;
};

class CL3_RPT2L3_Report: public CMessage
{
#pragma pack(1)
    struct T_RPTREPORT
	{		
		UINT16 usTransID;	      //
		UINT16 usRslt;
		UINT32 ulPID;
		UINT16 usPwr;        // 转发功率
		UINT16 usRcvFreq;    // 接收频点
		UINT16 usTransFreq;  // 转发频点
		UINT16 usUpGain;     // 下行增益
		UINT16 usDownGain;   // 上行增益
		UINT16 usHemperature;// 温度
		UINT16 usHumidity;   // 湿度
		UINT16 usHeartBeat;  // 心跳时间
		UINT32 usVerData;    // Ver_data
		UINT16 usRFAlrm;     // 射频告警
		UINT16 usEnvAlarm;   // 环境告警
		UINT16 usCtrlAlrm;   // 监控告警
		UINT16 usRsvAlrm;    // 保留告警
		UINT16 usBBStatus;
	};
#pragma pack()
public: 
    CL3_RPT2L3_Report(CMessage &rMsg);
    CL3_RPT2L3_Report();
    bool CreateMessage(CComEntity* Entity);
    ~CL3_RPT2L3_Report();
    UINT32  GetDefaultDataLen();
	UINT32  GetRptPID();
	BOOL    SetData(UINT8* pd) ;
};


#endif //0

#endif
