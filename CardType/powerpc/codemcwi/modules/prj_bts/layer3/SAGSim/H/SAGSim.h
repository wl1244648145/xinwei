//////////////////////////////////////////////////////////////////////////
//��SAGֻ����CPE��CPE֮��ĺ��У�������CPE������������ն˵Ļ�ͨ
#pragma warning(disable:4786)
#include <string>
#include <map>
#include <list>
#include <vector>

using namespace std;

typedef enum
{
	IDLE_STATE,
	NULL_STATE,
	U1_STATE,
	U2_STATE,
	U3_STATE,
	U4_STATE,
	U5_STATE,
	U6_STATE,
	U7_STATE,
	U8_STATE,
	U9_STATE,
	U10_STATE,
	U11_STATE,
	U12_STATE,
	U13_STATE,
	U14_STATE
}CPE_STATE;

enum
{
	AUTH_REASON_LOGIN,
	AUTH_REASON_MTCALL
};


class CBTS
{
public:
	CBTS():m_fdSignalSocket(-1),m_fdVoiceDataSocket(-1),m_BTSID(0),m_PC(0),m_LAI(0){}
	UINT16 getBTSID(){ return m_BTSID;}
	void setBTSID(UINT16 BTSID){m_BTSID = BTSID;}
	UINT32 getLAI(){return m_LAI;};
	void setLAI(UINT32 LAI){m_LAI = LAI;}
	UINT16 getPC(){ return m_BTSID;}
	void setPC(UINT16 BTSID){m_BTSID = BTSID;}





	int	m_fdSignalSocket;		//����TCPsocket
	int m_fdVoiceDataSocket;	//��������UDPsocket
	//��������UDP��ַ,IPAddr&Port	
	sockaddr_in m_destAddr;

protected:
private:
	UINT16 m_BTSID;
	UINT32 m_LAI;				//LAI
	UINT16 m_PC;				//��������
};



class CCCB;

#define M_MAX_CCB_NUM		20
#define M_MAX_BTS_NUM		10

typedef map<UINT32,CCCB*,less<UINT32> > UID_INTDEX_TABLE;
typedef map<UINT32,CCCB*,less<UINT32> > L3ADDR_INDEX_TABLE;	
typedef map<string,UINT32,less<string> > PHONE_ADDRESS_BOOK;	//<LocalNumber, UID>
typedef map<UINT32,string,less<UINT32> > VALID_USER_TABLE;		//<UID, LocalNumber>

class CCCB
{
public:
	CCCB(){ClearCCBInfo();}
	~CCCB(){}
	void clearCCBCallInfo();	//�ͷź��к����
	void ClearCCBInfo();		//�û�ע�����ͷ�CCBʱ����
	UINT32 getUID();
	void setUID(UINT32 uid);
	UINT32 getL3Addr();
	void setL3Addr(UINT32 l3Addr);
	CCCB* getRemoteCCB();
	void setRemoteCCB(CCCB* pCCB);
	char* getLocalNumber();
	void setLocalNumber(char* LocalNumber);
	void SaveDialedNumber(char Digit);
	char* getDialedNumber();
	void setDialedNumber(char* number);
	UINT16 getCCBTableIndex();
	void setCCBTableIndex(UINT16 TabIndex);
	UINT8 getCodec();
	void setCodec(UINT8 codec);
	UINT8 getDialedNumberLen();
	void setDialedNumberLen(UINT8 len);
	UINT16 getTabIndex();
	void setTabIndex(UINT16 index);
	UINT8 getState();
	void setState(UINT8 state);
	bool isOrigCall();
	void setOrigCall(bool blOrigCall);
	UINT8 getAuthReason();
	void setAuthReason(UINT8 reason);
/*
	UINT16 getBTSID();
	void setBTSID(UINT16 BTSID);
	UINT16 getLAI();
	void setLAI(UINT16 LAI);
*/
	inline CBTS* getSrvBTS(){ return m_SrvBTS;}
	inline void setSrvBTS(CBTS* pBTS){m_SrvBTS = pBTS;}
	inline CBTS* getOldBTS(){return m_OldBTS;}
	inline void setOldBTS(CBTS* pBTS){m_OldBTS = pBTS;}

protected:
private:
	UINT16 m_TabIndex;
	UINT32 m_Uid;
	UINT32 m_L3Addr;
	UINT32 m_Codec;
	CCCB* m_pRemoteCCB;
	char m_LocalNumber[20];			//���û��绰����
	char m_DialedNumber[20];		//���û�����ĺ���
	UINT8 m_DialedNumberLen;		//�������λ��
	
