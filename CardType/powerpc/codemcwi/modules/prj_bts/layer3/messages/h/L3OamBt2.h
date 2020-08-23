/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: L3OamBtsRegNotify.h
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3OAMBTSREGNOTIFY
#define _INC_L3OAMBTSREGNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif

// BTS ע�����̸�Ϊ BTS�������̨�ϱ�ע��ָʾ����̨�·�ע���������
// ǰ̨����ע������Ӧ������BTS�����ֻ��NOTIFY��Ϣ��
// BTS Register Notify��BTS��
class CBtsRegNotify : public CMessage
{
public: 
    CBtsRegNotify(CMessage &rMsg);
    CBtsRegNotify();
    bool CreateMessage(CComEntity&);
    ~CBtsRegNotify();
    UINT32 GetDefaultDataLen() const;
public: 
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);  

    UINT32 GetBtsHWVersion() const;
    void   SetBtsHWVersion(UINT32);  

    UINT32 GetBtsSWVersion() const;
    void   SetBtsSWVersion(UINT32);  

    UINT16 GetBtsRcvPort() const;
    void   SetBtsRcvPort(UINT16);  
    
    void   SetBtsIp(UINT32 ip);
    
    bool   GetBtsID(SINT8 *, UINT8) const;
    bool   SetBtsID(SINT8 *, UINT8);  
private:    
#pragma pack(1)
    struct T_BtsRegNotify
    {
        UINT16  TransId;
        UINT32  BtsHwVersion;
        UINT32  BtsSwVersion;
        UINT8   EncrypedBtsID[ENCRYPED_BTSID_LEN];
        UINT32  BtsCfgIp;
        UINT16  BtsRcvPort;   //BTS ���ݽ��ն˿ں�
    };
#pragma pack()
};
#endif
