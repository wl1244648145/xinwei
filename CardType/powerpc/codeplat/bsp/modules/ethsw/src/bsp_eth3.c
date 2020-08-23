/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* Դ�ļ���:           Bsp_ethsw_init.c 
* ����:                  
* �汾:                                                                  
* ��������:                              
* ����:                                              
*******************************************************************************/
/************************** �����ļ����� **********************************/
/**************************** ����ͷ�ļ�* **********************************/
#include <stdio.h>

/**************************** ˽��ͷ�ļ�* **********************************/
#include "bsp_types.h"
#include "../inc/bsp_ethsw_bcm5389.h"
#include "../inc/bsp_ethsw_port.h"
#include "../inc/bsp_ethsw_init.h"
#include "../inc/bsp_ethsw_spi.h"
#include "../inc/bsp_ethsw_arl.h"

/******************************* �ֲ��궨�� *********************************/


/*********************** ȫ�ֱ�������/��ʼ�� **************************/
//u16 g_pVlan[10] = {0x1ff,0x100,0x100,0x100,0x100,0x100,0x100,0x100,0x100,0x100};
u32 g_pVlan[10] = {0x1ffff,0x10000,0x10000,0x10000,0x10000,0x10000,0x10000,0x10000,0x10000,0x10000};
u32 g_u32InitFlag = 0x0;
extern u32 g_u32PrintRules;


/************************** �ֲ����������Ͷ��� ************************/



/*************************** �ֲ�����ԭ������ **************************/

/************************************ ����ʵ�� ************************* ****/
/*******************************************************************************
* ��������: ethsw_set_switch_mode
* ��������: ���ö˿ڵĽ�������,Enable/DisableоƬת������
* ����ĵ�:
* ��������:
* ��������:     ����        ����/���   ����
* u8Enable      u8          ����        �Ƿ�����оƬ��ת������,0:�ر�,1:��
*
* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------

*******************************************************************************/
s32 ethsw_set_switch_mode(u8 u8Enable)
{
    STRU_ETHSW_SWITCH_MODE  struSwitchMode; /* ���彻��ģʽ�ṹ */
    /* Read-Modified-Write */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, SWITCH_MODE, (u8 *)&struSwitchMode, ONE_BYTE_WIDTH));
    struSwitchMode.SwFwdEn = u8Enable;
    struSwitchMode.SwFwdMode = 1 - u8Enable;
    /* д����ģʽ�Ĵ��� */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, SWITCH_MODE, (u8 *)&struSwitchMode, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}
/*******************************************************************************
* ��������: ethsw_set_arl_multicast
* ��������: ����ARL���Ƿ�֧��Multicast����
* ����ĵ�:
* ��������:
* ��������:     ����        ����/���   ����
* u8Enable      u8          ����        ����оƬ֧���鲥����,0:��֧���鲥,1:֧���鲥
*
* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
*******************************************************************************/
s32 ethsw_set_arl_multicast(u8 u8Enable)
{
    STRU_ETHSW_MCAST_CTRL  struMcastCtrl;    /* �����鲥���ƽṹ */
    u8 u8test;
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&struMcastCtrl, ONE_BYTE_WIDTH));
    struMcastCtrl.MulticastEn = u8Enable; /* �޸Ĵ�λ */

    /* д�鲥���ƼĴ��� */
    //RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&struMcastCtrl, ONE_BYTE_WIDTH));
    u8test = 0xff;
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&struMcastCtrl, ONE_BYTE_WIDTH));


    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&u8test, ONE_BYTE_WIDTH));
    printf("\nu8test : 0x%x\n",u8test);	
    



	
    return (E_ETHSW_OK);
}

