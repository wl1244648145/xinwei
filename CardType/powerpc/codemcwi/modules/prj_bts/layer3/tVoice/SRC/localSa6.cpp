/*******************************************************************************
* Copyright (c) 2009 by Beijing AP Co.Ltd.All Rights Reserved   
* File Name      : localSagFsmProc.cpp
* Create Date    : 14-Oct-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "voiceTone.h"
#include "cpe_signal_struct.h"
#include "localSagDialPlan.h"
#include "localSagFsmCfg.h"
#include "localSag.h"
#include "localSagCfg.h"
#include "localSagStruct.h"
#include "voiceToolFunc.h"

SAG_CPE_STATE IDLE__ULSignal__AssignResReq__Proc(CMessage& msg, CCCB& ccb)
{
	//�л�Ӧ�ò�����״̬��,��handleXXX�����д���
	//��������L3Addr�򽲻���L3Addr
	UINT32 L3Addr = CSAG::getSagInstance()->AllocateL3Addr();
	ccb.setL3Addr(L3Addr);
	CSAG::getSagInstance()->AddL3AddrIndexTable(L3Addr, &ccb);
	//��Ӧ�ɹ���AssignResRsp
	ccb.sendDLSignalAssignResRsp(msg);
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)msg.GetDataPtr();
	UINT16 srvOpt = VGetU16BitVal(pSigBufRcv->sigPayload.AssignResReq.ServerOption);
	if(M_LOCALSAG_ASSIGNRES_REASON_GRP_TALKING==srvOpt)
	{
		//find GrpCCB
		SxcGrpCCB *pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(ccb.getGID());
		if(NULL!=pGrpCCB)
		{
			//GrpCCB stopTimer TIMERID_SAG_GRP_ASSIGNRESREQ
			pGrpCCB->stopGrpTimer(TIMERID_SAG_GRP_ASSIGNRESREQ, &pGrpCCB->m_pTmGrpAssignResReq);
		}
		//���������ֱ�ӽ��뽲��״̬
		return SAG_GRP_TALKING_STATE;
	}
	//������ʱ��TOSetup,�ȴ�����setup�����PttSetup����
	ccb.startTimer(TIMERID_SAG_O_SETUP);
	//����SAG_O_SETUP_STATE
	return SAG_O_SETUP_STATE;
}
SAG_CPE_STATE IDLE__INSignal__Setup__Proc(CMessage& msg, CCCB& ccb)
{
	//������������Ϣ�Զ�ccb
	InnerSignal_CallArriveT* pData = (InnerSignal_CallArriveT*)msg.GetDataPtr();
	UINT32 peerUID = VGetU32BitVal(pData->srcUID);
	CCCB* pPeerCCB = CSAG::getSagInstance()->FindCCBByUID(peerUID);
	if(pPeerCCB==NULL)
	{
		LOG3(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			"cannot find src CCB!!!InnerSignal CallArrive  , dstUID[0x%08X] srcUID[0x%08X] L3Addr[0x%08X] ", 
			VGetU32BitVal(pData->dstUID),
			VGetU32BitVal(pData->srcUID),
			VGetU32BitVal(pData->srcL3Addr));
		return SAG_IDLE_STATE;
	}
	//�Ƿ���ܺ���
	ccb.setEncryptCtrl(pPeerCCB->getEncryptCtrl());
	ccb.setPeerCCB(pPeerCCB);
	ccb.setOrigCall(false);
	//���䱻��L3Addr
	UINT32 L3Addr = CSAG::getSagInstance()->AllocateL3Addr();
	ccb.setL3Addr(L3Addr);
	CSAG::getSagInstance()->AddL3AddrIndexTable(L3Addr, &ccb);
	//����Ѱ����bts
	UINT16 appType;
	if(ENCRYPT_CTRL_NOTUSE==ccb.getEncryptCtrl())
	{
		appType = APPTYPE_VOICE_QCELP;
	}
	else
	{
		appType = APPTYPE_ENCRYPT_VOICE;
	}
	ccb.sendDLSignalLAPaging(appType);
	//����Ѱ����ʱ��ʱ��TlapagingRsp�ȴ���������LaPagingRsp
	ccb.startTimer(TIMERID_SAG_T_LAPAGING);
	//����SAG_PAGING_STATE;
	return SAG_PAGING_STATE;
}
SAG_CPE_STATE O_SETUP__ULSignal__PttSetupReq__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOSetup
	ccb.deleteTimer(TIMERID_SAG_O_SETUP);
	SAbisSignalT *pSigData = (SAbisSignalT*)msg.GetDataPtr();
	PTTSetupT_CPE *pDataCpe = (PTTSetupT_CPE*)(&pSigData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
	UINT16 gid = VGetU16BitVal(pDataCpe->GID);
	ccb.setGID(gid);
	//��bts��ӦPttSetupAck
	ccb.sendDLSignalPttSetupAck(pDataCpe->EncryptFlag, pDataCpe->CallPriority);
	ccb.setEncryptCtrl(pDataCpe->EncryptFlag);
	ccb.setPrio(pDataCpe->CallPriority);
	SxcGrpCCB *pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(gid);
	//�ж��û��Ƿ���Ȩ�޽������
	if(!CSAG::getSagInstance()->ifAllowGrpSetup(gid, ccb.getUID()))
	{
		ccb.sendDLSignalGrpCallingRls(REL_CAUSE_USRSRVNOTALLOW);
		ccb.sendDLSignalRlsTransResReq(REL_CAUSE_NORMAL);
		return SAG_IDLE_STATE;
	}
	//��������
	if(pGrpCCB!=NULL)
	{
#ifdef M__SUPPORT__ENC_RYP_TION	
		//����漰��JM��ܾ��˴��齨��
		if(pDataCpe->EncryptFlag || pGrpCCB->getEncryptCtrl())
		{
			LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"refuse GrpCalling...");
			//���ն˷���GrpCallingRls�����ͷ�������У�ԭ��Ϊ�û�æ
			ccb.sendDLSignalGrpCallingRls(REL_CAUSE_PEER_BUSY);
			//��bts����releaseResReq��Ϣ
			CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
			//����״̬SAG_IDLE_STATE
			return SAG_IDLE_STATE;
		}
#endif
		//��������
		if(!pGrpCCB->isSomeoneTalking())
		{
			//���軰Ȩ
			pGrpCCB->grantPttToUser(ccb.getUID());
			//���ն˷���PttConnect����
			ccb.sendDLSignalPttConnect(NOT_CALL_OWNER, pGrpCCB->getEncryptCtrl(), pGrpCCB->getPrio());
			//����״̬SAG_GRP_TALKING_STATE
			return SAG_GRP_TALKING_STATE;
		}
		//�н�����
		else
		{
			//���ն˷���GrpCallingRls�����ͷ�������У�ԭ��Ϊ�û�æ
			ccb.sendDLSignalGrpCallingRls(REL_CAUSE_PEER_BUSY);
			//��bts����releaseResReq��Ϣ
			CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
			//����״̬SAG_IDLE_STATE
			return SAG_IDLE_STATE;
		}
	}
	//�鲻����
	else
	{
		//AllocGrpCCB
		pGrpCCB = CSAG::getSagInstance()->AllocGrpCCB(gid);
		if(NULL==pGrpCCB)
		{
			ccb.sendDLSignalGrpCallingRls(REL_CAUSE_UNKNOWN);
			ccb.sendDLSignalRlsTransResReq(REL_CAUSE_NORMAL);
			return SAG_IDLE_STATE;
		}
		//��¼commType
		pGrpCCB->setCommType(pDataCpe->CommType);
		//��¼�Ƿ����
		pGrpCCB->setEncryptCtrl(pDataCpe->EncryptFlag);
#ifdef M__SUPPORT__ENC_RYP_TION
		//JMKEY
		if(pDataCpe->EncryptFlag)
		{
			pGrpCCB->setEncryptKey(pDataCpe->EncryptKey);
		}
#endif
		//��¼���ȼ�
		pGrpCCB->setPrio(pDataCpe->CallPriority);
		//MaybeFinishLater,Ҳ���Ժ��������д�������ȼ�
		//������������
		pGrpCCB->setGrpSetupUID(ccb.getUID());
		//��bts����LAGrpPaging
		pGrpCCB->sendDLSignalLAGrpPaging();
		pGrpCCB->setPagingUserFlag(true);
		//������ʱ��TIMERID_SAG_GRP_LAGRPPAGING
		pGrpCCB->startGrpTimer(TIMERID_SAG_GRP_LAGRPPAGING, &pGrpCCB->m_pTmGrpPaging);
		//������ʱ��TIMERID_SAG_GRP_PRESSINFO
		pGrpCCB->startGrpTimer(TIMERID_SAG_GRP_PRESSINFO, &pGrpCCB->m_pTmGrpPressInfo);
		//������ʱ��TIMERID_SAG_GRP_PTTCONNECT
		ccb.startTimer(TIMERID_SAG_GRP_PTTCONNECT);
		//����״̬SAG_GRP_CALLSETUP_STATE
		return SAG_GRP_CALLSETUP_STATE;
	}
}
SAG_CPE_STATE O_SETUP__ULSignal__Setup__Proc(CMessage& msg, CCCB& ccb)
{
	CCCB* pCCBCalled = NULL;
	//��Ӧ����SetupAck��bts
	ccb.sendDLSignalSetupAck();
	//ֹͣ��ʱ��TOSetup
	ccb.deleteTimer(TIMERID_SAG_O_SETUP);
	ccb.setOrigCall(true);
	//�Ʒ����
	T_TimeDate dt = bspGetDateTime();
	ccb.m_cdr.setStartDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
	//���setup������Я���˱��к���
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)msg.GetDataPtr();
	SetupUTT_CPE* pDataSetup = (SetupUTT_CPE*)(&pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
	//�Ƿ����
	if(SETUPCAUSE_ENCRYPT_VOICE==pDataSetup->SetupCause)
	{
		ccb.setEncryptCtrl(ENCRYPT_VOICE);
	}
	else
	{
		ccb.setEncryptCtrl(ENCRYPT_CTRL_NOTUSE);
	}
	//setupCause
	ccb.setSetupCause(pDataSetup->SetupCause);
	UINT8* pDataBegin = (UINT8*)pSigBufRcv;
	bool blWithCalledNumber=false;
	if( (pDataSetup->Codeclist+pDataSetup->Codeclist[0]+1 ) < 
		(pDataBegin+msg.GetDataLength()) )
	{
		blWithCalledNumber = true;
	}
	if(blWithCalledNumber)
	{
		void *pNO=pDataSetup->Codeclist+pDataSetup->Codeclist[0]+1;
		//���汻�к���
		ccb.setDialedNumberLen(convertDigitNO2Str(pNO, ccb.getDialedNumber()));
		bool blFound=false;
		UINT8 bFetId=0;
		UINT8 bAddDgts=0;
		UINT8 bParseResult = parseNumberDialed(ccb.getDialedNumberLen(), ccb.getDialedNumber(), bFetId, bAddDgts);
		//������к�����ϲ��żƻ����ұ����ڱ�sag(bts)��
		if(bParseResult==DIAL_COMPLETE)
		{
			//���ұ����Ƿ��ڱ�bts
			char *pCalledNumber = ccb.getDialedNumber();
			char pCalledNumberWithAreaCode[M_MAX_PHONE_NUMBER_LEN*2];
			pCCBCalled = CSAG::getSagInstance()->FindCCBByLocalNumber(pCalledNumber);
			//û�ҵ�
			if(pCCBCalled==NULL)
			{
				 if(pCalledNumber[0]=='0')
				 {
				 	//���к�������ţ���������������
				 	//���ش洢����������������ü���������(�����л�����)
				 	//������ش洢���벻�����ţ����򱻽�ʱҲ��Ӧ�ü�����(�ƶ��ֻ�����)
				 }
				 else
				 {
				 	//���к��벻�����ţ�������������һ��
				 	//���ش洢������������������������һ���п����ҵ�(�����л�����)
				 	//������ش洢���벻�����ţ�����һ�ο϶��Ҳ������˷�һ���ʱ�����(�ƶ��ֻ�����)
				 	strcpy(pCalledNumberWithAreaCode, getAreaCode());
					//������Ч
					if(pCalledNumber[0]!=0)
					{
						strcat(pCalledNumberWithAreaCode, ccb.getDialedNumber());
						pCCBCalled = CSAG::getSagInstance()->FindCCBByLocalNumber(pCalledNumberWithAreaCode);
						if(pCCBCalled!=NULL)
						{
							blFound = true;
						}
					}
				 }
			}
			else
			{
				blFound = true;
			}
		}

		if(blFound)
		{
			//������в���SAG_IDLE_STATE
			if(SAG_IDLE_STATE!=pCCBCalled->getState())
			{
				//��ʼ��æ��
				//���ն˷���Disconnect����
				ccb.sendDLSignalDisconnect(REL_CAUSE_PEER_BUSY);
				//�����ȴ��û��һ���ʱ��TOnhook
				ccb.startTimer(TIMERID_SAG_DISCONNECT);
				//����SAG_WAITONHOOK_STATE	
				return SAG_WAIT_ONHOOK_STATE;
			}
			else
			{
				//������п��������ڲ�����CallArrive������
				ccb.setPeerCCB(pCCBCalled);
				ccb.sendInnerSignalCallArrive(pCCBCalled->getUID());
				//������ʱ��TOAlerting�ȴ����л�Alerting
				ccb.startTimer(TIMERID_SAG_O_ALERTING);
				//����SAG_O_ALERTING_STATE
				return SAG_O_ALERTING_STATE;
			}
		}
		else
		{
			//������к��벻���ϲ��żƻ��򱻽в��ڱ�sag(bts)����ʼ��æ����
			ccb.sendDLSignalDisconnect(REL_CAUSE_SAG);
			//�����ȴ��û��һ���ʱ��TOnhook
			ccb.startTimer(TIMERID_SAG_DISCONNECT);
			//����SAG_WAITONHOOK_STATE
			return SAG_WAIT_ONHOOK_STATE;
		}
	}
	else
	{
		//setup������δЯ�����к���
		//���ն˷Ų�����
		ccb.playTone(DIAL_TONE);
		ccb.setDialToneStoppedFlag(false);
		//�������ų�ʱ��ʱ��TDial,
		ccb.startTimer(TIMERID_SAG_O_DIALNUMBER);
		//����SAG_DIAL_STATE
		return SAG_DIAL_STATE;
	}	
}
SAG_CPE_STATE O_SETUP__Timeout__OSetup__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOSetup
	ccb.deleteTimer(TIMERID_SAG_O_SETUP);
	//�����������æ��ֱ��֪ͨbts����տ���·(�������������������������ݲ�֧�֣�֧����Ҫ����״̬)
	//��bts����release transport resource req
	ccb.sendDLSignalRlsTransResReq(REL_CAUSE_TIMERTIMEOUT);
	//����SAG_IDLE_STATE	
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE O_SETUP__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOSetup
	ccb.deleteTimer(TIMERID_SAG_O_SETUP);
	//��bts��ӦerrNotiRsp����,����״̬��ǰ�Ѿ��Զ��ظ�����Ӧ
	//����SAG_IDLE_STATE	
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE O_SETUP__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOSetup
	ccb.deleteTimer(TIMERID_SAG_O_SETUP);
	//��bts����release transport resource req
	ccb.sendDLSignalRlsTransResReq(REL_CAUSE_SAG);
	//����SAG_IDLE_STATE	
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE DIAL__ULSignal__DialNumber__Proc(CMessage& msg, CCCB& ccb)
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)msg.GetDataPtr();
	InformationT_CPE* pDataCPE = (InformationT_CPE*)(&pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
	if(0==pDataCPE->type)//������Ϣ
	{
		//ֹͣ��ʱ��TDial
		ccb.deleteTimer(TIMERID_SAG_O_DIALNUMBER);
		//�״β���ͣ������
		if(!ccb.isDialToneStopped())
		{
			ccb.stopTone(DIAL_TONE);
			ccb.setDialToneStoppedFlag(true);
		}
		//�������
		ccb.SaveDialedNumber(pDataCPE->digi_No.num & 0x0f);	
		CCCB* pCCBCalled = NULL;
		bool blFound=false;
		UINT8 bFetId=0;
		UINT8 bAddDgts=0;
		UINT8 bParseResult = parseNumberDialed(ccb.getDialedNumberLen(), ccb.getDialedNumber(), bFetId, bAddDgts);
		//������к�����ϲ��żƻ����ұ����ڱ�sag(bts)��
		if(bParseResult==DIAL_COMPLETE)
		{
			//���ұ����Ƿ��ڱ�bts
			char *pCalledNumber = ccb.getDialedNumber();
			char pCalledNumberWithAreaCode[M_MAX_PHONE_NUMBER_LEN*2];
			pCCBCalled = CSAG::getSagInstance()->FindCCBByLocalNumber(pCalledNumber);
			//û�ҵ�
			if(pCCBCalled==NULL)
			{
				 if(pCalledNumber[0]=='0')
				 {
				 	//���к�������ţ���������������
				 	//���ش洢����������������ü���������(�����л�����)
				 	//������ش洢���벻�����ţ����򱻽�ʱҲ��Ӧ�ü�����(�ƶ��ֻ�����)
				 }
				 else
				 {
				 	//���к��벻�����ţ�������������һ��
				 	//���ش洢������������������������һ���п����ҵ�(�����л�����)
				 	//������ش洢���벻�����ţ�����һ�ο϶��Ҳ������˷�һ���ʱ�����(�ƶ��ֻ�����)
				 	strcpy(pCalledNumberWithAreaCode, getAreaCode());
					//������Ч
					if(pCalledNumber[0]!=0)
					{
						strcat(pCalledNumberWithAreaCode, ccb.getDialedNumber());
						pCCBCalled = CSAG::getSagInstance()->FindCCBByLocalNumber(pCalledNumberWithAreaCode);
						if(pCCBCalled!=NULL)
						{
							blFound = true;
						}
					}
				 }
			}
			else
			{
				blFound = true;
			}
		}
		else
		{
			if(bParseResult==DIAL_CONTINUE ||
				bParseResult==DIAL_VALID)
			{
				//������Լ�������
				//������ʱ��TDial
				ccb.startTimer(TIMERID_SAG_O_DIALNUMBER);
				//����״̬SAG_DIAL_STATE
				return SAG_DIAL_STATE;
			}
		}

		if(blFound)
		{
			//������в���SAG_IDLE_STATE
			if(SAG_IDLE_STATE!=pCCBCalled->getState())
			{
				//��ʼ��æ��
				//���ն˷���Disconnect����
				ccb.sendDLSignalDisconnect(REL_CAUSE_PEER_BUSY);
				//�����ȴ��û��һ���ʱ��TOnhook
				ccb.startTimer(TIMERID_SAG_DISCONNECT);
				//����SAG_WAITONHOOK_STATE	
				return SAG_WAIT_ONHOOK_STATE;
			}
			else
			{
				//������п��������ڲ�����CallArrive������
				ccb.setPeerCCB(pCCBCalled);
				ccb.sendInnerSignalCallArrive(pCCBCalled->getUID());
				//������ʱ��TOAlerting�ȴ����л�Alerting
				ccb.startTimer(TIMERID_SAG_O_ALERTING);
				//����SAG_O_ALERTING_STATE
				return SAG_O_ALERTING_STATE;
			}
		}
		else
		{
			//������к��벻���ϲ��żƻ��򱻽в��ڱ�sag(bts)����ʼ��æ����
			ccb.sendDLSignalDisconnect(REL_CAUSE_SAG);
			//�����ȴ��û��һ���ʱ��TOnhook
			ccb.startTimer(TIMERID_SAG_DISCONNECT);
			//����SAG_WAITONHOOK_STATE
			return SAG_WAIT_ONHOOK_STATE;
		}		

	}
	else
	{
		//�Ĳ��,������
		return SAG_DIAL_STATE;
	}	
}
SAG_CPE_STATE DIAL__Timeout__DialNumber__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TDial
	ccb.deleteTimer(TIMERID_SAG_O_DIALNUMBER);
	//��æ��
	//�����ȴ��û��һ���ʱ��TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//����Disconnect������ն�
	ccb.sendDLSignalDisconnect(REL_CAUSE_TIMERTIMEOUT);
	//����״̬SAG_WAITONHOOK_STATE
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE DIAL__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TDial
	ccb.deleteTimer(TIMERID_SAG_O_DIALNUMBER);
	//���û�����release����
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//������ʱ��Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//����״̬SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE DIAL__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TDial
	ccb.deleteTimer(TIMERID_SAG_O_DIALNUMBER);
	//��Ӧ����errNotiRsp
	//����SAG_IDLE_STATE	
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE DIAL__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TDial
	ccb.deleteTimer(TIMERID_SAG_O_DIALNUMBER);
	//��ʼ��æ��
	//���ն˷���Disconnect����
	ccb.sendDLSignalDisconnect(ccb.getDisconnectReason(msg));
	//�����ȴ��û��һ���ʱ��TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//����SAG_WAITONHOOK_STATE
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE O_ALERTING__INSignal__Alerting__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOAlerting
	ccb.deleteTimer(TIMERID_SAG_O_ALERTING);
	//������ʱ��TOConnect
	ccb.startTimer(TIMERID_SAG_O_CONNECT);
	//���ն˷���Alerting
	ccb.sendDLSignalAlerting();
	//��ʼ�Ż�����
	ccb.playTone(RINGBACK_TONE);
	//����״̬SAG_RINGBACK_STATE
	return SAG_RINGBACK_STATE;
}
SAG_CPE_STATE O_ALERTING__Timeout__Alerting__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOAlerting
	ccb.deleteTimer(TIMERID_SAG_O_ALERTING);
	//��ʼ��æ��
	//���ն˷���Disconnect
	ccb.sendDLSignalDisconnect(REL_CAUSE_TIMERTIMEOUT);
	//�����ȴ��û��һ���ʱ��TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//�򱻽��ն˷���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//����SAG_WAITONHOOK_STATE	
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE O_ALERTING__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOAlerting
	ccb.deleteTimer(TIMERID_SAG_O_ALERTING);
	//���ն˷���release����
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//������ʱ��Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//�򱻽��ն˷���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_NORMAL);
	//����״̬SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE O_ALERTING__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOAlerting
	ccb.deleteTimer(TIMERID_SAG_O_ALERTING);
	//��Ӧ����errNotiRsp
	//�򱻽��ն˷���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_AIRLINKFAIL);
	//����״̬SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE O_ALERTING__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOAlerting
	ccb.deleteTimer(TIMERID_SAG_O_ALERTING);
	//��ʼ��æ��
	//���ն˷���Disconnect����
	ccb.sendDLSignalDisconnect(ccb.getDisconnectReason(msg));
	//�����ȴ��û��һ���ʱ��TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//����SAG_WAITONHOOK_STATE
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE RINGBACK__INSignal__Connect__Proc(CMessage& msg, CCCB& ccb)
{
	//ͣ��ʱ��TOConnect
	ccb.deleteTimer(TIMERID_SAG_O_CONNECT);
	//���ն˷�������Connect
	ccb.sendDLSignalConnect();
	//������ʱ��TOConnectAck
	ccb.startTimer(TIMERID_SAG_O_CONNECTACK);
	//�Ʒ���ز���
	T_TimeDate dt = bspGetDateTime();
	ccb.m_cdr.setConnectDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);	
	//����״̬SAG_CONNECT_STATE
	return SAG_CONNECT_STATE;
}
SAG_CPE_STATE RINGBACK__Timeout__Connect__Proc(CMessage& msg, CCCB& ccb)
{
	//ͣ��ʱ��TOConnect
	ccb.deleteTimer(TIMERID_SAG_O_CONNECT);
	//��ʼ��æ��
	//���ն˷�������Disconnect
	ccb.sendDLSignalDisconnect(REL_CAUSE_TIMERTIMEOUT);
	//�����ȴ��û��һ���ʱ��TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//�򱻽��ն˷���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//����״̬SAG_WAITONHOOK_STATE
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE RINGBACK__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ͣ��ʱ��TOConnect
	ccb.deleteTimer(TIMERID_SAG_O_CONNECT);
	//���ն˷�������Release
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//������ʱ��Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//�򱻽��ն˷���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_NORMAL);
	//����״̬SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE RINGBACK__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//ͣ��ʱ��TOConnect
	ccb.deleteTimer(TIMERID_SAG_O_CONNECT);
	//��Ӧ����errNotiRsp
	//�򱻽��ն˷���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_AIRLINKFAIL);
	//����SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE RINGBACK__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ͣ��ʱ��TOConnect
	ccb.deleteTimer(TIMERID_SAG_O_CONNECT);
	//��ʼ��æ��
	//���ն˷���Disconnect
	ccb.sendDLSignalDisconnect(ccb.getDisconnectReason(msg));
	//�����ȴ��û��һ���ʱ��TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//����SAG_WAITONHOOK_STATE	
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE PAGING__ULSignal__LapagingRsp__Proc(CMessage& msg, CCCB& ccb)
{
	//ͣ��ʱ��TlapagingRsp
	ccb.deleteTimer(TIMERID_SAG_T_LAPAGING);
	SAbisSignalT* pData = (SAbisSignalT*)msg.GetDataPtr();
	if(M_LAPAGING_CAUSE_SUCCESS==pData->sigPayload.LAPagingRsp.Cause)
	{
		//������ʱ��TassignResReq�ȴ�����������Դ
		ccb.startTimer(TIMERID_SAG_T_ASSIGN_TRANS_RES);
		//����״̬SAG_T_ASSIGNRES_STATE
		return SAG_T_ASSIGNRES_STATE;
	}
	else
	{
		//�򱻽��ն˷���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
		ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_PEER_BUSY);
		//����״̬SAG_IDLE_STATE
		return SAG_IDLE_STATE;
	}
}
SAG_CPE_STATE PAGING__Timeout__LapagingRsp__Proc(CMessage& msg, CCCB& ccb)
{
	//ͣ��ʱ��TlapagingRsp
	ccb.deleteTimer(TIMERID_SAG_T_LAPAGING);
	//��Զ��ն˷���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//����״̬SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE PAGING__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ͣ��ʱ��TlapagingRsp
	ccb.deleteTimer(TIMERID_SAG_T_LAPAGING);
	//��bts����DeLaPaging����
	ccb.sendDLSignalDeLaPaging();
	//����״̬SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE T_ASSIGNRES__ULSignal__AssignResReq__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TassignResReq
	ccb.deleteTimer(TIMERID_SAG_T_ASSIGN_TRANS_RES);
	//��bts��ӦAssignResRsp��Ϣ
	ccb.sendDLSignalAssignResRsp(msg);
	//�򱻽�UT����Setup����
	ccb.sendDLSignalSetup();
	//����TTSetupAck��ʱ���ȴ�SetupAck����
	ccb.startTimer(TIMERID_SAG_T_SETUPACK);
	//����״̬SAG_T_SETUP_STATE
	return SAG_T_SETUP_STATE;
}
SAG_CPE_STATE T_ASSIGNRES__Timeout__AssignResReq__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TassignResReq
	ccb.deleteTimer(TIMERID_SAG_T_ASSIGN_TRANS_RES);
	//����ErrNotiReq��bts
	ccb.sendDLSignalErrNotiReq();
	//�����з���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//����״̬SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE T_ASSIGNRES__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TassignResReq
	ccb.deleteTimer(TIMERID_SAG_T_ASSIGN_TRANS_RES);
	//����ErrNotiRsp��bts
	//�����з���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_AIRLINKFAIL);
	//����״̬SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE T_ASSIGNRES__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TassignResReq
	ccb.deleteTimer(TIMERID_SAG_T_ASSIGN_TRANS_RES);
	//����ErrNotiReq��bts
	ccb.sendDLSignalErrNotiReq();
	//����״̬SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE T_SETUP__ULSignal__SetupAck__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣTTSetupAck��ʱ��
	ccb.deleteTimer(TIMERID_SAG_T_SETUPACK);
	//������ʱ��TTAlerting�ȴ�����Alerting
	ccb.startTimer(TIMERID_SAG_T_ALERTING);
	//����״̬SAG_T_ALERTING_STATE
	return SAG_T_ALERTING_STATE;
}
SAG_CPE_STATE T_SETUP__Timeout__SetupAck__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣTTSetupAck��ʱ��
	ccb.deleteTimer(TIMERID_SAG_T_SETUPACK);
	//���ն˷���Release����
	ccb.sendDLSignalRelease(REL_CAUSE_TIMERTIMEOUT);
	//������ʱ��Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//�����з���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//����״̬SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE T_SETUP__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣTTSetupAck��ʱ��
	ccb.deleteTimer(TIMERID_SAG_T_SETUPACK);
	//��ӦerrNotiRsp��bts
	//�����з���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_AIRLINKFAIL);
	//����״̬SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE T_SETUP__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣTTSetupAck��ʱ��
	ccb.deleteTimer(TIMERID_SAG_T_SETUPACK);
	//���ն˷���Release����
	ccb.sendDLSignalRelease(ccb.getDisconnectReason(msg));
	//������ʱ��Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//����״̬SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE T_ALERTING__ULSignal__Alerting__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TTAlerting
	ccb.deleteTimer(TIMERID_SAG_T_ALERTING);
	//�����з����ڲ�����Alerting
	ccb.sendInnerSignalAlerting();
	//������ʱ��TTConnect���ȴ�����ժ��
	ccb.startTimer(TIMERID_SAG_T_CONNECT);
	//����״̬SAG_RING_STATE
	return SAG_RING_STATE;
}
SAG_CPE_STATE T_ALERTING__Timeout__Alerting__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TTAlerting
	ccb.deleteTimer(TIMERID_SAG_T_ALERTING);
	//���ն˷���Release����
	ccb.sendDLSignalRelease(REL_CAUSE_TIMERTIMEOUT);
	//������ʱ��Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//�����з���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//����״̬SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE T_ALERTING__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TTAlerting
	ccb.deleteTimer(TIMERID_SAG_T_ALERTING);
	//��ӦerrNotiRsp��bts
	//�����з���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_AIRLINKFAIL);
	//����״̬SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE T_ALERTING__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TTAlerting
	ccb.deleteTimer(TIMERID_SAG_T_ALERTING);
	//���ն˷���Release����
	ccb.sendDLSignalRelease(ccb.getDisconnectReason(msg));
	//������ʱ��Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//����״̬SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE RING__ULSignal__Connect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TTConnect
	ccb.deleteTimer(TIMERID_SAG_T_CONNECT);
	//�����з����ڲ�����Connect
	ccb.sendInnerSignalConnect();
	//��bts�����ڲ�������������
	ccb.sendDLSignalDVoiceReq(ENABLE_BTS_VDATA_INNER_SWITCH);
	//������ʱ��TTConnectAck���ȴ����л�ӦconnectAck
	ccb.startTimer(TIMERID_SAG_T_CONNECTACK);
	//����״̬SAG_CONNECT_STATE
	return SAG_CONNECT_STATE;
}
SAG_CPE_STATE RING__Timeout__Connect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TTConnect
	ccb.deleteTimer(TIMERID_SAG_T_CONNECT);
	//���ն˷���Release����
	ccb.sendDLSignalRelease(REL_CAUSE_TIMERTIMEOUT);
	//������ʱ��Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//�����з���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//����״̬SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE RING__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TTConnect
	ccb.deleteTimer(TIMERID_SAG_T_CONNECT);
	//��ӦerrNotiRsp��bts
	//�����з���Disconnect�ڲ�����,�öԶ˿�ʼ�ͷź���
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_AIRLINKFAIL);
	//����״̬SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE RING__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TTConnect
	ccb.deleteTimer(TIMERID_SAG_T_CONNECT);
	//���ն˷���Release����
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//������ʱ��Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//����״̬SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE RING__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TTConnect
	ccb.deleteTimer(TIMERID_SAG_T_CONNECT);
	//�����з���disconnect	
	ccb.sendInnerSignalDisconnect2Peer(ccb.getDisconnectReason(msg));
	//���ն˷���Release����
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//������ʱ��Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//����״̬SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE CONNECT__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOConnectAck/TTConnectAck
	if(ccb.isOrigCall())
	{
		ccb.deleteTimer(TIMERID_SAG_O_CONNECTACK);
	}
	else
	{
		ccb.deleteTimer(TIMERID_SAG_T_CONNECTACK);
	}
	//���ն˷���Release����
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//������ʱ��Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//��Զ��ն˷���Disconnect�ڲ�����öԶ˿�ʼ�ͷ�
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_NORMAL);
	//����SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE CONNECT__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOConnectAck/TTConnectAck
	if(ccb.isOrigCall())
	{
		ccb.deleteTimer(TIMERID_SAG_O_CONNECTACK);
	}
	else
	{
		ccb.deleteTimer(TIMERID_SAG_T_CONNECTACK);
	}
	//��ӦerrNotiRsp��bts
	//��Զ��ն˷���Disconnect�ڲ�����öԶ˿�ʼ�ͷ�
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_AIRLINKFAIL);
	//����״̬SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE CONNECT__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOConnectAck/TTConnectAck
	if(ccb.isOrigCall())
	{
		ccb.deleteTimer(TIMERID_SAG_O_CONNECTACK);
	}
	else
	{
		ccb.deleteTimer(TIMERID_SAG_T_CONNECTACK);
	}
	//��ʼ��æ��
	//���ն˷���Disconnect����
	ccb.sendDLSignalDisconnect(ccb.getDisconnectReason(msg));
	//�����ȴ��û��һ���ʱ��TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//����״̬SAG_WAITONHOOK_STATE
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE CONNECT__INSignal__TConnectAck__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TTConnectAck
	ccb.deleteTimer(TIMERID_SAG_T_CONNECTACK);
	//��������ConnectAck�������ն�
	ccb.sendDLSignalConnectAck();
	//����״̬SAG_CONNECT_STATE
	return SAG_CONNECT_STATE;
}
SAG_CPE_STATE CONNECT__Timeout__TConnectAck__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TTConnectAck
	ccb.deleteTimer(TIMERID_SAG_T_CONNECTACK);
	//��ʼ��æ��
	//�򱻽��ն˷���Disconnect����
	ccb.sendDLSignalDisconnect(REL_CAUSE_TIMERTIMEOUT);
	//�����ȴ��û��һ���ʱ��TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//�������ն˷���Disconnect�ڲ�����öԶ˿�ʼ�ͷ�
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//����״̬SAG_WAITONHOOK_STATE
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE CONNECT__ULSignal__OConnectAck__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOConnectAck
	ccb.deleteTimer(TIMERID_SAG_O_CONNECTACK);
	//�����ڲ�����ConnectAck������
	ccb.sendInnerSignalConnectAck();
	//����״̬SAG_CONNECT_STATE
	return SAG_CONNECT_STATE;
}
SAG_CPE_STATE CONNECT__Timeout__OConnectAck__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOConnectAck
	ccb.deleteTimer(TIMERID_SAG_O_CONNECTACK);
	//��ʼ��æ��
	//�������ն˷���Disconnect����
	ccb.sendDLSignalDisconnect(REL_CAUSE_TIMERTIMEOUT);
	//�����ȴ��û��һ���ʱ��TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//�򱻽��ն˷���Disconnect�ڲ�����öԶ˿�ʼ�ͷ�
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//����״̬SAG_WAITONHOOK_STATE
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE WAITONHOOK__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOnhook
	ccb.deleteTimer(TIMERID_SAG_DISCONNECT);
	//ֹͣ��æ��
	//���ն˷�������Release
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//������ʱ��Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//����״̬SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE WAITONHOOK__Timeout__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOnhook
	ccb.deleteTimer(TIMERID_SAG_DISCONNECT);
	//ֹͣ��æ��
	//���ն˷�������Release
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//������ʱ��Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//����״̬SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE WAITONHOOK__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TOnhook
	ccb.deleteTimer(TIMERID_SAG_DISCONNECT);
	//��ӦerrNotiRsp��bts
	//ֹͣ��æ��
	//����״̬SAG_IDLE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE RELEASE__ULSignal__ReleaseComplete__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��Treleasecomplete
	ccb.deleteTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//����releaseTransResReq��bts�ͷ���Դ
	ccb.sendDLSignalRlsTransResReq(REL_CAUSE_NORMAL);
	//����״̬SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE RELEASE__Timeout__ReleaseComplete__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��Treleasecomplete
	ccb.deleteTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//����releaseTransResReq��bts�ͷ���Դ
	ccb.sendDLSignalRlsTransResReq(REL_CAUSE_NORMAL);
	//����״̬SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE RELEASE__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��Treleasecomplete
	ccb.deleteTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//��ӦerrNotiRsp��bts
	//����״̬SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_CALLSETUP__ULSignal__GrpDisconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TIMERID_SAG_GRP_PTTCONNECT
	ccb.deleteTimer(TIMERID_SAG_GRP_PTTCONNECT);
	//��bts����releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//releseGrpSrvByGrpMaker
	SxcGrpCCB* pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(ccb.getGID());
	pGrpCCB->releseGrpSrvByGrpMaker(ccb.getUID(), REL_CAUSE_NORMAL);
	//����SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_CALLSETUP__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TIMERID_SAG_GRP_PTTCONNECT
	ccb.deleteTimer(TIMERID_SAG_GRP_PTTCONNECT);
	//��bts����releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//releseGrpSrvByGrpMaker
	SxcGrpCCB* pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(ccb.getGID());
	pGrpCCB->releseGrpSrvByGrpMaker(ccb.getUID(), REL_CAUSE_AIRLINKFAIL);
	//����SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_CALLSETUP__Timeout__PttConnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TIMERID_SAG_GRP_PTTCONNECT
	ccb.deleteTimer(TIMERID_SAG_GRP_PTTCONNECT);
	//��bts����releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//releseGrpSrvByGrpMaker
	SxcGrpCCB* pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(ccb.getGID());
	pGrpCCB->releseGrpSrvByGrpMaker(ccb.getUID(), REL_CAUSE_TIMERTIMEOUT);
	//����SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_CALLSETUP__INSignal__PttConnect__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TIMERID_SAG_GRP_PTTCONNECT
	ccb.deleteTimer(TIMERID_SAG_GRP_PTTCONNECT);
	//��bts����PTT_Connect_MSG
	ccb.sendDLSignalPttConnect(IS_CALL_OWNER, ccb.getEncryptCtrl(), ccb.getPrio());
	//����SAG_GRP_TALKING_STATE
	return SAG_GRP_TALKING_STATE;
}
SAG_CPE_STATE GRP_CALLSETUP__INSignal__GrpCallingRelease__Proc(CMessage& msg, CCCB& ccb)
{
	//ֹͣ��ʱ��TIMERID_SAG_GRP_PTTCONNECT
	ccb.deleteTimer(TIMERID_SAG_GRP_PTTCONNECT);
	//���ն˷���GrpCallingRelease
	ccb.sendDLSignalGrpCallingRls(ccb.getGrpCallingRlsReason(msg));
	//��bts����releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//����SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_TALKING__ULSignal__GrpDisconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//��bts����releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	SxcGrpCCB* pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(ccb.getGID());
	//��Ȩ����
	if(pGrpCCB!=NULL)
	{
		pGrpCCB->dispatchTalkersInGrpQueue();
	}
	//����SAG_IDLE_STATE	
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_TALKING__ULSignal__PttRls__Proc(CMessage& msg, CCCB& ccb)
{
	//���ն˻�ӦPttRlsAck
	ccb.sendDLSignalPttRlsAck();
	//��bts����releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//��Ȩ����
	SxcGrpCCB* pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(ccb.getGID());
	if(pGrpCCB!=NULL)
	{
		pGrpCCB->dispatchTalkersInGrpQueue();
	}
	//����SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_TALKING__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//��bts����releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//��Ȩ����
	SxcGrpCCB* pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(ccb.getGID());
	if(pGrpCCB!=NULL)
	{
		pGrpCCB->dispatchTalkersInGrpQueue();
	}
	//����SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_TALKING__INSignal__GrpCallingRelease__Proc(CMessage& msg, CCCB& ccb)
{
	//���ն˷���GrpCallingRelease
	ccb.sendDLSignalGrpCallingRls(ccb.getGrpCallingRlsReason(msg));
	//��bts����releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//����SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_TALKING__INSignal__PttInterrupt__Proc(CMessage& msg, CCCB& ccb)
{
	//���ն˷���PttInterrupt
	ccb.sendDLSignalPttInterrupt(ccb.getPttInterruptReason(msg));
	//��bts����releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//����SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}


