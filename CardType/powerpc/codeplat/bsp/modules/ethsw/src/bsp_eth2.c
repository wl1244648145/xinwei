/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* Դ�ļ���:           Bsp_ethsw_arl.c 
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
#include "../inc/bsp_ethsw_arl.h"
#include "../inc/bsp_ethsw_spi.h"

/******************************* �ֲ��궨�� *********************************/

#define swap32(x) ((((x)&0xFF)<<24) | (((x)&0xFF00)<<8) | (((x)&0xFF0000)>>8) | ((x)>>24))

/*********************** ȫ�ֱ�������/��ʼ�� **************************/
extern u32 g_u32PrintRules;

/************************** �ֲ����������Ͷ��� ************************/



/*************************** �ֲ�����ԭ������ **************************/

/************************************ ����ʵ�� ************************* ****/
/*******************************************************************************
* ��������: save_arl_struct
* ��������: ����ARL_TABLE_MAC_VID_ENTRY0/1,ARL_TABLE_DATA_ENTRY0/1,
*           ARL_TABLE_SEARCH_MAC_RESULT/ARL_TABLE_SEARCH_DATA_RESULT�Ĵ����ж���
*           �������Ϣ,ת��ΪSTRU_ETHSW_ARL_TABLE�ṹ�����ֵ
* ����ĵ�:
* ��������:
* ��������:        ����                           ����/���   ����
* pstruMacEntry    STRU_ETHSW_ARL_MAC_VID_ENTRY*  ����        ��MAC_VID/MAC_RESULT
                                                              �Ĵ�������������(8�ֽ�)
* pstruDataEntry   STRU_ETHSW_ARL_DATA_ENTRY*     ����        ��DATA�Ĵ�����������
                                                              ��(2�ֽ�)
* pstruArlTable    STRU_ETHSW_ARL_TABLE*          ���        ��������Ϣת���󱣴�
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
void save_arl_struct(const STRU_ETHSW_ARL_MAC_VID *pstruMacEntry, const STRU_ETHSW_ARL_DATA *pstruDataEntry, STRU_ETHSW_ARL_TABLE *pstruArlTable)
{
	memcpy(pstruArlTable->u8MacAddr, pstruMacEntry->u8MacAddr, 6);
	pstruArlTable->valid = pstruDataEntry->valid;
	pstruArlTable->VlanId = pstruMacEntry->VlanId;
	pstruArlTable->StaticEntry = pstruDataEntry->StaticEntry;
	pstruArlTable->age = pstruDataEntry->age;
	pstruArlTable->PortId = pstruDataEntry->PortId;
	return;
	
}
/*******************************************************************************
* ��������: ethsw_lookup_arl_entry
* ��������: ����ָ��MAC��ַ��VLAN ID��ARL��,�������ҳ�����ARL���浽���������
* ����ĵ�:
* ��������:
* ��������:      ����                    ����/���  ����
* pu8MacAddr     u8*                     ����       ��Ҫ��ȡ��mac��ַ������׵�ַ
* pstruMacEntry  STRU_ETHSW_ARL_MAC_VID* ���       �����ҵ���ARL���MAC��ַVID��
                                                    ���ڴ˽ṹ��,,��һ����Ӧ��ENTRY0,
                                                    �ڶ�����Ӧ������Ӧ��ENTRY1
* pstruDataEntry STRU_ETHSW_ARL_DATA*    ���       �����ҵ���ARL���PortId��Ч��̬
                                                    ����Ϣ����ڴ˽ṹ��,,��һ����
                                                    Ӧ��ENTRY0,�ڶ�����Ӧ������Ӧ��
                                                    ENTRY1
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
s32 ethsw_lookup_arl_entry(const u8 *pu8MacAddr, STRU_ETHSW_ARL_MAC_VID *pstruMacEntry, STRU_ETHSW_ARL_DATA *pstruDataEntry)
{
	u32   u32i;              /* ѭ������ */
	STRU_ETHSW_ARL_RW_CTRL  struArlRWCtrl;  /* ��ʱ����,����ʱ�Ŀ��ƽṹ */
       u8 test[8];
	if ((NULLPTR == pu8MacAddr) || (NULLPTR == pstruMacEntry) || (NULLPTR == pstruDataEntry))
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_lookup_arl_entry:NULL Pointer!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}
#if 0
	printf("\n0x%x\n",*pu8MacAddr);
	printf("\n0x%x\n",*(pu8MacAddr+1));
	printf("\n0x%x\n",*(pu8MacAddr+2));
	printf("\n0x%x\n",*(pu8MacAddr+3));
	printf("\n0x%x\n",*(pu8MacAddr+4));
	printf("\n0x%x\n",*(pu8MacAddr+5));
