/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* Դ�ļ���:           Bsp_ethsw_app.c 
* ����:                  
* �汾:                                                                  
* ��������:                              
* ����:                                              
*******************************************************************************/
/************************** �����ļ����� **********************************/
/**************************** ����ͷ�ļ�* **********************************/
#include <stdio.h>
#include <string.h>

/**************************** ˽��ͷ�ļ�* **********************************/
#include "bsp_types.h"
#include "../inc/bsp_ethsw_bcm5389.h"
#include "../inc/bsp_ethsw_port.h"
#include "../inc/bsp_ethsw_arl.h"
#include "../inc/bsp_ethsw_app.h"
#include "../inc/bsp_ethsw_spi.h"

/******************************* �ֲ��궨�� *********************************/


/*********************** ȫ�ֱ�������/��ʼ�� **************************/
/* ��ӡ���,�������ƴ�ӡ�������,������ʲô�����ӡ���:
   0x0:����ӡ;0x1:������������;
   0x2:��ӡ��ʱ��Ϣ;
   0x4:��ӡ�쳣��Ϣ;
   0x8:����ִ��ʧ�ܴ�ӡ����ֵ;
   0x10:��ӡ��ʾ��Ϣ;
   ����:���ִ�ӡ��� ȱʡֵΪ
   ������ӡ,���ڵ���ʱ��̬�޸Ĵ�ֵ,�Դ�ӡ��������� */
   
u32 g_u32PrintRules = 0x1f;

/************************** �ֲ����������Ͷ��� ************************/



/*************************** �ֲ�����ԭ������ **************************/

/************************************ ����ʵ�� ************************* ****/
/*******************************************************************************
* ��������: ethsw_create_arl_map
* ��������: ethsw_ioctl�ķ�֧����,��Ҫ������:����������MAC��ַ��˿�ת����ϵ��
* ����ĵ�:
* ��������:
* ��������:    ����                    ����/���  ����
* u16MapNum    u16                     ����       Ҫ������MAC��ַ��˿�ת����
                                                  ϵ����
* pstruMacPort STRU_ETHSW_MAC_PORT *   ����       Ҫ�����Ķ˿���MAC��ַӳ���
                                                  ϵ�ṹָ��
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
s32 ethsw_create_arl_map(u16 u16MapNum, const STRU_ETHSW_MAC_PORT *pstruMacPort)
{
	u8 u8CastType; /* �������鲥���,0Ϊ����,1Ϊ�鲥 */
	s32 s32Rv;      /* ����ֵ */
	u32 u32i;          /* ѭ������ */
	
	/* MAC��ַ��˿ڶ�Ӧ��ϵ,��ʱ����,ע��˽ṹ������Ľṹ��ͬ */
	STRU_ETHSW_MAC_PORT   struMacPort;

	/* У���������Ч�� */
	if ((0 == u16MapNum) || (NULLPTR == pstruMacPort))
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_create_arl_map:invalid parameters!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}

	for (u32i = 0; u32i < u16MapNum; u32i++)
	{
		struMacPort.u16PortId = (pstruMacPort + u32i)->u16PortId; /* ����ʱΪ�˿ں�/�鲥ʱΪ�˿�MAP */
		memcpy(struMacPort.u8MacAddr, (pstruMacPort + u32i)->u8MacAddr, 6);
		if (0 != (struMacPort.u8MacAddr[5] & 0x1))
		{
		       printf("\nMulti\n");
			u8CastType = 1; /* MAC ��ַbit40Ϊ1ʱ,��ʾ�鲥��ַ */
		}
		else
		{
			u8CastType = 0; /* MAC ��ַbit40Ϊ0ʱ,��ʾ������ַ */
		}
		
		/* ���ú�������һ��ARL�� */
		s32Rv = ethsw_add_arl_entry(u8CastType, (STRU_ETHSW_MAC_PORT *)&struMacPort);
		if (0 != s32Rv)
		{
			return (s32Rv);
		}
	}
	
	return (E_ETHSW_OK);
	
}

