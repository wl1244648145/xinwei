/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* Դ�ļ���:           Bsp_ethsw_arl.h 
* ����:                  
* �汾:                                                                  
* ��������:                              
* ����:                                              
*******************************************************************************/

/******************************** �����ļ����� ********************************/

/******************************** ��ͳ������� ********************************/

/******************************** ���Ͷ��� ************************************/

/* ARL����Ϣ�ṹ */
typedef struct tag_STRU_ETHSW_ARL_TABLE
{
	u32 CastType:1,     /* ��ʾ�˱��ǵ����Ļ��Ƕಥ��0:Unicast,1:Multicast */
		VlanId:3,       /* VLAN IDӦ����12λ,�����ڸ��ֶβ���,����������λ */
		valid:1,        /* entry is valid,�Ƿ���Ч,0:��Ч;1:��Ч */
		StaticEntry:1,  /* entry is static,�Ƿ�Ϊ��̬,0:�Ǿ�̬;1:��̬ */
		age:1,          /* entry accessed/learned since ageing process */
		priority:3,     /* only valid in static entries,up to 4 priorities,but 3 bit */
		ChipId:2,       /* chip ID,00:��оƬ */
		PortId:20;      /* port ID,MAC��ַ��Ӧ�Ķ˿ڵ���0~(MAX_USED_PORTS - 1),�鲥ʱΪ�˿�MAP */
	u8  u8MacAddr[6];   /* MAC addr,��˿ڶ�Ӧ��MAC��ַ */
} STRU_ETHSW_ARL_TABLE;

/* ARL���/д���ƽṹ */
typedef struct tag_STRU_ETHSW_ARL_RW_CTRL
{
	u8 ArlStart:1;      /* ARL start/done (1 = start) */
	u8 rsvd:6;          /* reserved */
	u8 ArlRW:1;         /* ARL read/write (1 = read) */
} STRU_ETHSW_ARL_RW_CTRL;

/* MAC��ַ�ṹ */
typedef struct tag_STRU_ETHSW_MAC_ADDR
{
	u8 u8MacAddr[6];
} STRU_ETHSW_MAC_ADDR;

#if 0
/* MAC��ַ�����Ӧ�Ķ˿ں�/�˿�MAP */
typedef struct tag_STRU_ETHSW_MAC_PORT
{      
	u16 u16PortId;           
	u8  u8MacAddr[6];      
} STRU_ETHSW_MAC_PORT;
#endif
#if 1
/* MAC��ַ�����Ӧ�Ķ˿ں�/�˿�MAP */
typedef struct tag_STRU_ETHSW_MAC_PORT
{      
	u8  u8MacAddr[6];  
	u16 u16PortId;           
} STRU_ETHSW_MAC_PORT;
#endif

/*����ARL��ʱ�Ŀ��ƽṹ */
typedef struct tag_STRU_ETHSW_ARL_SEARCH_CTRL
{
	u8 ArlStart:1;         /* ARL search result valid */
	u8 rsvd:6;          /* reserved */
	u8 valid:1;      /* ARL start/done (1=start) */
} STRU_ETHSW_ARL_SEARCH_CTRL;
/* Aging Time Control Register */
typedef struct tag_STRU_ETHSW_AGE_TIME
{
	u32 AgeTime:20;    /* Specifies the aging time in seconds */
	u32	rsvd:12; 
} STRU_ETHSW_AGE_TIME;

#if 0
/* BCM5389��ARL��ʱ,ARL Table MAC/VID Entry0/1�Ĵ����ṹ */
typedef struct tag_STRU_ETHSW_ARL_MAC_VID
{
	u16 rsvd:4;
	u16 VlanId:12;
	u8  u8MacAddr[6];   /* MAC addr,��˿ڶ�Ӧ��MAC��ַ */
} STRU_ETHSW_ARL_MAC_VID;
#endif
/* BCM5389��ARL��ʱ,ARL Table MAC/VID Entry0/1�Ĵ����ṹ */
typedef struct tag_STRU_ETHSW_ARL_MAC_VID
{
	u8  u8MacAddr[6];   /* MAC addr,��˿ڶ�Ӧ��MAC��ַ */
	u16 rsvd:4;
	u16 VlanId:12;
} STRU_ETHSW_ARL_MAC_VID;

#if 0
/* BCM5389��ARL��ʱ,ARL Table Data Entry0/1�Ĵ����ṹ */
typedef struct tag_STRU_ETHSW_ARL_DATA
{
	u16 valid:1;     
	u16 StaticEntry:1;
	u16 age:1;
	u16 priority:3;
	u16 rsvd:1;
	u16 PortId:9;
} STRU_ETHSW_ARL_DATA;
/* BCM5389��ARL��ʱ,ARL Table Data Entry0/1�Ĵ����ṹ */
#endif
typedef struct tag_STRU_ETHSW_ARL_DATA
{
	u32 priority:3;
       u32 valid:1;
       u32 age:1;
       u32 StaticEntry:1;
       u32 PortId:16;
       u32 rsvd:10;
} STRU_ETHSW_ARL_DATA;

s32 ethsw_add_arl_entry(u8 u8CastType, const STRU_ETHSW_MAC_PORT *pstruMacPort);
s32 ethsw_remove_arl_entry(u8 u8CastType, const STRU_ETHSW_MAC_PORT *pstruMacPort);
s32 ethsw_dump_arl_entry(u16 u16ArlNum, STRU_ETHSW_ARL_TABLE *pstruArlTable);



/******************************* ͷ�ļ����� ***********************************/
