#include <stdio.h>
#include <Windows.h>
extern "C" void ether_pcap();
extern "C" int init_pcap_vars();

int main(int argc, char** argv)
{
	if (argc > 1)
	{
		printf("\r\n\tAbout Vlan.exe");
		printf("\r\n\t--������������Windows XP������WinPCAP����֮�ϣ��ṩ����������ת������");
		printf("\r\n\t--Ŀ���ǲ���BTS��VLAN���ܵ�֧�֣����б������WinXP��������ȷ��ʶ��ɾ��");
		printf("\r\n\t  BTS�����ݱ��������ӵ�VLAN��ʶ��Ȼ��ת����WAN.Ҳ�����ܰ�����WAN������");
		printf("\r\n\t  ���������û�ָ����VLAN ID��ʶ��ת����BTS");
		printf("\r\n\t--��������ʱ��ʾ�û�ָ������һ��������������BTS���û��ɸ���������������");
		printf("\r\n\t  ѡ����Ӧ��������");
		printf("\r\n\t--��������ʱ��ʾ�û��Ƿ���ת����BTS�����ݱ�������VLAN��ʶ");
		printf("\r\n\t  ���ѡ��Y/y����������ʾ�û�ѡ��һ��VLAN ID. VLAN ID������0~4095֮��");
		printf("\r\n\t--���б������PC����߱�����2������");
		printf("\r\n");
		return 0;
	}

	init_pcap_vars();
	while(1)
	{
		ether_pcap();
		Sleep(0);
	}
	return 0;
}