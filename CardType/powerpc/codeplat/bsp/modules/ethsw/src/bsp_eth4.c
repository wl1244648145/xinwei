/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* Դ�ļ���:           Bsp_ethsw_port.c 
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
#include "../inc/bsp_ethsw_spi.h"

/******************************* �ֲ��궨�� *********************************/


/*********************** ȫ�ֱ�������/��ʼ�� **************************/
extern u32 g_u32PrintRules;  /* ��ӡ���� */


#define swap32(x) ((((x)&0xFF)<<24) | (((x)&0xFF00)<<8) | (((x)&0xFF0000)>>8) | ((x)>>24))
#define swap16(x) ((((x)&0xFF)<<8) | ((x)>>8))
/****************************************************
*�� �� ��: port_vlanid_table
*��    ��: ���ڽ�����port�ڵ�vlan��
****************************************************/
static unsigned char port_vlanid_table[17] = {0, 0, 0xFF, 2, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF};
static unsigned int port_vlanid_table1[17] = {	 0x003f03,0x007f03,0x018004,0x000088,
												 0x000070,0x000070,0x000070,0x000088,
												 0x000103,0x000203,0x000403,0x000803,
												 0x001003,0x002003,0x004003,0x018004,
												 0x018004};

                        							#if 0
                        							{
                        							     {1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0},/* port0 */
                                                                          {1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0},/* port1 */
                                                                          {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},/* port2 */
                                                                          {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* port3 */
                                                                          {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* port4 */
                                                                          {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* port5 */
                                                                          {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* port6 */
                                                                          {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* port7 */
                                                                          {1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},/* port8 */
                                                                          {1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},/* port9 */
                                                                          {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},/* port10 */
                                                                          {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},/* port11 */
                                                                          {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},/* port12 */
                                                                          {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},/* port13 */
                                                                          {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},/* port14 */
                                                                          {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},/* port15 */
                                                                          {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1} /* port16 */
                                                                     };
							                    #endif

/************************** �ֲ����������Ͷ��� ************************/



/*************************** �ֲ�����ԭ������ **************************/

/************************************ ����ʵ�� ************************* ****/

/*******************************************************************************
* ��������: ethsw_set_status
* ��������: ��ɶ˿ڽ��ܺͷ��͹��ܵ�����
* ����ĵ�:
* ��������:
* ��������:     ����        ����/���   ����
* u8PortId      u8          ����        �˿�ID
* u8TxEnable    u8          ����        ����ʹ��
* u8RxEnable    u8          ����        ����ʹ��
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
s32 ethsw_set_status(u8 u8PortId, u8 u8TxEnable, u8 u8RxEnable)
{
    STRU_ETHSW_PORT_CTRL  struPortCtrl; /* �˿ڿ��ƼĴ����ṹ */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_TRAFIC_CTRL(u8PortId),
                                   (u8 *)&struPortCtrl, ONE_BYTE_WIDTH));
    struPortCtrl.StpState = 5; /* 0:unmanaged mode, do not use STP;101 = Forwarding State */
    /* �˼Ĵ���Ϊ�շ�����Disable�Ĵ���,��ʱ�Ĵ�����0,�ر�ʱ��1 */
    struPortCtrl.TxDisable = (1 - u8TxEnable);  /* ���÷���bit */
    struPortCtrl.RxDisable = (1 - u8RxEnable);  /* ���ý���bit */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, PORT_TRAFIC_CTRL(u8PortId),
                                    (u8 *)&struPortCtrl, ONE_BYTE_WIDTH));/* ����д�� */
    return (E_ETHSW_OK);
}