/*******************************************************************************
* ��������: ethsw_search_arl_map
* ��������: ��Ҫ������:����������MAC��ַ��˿�ת����ϵ��
* ����ĵ�:
* ��������:
* ��������:    ����                  ����/���  ����
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
s32 ethsw_search_arl_map(void)
{
    s32   s32Rv = 0;   /* ����ֵ */    
    s32Rv = ethsw_dump_arl_entry(0,NULLPTR);
    return (s32Rv);
	
}
/*******************************************************************************
* ��������: ethsw_set_port_mirror
* ��������: ��Ҫ������:���ö˿ڵ�mirror����
* ����ĵ�:
* ��������:
* ��������:       ����          ����/���  ����
* u8MirrorEnable  u8            ����       ʹ��MIRROR����
* u8MirrorRules   u8            ����       MIRROR�Ĺ���
* u8MirroredPort  u8            ����       ��MIRROR�Ķ˿�
* u8MirrorPort    u8            ����       MIRROR�˿�(���Ӷ˿�)
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
* ?
*******************************************************************************/
s32 ethsw_set_port_mirror(u8 u8MirrorEnable, u8 u8MirrorRules, u8 u8MirroredPort, u8 u8MirrorPort)
{
	u8   u8Enable;
	s32  s32Rv;
	
	/* �Բ��������ж� */
	if ((u8MirrorRules > 3) || (u8MirroredPort > MAX_USED_PORTS - 1) || (u8MirrorPort > MAX_USED_PORTS - 1))
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_set_port_mirror:invalid parameters!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}
	
	/* MIRRORʹ�ܱ������ */
	if (0 == u8MirrorEnable)
	{
		u8Enable = 0;
	}
	else
	{
		u8Enable = 1;
	}
	
	/* ���ú�����MIRROR���ܽ������� */
	s32Rv = ethsw_set_mirror(u8Enable, u8MirrorRules, u8MirroredPort, u8MirrorPort);
	
	return (s32Rv);
}

/*******************************************************************************
* ��������: ethsw_version
* ��������: �������İ汾�Ŵ�ӡ����
* ����ĵ�:
* ��������: ��
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
void ethsw_version(void)
{
    (void)printf("VERSION:<2013/10/08>\n");
}
/********************************************************************************
* ��������: ethsw_get_port_mib							
* ��    ��: ��ȡ�˿�ͳ��ֵ                             
* ����ĵ�:                    
* ��������:	int								
* ��    ��: 						     			
* ��������		   ����					����/��� 		����		
* u8PortId        u8          ����        �˿�ID

* ����ֵ: 0 ��ʾ�ɹ�������ֵ��ʾʧ�ܡ�								
* ˵   ��: 
*********************************************************************************/
s32 ethsw_get_port_mib(u8 u8PortId)
{
	u8   u8Buf[8];
	u32 u32temp1;
	u32 u32temp2;

	if(u8PortId > MAX_USED_PORTS - 1)
	{
		printf("\nInvalid Parameter! u8PortId = %d\n",u8PortId);
		return E_ETHSW_WRONG_PARAM;
	}
	
	//for (u8PortId = 0; u8PortId < MAX_USED_PORTS; u8PortId++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_DROP_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_DROP_PKTS: 0x%x\n", u8PortId, u32temp1);
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_BROADCAST_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_BROADCAST_PKTS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_MULTICAST_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_MULTICAST_PKTS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_UNICAST_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_UNICAST_PKTS: 0x%x\n", u8PortId, u32temp1);
#if 0
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_COLLISIONS, u8Buf, EIGHT_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_COLLISIONS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_SINGLE_COLLISIONS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_SINGLE_COLLISIONS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_MULTIPLE_COLLISIONS, u8Buf, EIGHT_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_MULTIPLE_COLLISIONS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_DEFERRED_TRANSMIT, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_DEFERRED_TRANSMIT: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_LATE_COLLISION, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_LATE_COLLISION: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_EXCESSIVE_COLLISION, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_EXCESSIVE_COLLISION: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_FRAME_INDISC, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_FRAME_INDISC: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_PAUSE_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_PAUSE_PKTS: 0x%x\n", u8PortId, u32temp1);
	
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_UNDERSIZE_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_UNDERSIZE_PKTS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_PAUSE_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_PAUSE_PKTS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_64_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_64_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_64TO127_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_64TO127_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_128TO255_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_128TO255_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_256TO511_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_256TO511_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_512TO1023_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_512TO1023_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_1024TO1522_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_1024TO1522_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_OVERSIZE_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_OVERSIZE_PKTS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_JABBER, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_JABBER: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_ALIGNMENT_ERRS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_ALIGNMENT_ERRS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_FCS_ERRS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_FCS_ERRS: 0x%x\n", u8PortId, u32temp1);

#endif	
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_DROP_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_DROP_PKTS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_UNICAST_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_UNICAST_PKTS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_MULTICAST_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_MULTICAST_PKTS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_BROADCAST_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_BROADCAST_PKTS: 0x%x\n", u8PortId, u32temp1);
#if 1

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_SA_CHANGES, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_SA_CHANGES: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_FRAGMENTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_FRAGMENTS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_EXCESSSIZE_DISC, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_EXCESSSIZE_DISC: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_SYMBOL_ERR, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_SYMBOL_ERR: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_QOS_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_QOS_PKTS: 0x%x\n", u8PortId, u32temp1);
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_1523TO2047_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_1523TO2047_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_2048TO4095_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_2048TO4095_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_4096TO8191_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_4096TO8191_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_8192TO9728_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_8192TO9728_OCTETS: 0x%x\n", u8PortId, u32temp1);
#endif	
	}
	return E_ETHSW_OK;
}

