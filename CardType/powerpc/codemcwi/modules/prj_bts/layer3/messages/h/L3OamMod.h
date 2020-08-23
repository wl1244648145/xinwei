/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ------------------------------------------------
----
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3OAMMODULEBOOTUPNOTIFY
#define _INC_L3OAMMODULEBOOTUPNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

//�յ�tboot��ϵͳ����ָʾ��Ϣ���д���Ϣ����
//��ϵͳ������־������BTS��������ģ��EMS���ȶ�˳���ͳ�ʼ��������Ϣ��
//���ù���ģ�飬��Ϊ����ģ������ʱ�������ϵͳ����ģ�鲻����ʱ�����յ�
//���ù���ģ���Ӧ��ʧ�ܣ��󣬷�����һ����������֪�������������ݷ����ꣻ
//����EMS����������EMS������������ָʾ��Ȼ�����while��������Ϣ��
//��ϵͳbootupָʾ��Ϣ��Ҫ���ж��壬����Ҫ����ÿ��ģ��/cpu���������
//ϵͳ�����澯��ⶨʱ����ʼϵͳ���
class CModuleBootupNotify : public CMessage
{
public: 
    CModuleBootupNotify(CMessage &rMsg);
    CModuleBootupNotify();
    bool CreateMessage(CComEntity&);
    ~CModuleBootupNotify();
    UINT32 GetDefaultDataLen() const;
    
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);  

private:
#pragma pack(1)
    struct T_BootupInfo
	{
	    UINT16 TransId;
	};
#pragma pack()
};
#endif