/*******************************************************************************
* ��������: ethsw_set_dlf_forward
* ��������: �Ե������鲥DLF(Ŀ�ĵ�ַ����ʧ��)����ת����������
* ����ĵ�:
* ��������:
* ��������:       ����        ����/���   ����
* u8MultiDlfEn    u8          ����        1:�鲥��DLF������Multicast Lookup Fail
                                            Forward Map register���ý���ת��
                                          0:�鲥��DLF���㲥(flood)
* u32MultiPortMap u32         ����        u8MultiDlfEnΪ1ʱ�鲥��DLF����ת��PortMap
* u8UniDlfEn      u8          ����        1:������DLF������Multicast Lookup Fail
                                            Forward Map register���ý���ת��
                                          0:������DLF���㲥(flood)
* u32UniPortMap   u32         ����        u16UniPortMapΪ1ʱ������DLF����ת��PortMap
*
* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
* 
*******************************************************************************/
s32 ethsw_set_dlf_forward(u8 u8MultiDlfEn, u16 u16MultiPortMap, u8 u8UniDlfEn, u16 u16UniPortMap)
{
    u16                    u16Temp;
    STRU_ETHSW_MCAST_CTRL  struMcastCtrl; /* �����鲥���ƽṹ */
    /* ֻ��Ҫ��ʹ���鲥��DLFת������ʱ����Ҫ����MULTICAST LOOKUP FAIL FORWARD MAP�Ĵ��� */
    if (1 == u8MultiDlfEn)
    {
        u16Temp = u16MultiPortMap;
    }
    else
    {
        u16Temp = 0;
    }
    /* дMULTICAST LOOKUP FAIL FORWARD MAP�Ĵ��� */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, MUL_DLF_FW_MAP, (u8 *)&u16Temp, TWO_BYTE_WIDTH));

    /* ֻ��Ҫ��ʹ�ܵ�����DLFת������ʱ����Ҫ����UNICAST LOOKUP FAIL FORWARD MAP�Ĵ��� */
    if (1 == u8UniDlfEn)
    {
        u16Temp = u16UniPortMap;
    }
    else
    {
        u16Temp = 0;
    }
    /* дUNICAST LOOKUP FAIL FORWARD MAP�Ĵ��� */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, UNI_DLF_FW_MAP, (u8 *)&u16Temp, TWO_BYTE_WIDTH));

    /* Read-Modified-Write */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&struMcastCtrl, ONE_BYTE_WIDTH));
    /* ���鲥/����ת������ʹ�ܱ�ǽ������� */
    struMcastCtrl.MlfFmEn = u8MultiDlfEn;
    struMcastCtrl.UniFmEn = u8UniDlfEn;
    /* д�鲥���ƼĴ��� */
   RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&struMcastCtrl, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}

/*******************************************************************************
* ��������: ethsw_set_port_stat
* ��������: ���ö˿�״̬
* ����ĵ�:
* ��������:
* ��������:     ����        ����/���   ����
* u8PortId        u8          ����        �˿�ID
* u8PhyScanEn     u8          ����        ʹ�ܱ�־
* u8OverrideEn    u8          ����        u8Overrideʹ�ܱ�־
* u8TxFlowEn      u8          ����        Tx����ʹ�ܱ�־
* u8RxFlowEn      u8          ����        Rx����ʹ�ܱ�־
* u8Speed         u8          ����        �˿�����
* u8Duplex        u8          ����        ˫��״̬
* u8LinkState     u8          ����        ����״̬

* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
* 
*******************************************************************************/
s32 ethsw_set_port_stat(u8 u8PortId, u8 u8PhyScanEn, u8 u8OverrideEn, u8 u8TxFlowEn, u8 u8RxFlowEn, u8 u8Speed, u8 u8Duplex, u8 u8LinkState)
{
    STRU_ETHSW_PORT_STAT struPortStat;
    struPortStat.PhyScanEn = u8PhyScanEn;
    struPortStat.OverrideEn = u8OverrideEn; /* ���ô˼Ĵ��������õ�����,˫����LINK״̬ */
    struPortStat.TxFlowEn = u8TxFlowEn;   /* ��������ʹ�� */
    struPortStat.RxFlowEn = u8RxFlowEn;   /* ��������ʹ�� */
    struPortStat.Speed = u8Speed;      /* ���� */
    struPortStat.Duplex = u8Duplex;    /* ˫��״̬ */
    struPortStat.LinkState = u8LinkState; 
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, MII_PORT_STATE_OR(u8PortId), (u8 *)&struPortStat, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}