/*******************************************************************************
* ��������: ethsw_display_all_reg
* ��������: ��оƬ���еļĴ�����ֵ������,��ӡ����
* ����ĵ�:
* ��������: ��
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
s32 ethsw_display_all_reg(void)
{
	u8   u8Buf[8];
	u8   u8PortId;
#if 1
	/* Display ETHSW_CTRL_PAGE */
	for (u8PortId = 0; u8PortId < MAX_USED_PORTS; u8PortId++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_TRAFIC_CTRL(u8PortId), u8Buf, ONE_BYTE_WIDTH));
		printf("ETHSW_CTRL_PAGE PORT_TRAFIC_CTRL[%d] is <0x%02x>\n", (s32)u8PortId, (s32)u8Buf[0]);
	}
	
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, IMP_TRAFIC_CTRL, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE IMP_TRAFIC_CTRL is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, SWITCH_MODE, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE SWITCH_MODE is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE PORT_FORWARD_CTRL is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, UNI_DLF_FW_MAP, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE UNI_DLF_FW_MAP is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, MUL_DLF_FW_MAP, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE MUL_DLF_FW_MAP is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_MAC,u8Buf, SIX_BYTE_WIDTH));
	printf("ETHSW_MGMT_PAGE INGRESS_MIRROR_MAC is <0x%02x%02x %02x%02x%02x%02x>\n",
	(s32)u8Buf[0], (s32)u8Buf[1], (s32)u8Buf[2], (s32)u8Buf[3], (s32)u8Buf[4], (s32)u8Buf[5]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_MAC,u8Buf, SIX_BYTE_WIDTH));
	printf("ETHSW_MGMT_PAGE EGRESS_MIRROR_MAC is <0x%02x%02x %02x%02x%02x%02x>\n",
	(s32)u8Buf[0], (s32)u8Buf[1], (s32)u8Buf[2], (s32)u8Buf[3], (s32)u8Buf[4], (s32)u8Buf[5]);


	for (u8PortId = 0; u8PortId < MAX_USED_PORTS; u8PortId++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, MII_PORT_STATE_OR(u8PortId), u8Buf, ONE_BYTE_WIDTH));
		printf("ETHSW_CTRL_PAGE MII_PORT_STATE_OR[%d] is <0x%02x>\n", (s32)u8PortId, (s32)u8Buf[0]);
	}
	
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, IMP_PORT_STATE_OR, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE IMP_PORT_STATE_OR is <0x%02x>\n", (s32)u8Buf[0]);

	for (u8PortId = 0; u8PortId < MAX_USED_PORTS - 1; u8PortId++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, EXTERN_PHY_SCAN_RESULT(u8PortId), u8Buf, ONE_BYTE_WIDTH));
		printf("ETHSW_CTRL_PAGE EXTERN_PHY_SCAN_RESULT[%d] is <0x%02x>\n", (s32)u8PortId, (s32)u8Buf[0]);
	}
	
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, FAST_AGING_CTRL, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE FAST_AGING_CTRL is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, FAST_AGING_PORT, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE FAST_AGING_PORT is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, FAST_AGING_VID, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE FAST_AGING_VID is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PAUSE_FRAME_DETECT_CTRL, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE PAUSE_FRAME_DETECT_CTRL is <0x%02x>\n", (s32)u8Buf[0]);

	/* Display ETHSW_STAT_PAGE */
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_STAT_PAGE, LINK_STATUS_SUMMARY, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_STAT_PAGE LINK_STATUS_SUMMARY is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_STAT_PAGE, LINK_STATUS_CHANGE, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_STAT_PAGE LINK_STATUS_CHANGE is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_STAT_PAGE, PORT_SPEED_SUMMARY, u8Buf, FOUR_BYTE_WIDTH));
	printf("ETHSW_STAT_PAGE PORT_SPEED_SUMMARY is <0x%02x%02x%02x%02x\n", (s32)u8Buf[0], (s32)u8Buf[1],(s32)u8Buf[2], (s32)u8Buf[3]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_STAT_PAGE, DUPLEX_STATUS_SUMMARY, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_STAT_PAGE DUPLEX_STATUS_SUMMARY is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_STAT_PAGE, PAUSE_STATUS_SUMMARY, u8Buf, FOUR_BYTE_WIDTH));
	printf("ETHSW_STAT_PAGE PAUSE_STATUS_SUMMARY is <0x%02x%02x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1],(s32)u8Buf[2], (s32)u8Buf[3]);

	/* Display ETHSW_MGMT_PAGE */
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, GLOBAL_CONFIG, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_MGMT_PAGE GLOBAL_CONFIG is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, AGING_TIME_CTRL, u8Buf, FOUR_BYTE_WIDTH));
	printf("ETHSW_MGMT_PAGE AGING_TIME_CTRL is <0x%02x%02x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1],(s32)u8Buf[2], (s32)u8Buf[3]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, MIRROR_CAPTURE_CTRL, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_MGMT_PAGE MIRROR_CAPTURE_CTRL is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_CTRL, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_MGMT_PAGE INGRESS_MIRROR_CTRL is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_DIVIDER, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_MGMT_PAGE INGRESS_MIRROR_DIVIDER is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_CTRL, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_MGMT_PAGE EGRESS_MIRROR_CTRL is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
#endif	
	/* Display ETHSW_ARL_CTRL_PAGE */
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, GLOBAL_ARL_CFG, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_ARL_CTRL_PAGE GLOBAL_ARL_CFG is <0x%02x>\n", (s32)u8Buf[0]);

	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, BPDU_MULTICAST_ADDR, u8Buf, SIX_BYTE_WIDTH));
	printf("ETHSW_ARL_CTRL_PAGE BPDU_MULTICAST_ADDR is <0x%02x%02x %02x%02x%02x%02x>\n",
	(s32)u8Buf[5], (s32)u8Buf[4], (s32)u8Buf[3], (s32)u8Buf[2], (s32)u8Buf[1], (s32)u8Buf[0]);

	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, MULTIPORT_ADDR1, u8Buf, SIX_BYTE_WIDTH));
	printf("ETHSW_ARL_CTRL_PAGE MULTIPORT_ADDR1 is <0x%02x%02x %02x%02x%02x%02x>\n",
	(s32)u8Buf[0], (s32)u8Buf[1], (s32)u8Buf[2], (s32)u8Buf[3], (s32)u8Buf[4], (s32)u8Buf[5]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, MULTIPORT_VECTOR1, u8Buf, FOUR_BYTE_WIDTH));
	printf("ETHSW_ARL_CTRL_PAGE MULTIPORT_VECTOR1 is <0x%02x%02x%02x%02x>\n", u8Buf[0], u8Buf[1],
	(s32)u8Buf[2], (s32)u8Buf[3]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, MULTIPORT_ADDR2, u8Buf, SIX_BYTE_WIDTH));
	printf("ETHSW_ARL_CTRL_PAGE MULTIPORT_ADDR2 is <0x%02x%02x %02x%02x%02x%02x>\n",
	(s32)u8Buf[0], (s32)u8Buf[1], (s32)u8Buf[2], (s32)u8Buf[3], (s32)u8Buf[4], (s32)u8Buf[5]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, MULTIPORT_VECTOR2, u8Buf, FOUR_BYTE_WIDTH));
	printf("ETHSW_ARL_CTRL_PAGE MULTIPORT_VECTOR2 is <0x%02x%02x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1],
	(s32)u8Buf[2], (s32)u8Buf[3]);

	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QOS_PAGE, GLOBAL_CONFIG, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_QOS_PAGE GLOBAL_CON is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QOS_PAGE, TX_QUEUE_CTRL, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_QOS_PAGE TX_QUEUE_CTRL is <0x%02x>\n", (s32)u8Buf[0]);

	for (u8PortId = 0; u8PortId < MAX_USED_PORTS; u8PortId++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QOS_PAGE, 0x50+2*u8PortId, u8Buf, TWO_BYTE_WIDTH));
		printf("ETHSW_QOS_PAGE PRI_MAP[%d] is <0x%02x%02x>\n", u8PortId,(s32)u8Buf[0], (s32)u8Buf[1]);
	}


	for (u8PortId = 0; u8PortId < MAX_USED_PORTS; u8PortId++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, 0x10+2*u8PortId, u8Buf, TWO_BYTE_WIDTH));
		printf("ETHSW_QVLAN_PAGE PORT_QOS_PRI[%d] is <0x%02x%02x>\n", u8PortId,(s32)u8Buf[0], (s32)u8Buf[1]);
	}

	/* Display ETHSW_PVLAN_PAGE */
	for (u8PortId = 0; u8PortId < MAX_USED_PORTS; u8PortId++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PVLAN_PAGE, (u8)PVLAN_PORT(u8PortId), u8Buf, TWO_BYTE_WIDTH));
		printf("ETHSW_PVLAN_PAGE PVLAN_PORT(%d) is <0x%02x%02x>\n", u8PortId, (s32)u8Buf[0], (s32)u8Buf[1]);
	}
	
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PVLAN_PAGE, PVLAN_IMP, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_PVLAN_PAGE PVLAN_IMP is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
#if 0
	/* Display ETHSW_QVLAN_PAGE */
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL0, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_VLAN_PAGE VLAN_CTRL0 is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL1, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_VLAN_PAGE VLAN_CTRL1 is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL2, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_VLAN_PAGE VLAN_CTRL2 is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL3, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_VLAN_PAGE VLAN_CTRL3 is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL4, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_VLAN_PAGE VLAN_CTRL4 is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL5, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_VLAN_PAGE VLAN_CTRL5 is <0x%02x>\n", (s32)u8Buf[0]);
#endif
	/* Display ETHSW_PHY_PAGE,to be done */
	return (E_ETHSW_OK);
	
}
s32 ethsw_clear_port_mib(void)
{
    u8 u8data;
	u8data = 1;
    RETURN_IF_ERROR(ethsw_write_reg(0x02, 0, &u8data, ONE_BYTE_WIDTH));
	u8data = 0;
    RETURN_IF_ERROR(ethsw_write_reg(0x02, 0, &u8data, ONE_BYTE_WIDTH));
	return (E_ETHSW_OK);
}
s32 ethsw_set_jumbo(void)
{
    u8 u8data[4];
	u8data[0] = 0xff;
	u8data[1] = 0xff;
	u8data[2] = 0x01;
	u8data[3] = 0x00;
    RETURN_IF_ERROR(ethsw_write_reg(0x40, 1, u8data, FOUR_BYTE_WIDTH));
	return (E_ETHSW_OK);
}
void ethsw_help(void)
{
    printf("1.ethsw_set_port_mirror(u8MirrorEnable,u8MirrorRules,u8MirroredPort,u8MirrorPort)\n");
    printf("    u8MirrorEnable: 0-disable;1-enable;\n");
    printf("    u8MirrorRules: 1-Ingress;2-Egress;3-Ingress andEgress;\n");
    printf("2.ethsw_get_port_mib(u8PortId)\n");
    printf("3.ethsw_clear_port_mib(void)\n");
    printf("4.ethsw_display_all_reg(void)\n");
    printf("5.ethsw_get_status(u8PortId)\n");
    printf("6.ethsw_set_status(u8PortId,u8TxEnable,u8RxEnable)\n");
    printf("    u8TxEnable: 0-disable;1-enable;\n");
    printf("    u8RxEnable: 0-disable;1-enable;\n");
    printf("7.ethsw_set_jumbo(void)\n");
    printf("8.ethsw_arl_add(u16 portid,u8 mac0,u8 mac1,u8 mac2,u8 mac3,u8 mac4,u8 mac5)\n");
    printf("9.ethsw_arl_del(u8 mac0,u8 mac1,u8 mac2,u8 mac3,u8 mac4,u8 mac5)\n");
    printf("10.ethsw_dump_arl_entry\n");
    return;
}