/*******************************************************************************
* ��������: ethsw_get_status
* ��������: ��ɻ�ȡ�˿�LINK״̬�Ĺ���
* ����ĵ�:
* ��������:
* ��������:       ����        ����/���   ����
* u8PortId        u8          ����        �˿�ID
* pu8PortStatus   u8 *        ���        �˿�״̬
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
s32 ethsw_get_status(u8 u8PortId, u8 *pu8PortStatus)
{
    u8   u8Val[8];         /* �ֲ����� */
    STRU_ETHSW_PORT_CTRL    struPortCtrl;   /* �˿ڿ��ƼĴ����ṹ */
    STRU_ETHSW_SWITCH_MODE  struSwitchMode; /* ����ģʽ�Ĵ����ṹ */

    /* ��ȡоƬ�˿��շ����ƼĴ�����ֵ */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_TRAFIC_CTRL(u8PortId),
                                   (u8 *)&struPortCtrl, ONE_BYTE_WIDTH));
    /* �˼Ĵ���Ϊ�շ�����Disable�Ĵ���,��ʱ�Ĵ�����0,�ر�ʱ��1 */
    if (0 != struPortCtrl.TxDisable)
    {
        /* PORT TX FAIL */
        if (NULLPTR == pu8PortStatus) /*���Ϊ������д�ӡ*/
        {
            printf("\nPort[%d] Tx Disable!\n", u8PortId);
        }
        else
        {
            *pu8PortStatus = PORT_LINK_DOWN; /* ������м�¼ */
        }
        return (E_ETHSW_OK); /* ֱ�ӷ��� */
    }

    if (0 != struPortCtrl.RxDisable)
    {
        /* PORT RX FAIL */
        if (NULLPTR == pu8PortStatus) /*���Ϊ������д�ӡ*/
        {
            printf("\nPort[%d] Rx Disable!\n", u8PortId);
        }
        else
        {
            *pu8PortStatus = PORT_LINK_DOWN; /* ������м�¼ */
        }
        return (E_ETHSW_OK);   /* ֱ�ӷ��� */
    }

    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, SWITCH_MODE, (u8 *)&struSwitchMode, ONE_BYTE_WIDTH));
    if (0 == struSwitchMode.SwFwdEn)
    {
        if (NULLPTR == pu8PortStatus)/*���Ϊ������д�ӡ*/
        {
            printf("\nPort[%d] FWD Disable!\n", u8PortId);
        }
        else
        {
            *pu8PortStatus = PORT_LINK_DOWN;  /* ������м�¼ */
        }
        return (E_ETHSW_OK); /* ֱ�ӷ��� */
    }

    /* ��ȡ�˿�LINK״̬�Ĵ��� */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_STAT_PAGE, LINK_STATUS_SUMMARY, u8Val, TWO_BYTE_WIDTH));
    //printf("ETHSW_STAT_PAGE LINK_STATUS_SUMMARY is <0x%02x%02x>\n", (s32)u8Val[0], (s32)u8Val[1]);
    if(0 != (u8Val[0]&(1<<u8PortId)))
    {
                /* PORT OK */
            if (NULLPTR == pu8PortStatus) /*���Ϊ������д�ӡ*/
            {
                printf("\nPort[%d] is Link Up!\n", u8PortId);
            }
            else
            {
                *pu8PortStatus = PORT_LINK_UP; /* ������м�¼ */
            }
    }
    else
    {
            /* PORT FAIL */
            if (NULLPTR == pu8PortStatus)/*���Ϊ������д�ӡ*/
            {
                printf("\nPort[%d] is Link Down!\n", u8PortId);
            }
            else
            {
                *pu8PortStatus = PORT_LINK_DOWN;  /* ������м�¼ */
            }
    }
    return (E_ETHSW_OK); /* ��󷵻� */
}

s32 ethsw_get_vport(u8 port)
{
    u32 val = 0;
    if(port>16)
    {
        return BSP_ERROR;
    }
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PVLAN_PAGE, PVLAN_PORT(port), (u8 *)&val, FOUR_BYTE_WIDTH));
    printf("port:%d -> 0x%x\r\n", port, val);
    return val;
}
s32 ethsw_set_vport(u8 port, u32 val)
{
    if(port>16)
    {
        return BSP_ERROR;
    }
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, PVLAN_PORT(port), (u8 *)&val, FOUR_BYTE_WIDTH));
    return BSP_OK;
}