/*******************************************************************************
* ��������: ethsw_set_phy_scan
* ��������: �Խ���оƬ��PHY �Զ�ɨ�蹦�ܽ�������
* ����ĵ�:
* ��������:
* ��������:     ����        ����/���   ����
*
* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
* 
*******************************************************************************/
s32 ethsw_set_phy_scan(void)
{
    u8   u8PortId;

    for(u8PortId = 0; u8PortId < 8; u8PortId++)
    {
        RETURN_IF_ERROR(ethsw_set_port_stat(u8PortId,1,0,0,0,0,0,0));
    }

    return (E_ETHSW_OK);
}
/*******************************************************************************
* ��������: ethsw_set_arl_hash
* ��������: ����ARL�������,HASH_DISABLE,оƬ����ARL��ʱ���ȼ���HASH INDEX,��0
* ����ĵ�:
* ��������:
* ��������:     ����        ����/���   ����
* u8Val         u8          ����        ���õ�ֵ
*
* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
* 
*******************************************************************************/
s32 ethsw_set_arl_hash(u8 u8Val)
{
    u8   u8Temp;
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, GLOBAL_ARL_CFG, &u8Temp, ONE_BYTE_WIDTH));
    printf("\nu8Temp is 0x%x\n",u8Temp);	
    if (0 == u8Val)
    {
        u8Temp &= 0xfe; /* �����һλ��Ϊ0 */
    }
    else
    {
        u8Temp |= 0x1;/* �����һλ��Ϊ1 */
    }
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_CTRL_PAGE, GLOBAL_ARL_CFG, &u8Temp, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}

/*******************************************************************************
* ��������: ethsw_set_qos
* ��������: ��оƬQoS�Ĵ�����������
* ����ĵ�:
* ��������:
* ��������:     ����        ����/���   ����
* u8Enable      u8          ����        ʹ�ܻ�ֹ��QoS����
*
* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
* 
*******************************************************************************/
s32 ethsw_set_qos(u8 u8Enable)
{
    u8   u8Temp;
    /* ����QoSʹ�ܼĴ��� */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QOS_PAGE, QOS_GB_CTRL, (u8 *)&u8Temp, ONE_BYTE_WIDTH));
    if (0 == u8Enable)
    {
        u8Temp &= 0xbf;  /* Bit6��Ϊ0,Port Base QOS Disable */
    }
    else
    {
        u8Temp |= 0x40;  /* Bit6��Ϊ1,Port Base QOS En  */
    }
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_QOS_PAGE, QOS_GB_CTRL, (u8 *)&u8Temp, ONE_BYTE_WIDTH));
    /* ����TxQ���ƼĴ��� */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QOS_PAGE, TX_QUEUE_CTRL, (u8 *)&u8Temp, ONE_BYTE_WIDTH));
    if (0 == u8Enable)
    {
        u8Temp &= 0xf3;  /* Bit3:2��Ϊ0,QOS_MODE b00: Singe Queue (No QoS) b01: Two Queues Mode
                                                 b10: Three Queues Mode    b11: Four Queues Mode */
    }
    else
    {
        u8Temp |= 0x0c;  /* Bit3:2��Ϊ11,QOS_MODE Four Queues */
    }
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_QOS_PAGE, TX_QUEUE_CTRL, (u8 *)&u8Temp, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}
/*******************************************************************************
* ��������: ethsw_set_pvlan
* ��������: ��PVLAN�Ĵ�����������
* ����ĵ�:
* ��������:
* ��������:     ����        ����/���   ����
*
* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
* 
*******************************************************************************/
s32 ethsw_set_pvlan(u32 u32VlanMask)
{
    u8    u8PortId;    /* �˿�ID */
    //u16   u16VlanMask; /* VLAN MASK */
    u8    u8i;
    //u16VlanMask = 0x1ff;/* ���еĶ˿ڶ�����ͬһ��PVLAN */
    //u16VlanMask = 0x83;/* ���еĶ˿ڶ�����ͬһ��PVLAN */

    g_pVlan[0] = u32VlanMask;
    for(u8i = 1; u8i < 10; u8i++)
    {
        g_pVlan[u8i] = 0x10000;
    }
    for (u8PortId = 0; u8PortId < MAX_USED_PORTS; u8PortId++)
    {
        RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, (u8)PVLAN_PORT(u8PortId), (u8 *)&u32VlanMask, FOUR_BYTE_WIDTH));
    }
    return (E_ETHSW_OK);
}