#endif
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, MAC_ADDRESS_INDEX, pu8MacAddr, SIX_BYTE_WIDTH));

	//RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, MAC_ADDRESS_INDEX,(u8 *)test, SIX_BYTE_WIDTH));
	//printf("\ntest is 0x%x-%x-%x-%x-%x-%x\n",test[0],test[1],test[2],test[3],test[4],test[5]);


	struArlRWCtrl.ArlStart = 1;/* ��ʼ��д��� */
	struArlRWCtrl.ArlRW = 1;   /* ����� */
	struArlRWCtrl.rsvd = 0;
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));

	for (u32i = 0; u32i < ETHSW_TIMEOUT_VAL; u32i++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));
		if (0 == struArlRWCtrl.ArlStart)
		{
			break;
		}
	}
	
	if (u32i >= ETHSW_TIMEOUT_VAL) /* ��ʱ */
	{
		//if (0 != (g_u32PrintRules & ETHSW_PRINT_TIMEOUT))
		{
			printf("ethsw_lookup_arl_entry:Timed Out!\n");
		}
		return (E_ETHSW_TIMEOUT);
	}
	else  
	{
		/* ��ȡMAC_VID_ENRTY0������,��ȡ8�ֽ����� */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0, (u8 *)pstruMacEntry, EIGHT_BYTE_WIDTH));
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0,(u8 *)test, EIGHT_BYTE_WIDTH));
		//	printf("\ntest is 0x%x-%x-%x-%x-%x-%x-%x-%x\n",test[0],test[1],test[2],test[3],test[4],test[5],test[6],test[7]);


		/* ��ȡDATA_ENRTY0������,��ȡ2�ֽ����� */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,(u8 *)pstruDataEntry, FOUR_BYTE_WIDTH));
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,(u8 *)test, FOUR_BYTE_WIDTH));

		/* ��ȡMAC_VID_ENRTY1������,��ȡ8�ֽ����� */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1, (u8 *)(pstruMacEntry + 1), EIGHT_BYTE_WIDTH));
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1,(u8 *)test, EIGHT_BYTE_WIDTH));
		//	printf("\ntest is 0x%x-%x-%x-%x-%x-%x-%x-%x\n",test[0],test[1],test[2],test[3],test[4],test[5],test[6],test[7]);

		/* ��ȡDATA_ENRTY1������,��ȡ2�ֽ����� */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1,(u8 *)pstruDataEntry, FOUR_BYTE_WIDTH));
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1,(u8 *)test, FOUR_BYTE_WIDTH));
		//	printf("\ntest is 0x%x-%x\n",test[0],test[1]);

	}
	return (E_ETHSW_OK);
	
}