s32 ethsw_init_vport(void)
{
	int i = 0;
	int s32ret_tmp=BSP_OK;
	//unsigned int vplan_val = 0xDFFF0100;
	unsigned int set_val = 0;
	
	for(i=0;i<sizeof(port_vlanid_table1)/sizeof(int);i++)
	{
		set_val = port_vlanid_table1[i];
		set_val = swap32(set_val);
		if(BSP_ERROR == ethsw_set_vport(i,set_val))
		{
			s32ret_tmp = BSP_ERROR;
		}
	}
	return s32ret_tmp;
	#if 0
	// corenet[port5] --> eth3[port4]
	vplan_val = 0x10000000;
	ethsw_set_vport(5, vplan_val);
	// eth3[port4] --> All
	vplan_val = 0xFFFF0100;
	ethsw_set_vport(4, vplan_val);
	#endif
	return 0;
}

u32 ppc_ethsw_get_port_status(void)
{
    u32   u32RegVal = 0;         /* �ֲ����� */

    /* ��ȡоƬ�˿�״̬�Ĵ�����ֵ */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_STAT_PAGE, LINK_STATUS_SUMMARY, (u8 *)&u32RegVal, FOUR_BYTE_WIDTH));
    u32RegVal = ((u32RegVal&0x000000ff)<<24) | ((u32RegVal&0x0000ff00)<<8) | ((u32RegVal&0x00ff0000)>>8) | ((u32RegVal&0xff000000)>>24);
    
    if(u32RegVal&PPC_ETHNET0_PORT)
        printf("PPC_ETHNET0_PORT is link up!\r\n");
    else
        printf("PPC_ETHNET0_PORT is link down!\r\n");
    if(u32RegVal&PPC_ETHNET1_PORT)
        printf("PPC_ETHNET1_PORT is link up!\r\n");
    else
        printf("PPC_ETHNET1_PORT is link down!\r\n");
    if(u32RegVal&PPC_ETHNET2_PORT)
        printf("PPC_ETHNET2_PORT is link up!\r\n");
    else
        printf("PPC_ETHNET2_PORT is link down!\r\n");
    if(u32RegVal&PPC_ETHNET3_PORT)
        printf("PPC_ETHNET3_PORT is link up!\r\n");
    else
        printf("PPC_ETHNET3_PORT is link down!\r\n"); 
    if(u32RegVal&IPRAN_PHY1_PORT)
        printf("IPRAN_PHY1_PORT is link up!\r\n");
    else
        printf("IPRAN_PHY1_PORT is link down!\r\n");
    if(u32RegVal&IPRAN_PHY2_PORT)
        printf("IPRAN_PHY2_PORT is link up!\r\n");
    else
        printf("IPRAN_PHY2_PORT is link down!\r\n");
    if(u32RegVal&TRACERAN_PHY_PORT)
        printf("TRACERAN_PHY_PORT is link up!\r\n");
    else
        printf("TRACERAN_PHY_PORT is link down!\r\n");
    if(u32RegVal&PPC_SLOT2_PORT)
        printf("PPC_SLOT2_PORT is link up!\r\n");
    else
        printf("PPC_SLOT2_PORT is link down!\r\n"); 
    if(u32RegVal&PPC_SLOT3_PORT)
        printf("PPC_SLOT3_PORT is link up!\r\n");
    else
        printf("PPC_SLOT3_PORT is link down!\r\n");
    if(u32RegVal&PPC_SLOT4_PORT)
        printf("PPC_SLOT4_PORT is link up!\r\n");
    else
        printf("PPC_SLOT4_PORT is link down!\r\n"); 
    if(u32RegVal&PPC_SLOT5_PORT)
        printf("PPC_SLOT5_PORT is link up!\r\n");
    else
        printf("PPC_SLOT5_PORT is link down!\r\n"); 
    if(u32RegVal&PPC_SLOT6_PORT)
        printf("PPC_SLOT6_PORT is link up!\r\n");
    else
        printf("PPC_SLOT6_PORT is link down!\r\n");
    if(u32RegVal&PPC_SLOT7_PORT)
        printf("PPC_SLOT7_PORT is link up!\r\n");
    else
        printf("PPC_SLOT7_PORT is link down!\r\n");
    return u32RegVal;
}