int ethsw_arl_add(u16 portid,u8 mac0,u8 mac1,u8 mac2,u8 mac3,u8 mac4,u8 mac5)
{
    STRU_ETHSW_MAC_PORT  mac_port;
     s32 ret;

    mac_port.u16PortId = portid;

    mac_port.u8MacAddr[0] = mac5;
    mac_port.u8MacAddr[1] = mac4;
    mac_port.u8MacAddr[2] = mac3;
    mac_port.u8MacAddr[3] = mac2;
    mac_port.u8MacAddr[4] = mac1;
    mac_port.u8MacAddr[5] = mac0;

    ret = ethsw_create_arl_map(1, &mac_port);
    if(E_ETHSW_OK!= ret)
    {
        printf("\nethsw_create_arl_map failed!\n");
    }
	return ret;
}

struct arl_config {
	u8 port;
	u8 mac[6];
};

static const struct arl_config arl_config[ ] = {
	{
		.port = 0,	
		.mac = {0x00, 0xA0, 0x1E, 0x01, 0x01, 0x01},
	},
	{
		.port = 1,	
		.mac = {0x00, 0xA0, 0x1E, 0x01, 0x01, 0x02},
	},
	{
		.port = 8,	
		.mac = {0x00, 0x01, 0x02, 0x00, 0x02, 0x51},
	},
	{
		.port = 9,	
		.mac = {0x00, 0x01, 0x02, 0x00, 0x03, 0x51},
	},
	{
		.port = 10,	
		.mac = {0x00, 0x01, 0x02, 0x00, 0x04, 0x51},
	},
	{
		.port = 11,	
		.mac = {0x00, 0x01, 0x02, 0x00, 0x05, 0x51},
	},
	{
		.port = 12,	
		.mac = {0x00, 0x01, 0x02, 0x00, 0x06, 0x51},
	},
	{
		.port = 13,	
		.mac = {0x00, 0x01, 0x02, 0x00, 0x07, 0x51},
	},
	{
		.port = 14,	
		.mac = {0x00, 0xa0, 0x1e, 0x01, 0x02, 0x02},
	},
};