/*******************************************************************************
* ��������: ethsw_add_port_vlan
* ��������: ���˿���ӵ�ָ����Vlan��
* ����ĵ�:
* ��������:
* ��������:     ����                    ����/���   ����
* u8ChipId      u8                      ����        оƬID(single should be 0)
* u8PortId      u8                      ����        �˿ں�
* u8VlanId      u8                      ����        ָ����Vlan��
*
* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
* 
*******************************************************************************/
s32 ethsw_add_port_vlan(u8 u8PortId, u8 u8VlanId)
{
    u8    u8PortIdtemp;
    u32   u32VlanMask;
    u8    u8i;
    u32VlanMask = 0x1ffff;
    g_pVlan[0] = g_pVlan[0] & (~(u32)(1 << u8PortId));
    g_pVlan[u8VlanId] = g_pVlan[u8VlanId] | (1 << u8PortId);
    for (u8PortIdtemp = 0; u8PortIdtemp < MAX_USED_PORTS; u8PortIdtemp++)
    {
        for(u8i = 0;u8i < 10; u8i++)
        {
            if(0 != ((1 << u8PortIdtemp) & g_pVlan[u8i]))
            {
                u32VlanMask = g_pVlan[u8i];
                break;
            }
        }
        if(u8i == 10)
        {
            return (E_ETHSW_FAIL);
        }
        RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, (u8)PVLAN_PORT(u8PortIdtemp), (u8 *)&u32VlanMask, FOUR_BYTE_WIDTH));
    }

    return (E_ETHSW_OK);
}