	UINT8 m_State;					//��ǰ״̬
	bool m_blOrigCall;				//�Ƿ�����
	UINT8 m_AuthReason;				//��ǰ��Ȩԭ��

	UINT16 m_BTSID;
	UINT16 m_LAI;

	CBTS* m_SrvBTS;
	CBTS* m_OldBTS;
};




class CSAG
{
public:
	CSAG(){};
	~CSAG(){};
	void Init();
	UINT32 AllocateL3Addr();
	UINT16 getPC(){return m_PC;}
	void setPC(UINT16 pc){m_PC = pc;};
	UINT32 getSAGID(){return m_SAGID;}
	void setSAGID(UINT32 sagID){m_SAGID=sagID;}

	CCCB* AllocCCB(UINT32 uid);
	void DeAllocCCB(UINT16 TabIndex);

	CCCB* FindCCBByUID(UINT32 uid);
	CCCB* FindCCBByL3Addr(UINT32 l3Addr);
	CCCB* FindCCBByLocalNumber(char* number);
	bool IsValidUser(UINT32 uid);

	int checkDialPlan(char *calledNumber, UINT32 UIDcalling);

	void AddUIDIndexTable(UINT32 uid, CCCB* pCCB);
	void DelUIDIndexTable(UINT32 uid);
	void AddPhoneAddressBook(char* number, UINT32 uid);
	void DelPhoneAddressBook(char* number);
	void AddL3AddrIndexTable(UINT32 l3Addr, CCCB* pCCB);
	void DelL3AddrIndexTable(UINT32 l3Addr);
	void AddValidUserList(UINT32 uid, char* number);
	void DelValidUserList(UINT32 uid);

	void sendSignalToBTS(CSAbisSignal& signal, int fdSocket);
	void ParseAndHandleSignal(CSAbisSignal& signal, int fdSocket);

	//void handleSignalLAPagingReq(CSAbisSignal& signal, int fdSocket);
	void handleSignalLAPagingRsp(CSAbisSignal& signal, int fdSocket); 
	//void handleSignalDeLAPagingReq(CSAbisSignal& signal, int fdSocket); 
	void handleSignalDeLAPagingRsp(CSAbisSignal& signal, int fdSocket); 
	void handleSignalAssignResReq(CSAbisSignal& signal, int fdSocket); 
	void handleSignalAssignResRsp(CSAbisSignal& signal, int fdSocket); 
	//void handleSignalReleaseResReq(CSAbisSignal& signal, int fdSocket); 
	void handleSignalReleaseResRsp(CSAbisSignal& signal, int fdSocket);  

	void handleSignalReset(CSAbisSignal& signal, int fdSocket);
	void handleSignalResetAck(CSAbisSignal& signal, int fdSocket);
	void handleSignalErrNotiReq(CSAbisSignal& signal, int fdSocket);
	void handleSignalErrNotiRsp(CSAbisSignal& signal, int fdSocket);
	void handleSignalBeatHeart(CSAbisSignal& signal, int fdSocket);
	void handleSignalBeatHeartAck(CSAbisSignal& signal, int fdSocket);
	void handleSignalCongestReq(CSAbisSignal& signal, int fdSocket);
	void handleSignalCongestRsp(CSAbisSignal& signal, int fdSocket);