int ethsw_arl_set(void)
{
	int i, config_size = sizeof(arl_config) / sizeof(arl_config[0]);
	for (i = 0; i < config_size; i++) 
	{
		u8 *mac = arl_config[i].mac;
		u8 u8mac = 0;
		if(1 == arl_config[i].port)
		{
			ethsw_arl_add(arl_config[i].port, mac[0], mac[1], mac[2], mac[3], bsp_get_slot_id()+1, mac[5]);
			continue;
		}
		if(14 == arl_config[i].port)	//port14��mac��ַ���ݲ�λ������
		{
		    if(0 == bsp_get_slot_id())
		        u8mac = 0x02;
		    else if(1 == bsp_get_slot_id())
		        u8mac = 0x01;
		    ethsw_arl_add(arl_config[i].port, mac[0], mac[1], mac[2], mac[3], u8mac, mac[5]);
                  continue;
        }
		ethsw_arl_add(arl_config[i].port, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}

	printf("switch mac and port map\n");

	for (i = 0; i < config_size; i++) 
	{
		u8 *mac = arl_config[i].mac;
		if(1 == arl_config[i].port)
		{
			printf("\tport-%d, mac %02x:%02x:%02x:%02x:%02x:%02x\n", arl_config[i].port,
				mac[0], mac[1], mac[2], mac[3], bsp_get_slot_id()+1, mac[5]);
			continue;
		}
		printf("\tport-%d, mac %02x:%02x:%02x:%02x:%02x:%02x\n", arl_config[i].port,
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
	printf("\n");

	return 0;
}

void ethsw_arl_del(u8 mac0,u8 mac1,u8 mac2,u8 mac3,u8 mac4,u8 mac5)
{

    STRU_ETHSW_MAC_PORT  mac_port;
     s32 ret;

    mac_port.u8MacAddr[0] = mac5;
    mac_port.u8MacAddr[1] = mac4;
    mac_port.u8MacAddr[2] = mac3;
    mac_port.u8MacAddr[3] = mac2;
    mac_port.u8MacAddr[4] = mac1;
    mac_port.u8MacAddr[5] = mac0;

    ethsw_remove_arl_entry(0, &mac_port);

}


void ethsw_create_primap(void)
{
    u8 u8portid;
    u16 u16Buf;

    for(u8portid = 0; u8portid < 8; u8portid ++)
    {
        u16Buf = 0xff00;
	 RETURN_IF_ERROR(ethsw_write_reg(ETHSW_QOS_PAGE, 0x50+2*u8portid, (u8 *)&u16Buf, TWO_BYTE_WIDTH));
    }
    return;
}

void ethsw_set_portpri(u8 portid, u8 pri)
{
    u16 u16Buf;
    if(pri > 7)
    {
        printf("\nPRI input error!\n");
    }
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, 0x10+2*portid, (u8 *)&u16Buf, TWO_BYTE_WIDTH));
    //printf("ETHSW_QVLAN_PAGE PORT_QOS_PRI[%d] is <0x%x>\n", portid,u16Buf);
    u16Buf = u16Buf & 0x1fff;
    u16Buf = u16Buf | (pri<<13);
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_QVLAN_PAGE, 0x10+2*portid, (u8 *)&u16Buf, TWO_BYTE_WIDTH));
       return;
}