/*******************************************************************************
* ��������: ethsw_add_arl_entry
* ��������: ���һ��ARL��:��ָ��MAC��ַ(VLAN ID)��ARL��д�뵽оƬ�ڲ�����Memory��
* ����ĵ�:
* ��������:
* ��������:      ����                    ����/���  ����
* u8CastType     u8                      ����        �����鲥���,0:����,1:�鲥
* pstruMacPort   STRU_ETHSW_MAC_PORT*    ����        ���˽ṹ��ӦMAC��ַ��˿ڲ�
                                                     �뵽ARL����
*
* ����ֵ:  s32
*          0:�ɹ�
*          ��������
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
s32 ethsw_add_arl_entry(u8 u8CastType, const STRU_ETHSW_MAC_PORT *pstruMacPort)
{
	s32                     s32Rv1;
	s32                     s32Rv2;           /* ����ֵ */
	u32                     u32i;             /* ѭ������ */
	u32                     u32Entry0 = 0;
	u32                     u32Entry1 = 0;    /* ���,��Ϊ1ʱ��ʾд��ӦEntry�Ĵ��� */
	STRU_ETHSW_ARL_MAC_VID  struMacEntry[2];  /* ��ʱ����,MAC/VID Entry */
	STRU_ETHSW_ARL_DATA     struDataEntry[2]; /* ��ʱ����,DATA Entry */
	STRU_ETHSW_ARL_RW_CTRL  struArlRWCtrl;    /* ��ʱ����,ARL���д�����ṹ */
    u8 test[8];
	u32 u32regval = 0;

	if (NULLPTR == pstruMacPort)
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_add_arl_entry:NULL Pointer!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}

	/* ��0��ʱ���� */
	memset((void *)struMacEntry, 0x00, sizeof(struMacEntry));
	memset((void *)struDataEntry, 0x00, sizeof(struDataEntry));

	RETURN_IF_ERROR(ethsw_lookup_arl_entry(pstruMacPort->u8MacAddr, struMacEntry, struDataEntry));

	s32Rv1 = memcmp(struMacEntry[0].u8MacAddr, pstruMacPort->u8MacAddr, 6);
	s32Rv2 = memcmp(struMacEntry[1].u8MacAddr, pstruMacPort->u8MacAddr, 6);

	if (0 == s32Rv1)       /* �Ѿ�����һ��MAC��ַ��ͬ����Ŀ,ֱ�Ӹ��� */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_add_arl_entry:The Same MAC ARL Entry0!\n");
			if (0 == s32Rv2)   /* 2����ͬ��MAC��ַ */
			{
				printf("ethsw_add_arl_entry:Two Same MAC ARLs have EXISTED!\n");
				return (E_ETHSW_FAIL);
			}
		}
		u32Entry0 = 1;
	}
	else if (0 == s32Rv2)   /* �Ѿ�����һ��MAC��ַ��ͬ����Ŀ,ֱ�Ӹ��� */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_add_arl_entry:The Same MAC ARL Entry1!\n");
		}
		u32Entry1 = 1;
	}
	else                    /* û��MAC��ַ��ͬ�ı��� */
	{
		if (0 == struDataEntry[0].valid)
		{
			u32Entry0 = 1;  /* Entry0��Ч,ѡ��д��Entry0 */
		}
		else if (0 == struDataEntry[1].valid)
		{
			u32Entry1 = 1;  /* Entry0��Ч,Entry1��Ч,ѡ��д��Entry1 */
		}
		else  /* Entry0,Entry1����Ч */
		{
			if (0 != (g_u32PrintRules & ETHSW_PRINT_EXCEPTION))
			{
				printf("ethsw_add_arl_entry:ARL Bucket have no Space!\n");
			}
			return (E_ETHSW_BUCKET_FULL);
		} /* end of else Entry0,1����Ч */
	} /* end of else,û��MAC��ַ��ͬ�ı��� */

	memcpy(struMacEntry[0].u8MacAddr, pstruMacPort->u8MacAddr, 6);

	if (1 == u32Entry0)      /* ѡ�����ENTRY0 */
	{
		/* д��ARL TABLE MAC/VID Entry0 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0, (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));
        RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0,(u8 *)test, EIGHT_BYTE_WIDTH));
		//printf("\ntest is 0x%x-%x-%x-%x-%x-%x-%x-%x\n",test[0],test[1],test[2],test[3],test[4],test[5],test[6],test[7]);
		u32regval = (pstruMacPort->u16PortId<<6)|0x38;
        u32regval = swap32(u32regval);
		//printf("u32regval = 0x%x\r\n", u32regval);
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0, (u8 *)&u32regval, FOUR_BYTE_WIDTH));
		//RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,(u8 *)&struDataEntry[0], FOUR_BYTE_WIDTH));
		//printf("\ntest is 0x%x-%x-%x-%x\n",struDataEntry[0].PortId,struDataEntry[0].StaticEntry,struDataEntry[0].age,struDataEntry[0].valid);
		
	}
	else if (1 == u32Entry1) /* ѡ�����ENTRY1 */
	{
		/* д��ARL TABLE MAC/VID Entry1 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1, (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));
        RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0,(u8 *)test, EIGHT_BYTE_WIDTH));
		u32regval = (pstruMacPort->u16PortId<<6)|0x38;
		printf("u32regval = 0x%x\r\n", u32regval);
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1, (u8 *)&u32regval, FOUR_BYTE_WIDTH));
		//RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,(u8 *)&struDataEntry[1], FOUR_BYTE_WIDTH));
		//printf("\ntest is 0x%x-%x-%x-%x\n",struDataEntry[1].PortId,struDataEntry[1].StaticEntry,struDataEntry[1].age,struDataEntry[1].valid);
	}

	struArlRWCtrl.ArlStart = 1; /* ��д��ʼ��� */
	struArlRWCtrl.ArlRW = 0;    /* д��� */
	struArlRWCtrl.rsvd = 0;
	
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));

	for (u32i = 0; u32i < ETHSW_TIMEOUT_VAL; u32i++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));
		if (0 == struArlRWCtrl.ArlStart)
		{
			break;
		}
	}
	if (u32i >= ETHSW_TIMEOUT_VAL) /* ��ʱ */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_TIMEOUT))
		{
			printf("ethsw_add_arl_entry:Timed Out!\n");
		}
		return (E_ETHSW_TIMEOUT);
	}
	
	return (E_ETHSW_OK);
	
}