	void handleSignalSetup(CSAbisSignal& signal, int fdSocket);
	void handleSignalSetupAck(CSAbisSignal& signal, int fdSocket);
	void handleSignalAlerting(CSAbisSignal& signal, int fdSocket);	
	void handleSignalConnect(CSAbisSignal& signal, int fdSocket);
	void handleSignalConnectAck(CSAbisSignal& signal, int fdSocket);
	void handleSignalDisconnect(CSAbisSignal& signal, int fdSocket);
	void handleSignalInformation(CSAbisSignal& signal, int fdSocket);
	//void handleSignalRelease(CSAbisSignal& signal, int fdSocket);
	void handleSignalReleaseComplete(CSAbisSignal& signal, int fdSocket);
	void handleSignalModifyMediaTypeReq(CSAbisSignal& signal, int fdSocket);
	void handleSignalModifyMediaTypeRsp(CSAbisSignal& signal, int fdSocket);
	
	void handleSignalHandOverReq(CSAbisSignal& signal, int fdSocket);
	//void handleSignalHandOverRsp(CSAbisSignal& signal, int fdSocket);
	void handleSignalHandOverComplete(CSAbisSignal& signal, int fdSocket);

	void handleSignalLoginReq(CSAbisSignal& signal, int fdSocket);
	//void handleSignalLoginRsp(CSAbisSignal& signal, int fdSocket);
	void handleSignalLogout(CSAbisSignal& signal, int fdSocket);
	//void handleSignalAuthCmdReq(CSAbisSignal& signal, int fdSocket);
	void handleSignalAuthCmdRsp(CSAbisSignal& signal, int fdSocket);

	void handleSignalMOSMSDataReq(CSAbisSignal& signal, int fdSocket);
	//void handleSignalMOSMSDataRsp(CSAbisSignal& signal, int fdSocket);
	//void handleSignalMTSMSDataReq(CSAbisSignal& signal, int fdSocket);
	void handleSignalMTSMSDataRsp(CSAbisSignal& signal, int fdSocket);
	void handleSignalSMSMemAvailReq(CSAbisSignal& signal, int fdSocket);
	//void handleSignalSMSMemAvailRsp(CSAbisSignal& signal, int fdSocket);

	
	void sendDisconnect(CCCB* pCCB, int fdSocket, UINT8 RelCause);
	void sendRelease(CCCB* pCCB, int fdSocket, UINT8 RelCause);


	CBTS* findBTSByPC(UINT16 pc);
	CBTS* findBTSByBTSID(UINT16 btsid);

	void setAllHeadTailFields(CBTS* pBTS,
								  CSAG* pSAG,
								  UINT16 nPayloadLen,
								  SignalType sigType,
								  CSAbisSignal& signal);



	void handleVoiceData(char* buf, int bufLen, int fdUdpSocket);
protected:

private:
	static UINT32 m_L3AddrRes;
	CCCB m_CCBTable[M_MAX_CCB_NUM];

	vector < UINT32 > m_UidList;
	UID_INTDEX_TABLE	m_Uid_index_Table;
	L3ADDR_INDEX_TABLE	m_L3Addr_index_Table;
	PHONE_ADDRESS_BOOK  m_Phone_Address_Book;
	VALID_USER_TABLE	m_valid_User_Table;
	
	UINT16 m_PC;	//��������
	UINT32 m_SAGID;	//SAGID

	CBTS btsTbl[M_MAX_BTS_NUM];	//SAG��Χ�ڵ�BTS
};



// typedef map<LAIT, CBTS*, less<LAIT> > BTSTABLE;
class CLAI
{
public:
	void AddOneBTS(CBTS* pBTS) {m_BTSList.insert(m_BTSList.begin(), pBTS);}
	void DelOneBTS(CBTS* pBTS) {m_BTSList.remove(pBTS);}

protected:
private:
	UINT8	m_LAI[3];
	list<CBTS*> m_BTSList;
};

/*
SAG����������Ϊ80
SAG��������BTS��BTSIDΪ1��2���ֱ���������LAIΪ1��2, PC����������Ϊ1��2��
SAG����2��CPE��UID�ֱ�Ϊ1��2������Ϊ31��32�����żƻ�Ϊ3��ͷ����λ����

SAG�������������������������������
�����������������ͨ��udp socket�����������Ϣ

�ݲ�ʵ�ֵĹ���

���TCP���ӶϿ��������֮������CCB��BTS��Ϣ
�������״̬������ͨ��һ��ʱ��û�����������ͣ������������Ϣ

*/