static int ethsw_set_mulitport(u8 port_bitmap, u8 *mac, u8 id)
{
	u8 mac_le[6], i, port_vector[4];

	/* change byte endian */
	for (i = 0; i < 6; i++)
		mac_le[i] = mac[5 - i];

	port_vector[0] = port_bitmap;


	if (id == 0) {
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_CTRL_PAGE, 0x10, mac_le, sizeof(mac_le)));
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_CTRL_PAGE, 0x16, port_vector, sizeof(port_vector)));
	} else if (id == 1) {
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_CTRL_PAGE, 0x20, mac_le, sizeof(mac_le)));
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_CTRL_PAGE, 0x26, port_vector, sizeof(port_vector)));
	} else {
		printf("ETHSW support only 2 mac for mulitport foward\n");
		return BSP_ERROR;
	}

	printf("port bit map %02x, mac addr %02x:%02x:%02x:%02x:%02x:%02x\n",
		port_bitmap, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	return BSP_OK;
}

int ethsw_mulitport_enable(void)
{
	u8 mac1[6] = {0x00, 0xA0, 0x1E, 0x01, 0x01, 0x01};
	u8 mac2[6] = {0x00, 0xA0, 0x1E, 0x01, 0x01, 0x02};
	u8 value = 0x10;
	static u8 init_flag = 0;

	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_CTRL_PAGE, 0, &value, 1));

	if (init_flag == 0) {
		/* port 0 port 7 */
		RETURN_IF_ERROR(ethsw_set_mulitport((1 << 7) | (1 << 0), mac1, 0));
		/* port 1 port7 */
		RETURN_IF_ERROR(ethsw_set_mulitport((1 << 7) | (1 << 1), mac2, 1));
		init_flag = 1;
	}

	return BSP_OK;
}

