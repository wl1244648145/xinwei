/*******************************************************************************
* Copyright (c) 2010 by Beijing AP Co.Ltd.All Rights Reserved   
* File Name      : localSagSrvCfgFile.cpp
* Create Date    : 24-Jan-2010
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "localSag.h"
#include "localSagCfg.h"
#include "voiceToolFunc.h"
#include "stdio.h"

bool CSAG::isUserInACL(UINT32 uid, UINT32 pid)
{
	map<UINT32, UINT32, less<UINT32> >::iterator it = m_AccessLst.find(uid);
	return (it==m_AccessLst.end()) ? false: ((*it).second==pid);	
}
void CSAG::addACLUser(UINT32 uid, UINT32 pid)
{
	if(INVALID_UID==uid ||0==uid || NO_EID==pid || 0==pid)
		return ;
	map<UINT32, UINT32, less<UINT32> >::iterator it = m_AccessLst.find(uid);
	if(m_AccessLst.end()!=it)
	{
		LOG4(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			"{uid[0x%08X] pid[0x%08X]} already in ACL when adding {uid[0x%08X] pid[0x%08X]}!!!",
			(*it).first, (*it).second, uid, pid);
	}
	else
	{
		m_AccessLst.insert(map<UINT32, UINT32, less<UINT32> >::value_type(uid,pid));
	}
}
void CSAG::delACLUser(UINT32 uid)
{
	if(INVALID_UID==uid || 0==uid)
		return;
	map<UINT32, UINT32, less<UINT32> >::iterator it = m_AccessLst.find(uid);
	if(m_AccessLst.end()!=it)
		m_AccessLst.erase(it);	
}
bool CSAG::findUserInfoByUID(UINT32 uid, map<UINT32, userInfo, less<UINT32> >::iterator& itFound)
{
	map<UINT32, userInfo, less<UINT32> >::iterator it = m_UserInfoTbl.find(uid);
	if(m_UserInfoTbl.end()==it) 
	{
		return false;
	}
	else
	{
		itFound = it;
		return true;
	}
}
void CSAG::addUserInfo(UINT32 uid, UINT32 pid, char* telNO, UINT8 prio)
{
	if(INVALID_UID==uid || 0==uid || NO_EID==pid || 0==pid)
		return;
	map<UINT32, userInfo, less<UINT32> >::iterator itFound;
	if(!findUserInfoByUID(uid, itFound))
	{
		userInfo UserInfo(uid, pid, prio, telNO);
		m_UserInfoTbl.insert(map<UINT32,userInfo,less<UINT32> >::value_type(uid, UserInfo));
	}
	else
	{
		LOG4(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			"{uid[0x%08X] pid[0x%08X] } already in ACL when adding {uid[0x%08X] pid[0x%08X] }!!!",
			(*itFound).second.uid, (*itFound).second.pid, uid, pid);
	}
}
void CSAG::delUserInfo(UINT32 uid)
{
	if(INVALID_UID==uid || 0==uid)
		return;
	map<UINT32, userInfo, less<UINT32> >::iterator it = m_UserInfoTbl.find(uid);
	if(m_UserInfoTbl.end()!=it) 
	{
		(*it).second.grpUserInfoTbl.clear();
		m_UserInfoTbl.erase(it);
	}
}
void CSAG::showACL(UINT32 uid)
{
	VPRINT("\nshowXXX(AAA), AAA=0 means showAllXXX\n");
	map<UINT32, UINT32 >::iterator itUID;
	//show ALL
	if(0==uid || INVALID_UID==uid)
	{
		VPRINT("\n====================================================================");
		VPRINT("\n----------ACL Size[%d]  Format[ UID , PID ]-------", m_AccessLst.size());
		int count = 0;
		for(itUID=m_AccessLst.begin();itUID!=m_AccessLst.end();itUID++)
		{
			if(!(count++ & 0x03))
			{
				VPRINT("\n");
			}
			VPRINT("0x%08X,0x%08X|", 
					(*itUID).first, (*itUID).second);
		}
		VPRINT("\n====================================================================\n");		
	}
	//show the one wanted
	else
	{
		itUID = m_AccessLst.find(uid);
		if(itUID!=m_AccessLst.end())
		{
			VPRINT("\n [ 0x%08X , 0x%08X ] in ACL...\n", 
					(*itUID).first, (*itUID).second);

		}
		else
		{
			VPRINT("\nUID[0x%08X] not in ACL...\n", uid);
		}
	}
}
UINT32 CSAG::saveACLToFile(char *pBuf, UINT32 *pSize)
{
	if(NULL==pBuf || NULL==pSize)
		return 0;
	char *pNow = pBuf;
	UINT32 nLen = 0;
	nLen = sprintf(pNow, "PID,UID\n");
	pNow += nLen;
	map<UINT32, UINT32 >::iterator itUID;
	for(itUID=m_AccessLst.begin();itUID!=m_AccessLst.end();itUID++)
	{
		nLen = sprintf(pNow, "0x%08X,0x%08X\n", 
				(*itUID).second, (*itUID).first);
		pNow +=nLen;
	}	
	return (*pSize = (pNow-pBuf+1));
}
void CSAG::clearACL()
{
	m_AccessLst.clear();
}
void CSAG::initACLFromFile(char *pBuf, UINT32 size)
{
	m_AccessLst.clear();
	
	//long lines=0;
	long lineLen=0;
	long spaceLen=0;
	char line[256];
	long LeftLen=size;
	char *pReadBuf = pBuf;
	char *pEnd = pBuf + size;
	while(pReadBuf<pEnd)
	{
		pReadBuf = jumpSpaces(pReadBuf, LeftLen, &spaceLen);
		LeftLen -= spaceLen;
		if(0==pReadBuf[0])
		{
			return;
		}
		if(1==sscanf(pReadBuf,"%s",line))
		{
			lineLen = strlen(line);
			LeftLen -= lineLen;
			pReadBuf += lineLen;
			//VPRINT("\n[%d] line[%s] len[%d]", ++lines, line, lineLen);
			//PID,UID
			UINT32 pid,uid;
			if(2==sscanf(line, "%x,%x", &pid, &uid))
			{
				addACLUser(uid, pid);
			}
			else
			{
				//skip useless lines
			}
		}
		else
		{
			//error
		}	
	}
}
extern bool sagStatusFlag;
bool CSAG::ifAllowUserAccess(UINT32 pid, UINT32 uid)
{
	//使用用户列表文件意味着故障弱化时只允许文件中的用户接入
	if(g_blUseUserListFile)
	{
		if(!sagStatusFlag)
		{
			bool ret = isUserInACL(uid, pid);
			if(ret)
			{
				return true;
			}
			else
			{
				LOG2(LOG_DEBUG3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"PID[0x%08X] UID[0x%08X] not in ACL when using localSag.", pid, uid);
				return false;
			}
		}
	}
	return true;
}
extern "C" bool ifCpeCanAccess(UINT32 pid, UINT32 uid)
{
	return CSAG::getSagInstance()->ifAllowUserAccess(pid, uid);
}
void CSAG::initUserInfoFromFile(char *pBuf, UINT32 size)
{
	clearAllUserInfo();
	
	//long lines=0;
	long lineLen=0;
	long spaceLen=0;
	char line[256];
	long LeftLen=size;
	char *pReadBuf = pBuf;
	char *pEnd = pBuf + size;
	while(pReadBuf<pEnd)
	{
		pReadBuf = jumpSpaces(pReadBuf, LeftLen, &spaceLen);
		LeftLen -= spaceLen;
		if(0==pReadBuf[0])
		{
			return;
		}		
		if(1==sscanf(pReadBuf,"%s",line))
		{
			lineLen = strlen(line);
			LeftLen -= lineLen;
			pReadBuf += lineLen;
			//VPRINT("\n[%d] line[%s] len[%d]", ++lines, line, lineLen);
			//PID,UID,电话号码,个呼优先级
			char sep[]=",";
			UINT32 pid,uid;
			UINT8 prio;
			char telNO[M_MAX_PHONE_NUMBER_LEN];
			char *pNext = NULL;
			char *pItem = NULL;
			pItem = own_strtok_r(line, sep, &pNext);
			pid = strtoul(pItem, NULL, 16);
			if(0==pid || 0xffffffff==pid)
				continue;
			pItem = own_strtok_r(NULL, sep, &pNext);
			uid = strtoul(pItem, NULL, 16);
			if(0==uid || 0xffffffff==uid)
				continue;
			pItem = own_strtok_r(NULL, sep, &pNext);
			strcpy(telNO, pItem);
			pItem = own_strtok_r(NULL, sep, &pNext);
			prio = strtoul(pItem, NULL, 16) & 0xff;
			//add user info
			addUserInfo(uid, pid, telNO, prio);
		}
		else
		{
			//error or end
		}	
	}	
}

void CSAG::addUserGrpInfo(UINT32 uid, UINT16 gid, UINT8 prioInGrp)
{
	if(INVALID_UID==uid ||0==uid || M_INVALID_GID==gid || 0==gid)
		return ;
	map<UINT16, grpInfo, less<UINT16> >::iterator itFoundGrp;
	if(!findGrpInfoByGID(gid, itFoundGrp))
	{
		LOG2(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"GrpInfo not found when addUserGrpInfo {uid[0x%08X] gid[0x%04X]}!!!",
				 uid, gid);	
		return;
	}
	map<UINT32, userInfo, less<UINT32> >::iterator itFound;
	if(findUserInfoByUID(uid, itFound))
	{
		map<UINT16,grpUserInfo,less<UINT16> >::iterator it = (*itFound).second.grpUserInfoTbl.find(gid);
		if((*itFound).second.grpUserInfoTbl.end()!=it)
		{
			LOG4(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"{uid[0x%08X] gid[0x%04X]} already in grpUserInfoTbl when adding {uid[0x%08X] gid[0x%04X]}!!!",
				(*itFound).second.uid, (*it).second.gid, uid, gid);
		}
		else
		{
			grpUserInfo grpUserInfoItem(gid, prioInGrp);
			(*itFound).second.grpUserInfoTbl.insert(map<UINT16, grpUserInfo, less<UINT16> >::value_type(gid,grpUserInfoItem));
		}
	}
	else
	{
		LOG2(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"userInfo not found when addUserGrpInfo {uid[0x%08X] gid[0x%04X]}!!!",
				 uid, gid);		
	}	
}
void CSAG::delUserGrpInfo(UINT32 uid, UINT16 gid)
{
	map<UINT32, userInfo, less<UINT32> >::iterator itFound;
	if(findUserInfoByUID(uid, itFound))
	{
		map<UINT16,grpUserInfo,less<UINT16> >::iterator it = (*itFound).second.grpUserInfoTbl.find(gid);
		if((*itFound).second.grpUserInfoTbl.end()!=it)
		{
			(*itFound).second.grpUserInfoTbl.erase(it);
		}
		else
		{
			LOG2(LOG_DEBUG3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"delUserGrpInfo,uid[0x%08X] gid[0x%04X], user not in group!!!",
				uid, gid);
		}
	}
	else
	{
		LOG2(LOG_DEBUG3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"userInfo not found when delUserGrpInfo {uid[0x%08X] gid[0x%04X]}!!!",
				 uid, gid);	
	}
}
void CSAG::clearUserGrpInfo(UINT32 uid)
{
	map<UINT32, userInfo, less<UINT32> >::iterator itFound;
	if(findUserInfoByUID(uid, itFound))
	{
		(*itFound).second.grpUserInfoTbl.clear();
	}
}
bool CSAG::findGrpInfoByGID(UINT16 gid, map<UINT16, grpInfo, less<UINT16> >::iterator& itFound)
{
	map<UINT16, grpInfo, less<UINT16> >::iterator it = m_GrpInfoTbl.find(gid);
	if(m_GrpInfoTbl.end()==it) 
	{
		return false;
	}
	else
	{
		itFound = it;
		return true;
	}
}
void CSAG::addGrpInfo(UINT16 gid, char* grpName, UINT8 grpPrio)
{
	if(M_INVALID_GID==gid || 0==gid)
		return;
	map<UINT16, grpInfo, less<UINT16> >::iterator itFound;
	if(!findGrpInfoByGID(gid, itFound))
	{
		grpInfo GrpInfo(gid, grpPrio, grpName);
		m_GrpInfoTbl.insert(map<UINT16,grpInfo,less<UINT16> >::value_type(gid, GrpInfo));
	}
	else
	{
		LOG4(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			"{gid[0x%08X] grpPrio[0x%08X] } already in ACL when adding {gid[0x%08X] grpPrio[0x%08X] }!!!",
			(*itFound).second.gid, (*itFound).second.grpPrio, gid, grpPrio);
	}	
}
void CSAG::delGrpInfo(UINT16 gid)
{
	if(M_INVALID_GID==gid || 0==gid)
		return;
	map<UINT16, grpInfo, less<UINT16> >::iterator it = m_GrpInfoTbl.find(gid);
	if(m_GrpInfoTbl.end()!=it) 
	{
		m_GrpInfoTbl.erase(it);
	}	
}
void CSAG::initGrpInfoFromFile(char *pBuf, UINT32 size)
{
	m_GrpInfoTbl.clear();
	
	//long lines=0;
	long lineLen=0;
	long spaceLen=0;
	char line[256];
	long LeftLen=size;
	char *pReadBuf = pBuf;
	char *pEnd = pBuf + size;
	while(pReadBuf<pEnd)
	{
		pReadBuf = jumpSpaces(pReadBuf, LeftLen, &spaceLen);
		LeftLen -= spaceLen;
		if(0==pReadBuf[0])
		{
			return;
		}		
		if(1==sscanf(pReadBuf,"%s",line))
		{
			lineLen = strlen(line);
			LeftLen -= lineLen;
			pReadBuf += lineLen;
			//VPRINT("\n[%d] line[%s] len[%d]", ++lines, line, lineLen);
			//GID,GrpName,组优先级
			char sep[]=",";
			UINT16 gid;
			UINT8 prio;
			char grpName[20];
			char *pNext = NULL;
			char *pItem = NULL;
			pItem = own_strtok_r(line, sep, &pNext);
			gid = strtoul(pItem, NULL, 16);
			if(0==gid || 0xffff==gid)
				continue;
			pItem = own_strtok_r(NULL, sep, &pNext);
			strcpy(grpName, pItem);
			pItem = own_strtok_r(NULL, sep, &pNext);
			prio = strtoul(pItem, NULL, 16) & 0xff;
			//add grp info
			grpInfo GrpInfo(gid, prio, grpName);
			m_GrpInfoTbl.insert(map<UINT16,grpInfo,less<UINT16> >::value_type(gid, GrpInfo));
		}
		else
		{
			//error or end
		}	
	}	
}
void CSAG::initGrpUserInfoFromFile(char *pBuf, UINT32 size)
{
	//long lines=0;
	long lineLen=0;
	long spaceLen=0;
	char line[256];
	long LeftLen=size;
	char *pReadBuf = pBuf;
	char *pEnd = pBuf + size;
	while(pReadBuf<pEnd)
	{
		pReadBuf = jumpSpaces(pReadBuf, LeftLen, &spaceLen);
		LeftLen -= spaceLen;
		if(0==pReadBuf[0])
		{
			return;
		}		
		if(1==sscanf(pReadBuf,"%s",line))
		{
			lineLen = strlen(line);
			LeftLen -= lineLen;
			pReadBuf += lineLen;
			//VPRINT("\n[%d] line[%s] len[%d]", ++lines, line, lineLen);
			//GID,UID,组内优先级
			char sep[]=",";
			UINT16 gid;
			UINT32 uid;
			UINT8 prio;
			char *pNext = NULL;
			char *pItem = NULL;
			pItem = own_strtok_r(line, sep, &pNext);
			gid = strtoul(pItem, NULL, 16);
			if(0==gid || 0xffff==gid)
				continue;
			pItem = own_strtok_r(NULL, sep, &pNext);
			uid = strtoul(pItem, NULL, 16);
			if(0==uid || 0xffffffff==uid)
				continue;
			pItem = own_strtok_r(NULL, sep, &pNext);
			prio = strtoul(pItem, NULL, 16) & 0xff;
			//add user-grp info
			addUserGrpInfo(uid, gid, prio);
		}
		else
		{
			//error or end
		}	
	}	
}
bool CSAG::ifAllowGrpSetup(UINT16 gid, UINT32 uid)
{
	if(g_blUseLocalGrpInfoFile)
	{
		//gid in gidList
		map<UINT16, grpInfo, less<UINT16> >::iterator itFound;
		if(findGrpInfoByGID(gid, itFound))
		{
			if(g_blUseLocalUserInfoFile)
			{
				//uid belongs to gid
				if(isUserInGroup(uid, gid))
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return true;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return true;
	}
}
bool CSAG::isUserInGroup(UINT32 uid, UINT16 gid)
{
	if(g_blUseLocalUserInfoFile)
	{
		map<UINT32, userInfo, less<UINT32> >::iterator itFound;
		if(findUserInfoByUID(uid, itFound))
		{
			map<UINT16,grpUserInfo,less<UINT16> >::iterator it = (*itFound).second.grpUserInfoTbl.find(gid);
			if((*itFound).second.grpUserInfoTbl.end()!=it)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}		
	}
	else
	{
		return true;
	}
}
UINT16 CSAG::formatUserGrpListInfo(char*buf, UINT16& len, UINT8& grpNum, UINT32 uid)
{
	len = 0;
	grpNum = 0;
	if(NULL==buf)
	{
		return 0;
	}
	if(0==uid || INVALID_UID==uid)
	{
		len = 0;
		return 0;
	}
	map<UINT32, userInfo, less<UINT32> >::iterator itFound;
	if(findUserInfoByUID(uid, itFound))
	{
		grpNum = (*itFound).second.grpUserInfoTbl.size();
		map<UINT16, grpUserInfo>::iterator itGID;
		for(itGID=(*itFound).second.grpUserInfoTbl.begin();itGID!=(*itFound).second.grpUserInfoTbl.end();itGID++)
		{
			UINT16 tmpGid = (*itGID).first;
			map<UINT16, grpInfo, less<UINT16> >::iterator itGrpFound;
			UINT8 GrpNameLen;

			//GIDx
			VSetU16BitVal((UINT8*)&buf[len], tmpGid);
			len += sizeof(UINT16);
			//GNAMEx & grpPrio
			UINT16 tmpGrpPrio;
			if(findGrpInfoByGID(tmpGid, itGrpFound))
			{
				GrpNameLen = (*itGrpFound).second.grpName.size()+1;
				strcpy(&buf[len+1], (*itGrpFound).second.grpName.c_str());
				tmpGrpPrio = (*itGrpFound).second.grpPrio;
			}
			else
			{
				GrpNameLen = 1;
				tmpGrpPrio = 7;//最低组优先级，避免异常
			}
			buf[len] = GrpNameLen;
			len++;
			buf[len+GrpNameLen-1]=0;
			len+=GrpNameLen;
			//PRIOx
			buf[len]=tmpGrpPrio;
			len++;
		}		
		return len;
	}
	else
	{
		len = 0;
		return 0;
	}
}
void CSAG::clearAllUserInfo()
{
	map<UINT32, userInfo>::iterator itUID;
	map<UINT16, grpUserInfo>::iterator itGID;
	for(itUID=m_UserInfoTbl.begin();itUID!=m_UserInfoTbl.end();itUID++)
	{
		(*itUID).second.grpUserInfoTbl.clear();
	}
	m_UserInfoTbl.clear();
}
void CSAG::clearAllGrpInfo()
{
	m_GrpInfoTbl.clear();
}
void CSAG::showUserInfo(UINT32 uid)
{
	VPRINT("\nshowXXX(AAA), AAA=0 means showAllXXX\n");
	
	map<UINT32, userInfo>::iterator itUID;
	map<UINT16, grpUserInfo>::iterator itGID;
	//show ALL
	if(0==uid || INVALID_UID==uid)
	{
		VPRINT("\n====================================================================");
		VPRINT("\n--UserInfoTable Size[%d]  Format[UID,PID,telNO,prio,GrpInfo(GID1,prio1;GID2,prio2;...) ]--", m_UserInfoTbl.size());
		for(itUID=m_UserInfoTbl.begin();itUID!=m_UserInfoTbl.end();itUID++)
		{
			VPRINT("\n[0x%08X,0x%08X,\"%s\",0x%02X,GrpInfo(", 
					(*itUID).first, (*itUID).second.pid, (*itUID).second.telNO.c_str(), (*itUID).second.prio);
			for(itGID=(*itUID).second.grpUserInfoTbl.begin();itGID!=(*itUID).second.grpUserInfoTbl.end();itGID++)
			{
				VPRINT("0x%04X,0x%02X;", (*itGID).first, (*itGID).second.prio);
			}
			VPRINT(")]");
		}
		VPRINT("\n====================================================================\n");		
	}
	//show the one wanted
	else
	{
		itUID = m_UserInfoTbl.find(uid);
		if(itUID!=m_UserInfoTbl.end())
		{
			VPRINT("\n [ 0x%08X, 0x%08X, \"%s\", 0x%02X, GrpInfo(", 
					(*itUID).first, (*itUID).second.pid, (*itUID).second.telNO.c_str(), (*itUID).second.prio);
			for(itGID=(*itUID).second.grpUserInfoTbl.begin();itGID!=(*itUID).second.grpUserInfoTbl.end();itGID++)
			{
				VPRINT("0x%04X,%02X;", (*itGID).first, (*itGID).second.prio);
			}
			VPRINT(") ]");
		}
		else
		{
			VPRINT("\nUID[0x%08X] not in UserInfoTable...\n", uid);
		}
	}	
}
void CSAG::showGrpInfo(UINT16 gid)
{
	VPRINT("\nshowXXX(AAA), AAA=0 means showAllXXX\n");
	
	map<UINT16, grpInfo>::iterator itGID;
	//show ALL
	if(0==gid || M_INVALID_GID==gid)
	{
		VPRINT("\n====================================================================");
		VPRINT("\n----------GrpInfoTable Size[%d]  Format[ GID, GrpName, prio ]-------", m_GrpInfoTbl.size());
		for(itGID=m_GrpInfoTbl.begin();itGID!=m_GrpInfoTbl.end();itGID++)
		{
			VPRINT("\n [ 0x%04X, \"%s\", 0x%02X ]", 
					(*itGID).first, (*itGID).second.grpName.c_str(), (*itGID).second.grpPrio);
		}
		VPRINT("\n====================================================================\n");		
	}
	//show the one wanted
	else
	{
		itGID = m_GrpInfoTbl.find(gid);
		if(itGID!=m_GrpInfoTbl.end())
		{
			VPRINT("\n [ 0x%04X, \"%s\", 0x%02X ]", 
					(*itGID).first, (*itGID).second.grpName.c_str(), (*itGID).second.grpPrio);
		}
		else
		{
			VPRINT("\nGID[0x%04X] not in GrpInfoTable...\n", gid);
		}
	}	
}	
UINT32 CSAG::saveGrpInfoToFile(char *pBuf, UINT32 *pSize)
{
	if(NULL==pBuf || NULL==pSize)
		return 0;
	char *pNow = pBuf;
	UINT32 nLen = 0;
	nLen = sprintf(pNow, "GID,GrpName,GrpPiro\n");
	pNow += nLen;
	map<UINT16, grpInfo>::iterator itGID;
	for(itGID=m_GrpInfoTbl.begin();itGID!=m_GrpInfoTbl.end();itGID++)
	{
		if( (*itGID).second.grpName.size()>0 )
		{
			nLen = sprintf(pNow, "0x%04X,%s,0x%02X\n", 
					(*itGID).second.gid, (*itGID).second.grpName.c_str(), (*itGID).second.grpPrio);
		}
		else
		{
			nLen = sprintf(pNow, "0x%04X,0x%04X,0x%02X\n", 
					(*itGID).second.gid, (*itGID).second.gid, (*itGID).second.grpPrio);
		}
		pNow +=nLen;
	}
	return (*pSize = (pNow-pBuf+1));
}
UINT32 CSAG::saveUserInfoToFile(char *pBuf, UINT32 *pSize)
{
	if(NULL==pBuf || NULL==pSize)
		return 0;
	char *pNow = pBuf;
	UINT32 nLen = 0;
	nLen = sprintf(pNow, "PID,UID,TelNO,Piro\n");
	pNow += nLen;
	map<UINT32, userInfo>::iterator itUID;
	for(itUID=m_UserInfoTbl.begin();itUID!=m_UserInfoTbl.end();itUID++)
	{
		nLen = sprintf(pNow, "0x%08X,0x%08X,%s,0x%02X\n", 
				(*itUID).second.pid, (*itUID).second.uid, (*itUID).second.telNO.c_str(), (*itUID).second.prio);
		pNow +=nLen;
	}	
	return (*pSize = (pNow-pBuf+1));
}
UINT32 CSAG::saveUserGrpInfoToFile(char *pBuf, UINT32 *pSize)
{
	if(NULL==pBuf || NULL==pSize)
		return 0;
	char *pNow = pBuf;
	UINT32 nLen = 0;
	nLen = sprintf(pNow, "GID,UID,Piro \n");
	pNow += nLen;
	map<UINT32, userInfo>::iterator itUID;
	for(itUID=m_UserInfoTbl.begin();itUID!=m_UserInfoTbl.end();itUID++)
	{
		map<UINT16, grpUserInfo>::iterator itGID;
		for(itGID=(*itUID).second.grpUserInfoTbl.begin();itGID!=(*itUID).second.grpUserInfoTbl.end();itGID++)
		{
			nLen = sprintf(pNow, "0x%04X,0x%08X,0x%02X\n", 
					(*itGID).second.gid, (*itUID).first, (*itGID).second.prio);
			pNow +=nLen;
		}		
	}
	return (*pSize = (pNow-pBuf+1));
}