s32 ethsw_set_mirror(u8 u8MirrorEnable, u8 u8MirrorRules, u8 u8MirroredPort, u8 u8MirrorPort)
{
    u16 config= 0;
	u8 temp[4]={0};
	u32 temp1 = 0;

    /* ����ʹ��MIRROR����ʱ����Ҫ����Ingress/Egress Mirror���ƼĴ���  */
    if (1 == u8MirrorEnable)
    {
         //config = config | (1 << (u8MirroredPort+8));
		 temp1 = 1 << u8MirroredPort;
		 memcpy(temp,(u8 *)&temp1,4);
		 temp1 = temp[3]<<24;
		 temp1 |= temp[2]<<16;
		 temp1 |= temp[1]<<8;
		 temp1 |= temp[0];
        	
        /* ������Trunking������صļĴ���,�����ṩֻMirrorĳMAC��ַ,��һ�����Mirror�ȹ��� */
        if (0x1 == u8MirrorRules) /* INGRESS���� */
        {
            /* ����Ingress Mirror���ƼĴ��� */
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_CTRL, (u8 *)&temp1, FOUR_BYTE_WIDTH));
        }
        else if (0x2 == u8MirrorRules)/* EGRESS���� */
        {
            /* ����Egress Mirror���ƼĴ��� */
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_CTRL, (u8 *)&temp1, FOUR_BYTE_WIDTH));
        }
        else if (0x3 == u8MirrorRules) /* ˫���� */
        {
            /* ����Egress Mirror���ƼĴ��� */
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_CTRL, (u8 *)&temp1, FOUR_BYTE_WIDTH));
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_CTRL, (u8 *)&temp1, FOUR_BYTE_WIDTH));
        }
        //config = 0x0780;
        config = (u8MirrorPort << 8) | 0x80;	
        RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, MIRROR_CAPTURE_CTRL, (u8 *)&config, TWO_BYTE_WIDTH));

    }
    else
    {
        config = 0;
        temp1 = 0;
        RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_CTRL, (u8 *)&temp1, FOUR_BYTE_WIDTH));
        RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_CTRL, (u8 *)&temp1, FOUR_BYTE_WIDTH));
        RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, MIRROR_CAPTURE_CTRL, (u8 *)&config, TWO_BYTE_WIDTH));
}
    return (E_ETHSW_OK);
}


s32 ethsw_set_filters(u8 inorout, u8 filterflag)  /*inorout---0: ingress; 1: exgress;    filterflag----0:srcmac; 1:dstmac*/
{
    u16 value= 0;

    if(0 == inorout)/* INGRESS���� */
    {
        RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_CTRL, (u8 *)&value, TWO_BYTE_WIDTH));
	 printf("\nvalue = 0x%x\n",value);
	 if(0 == filterflag)
	 {
            value |= 0x10;
	 }
	 else if(1 == filterflag)
	 {
            value |= 0x01;
	 }
	 else
	 {}
        RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_CTRL, (u8 *)&value, TWO_BYTE_WIDTH));

    }
    else if(1 == inorout)/* EGRESS���� */
    {
        RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_CTRL, (u8 *)&value, TWO_BYTE_WIDTH));
	 printf("\nvalue = 0x%x\n",value);
	 if(0 == filterflag)
	 {
            value |= 0x10;
	 }
	 else if(1 == filterflag)
	 {
            value |= 0x01;
	 }
	 else
	 {}
        RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_CTRL, (u8 *)&value, TWO_BYTE_WIDTH));
    }
    else
    {}
    return (E_ETHSW_OK);
}
s32 ethsw_set_mac(u8 inorout, u8* u8pmac) 
{
    u8 u8mac[6];

	memcpy((void*)u8mac, (const void *)u8pmac,6 );
    if(0 == inorout)/* INGRESS���� */
     {
        RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_MAC, (u8 *)&u8mac, SIX_BYTE_WIDTH));
     }
    else if(1 == inorout)/* EGRESS���� */
    {
        RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_MAC, (u8 *)&u8mac, SIX_BYTE_WIDTH));
    }
    return (E_ETHSW_OK);
}
void ethsw_set_mactest(void)
{
    u8 u8mac[6];

    u8mac[0] = 0x0;
    u8mac[1] = 0xa0;
    u8mac[2] = 0x1e;
    u8mac[3] = 0x1;
    u8mac[4] = 0x1;
    u8mac[5] = 0x1;
    ethsw_set_mac(1,u8mac);
    return;

}
/****************************************************
*�� �� ��: void bsp_config_vport(void)
*��������: ���ý���������port��vlan����
*��    ��:
*
*��    ��:
****************************************************/
int bsp_config_vport(void)
{
    int i = 0, j = 0;
    for(i=0; i<sizeof(port_vlanid_table); i++)
    {
        unsigned int set_val = 0;
        unsigned temp = 0;
        for(j=0; j<sizeof(port_vlanid_table); j++)
        {
            if((port_vlanid_table[i] == port_vlanid_table[j]))
            {
                set_val |= (1<<j);
            }
        }
        //��set_valд��ethsw�Ĵ���(ע���С��)
        //PPC
        set_val = swap32(set_val);
        RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, PVLAN_PORT(i), (u8 *)&set_val, FOUR_BYTE_WIDTH));
    }
    return BSP_OK;
}