int ethsw_mulitport_disable(void)
{
	u8 value = 0x00;

	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_CTRL_PAGE, 0, &value, 1));

	return BSP_OK;
}

int ethsw_port_enable(int portid)
{
	if(portid < 0 || portid >15)
	{
		return BSP_ERROR;
	}
	u8 value = 0xa0;
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, portid, &value, 1));
	return BSP_OK;	
}

u32 g_u32aging_test = 1;
int bsp_close_eth3_net()
{
    u8 ms_ctrl;
    printf("down eth3\n");
    if(0 == g_u32aging_test)
    {
        ethsw_port_disable(5);
        ethsw_port_disable(4);
    }
}

int bsp_open_eth3_net()
{
    u8 ms_ctrl;
    printf("up eth3\n");
#if 0
    if(system("ifconfig eth3 down;\
    			ifconfig eth3 up")<0)
    	perror("ERROR:Up eth3 failed!\n");
#endif
    if(0 == g_u32aging_test)
    {
        ethsw_port_enable(4);
        ethsw_port_enable(5);	
    }
}

int ethsw_port_disable(int portid)
{
	if(portid < 0 || portid >15)
	{
		return BSP_ERROR;
	}
	u8 value = 0xa3;
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, portid, &value, 1));
	return BSP_OK;
}
int bsp_get_trace_port_mac(char *mac)
{
	int i;

	if (mac == NULL) {
		printf("%s:mac == NULL\n");
		return -1;
	}

	for (i = 0; i < sizeof(arl_config) / sizeof(arl_config[0]); i++) {
		if (arl_config[i].port == 7) {
			memcpy(mac, arl_config[i].mac, 6);
			return 0;
		}
	}

	return -1;
}
/******************************* Դ�ļ����� ********************************/