/*******************************************************************************
* ��������: ethsw_set_port_vlan
* ��������: ��PVLAN�Ĵ�����������
* ����ĵ�:
* ��������:
* ��������:     ����        ����/���   ����
* u8PortId1      u8          ����       �˿ں�1
* u8PortId2      u8          ����       �˿ں�2
*
* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
* 
*******************************************************************************/
s32 ethsw_set_port_vlan(u8 u8PortId1, u8 u8PortId2)
{
	u32 u32VlanMask = 0x0001ffff;
    u8 u8Pbuf[4] = {0};
    /* ���ö˿�PortId1����˿�PortId2����� */
	u32VlanMask &= ~(1<<u8PortId2);
    u8Pbuf[3] = (u32VlanMask & 0xff000000)>>24;
    u8Pbuf[2] = (u32VlanMask & 0x00ff0000)>>16;
    u8Pbuf[1] = (u32VlanMask & 0x0000ff00)>>8;
    u8Pbuf[0] = (u32VlanMask & 0x000000ff)>>0;
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, (u8)(4*u8PortId1), u8Pbuf, FOUR_BYTE_WIDTH));
    /* ���ö˿�PortId2����˿�PortId1����� */
    u32VlanMask = 0x0001ffff;
    u32VlanMask &= ~(1<<u8PortId1);
    u8Pbuf[3] = (u32VlanMask & 0xff000000)>>24;
    u8Pbuf[2] = (u32VlanMask & 0x00ff0000)>>16;
    u8Pbuf[1] = (u32VlanMask & 0x0000ff00)>>8;
    u8Pbuf[0] = (u32VlanMask & 0x000000ff)>>0;
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, (u8)(4*u8PortId2), u8Pbuf, FOUR_BYTE_WIDTH));
	return (E_ETHSW_OK);

}
/*******************************************************************************
* ��������: ethsw_set_qvlan
* ��������: ��PVLAN�Ĵ�����������
* ����ĵ�:
* ��������:
* ��������:     ����        ����/���   ����
* u8Enable      u8          ����        0:Disable VLAN,�ڼ���HASHʱ,��ʹ��VID
*                                       1:Enable VLAN
*
* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
* 
*******************************************************************************/
s32 ethsw_set_qvlan(u8 u8Enable)
{
    u8   u8Temp;

    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL0, &u8Temp, ONE_BYTE_WIDTH));
    if (0 == u8Enable)
    {
        u8Temp &= 0x1f; /* �����3λ��Ϊ0 */
    }
    else
    {
        u8Temp |= 0xe0; /* �����3λ��Ϊ1 */
    }
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL0, &u8Temp, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}
/*******************************************************************************
*  name         : ethsw_set_port_mode
*  description  : �ֶ����ö˿�ģʽ
*  input para   : u8PortId      - �˿ں�
*                     u8SerdesEn   - ʹ��Serdesģʽ��־
*                     u8AutodetectEn  - �Զ����ʹ�ܱ�־
*  output para  : ��
*  return value : ״̬
*  others       : 
*******************************************************************************/
s32 ethsw_set_port_serdes_mode(u8 u8PortId, u8 u8SerdesEn)
{
    u16 struPortMode;

    ethsw_read_reg(ETHSW_SERDES_PAGE(u8PortId), SERD_SGMII_CTRL1, (u8 *)&struPortMode, 2);
    if(u8SerdesEn)
        struPortMode |= (1 << 16);       /* SerDesģʽʹ�� */
    ethsw_write_reg(ETHSW_SERDES_PAGE(u8PortId), SERD_SGMII_CTRL1, (u8 *)&struPortMode, 2);

    return (E_ETHSW_OK);
}
/*******************************************************************************
* ��������: ethsw_set_aging_time
* ��������: ����оƬ���ϻ�ʱ��(ARL����ϻ�ʱ��(���ڲ��þ�̬ARL������������Ҫ��),
*           Ҳ�ǰ����ϻ�ʱ��)
* ����ĵ�:
* ��������:
* ��������:     ����        ����/���   ����
* u32Age_Time   u32         ����        �˼Ĵ���Ӧ�����õ�Agingֵ
*
* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
* 
*******************************************************************************/

s32 ethsw_set_aging_time(u32 u32AgeTime)
{
    STRU_ETHSW_AGE_TIME struAgeTime;   /* ֱ�������ϻ�ʱ�� */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, AGING_TIME_CTRL, (u8 *)&struAgeTime, FOUR_BYTE_WIDTH));
    struAgeTime.AgeTime = u32AgeTime;
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, AGING_TIME_CTRL, (u8 *)&struAgeTime, FOUR_BYTE_WIDTH));
    return (E_ETHSW_OK);
}

/*******************************************************************************
* ��������: ethsw_bcm5389_init
* ��������: ��ʼ��BCM5389оƬ
* ����ĵ�:
* ��������:
* ��������:     ����        ����/���   ����
*
* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
* 
*******************************************************************************/

