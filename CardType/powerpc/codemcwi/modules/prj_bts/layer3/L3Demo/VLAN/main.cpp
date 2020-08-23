#include <stdio.h>
#include <Windows.h>
extern "C" void ether_pcap();
extern "C" int init_pcap_vars();

int main(int argc, char** argv)
{
	if (argc > 1)
	{
		printf("\r\n\tAbout Vlan.exe");
		printf("\r\n\t--本程序运行于Windows XP，基于WinPCAP基础之上，提供网卡间数据转发功能");
		printf("\r\n\t--目的是测试BTS对VLAN功能的支持，运行本程序的WinXP必须能正确认识并删除");
		printf("\r\n\t  BTS在数据报文中增加的VLAN标识，然后转发到WAN.也必须能把来自WAN的数据");
		printf("\r\n\t  报文增加用户指定的VLAN ID标识，转发给BTS");
		printf("\r\n\t--程序启动时提示用户指定其中一个网卡用于连接BTS，用户可根据网络的连接情况");
		printf("\r\n\t  选择相应的网卡号");
		printf("\r\n\t--程序启动时提示用户是否在转发给BTS的数据报文增加VLAN标识");
		printf("\r\n\t  如果选择Y/y，则会继续提示用户选择一个VLAN ID. VLAN ID必须在0~4095之间");
		printf("\r\n\t--运行本程序的PC必须具备至少2个网卡");
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