/****************************************************
*�� �� ��: int bsp_set_vlan(u8 port, u8 vlan_id)
*��������: �޸Ķ˿�����vlan ID
*��    ��: u8 port  �˿ں� bcm5389:0~8  bcm5396:0~16
*          u8 vlan_id vlan��
*��    ��: �޸ĺ�˿�port��Ӧ��vlan_id
****************************************************/
int bsp_set_vlan(u8 port, u8 vlan_id)
{
	if(port > (sizeof(port_vlanid_table)-1))
		return BSP_ERROR;

    if((port_vlanid_table[port]!=vlan_id))
    {
        port_vlanid_table[port] = vlan_id;
        bsp_config_vport();
    }
    return port_vlanid_table[port];
}


/****************************************************
*�� �� ��: int bsp_get_vlan(u8 port)
*��������: ��ȡ�˿ڵ�vlan ID
*��    ��: u8 port  �˿ں� bcm5389:0~8  bcm5396:0~16
*��    ��: �˿�port��Ӧ��vlan_id
****************************************************/
int bsp_get_vlan(u8 port)
{
	if(port > (sizeof(port_vlanid_table)-1))
		return BSP_ERROR;
	return port_vlanid_table[port];
}

/****************************************************
*�� �� ��: int bsp_show_vlan(void)
*��������: ��ӡ�˿�VLAN��Ϣ
*��    ��:
*��    ��:
****************************************************/
void bsp_show_vlan(void)
{
    int i = 0;
    for(i=0; i<sizeof(port_vlanid_table); i++)
    {
        u32 regVal = 0;
        ethsw_read_reg(ETHSW_PVLAN_PAGE, PVLAN_PORT(i), &regVal, 4);
        printf("portID[%d]---vlanID[%d]---0x%x\r\n", i, port_vlanid_table[i], swap32(regVal));
    }
    printf("\r\n");
}

#if 1
void set_mirror_test1(void)
{

    u16 test;
		printf("1111111111111");

    test = 0x8000;
		printf("222222222222222");

    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_CTRL, (u8 *)&test, TWO_BYTE_WIDTH));
		printf("INGRESS_MIRROR_CTRL");

    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_CTRL, (u8 *)&test, TWO_BYTE_WIDTH));
		printf("EGRESS_MIRROR_CTRL");
    test = 0x0180;

    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, MIRROR_CAPTURE_CTRL, (u8 *)&test, TWO_BYTE_WIDTH));
		printf("MIRROR_CAPTURE_CTRL");

    return;

}

#endif

/****************************************************
*�� �� ��: bsp_ethsw_set_port_sgmii_mode
*��������: ���ö˿�ģʽ
*��    ��: 
*��    ��: 
****************************************************/
int bsp_ethsw_set_port_sgmii_mode(u8 port)
{
	u8 portaddr;
	portaddr = port + 0x10;
	ethsw_write_reg_test(portaddr,0x20, 0x01D1,2);
	return BSP_OK;
}