s32 ethsw_bcm5389_init(void)
{
    u8   u8PortId;
    
    if (0 != (g_u32InitFlag & ETHSW_CHIP_INIT_FLAG))
    {
        (void)printf("CHIP has been initialized!\n");
        return (E_ETHSW_CHIP_REINIT);
    }
    /* 1.�򿪶˿ڵ��շ����� */
    for (u8PortId = 0; u8PortId < 1; u8PortId++)
    {
        RETURN_IF_ERROR(ethsw_set_status(u8PortId, 1, 1));
    }

    /* 2.��оƬת������ */
    RETURN_IF_ERROR(ethsw_set_switch_mode(1));

    /* 3.��ARL��֧��Multicast���� */
    RETURN_IF_ERROR(ethsw_set_arl_multicast(1));

    /* 4.�Ե������鲥DLF����ת����������:�����鲥DLF����ת�������ж˿� */
    RETURN_IF_ERROR(ethsw_set_dlf_forward(0, 0x00, 0, 0x00));

    /* 5.ǿ�����ø��˿ڵ�״̬,DSP���˿���Ҫǿ������ */
    for (u8PortId = 0; u8PortId < 8; u8PortId++)
    {
        //RETURN_IF_ERROR(ethsw_force_port_up(u8PortId));
        RETURN_IF_ERROR(ethsw_set_port_stat(u8PortId,0,1,1,1,2,1,1));
    }

    /* 6.�Խ���оƬ��PHY�Զ�ɨ�蹦�ܽ������� */
    RETURN_IF_ERROR(ethsw_set_phy_scan());

    /* 7.����ARL���üĴ���,HASH_ENABLE */
    RETURN_IF_ERROR(ethsw_set_arl_hash(0));

    /* 8.����QoS���ƼĴ���,Disable QoS���� */
    RETURN_IF_ERROR(ethsw_set_qos(0));

    /* 9.����PVLAN */
    //RETURN_IF_ERROR(ethsw_set_pvlan());

    /* 10.����VLAN���ƼĴ���*/
    RETURN_IF_ERROR(ethsw_set_qvlan(0));

    /* 11.����AGING TIME���ƼĴ���,�����ϻ�ʱ��,����Ϊ���ϻ�*/
    RETURN_IF_ERROR(ethsw_set_aging_time(0));

    /* оƬ��ʼ�����,���ñ�� */
    g_u32InitFlag |= ETHSW_CHIP_INIT_FLAG;
    return (E_ETHSW_OK);
}

#if 0
s32 ethsw_write_test(void)
{
    u8   u8Temp[8];
    u8   u8Temp1[8];
    u8   u8Temp2[8];
    int i;

	
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, MULTIPORT_ADDR1, u8Temp, SIX_BYTE_WIDTH));
    #if 0
    for(i =0;i<6;i++)
    {
          printf("\ndata is 0x%x\n",u8Temp[i]);
    }
    #endif

    for(i =0;i<6;i++)
    {
        u8Temp1[i] = 1+i;
    }
	#if 0
    for(i =0;i<6;i++)
    {
          printf("\ndata is 0x%x\n",u8Temp1[i]);
    }
    #endif
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_CTRL_PAGE, MULTIPORT_ADDR1, u8Temp1, SIX_BYTE_WIDTH));

    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, MULTIPORT_ADDR1, u8Temp2, SIX_BYTE_WIDTH));
    #if 0
    for(i =0;i<6;i++)
    {
          printf("\ndata is 0x%x\n",u8Temp2[i]);
    }
    #endif
    if(memcmp(u8Temp1, u8Temp2, 6) != 0)
        return (E_ETHSW_FAIL);

    return (E_ETHSW_OK);
}
#endif

s32 ethsw_write_test(void)
{
    u16 tmpIn = 0;
    u16 tmpOut = 0;

    // ��ȡ�Ĵ���ֵ    
    ethsw_read_reg(ETHSW_CTRL_PAGE, LED_CONTROL_A, (u8 *)(&tmpIn), 2);
    // ���üĴ���
    ethsw_write_reg(ETHSW_CTRL_PAGE, LED_CONTROL_A, (u8 *)(&tmpIn), 2); 
    // ���¶�ȡ�Ĵ���ֵ
    ethsw_read_reg(ETHSW_CTRL_PAGE, LED_CONTROL_A, (u8 *)(&tmpOut), 2);
    if(tmpOut == tmpIn)
    {
        return E_ETHSW_OK;
    }
    else
    {
        return E_ETHSW_FAIL;
    }
    
}

int ethsw_set_portvlan(u8 u8PortIdtemp,u32 u32VlanMask)
{
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, (u8)PVLAN_PORT(u8PortIdtemp), (u8 *)&u32VlanMask, FOUR_BYTE_WIDTH));
}

int ethsw_read_portvlan(u8 u8PortIdtemp)
{
    u32 u32VlanMask;
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PVLAN_PAGE, (u8)PVLAN_PORT(u8PortIdtemp), (u8 *)&u32VlanMask, FOUR_BYTE_WIDTH));
    printf("read u32VlanMask = 0x%x.\n",u32VlanMask);
}

