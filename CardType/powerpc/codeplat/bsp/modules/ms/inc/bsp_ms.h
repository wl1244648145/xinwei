#ifndef __BSP_MS_H
#define __BSP_MS_H



/*����״̬*/
#define MCT_MASTER 			1	//Ϊ��
#define MCT_SLAVE 			0	//Ϊ��
#define MCT_AVIABLE 		1	//����
#define MCT_UNAVIABLE		0	//������
#define MCT_OPP_PD_ON		1	//�԰���λ
#define MCT_OPP_PD_OUT		0	//�԰岻��λ
#define MCT_OPP_INRESET		1	//�԰��ڸ�λ
#define MCT_OPP_UNRESET 	0	//�԰�û�и�λ
#define MCT_SLAVE_ONLINE	1	//������λ
#define MCT_SLAVE_UNONLINE	0	//���岻��λ

/*CPLD�Ĵ���*/
#define CPLD_MS_CTR_REG 			76	//���ƼĴ���
#define CPLD_MS_STATE_REG 			73	//״̬�Ĵ���
#define CPLD_MS_SELF_INCIDENT_REG 	74	//�¼��Ĵ���1
#define CPLD_MS_OPP_INCIDENT_REG 	75	//�¼��Ĵ���2

/*״̬�Ĵ���*/
#define SELFSLOT_FLAG 	7	//��������״̬
#define SELFSLOT_IND  	6	//�������״̬
#define	OPPSLOT_FLAG	5	//�԰�����״̬
#define	OPPSLOT_IND		4	//�԰����״̬
#define OPPSLOT_PD		3	//�԰���λ״̬
#define OPPSLOT_RST		2	//�԰帴λ״̬
#define OPPSLOT_CLK		1	//�԰�ʱ��״̬
#define OPPSLOT_OTH		0	//����ʱ������״̬

/*�¼��Ĵ���1*/
#define MCT_SELF_MASTER_to_SLAVE	7	//����������
#define MCT_SELF_SLAVE_to_MASTER	6	//���屸����
#define MCT_SELF_ENABLE_to_DISABLE	5	//������ñ䲻����
#define MCT_SELF_CLOCK_to_DISABLE	4	//����ʱ�ӿ��ñ䲻����

/*�¼��Ĵ���2*/
#define MCT_OPP_MASTER_to_SLAVE		7	//�԰�������
#define MCT_OPP_ENABLE_to_DISABLE	6	//�԰���ñ䲻����
#define MCT_OPP_PD_ON_to_OUT		5	//�԰�γ�
#define MCT_OPP_RESET				4	//�԰帴λ
#define MCT_OPP_CLOCK_to_DISABLE	3	//�԰�ʱ�ӿ��ñ䲻����

/*��ʾ֪ͨBBP/DSP�����л�����*/
#define MCT_MS_SWITCH_EVENT_CLEAR	0	//û�������л�
#define	MCT_MS_SWITCH_INFORMED		1	//��֪ͨBBP/DSP
#define	MCT_MS_SWITCH_RCV_ACK		2	//�յ�BBP/DSP��Ӧ��Ϣ

/*������*/
#define GE0	0
#define GE1	1
#define LMT	2

/*������*/
#define ETH0	0
#define ETH1	1
#define ETH2	2
#define ETH3	3

/*CPLD PLL�Ĵ���*/
#define PLL1_CPLD_CLOCK	3
#define PLL2_CPLD_CLOCK	7

/*CPLD RESET�Ĵ���*/
#define SWITCH_CPLD_RESET	2	

typedef s8 (*MSSWITCH_FUNCPTR)(u8 switchto,u32 cause);
s8(*funcMasterSlaveSwitch)(u8 switchto,u32 cause);		//����ָ�룬ָ��L3ע�ắ��
	
/*�����л�ԭ�򼰲����л�ԭ��*/
#define MCT_MS_SWITCH_CASE_CLEAR 	0x00	//��������л�ԭ��
#define MCT_SWITCH_CASE_OPP_OUT		0x01	//�԰�γ�
#define	MCT_SWITCH_CASE_OPP_RESET 	0x02	//�԰帴λ
#define MCT_SWITCH_CASE_OPP_DISABLE	0x03	//�԰岻����
#define MCT_SWITCH_CASE_COMPETITION	0x04	//��������
#define MCT_SWITCH_CASE_GE0_UNLINK  0x05	//����������û������
#define MCT_SWITCH_CASE_GPS_UNLOCK	0x06	//GPS������������
#define MCT_SWITCH_CASE_CPLD_PLL	0x07	//CPLD PLLʧ��
#define MCT_SWITCH_CASE_SWITCH		0x08	//SWITCH����ʧ��
#define MCT_SWITCH_CASE_CLK_DISABLE	0x09	//����ʱ�Ӳ�����

#endif