s32 ethsw_remove_arl_entry(u8 u8CastType, const STRU_ETHSW_MAC_PORT *pstruMacPort)
{
	s32                     s32Rv1;
	s32                     s32Rv2;           /* ����ֵ */
	u32                     u32i;             /* ѭ������ */
	u32                     u32Entry0 = 0;
	u32                     u32Entry1 = 0;    /* ���,��Ϊ1ʱ��ʾд��ӦEntry�Ĵ��� */
	STRU_ETHSW_ARL_MAC_VID  struMacEntry[2];  /* ��ʱ����,MAC/VID Entry */
	STRU_ETHSW_ARL_DATA     struDataEntry[2]; /* ��ʱ����,DATA Entry */
	STRU_ETHSW_ARL_RW_CTRL  struArlRWCtrl;    /* ��ʱ����,ARL���д�����ṹ */
       u8 test[8];

	if (NULLPTR == pstruMacPort)
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_add_arl_entry:NULL Pointer!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}

	/* ��0��ʱ���� */
	memset((void *)struMacEntry, 0x00, sizeof(struMacEntry));
	memset((void *)struDataEntry, 0x00, sizeof(struDataEntry));

	RETURN_IF_ERROR(ethsw_lookup_arl_entry(pstruMacPort->u8MacAddr, struMacEntry, struDataEntry));

	s32Rv1 = memcmp(struMacEntry[0].u8MacAddr, pstruMacPort->u8MacAddr, 6);
	s32Rv2 = memcmp(struMacEntry[1].u8MacAddr, pstruMacPort->u8MacAddr, 6);

	if (0 == s32Rv1)       /* �Ѿ�����һ��MAC��ַ��ͬ����Ŀ,ֱ�Ӹ��� */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_add_arl_entry:The Same MAC ARL Entry0!\n");
			if (0 == s32Rv2)   /* 2����ͬ��MAC��ַ */
			{
				printf("ethsw_add_arl_entry:Two Same MAC ARLs have EXISTED!\n");
				return (E_ETHSW_FAIL);
			}
		}
		
		u32Entry0 = 1;
	}
	else if (0 == s32Rv2)   /* �Ѿ�����һ��MAC��ַ��ͬ����Ŀ,ֱ�Ӹ��� */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_add_arl_entry:The Same MAC ARL Entry1!\n");
		}
		u32Entry1 = 1;
	}
	else                    /* û��MAC��ַ��ͬ�ı��� */
	{
		if (0 == struDataEntry[0].valid)
		{
			u32Entry0 = 1;  /* Entry0��Ч,ѡ��д��Entry0 */
		}
		else if (0 == struDataEntry[1].valid)
		{
			u32Entry1 = 1;  /* Entry0��Ч,Entry1��Ч,ѡ��д��Entry1 */
		}
		else  /* Entry0,Entry1����Ч */
		{
			if (0 != (g_u32PrintRules & ETHSW_PRINT_EXCEPTION))
			{
				printf("ethsw_add_arl_entry:ARL Bucket have no Space!\n");
			}
			return (E_ETHSW_BUCKET_FULL);
		} /* end of else Entry0,1����Ч */
	} /* end of else,û��MAC��ַ��ͬ�ı��� */

	memcpy(struMacEntry[0].u8MacAddr, pstruMacPort->u8MacAddr, 6);

	if (1 == u32Entry0)      /* ѡ�����ENTRY0 */
	{
		/* д��ARL TABLE MAC/VID Entry0 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0, (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));

RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0,(u8 *)test, EIGHT_BYTE_WIDTH));
//printf("\ntest is 0x%x-%x-%x-%x-%x-%x-%x-%x\n",test[0],test[1],test[2],test[3],test[4],test[5],test[6],test[7]);
		
              test[1] = 0x60;


		//RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0, (u8 *)&struDataEntry[0], TWO_BYTE_WIDTH));
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0, (u8 *)test, TWO_BYTE_WIDTH));

//RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,(u8 *)test, TWO_BYTE_WIDTH));
//printf("\ntest is 0x%x-%x\n",test[0],test[1]);
		
	}
	else if (1 == u32Entry1) /* ѡ�����ENTRY1 */
	{
		/* д��ARL TABLE MAC/VID Entry1 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1, (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));
              RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0,(u8 *)test, EIGHT_BYTE_WIDTH));

              test[1] = 0x60;
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1, (u8 *)test, TWO_BYTE_WIDTH));
	}

	struArlRWCtrl.ArlStart = 1; /* ��д��ʼ��� */
	struArlRWCtrl.ArlRW = 0;    /* д��� */
	struArlRWCtrl.rsvd = 0;
	
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));

	for (u32i = 0; u32i < ETHSW_TIMEOUT_VAL; u32i++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));
		if (0 == struArlRWCtrl.ArlStart)
		{
			break;
		}
	}
	if (u32i >= ETHSW_TIMEOUT_VAL) /* ��ʱ */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_TIMEOUT))
		{
			printf("ethsw_add_arl_entry:Timed Out!\n");
		}
		return (E_ETHSW_TIMEOUT);
	}
	
	return (E_ETHSW_OK);
	
}


