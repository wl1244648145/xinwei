/*******************************************************************************
* Copyright (c) 2009 by AP Co.Ltd.All Rights Reserved   
* File Name      : voiceTestAssist.cpp
* Create Date    : 16-Nov-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "voiceCommon.h"
#include "voiceFsm.h"
#include "BtsVMsgId.h"
#include "tVoice.h"
#include "l3OamCfgCommon.h"
#include "localSag.h"
#include "localSagCommon.h"
#include "localSagStruct.h"
#include "localSagDialPlan.h"
#include "localSagCfg.h"
#include "l3oamfile.h"
#include "voiceToolFunc.h"

extern T_NvRamData *NvRamDataAddr;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void printLocalSagCfgFileInfo(T_UserFileHead *pHead)
{
	if(pHead)
	{
		VPRINT("\n-----name[%s] length[%d] flag[0x%04X]------\n", 
			pHead->userFileName, pHead->userFileLen, pHead->flag);
		if(M_FILE_READY_FLAG==pHead->flag && pHead->userFileLen>0)
		{
			VPRINT("%s", pHead->fileData);
		}
		VPRINT("\n-------------------------------------------\n");
	}
}

extern "C" void printLocalSagCfgFilesInfo()
{
	printLocalSagCfgFileInfo(&userFileRecord.User_ACL_List);
	printLocalSagCfgFileInfo(&userFileRecord.User_Voice_List);
	printLocalSagCfgFileInfo(&userFileRecord.Trunk_Group_list);
	printLocalSagCfgFileInfo(&userFileRecord.Trunk_Group_User_List);
}

void loadSrvCfgInfoFromFile()
{
	if(M_LOCALSAG_CFG_VALID_FLAG==VGetU32BitVal(NvRamDataAddr->localSagCfg.validFlag))
	{
		if(NvRamDataAddr->localSagCfg.blUseLocalSag)
		{
			//ACL
			if(NvRamDataAddr->localSagCfg.blUseUserListFile && M_FILE_READY_FLAG==userFileRecord.User_ACL_List.flag)
			{
				initACLFromFile((char*)userFileRecord.User_ACL_List.fileData, userFileRecord.User_ACL_List.userFileLen);
			}
			//user info
			if(NvRamDataAddr->localSagCfg.blUseLocalUserInfoFile && M_FILE_READY_FLAG==userFileRecord.User_Voice_List.flag)
			{
				initUserInfoFromFile((char*)userFileRecord.User_Voice_List.fileData, userFileRecord.User_Voice_List.userFileLen);
			}
			//grp info
			if(NvRamDataAddr->localSagCfg.blUseLocalGrpInfoFile && M_FILE_READY_FLAG==userFileRecord.Trunk_Group_list.flag)
			{
				initGrpInfoFromFile((char*)userFileRecord.Trunk_Group_list.fileData, userFileRecord.Trunk_Group_list.userFileLen);
			}
			//user-grp-info
			if(NvRamDataAddr->localSagCfg.blUseLocalUserInfoFile &&
				NvRamDataAddr->localSagCfg.blUseLocalGrpInfoFile && M_FILE_READY_FLAG==userFileRecord.Trunk_Group_User_List.flag)
			{
				initGrpUserInfoFromFile((char*)userFileRecord.Trunk_Group_User_List.fileData, userFileRecord.Trunk_Group_User_List.userFileLen);
			}
		}		
	}
}
extern "C" void saveSrvCfgInfoToFile()
{
	userFileRecord.User_ACL_List.flag = 0;
	userFileRecord.User_Voice_List.flag = 0;
	userFileRecord.Trunk_Group_list.flag = 0;
	userFileRecord.Trunk_Group_User_List.flag = 0;
	if(M_LOCALSAG_CFG_VALID_FLAG==VGetU32BitVal(NvRamDataAddr->localSagCfg.validFlag))
	{
		if(NvRamDataAddr->localSagCfg.blUseLocalSag)
		{
			//ACL
			if(NvRamDataAddr->localSagCfg.blUseUserListFile)
			{
				saveACLToFile((char*)userFileRecord.User_ACL_List.fileData, &userFileRecord.User_ACL_List.userFileLen);
				userFileRecord.User_ACL_List.flag = M_FILE_READY_FLAG;
			}
			//user info
			if(NvRamDataAddr->localSagCfg.blUseLocalUserInfoFile)
			{
				saveUserInfoToFile((char*)userFileRecord.User_Voice_List.fileData, &userFileRecord.User_Voice_List.userFileLen);
				userFileRecord.User_Voice_List.flag = M_FILE_READY_FLAG;
			}
			//grp info
			if(NvRamDataAddr->localSagCfg.blUseLocalGrpInfoFile)
			{
				saveGrpInfoToFile((char*)userFileRecord.Trunk_Group_list.fileData, &userFileRecord.Trunk_Group_list.userFileLen);
				userFileRecord.Trunk_Group_list.flag = M_FILE_READY_FLAG;
			}
			//user-grp-info
			if(NvRamDataAddr->localSagCfg.blUseLocalUserInfoFile &&
				NvRamDataAddr->localSagCfg.blUseLocalGrpInfoFile)
			{
				saveUserGrpInfoToFile((char*)userFileRecord.Trunk_Group_User_List.fileData, &userFileRecord.Trunk_Group_User_List.userFileLen);
				userFileRecord.Trunk_Group_User_List.flag = M_FILE_READY_FLAG;
			}
		}		
	}	
}
void initACLFromFile(char *pBuf, UINT32 size)
{
	CSAG::getSagInstance()->initACLFromFile(pBuf, size);
}
void initGrpInfoFromFile(char *pBuf, UINT32 size)
{
	CSAG::getSagInstance()->initGrpInfoFromFile(pBuf, size);
}
void initUserInfoFromFile(char *pBuf, UINT32 size)
{
	CSAG::getSagInstance()->initUserInfoFromFile(pBuf, size);
}
void initGrpUserInfoFromFile(char *pBuf, UINT32 size)
{
	CSAG::getSagInstance()->initGrpUserInfoFromFile(pBuf, size);
}
void showACL(UINT32 uid)
{
	CSAG::getSagInstance()->showACL(uid);
}
void showUserInfo(UINT32 uid)
{
	CSAG::getSagInstance()->showUserInfo(uid);
}
void showGrpInfo(UINT16 gid)
{
	CSAG::getSagInstance()->showGrpInfo(gid);
}
UINT32 saveACLToFile(char *pBuf, UINT32 *pSize)
{
	return CSAG::getSagInstance()->saveACLToFile(pBuf, pSize);
}
UINT32 saveGrpInfoToFile(char *pBuf, UINT32 *pSize)
{
	return CSAG::getSagInstance()->saveGrpInfoToFile(pBuf, pSize);
}
UINT32 saveUserInfoToFile(char *pBuf, UINT32 *pSize)
{
	return CSAG::getSagInstance()->saveUserInfoToFile(pBuf, pSize);
}
UINT32 saveUserGrpInfoToFile(char *pBuf, UINT32 *pSize)
{
	return CSAG::getSagInstance()->saveUserGrpInfoToFile(pBuf, pSize);
}
bool ifAllowUserAccess(UINT32 pid, UINT32 uid)
{
	return CSAG::getSagInstance()->ifAllowUserAccess(pid, uid);
}
bool isUserInACL(UINT32 uid, UINT32 pid)
{
	return CSAG::getSagInstance()->isUserInACL(uid, pid);
}
bool ifAllowGrpSetup(UINT16 gid, UINT32 uid)
{
	return CSAG::getSagInstance()->ifAllowGrpSetup(gid, uid);
}
bool isUserInGroup(UINT32 uid, UINT16 gid)
{
	return CSAG::getSagInstance()->isUserInGroup(uid, gid);
}
UINT16 formatUserGrpListInfo(char*buf, UINT16& len, UINT8& grpNum, UINT32 uid)
{
	return CSAG::getSagInstance()->formatUserGrpListInfo(buf, len, grpNum, uid);
}
void addACLUser(UINT32 uid, UINT32 pid)
{
	CSAG::getSagInstance()->addACLUser(uid, pid);
}
void delACLUser(UINT32 uid)
{
	CSAG::getSagInstance()->delACLUser(uid);
}
#if 0
bool findUserInfoByUID(UINT32 uid, map<UINT32, userInfo, less<UINT32> >::iterator& itFound)
{
	CSAG::getSagInstance()->findUserInfoByUID(uid, itFound);
}
#endif
void addUserInfo(UINT32 uid, UINT32 pid, char* telNO, UINT8 prio)
{
	CSAG::getSagInstance()->addUserInfo(uid, pid, telNO, prio);
}
void delUserInfo(UINT32 uid)
{
	CSAG::getSagInstance()->delUserInfo(uid);
}
void addUserGrpInfo(UINT32 uid, UINT16 gid, UINT8 prioInGrp)
{
	CSAG::getSagInstance()->addUserGrpInfo(uid, gid, prioInGrp);
}
void delUserGrpInfo(UINT32 uid, UINT16 gid)
{
	CSAG::getSagInstance()->delUserGrpInfo(uid, gid);
}
void clearUserGrpInfo(UINT32 uid)
{
	CSAG::getSagInstance()->clearUserGrpInfo(uid);
}
#if 0
bool findGrpInfoByGID(UINT16 gid, map<UINT16, grpInfo, less<UINT16> >::iterator& itFound)
{
	return CSAG::getSagInstance()->findGrpInfoByGID(gid, itFound);
}
#endif
void addGrpInfo(UINT16 gid, char* grpName, UINT8 grpPrio)
{
	CSAG::getSagInstance()->addGrpInfo(gid, grpName, grpPrio);
}
void delGrpInfo(UINT16 gid)
{
	CSAG::getSagInstance()->delGrpInfo(gid);
}
void clearACL()
{
	CSAG::getSagInstance()->clearACL();
}
void clearAllUserInfo()
{
	CSAG::getSagInstance()->clearAllUserInfo();
}
void clearAllGrpInfo()
{
	CSAG::getSagInstance()->clearAllGrpInfo();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
extern "C" void showSagGrpInfo(UINT16 gid)
{
	SxcGrpCCB *pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(gid);
	if(NULL!=pGrpCCB)
	{
		pGrpCCB->showInfo();
	}
}

extern "C" void notifyVoiceCfgChanged()
{
	//send msg to tVCRs&tVDRs to tell them cfg changed
	sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, M_TID_VOICE, MSGID_VOICE_SET_CFG);
}

UINT32 tmpSAGIP = 0;
extern "C" void disableMasterSag()
{
	tmpSAGIP = 0;
	writeData2NvRam((char*) &NvRamDataAddr->BtsGDataCfgEle.SAGSignalIP, (char*)&tmpSAGIP, 4);
}

T_SagBkp tmpBackupSagCfg;
extern "C" void disableBackupSag()
{
	memset(&tmpBackupSagCfg, 0, sizeof(T_SagBkp));
	writeData2NvRam((char*)&NvRamDataAddr->SagBkp, (char*)&tmpBackupSagCfg, sizeof(T_SagBkp));
	//send msg to tVCRs&tVDRs to tell them cfg changed
	sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, M_TID_VOICE, MSGID_VOICE_SET_CFG);
}

//模拟sag给终端发送电话号码信令VoiceInfo.req
extern "C" void fakeUser(UINT32 PID, UINT8 CID, char *szNumber)
{
	typedef struct __tmpVoiceInfoReqT
	{
		UINT8 cid;
		UINT8 msgType;
		UINT8 tag;
		UINT8 len;
		UINT8 value[100];
	}tmpVoiceInfoReqT;
	if(!isValidTelNO(szNumber, M_MAX_PHONE_NUMBER_LEN))
	{
		VPRINT("\nThe telephone number is not valid!!!\n");
		return;
	}
	UINT32 nNumLen = strlen(szNumber);
	UINT8 nTLVNumberValLen;
	if(nNumLen & 0x01)
	{
		nTLVNumberValLen = ((nNumLen+1)>>1);
	}
	else
	{
		nTLVNumberValLen = (nNumLen>>1);
	}
	tmpVoiceInfoReqT *pData;
	UINT16 msgBodyLen = sizeof(tmpVoiceInfoReqT) - sizeof(pData->value) + nTLVNumberValLen;
	
	CComMessage *pComMsg = new (CTVoice::GetInstance(), msgBodyLen) CComMessage;
	if(pComMsg!=NULL)
	{
		pComMsg->SetSrcTid(M_TID_VOICE);
		pComMsg->SetDstTid(M_TID_UTV);
		pComMsg->SetMessageId(MSGID_VOICE_DAC_SIGNAL);
		pComMsg->SetDataLength(msgBodyLen);
		pComMsg->SetEID(PID);

		pData = (tmpVoiceInfoReqT*)pComMsg->GetDataPtr();
		pData->cid = CID;
		pData->msgType = M_MSGTYPE_VoiceInfo_Req;
		convertStr2DigitNO((void*)(&pData->tag), szNumber);
		
		postComMsg(pComMsg);
	}
	else
	{
		VPRINT("\nCreate message error!!!\n");
	}
}

//修改第n条拨号计划
//设置一条内存拨号计划
extern "C" void setDialPlanItem(UINT16 id, char *szPrefix, UINT8 fullLen)
{
	if(id>=M_MAX_DIALPLAN_COUNT)
	{
		VPRINT("\nID should be no less than %d!!!\n", M_MAX_DIALPLAN_COUNT);
		return;
	}
	//拨号计划如果支持补充业务需要则需要使用isValidDialedNO(char * szNumber, UINT8 maxNumberLen)	
	if(!isValidTelNO(szPrefix, M_MAX_PREFIX_LEN))
	{
		VPRINT("\nThe szPrefix is not valid!!!\n");
		return;	
	}
	
	UINT32 prefixLen = strlen(szPrefix);
	if(fullLen<prefixLen)
	{
		VPRINT("\nThe dialplan Item is not valid!!!\n");
		return;
	}

	if(dialPlan_Para.lst_DialPlanItems[id].Len>0 && 
		dialPlan_Para.lst_DialPlanItems[id].PrefixNumber[0])
	{
		//原来的拨号计划条目合法
	}
	else
	{
		//原来的拨号计划条目不合法，增加计数
		dialPlan_Para.Count++;
	}
	dialPlan_Para.lst_DialPlanItems[id].Len = fullLen;
	strcpy(dialPlan_Para.lst_DialPlanItems[id].PrefixNumber, szPrefix);

}

extern "C" void setLocalSagAreaCode(char *szAreaCode)
{
	if(!isValidTelNO(szAreaCode, M_MAX_AREACODE_LENGTH))
	{
		VPRINT("\nAreaCode is not valid!!!\n");
		return;
	}
	strcpy(g_AreaCode, szAreaCode);
}

extern "C" void setLocalSagEnableFlag(char flag)
{
	g_blUseLocalSag = flag;
}

extern "C" void setUseUserListFileFlag(char flag)
{
	g_blUseUserListFile = flag;
}

LocalSagCfgT g_tmpLocalSagCfg;
extern "C" void cleanLocalSagCfg()
{
	memset((void*)&dialPlan_Para, 0, sizeof(DialPlanT));
	//if use localSag
	memset(&g_tmpLocalSagCfg, 0, sizeof(LocalSagCfgT));
	
	VSetU16BitVal((UINT8*)g_tmpLocalSagCfg.maxGrpAliveTime, 0xffff);
	VSetU16BitVal((UINT8*)g_tmpLocalSagCfg.maxGrpTalkTime, 0xffff);
	VSetU16BitVal((UINT8*)g_tmpLocalSagCfg.maxGrpIdleTime, 5);
	VSetU16BitVal((UINT8*)g_tmpLocalSagCfg.GrpLePagingLoopTime, 5);

	VSetU32BitVal((UINT8*)g_tmpLocalSagCfg.validFlag, M_LOCALSAG_CFG_VALID_FLAG);	
	//write to nvram
	writeData2NvRam((char*)&NvRamDataAddr->localSagCfg, (char*)&g_tmpLocalSagCfg, sizeof(LocalSagCfgT));	
}
//把当前内存中的localSag的配置存入nvram
extern "C" void saveLocalSagCfg2NvRam()
{
	UINT32 i;
	//if use localSag
	g_tmpLocalSagCfg.blUseLocalSag = g_blUseLocalSag;
	//if use userlist file
	g_tmpLocalSagCfg.blUseUserListFile = g_blUseUserListFile;
	//area code
	strcpy(g_tmpLocalSagCfg.AreaCode, g_AreaCode);
	//dial plan
	for(i=0;i<dialPlan_Para.Count;i++)
	{
		g_tmpLocalSagCfg.DialPlanTbl.DialPlanTbl[i].Len = 	dialPlan_Para.lst_DialPlanItems[i].Len;
		convertStr2BCDCode(g_tmpLocalSagCfg.DialPlanTbl.DialPlanTbl[i].PrefixNumber, 
			dialPlan_Para.lst_DialPlanItems[i].PrefixNumber);		
	}
	//grp configuration
	g_tmpLocalSagCfg.blUseLocalUserInfoFile = g_blUseLocalUserInfoFile;
	g_tmpLocalSagCfg.blUseLocalGrpInfoFile = g_blUseLocalGrpInfoFile;
	VSetU16BitVal((unsigned char*)g_tmpLocalSagCfg.maxGrpIdleTime, g_maxGrpIdleTime);
	VSetU16BitVal((unsigned char*)g_tmpLocalSagCfg.maxGrpTalkTime, g_maxGrpTalkTime);
	VSetU16BitVal((unsigned char*)g_tmpLocalSagCfg.maxGrpAliveTime, g_maxGrpAliveTime);
	VSetU16BitVal((unsigned char*)g_tmpLocalSagCfg.GrpLePagingLoopTime, g_GrpLePagingLoopTime);
	VSetU32BitVal((unsigned char*)g_tmpLocalSagCfg.validFlag, M_LOCALSAG_CFG_VALID_FLAG);
	//write to nvram
	writeData2NvRam((char*)&NvRamDataAddr->localSagCfg, (char*)&g_tmpLocalSagCfg, sizeof(LocalSagCfgT));	
}

//使当前设置的内存中的拨号计划生效
extern "C" void makeDialPlanEffect()
{
	buildDialPlanTree(&dialPlan_Para);
}

//在localSAG增加终端用户,即模拟终端向localSAG发送电话号码登记消息
extern "C" void addUser(UINT32 UID, UINT32 PID, UINT8 CID, char *szNumber)
{
	if(!isValidTelNO(szNumber, M_MAX_PHONE_NUMBER_LEN))
	{
		VPRINT("\nThe telephone number is not valid!!!\n");
		return;
	}
	UINT32 nNumLen = strlen(szNumber);
	UINT8 nTLVNumberValLen;
	if(nNumLen & 0x01)
	{
		nTLVNumberValLen = ((nNumLen+1)>>1);
	}
	else
	{
		nTLVNumberValLen = (nNumLen>>1);
	}
	TelNOT *pData;
	UINT16 nPayloadLen = sizeof(TelNOT)-sizeof(pData->telNO)+nTLVNumberValLen;
	CComMessage *pComMsg = new (CTVoice::GetInstance(), nPayloadLen) CComMessage;
	if(pComMsg!=NULL)
	{
		pComMsg->SetSrcTid(M_TID_VOICE);
		pComMsg->SetDstTid(M_TID_SAG);
		pComMsg->SetMessageId(MSGID_DAC_UL_CPE_TELNO_MSG);
		pComMsg->SetDataLength(nPayloadLen);
		pComMsg->SetEID(PID);

		pData = (TelNOT*)pComMsg->GetDataPtr();
		pData->cid = CID;
		VSetU32BitVal(pData->uid, UID);
		pData->telLen = convertStr2BCDCode((void *)pData->telNO, szNumber);
		
		postComMsg(pComMsg);	
	}
	else
	{


		VPRINT("\nCreate message error!!!\n");
	}	
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define self_test_cfg_file
#if 0
addUser(0x0410062, 0x80906289, 0, "0103838374")
addUser(0x0410235, 0x80906235, 0, "0103838347")
addUser(0x041007B, 0x80906232, 0, "0103939088")
addUser(0x0410097, 0x80906240, 0, "0103939092")
#endif
#ifdef self_test_cfg_file
char ACLStr[2000]="PID,UID\n0x80906289,0x0410062\n0x80906235,0x0410235 \n0x80906232,0x041007B \n0x80906240,0x0410097";
char UserInfoStr[2000]="PID,UID,telNO,prio\n0x80906289,0x0410062,0103838374,4\n0x80906235,0x0410235,0103838347,4 \n0x80906232,0x041007B,0103939088,4 \n0x80906240,0x0410097,0103939092,4";
char GrpInfoStr[2000]="GID,GrpName,GrpPrio\n1,grp1,3\n2,grp2,3\n3,3,3\n4,4,4\n0x5,5,4\n";
char GrpUserInfoStr[2000]="GID,UID,Prio\n1,0x0410062,4\n1,0x0410235,4\n1,0x041007B,4\n1,0x0410097,4\n2,0x0410062,5\n2,0x0410235,5\n3,0x041007B,2\n3,0x0410097,2\n5,0x0410062,6\n5,0x0410235,6\n5,0x041007B,6\n5,0x0410097,6\n";
#endif
extern "C" void testVCfgFile()
{

#ifdef self_test_cfg_file
	userFileRecord.User_ACL_List.fileData = (UINT8*)ACLStr;
	userFileRecord.User_Voice_List.fileData = (UINT8*)UserInfoStr;
	userFileRecord.Trunk_Group_list.fileData = (UINT8*)GrpInfoStr;
	userFileRecord.Trunk_Group_User_List.fileData = (UINT8*)GrpUserInfoStr;

	userFileRecord.User_ACL_List.userFileLen = strlen(ACLStr);
	userFileRecord.User_Voice_List.userFileLen = strlen(UserInfoStr);
	userFileRecord.Trunk_Group_list.userFileLen = strlen(GrpInfoStr);
	userFileRecord.Trunk_Group_User_List.userFileLen = strlen(GrpUserInfoStr);
	
	userFileRecord.User_ACL_List.flag = M_FILE_READY_FLAG;
	userFileRecord.User_Voice_List.flag = M_FILE_READY_FLAG;
	userFileRecord.Trunk_Group_list.flag = M_FILE_READY_FLAG;
	userFileRecord.Trunk_Group_User_List.flag = M_FILE_READY_FLAG;
	
	initACLFromFile((char*)userFileRecord.User_ACL_List.fileData, userFileRecord.User_ACL_List.userFileLen);
	initUserInfoFromFile((char*)userFileRecord.User_Voice_List.fileData, userFileRecord.User_Voice_List.userFileLen);
	initGrpInfoFromFile((char*)userFileRecord.Trunk_Group_list.fileData, userFileRecord.Trunk_Group_list.userFileLen);
	initGrpUserInfoFromFile((char*)userFileRecord.Trunk_Group_User_List.fileData, userFileRecord.Trunk_Group_User_List.userFileLen);


	saveACLToFile((char*)userFileRecord.User_ACL_List.fileData, &userFileRecord.User_ACL_List.userFileLen);
	saveUserInfoToFile((char*)userFileRecord.User_Voice_List.fileData, &userFileRecord.User_Voice_List.userFileLen);
	saveGrpInfoToFile((char*)userFileRecord.Trunk_Group_list.fileData, &userFileRecord.Trunk_Group_list.userFileLen);
	saveUserGrpInfoToFile((char*)userFileRecord.Trunk_Group_User_List.fileData, &userFileRecord.Trunk_Group_User_List.userFileLen);

	
#endif	
	
}

extern "C" void initLocalSagCfg4Test()
{
	disableBackupSag();
	cleanLocalSagCfg();
#if 0	
	setLocalSagEnableFlag(1);
	setLocalSagAreaCode("010");
		
	setDialPlanItem(0, "3838", 7);
	setDialPlanItem(1, "0103838", 10);
	makeDialPlanEffect();
	
	saveLocalSagCfg2NvRam();

	addUser(0x41cc1d, 0x27000029, 0, "0103838415");
	addUser(0x5f4492, 0x88900559, 0, "0103838031");
	addUser(0x5f418c, 0x005f418c, 0, "0103838412");
#endif	
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include "NatpiApp.h"
extern "C" void stopVdrNatapSession()
{
	getVdrNatpiSession()->stop();
	getVdr1NatpiSession()->stop();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
T_DcsCfgBuffer tmpDcsCfgBuf;
extern "C" void setDcsCfg(UINT32 dcsIP, UINT16 dcsPort, UINT16 localPort, UINT8 natKey)
{
	VSetU32BitVal(tmpDcsCfgBuf.dcsCfg.DCS_IP, dcsIP);
	VSetU16BitVal(tmpDcsCfgBuf.dcsCfg.DCS_Port, dcsPort);
	VSetU16BitVal(tmpDcsCfgBuf.dcsCfg.BTS_Port, localPort);
	tmpDcsCfgBuf.dcsCfg.NatApKey = natKey;
	if(0==dcsIP)
	{
		VSetU32BitVal(tmpDcsCfgBuf.validFlag, 0);
	}
	else
	{
		VSetU32BitVal(tmpDcsCfgBuf.validFlag, M_DCS_CFG_VALID_FLAG);
	}
	writeData2NvRam((char*)&NvRamDataAddr->dcsCfgBuffer, 
		(char*)&tmpDcsCfgBuf, 
		sizeof(T_DcsCfgBuffer));
}