s32 ethsw_dump_arl_entry(u16 u16ArlNum, STRU_ETHSW_ARL_TABLE *pstruArlTable)
{
	u32                         u32i = 0;         /* ѭ������,�������ָ���Ƿ�Խ�� */
	u32                         u32Count = 0;  /* ������,������Ƕ��Ĵ����Ƿ�ʱ */

	STRU_ETHSW_ARL_SEARCH_CTRL  struSearchCtrl; /* ��ʱ����,����ʱ�Ŀ��ƽṹ */
	STRU_ETHSW_ARL_MAC_VID      struMacEntry;   /* ��ʱ����,MAC/VID Entry */
	STRU_ETHSW_ARL_DATA         struDataEntry;  /* ��ʱ����,DATA Entry */

	struSearchCtrl.ArlStart = 1;
	struSearchCtrl.rsvd = 0;
       struSearchCtrl.valid = 0;
	u8 test;
	u8 readdata[8];
    u32 u32regval = 0;
	/* ��ʼ������Ч��ARL�� */
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_CTRL, (u8 *)&struSearchCtrl, ONE_BYTE_WIDTH));

	//test = 0x80;
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_CTRL, (u8 *)&test, ONE_BYTE_WIDTH));
	//RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_CTRL, (u8 *)&test, ONE_BYTE_WIDTH));

	//printf("\(u8 *)&struSearchCtrl is %d\n",*(u8 *)&struSearchCtrl);
	/* ��ѯ���ҿ��ƼĴ���,�ж���������ARL���Ƿ���Ч,���������Ƿ���� */
	while (1) /* ��ѭ���ж�����,���ڲ��������� */
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_CTRL, (u8 *)&struSearchCtrl, ONE_BYTE_WIDTH));
		//RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_CTRL, (u8 *)&test, ONE_BYTE_WIDTH));
		//printf("\ntest is %d\n",test);

		//printf("\(u8 *)&struSearchCtrl is %d\n",*(u8 *)&struSearchCtrl);

		/* �ж���������ARL�Ƿ���Ч,����Ч�򱣴�,��Ч������Է���ʱ */
		if (0x1 == (struSearchCtrl.valid))       /* ������һ����ЧARL��,�������� */
		{
			u32Count = 0;  /* ��������0 */
			RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_MAC_RESULT, (u8 *)readdata, EIGHT_BYTE_WIDTH));
			printf("\nMAC : 0x%x-%x-%x-%x-%x-%x    ",readdata[5],readdata[4],readdata[3],readdata[2],readdata[1],readdata[0]);
			if(0 == (readdata[5] & 0x1))
	     	{
                RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_DATA_RESULT, (u8 *)readdata, FOUR_BYTE_WIDTH));
                u32regval = (readdata[3]<<24)|(readdata[2]<<16)|(readdata[1]<<8)|(readdata[0]);
                //printf("u32regval = 0x%x\r\n", u32regval);
                if(0 == (u32regval & 0x20))
                 {
                    printf("LEARNING!  ");
                 }
                 else
                 {
                    printf("    STATIC!    ");        /* Static */
                 }
                 printf("port : %d \n",(u32regval>>6)&0x1f);
            }
            else
            {
                RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_DATA_RESULT1, (u8 *)readdata, FOUR_BYTE_WIDTH));
                u32regval = (readdata[3]<<24)|(readdata[2]<<16)|(readdata[1]<<8)|(readdata[0]);
                printf("portmap : 0x%x \n",(u32regval>>6)&0xffff);
            }   			
		}
		else /* 0x00 == struSearchCtrl.valid,��ǰ��������ARL����Ч */
		{
			/* ��������,����������,�������Ƿ�ʱ */
			u32Count++;
			if (u32Count >= ETHSW_TIMEOUT_VAL)
			{
				/* �ڽ��в��ҵ�ʱ��ʱ */
				printf("\ntime out!!!\n");
				return (E_ETHSW_TIMEOUT);
			}
			
		}
		/* ������������Ƿ���� */
		if (0x0 == (struSearchCtrl.ArlStart)) /* �������̽���,ֹͣѭ�� */
		{
			break;
		}
		/*else, 0x01 == struSearchCtrl.ArlStart,��������δ����,do nothing */
	}
	
	return (E_ETHSW_OK);
}

/******************************* Դ�ļ����� ***********************************/

