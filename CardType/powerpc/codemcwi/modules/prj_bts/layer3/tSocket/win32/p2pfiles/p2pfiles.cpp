#pragma comment(lib, "ws2_32.lib")
//#include "afx.h"
//#include "windows.h"
#include "p2pfileserver.h"
#include "proto.h"
#include "exception.h"
#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif

UserList ClientList;

void readfile(SOCKET  so,int seek,int len,int fino)
{
	//文件名
	CString myname;
	myname.Format("%s",nameph[fino]);
	CFile myFile;
	//打开文件
	myFile.Open(myname, Cfile::modeRead | Cfile::typeBinary|Cfile::shareDenyNone); 
	//传到指定位置　
	myFile.Seek(seek,Cfile::begin);
	char m_buf[SIZE];
	int len2;
	int len1;
	len1=len;
	//开始接收，直到发完整个文件
	while(len1>0){
		len2=len>SIZE?SIZE:len;
		myFile.Read(m_buf, len2);
		int aa=sendn(so,m_buf,len2);
		if(aa<0){ 
			closesocket (so);
			break;
		}
		len1=len1-aa;
		len=len-aa;
	}
	myFile.Close();
} 

DWORD WINAPI clientthread(LPVOID lpparam)
{
	//文件消息
	fileinfo* fiinfo;
	//接收缓存
	char* m_buf;
	m_buf=new char[100];
	//监听函数传来的用户套接字
	SOCKET  pthis=(SOCKET)lpparam;
	//读传来的信息
	int aa=readn(pthis,m_buf,100);
	//如果有错就返回
	if(aa<0){
		closesocket (pthis);
		return -1;
	}
	//把传来的信息转为定义的文件信息
	fiinfo=(fileinfo*)m_buf;
	//CString aaa;
	//检验客户想说什么
	switch(fiinfo->type)
	{
		//我要读文件信息
	case 0:
		//读文件
		aa=sendn(pthis,(char*)zmfile,1080);
		//有错
		if(aa<0){ 
			closesocket (pthis);
			return -1;
		}
		//发消息给主函数
		aaa="收到LIST命令\n";
		//AfxGetMainWnd()->SendMessageToDescendants(WM_AGE1,(LPARAM)aaa.GetBuffer(0),1);
		printf(aaa);
		break;
		//我准备好了，可以传文件了 

	case 2:
		//发文件消息给主函数
		//aaa.Format("%s  文件被请求！%s\n",zmfile[fiinfo->fileno].name,nameph[fiinfo->fileno]);
		//AfxGetMainWnd()->SendMessageToDescendants(WM_AGE1,(LPARAM)aaa.GetBuffer(0),1);
		printf("\r\n 文件被请求！ \r\n");	
		//读文件，并传送
		readfile(pthis,fiinfo->seek,fiinfo->len,fiinfo->fileno);
		//听不懂你说什么 

	default:
		//aaa="接收协议错误！\n";
		//AfxGetMainWnd()->SendMessageToDescendants(WM_AGE1,(LPARAM)aaa.GetBuffer(0),1);
		printf("\r\n 接收协议错误！\r\n");
		break;
	} 

	return 0;
}

DWORD WINAPI listenthread(LPVOID lpparam)
{ 

//由主函数传来的套接字
	SOCKET  pthis=(SOCKET)lpparam;
	//开始监听
	int rc=listen(pthis,30);
	//如果错就显示信息
	if(rc<0)
	{
		//CString aaa;
		//aaa="listen错误\n";
		////AfxGetMainWnd()->SendMessageToDescendants(WM_AGE1,(LPARAM)aaa.GetBuffer(0),1);
		//AfxGetMainWnd()->SendMessageToDescendants(WM_ACTIVATE,(LPARAM)aaa.GetBuffer(0),1);

		//aaa.ReleaseBuffer();
		return 0;
	}
	//进入循环，并接收到来的套接字
	while(1)
	{
		//新建一个套接字，用于客户端
		SOCKET s1;
		s1=accept(pthis,NULL,NULL);

		//给主函数发有人联入消息
		//CString aa;
		//aa="一人联入！\n";
		////AfxGetMainWnd()->SendMessageToDescendants(WM_AGE1,(LPARAM)aa.GetBuffer(0),1);
		//AfxGetMainWnd()->SendMessageToDescendants(WM_ACTIVATE,(LPARAM)aa.GetBuffer(0),1);
		//aa.ReleaseBuffer();
		DWORD dwthread;
		//建立用户线程
		::CreateThread(NULL,0,clientthread,(LPVOID)s1,0,&dwthread); 
	}
	return 0;
} 

void InitWinSock()
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Windows sockets 2.2 startup");
		throw Exception("");
	}
	else{
		printf("Using %s (Status: %s)\n",
			wsaData.szDescription, wsaData.szSystemStatus);
		printf("with API versions %d.%d to %d.%d\n\n",
			LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion),
			LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));

	}
}

SOCKET mksock(int type)
{
	SOCKET sock = socket(AF_INET, type, 0);
	if (sock < 0)
	{
		printf("create socket error");
		throw Exception("");
	}
	return sock;
}

stUserListNode GetUser(char *username)
{
	for(UserList::iterator UserIterator=ClientList.begin();
		UserIterator!=ClientList.end();
		++UserIterator)
	{
		if( strcmp( ((*UserIterator)->userName), username) == 0 )
			return *(*UserIterator);
	}
	throw Exception("not find this user");
}

int main(int argc, char* argv[])
{
	try{
		InitWinSock();

		SOCKET PrimaryUDP;
		PrimaryUDP = mksock(SOCK_DGRAM);

		sockaddr_in local;
		local.sin_family=AF_INET;
		local.sin_port= htons(SERVER_PORT); 
		local.sin_addr.s_addr = htonl(INADDR_ANY);
		int nResult=bind(PrimaryUDP,(sockaddr*)&local,sizeof(sockaddr));
		if(nResult==SOCKET_ERROR)
			throw Exception("bind error");

		sockaddr_in sender;
		stMessage recvbuf;
		memset(&recvbuf,0,sizeof(stMessage));

		// 开始主循环.
		// 主循环负责下面几件事情:
		// 一:读取客户端登陆和登出消息,记录客户列表
		// 二:转发客户p2p请求
		for(;;)
		{
			int dwSender = sizeof(sender);
			int ret = recvfrom(PrimaryUDP, (char *)&recvbuf, sizeof(stMessage), 0, (sockaddr *)&sender, &dwSender);
			if(ret <= 0)
			{
				printf("recv error");
				continue;
			}
			else
			{
				int messageType = recvbuf.iMessageType;
				switch(messageType){
	case LOGIN:
		{
			//  将这个用户的信息记录到用户列表中
			printf("has a user login : %s\n", recvbuf.message.loginmember.userName);
			stUserListNode *currentuser = new stUserListNode();
			strcpy(currentuser->userName, recvbuf.message.loginmember.userName);
			currentuser->ip = ntohl(sender.sin_addr.S_un.S_addr);
			currentuser->port = ntohs(sender.sin_port);

			ClientList.push_back(currentuser);

			// 发送已经登陆的客户信息
			int nodecount = (int)ClientList.size();
			sendto(PrimaryUDP, (const char*)&nodecount, sizeof(int), 0, (const sockaddr*)&sender, sizeof(sender));
			for(UserList::iterator UserIterator=ClientList.begin();
				UserIterator!=ClientList.end();
				++UserIterator)
			{
				sendto(PrimaryUDP, (const char*)(*UserIterator), sizeof(stUserListNode), 0, (const sockaddr*)&sender, sizeof(sender)); 
			}

			break;
		}
	case LOGOUT:
		{
			// 将此客户信息删除
			printf("has a user logout : %s\n", recvbuf.message.logoutmember.userName);
			UserList::iterator removeiterator = NULL;
			for(UserList::iterator UserIterator=ClientList.begin();
				UserIterator!=ClientList.end();
				++UserIterator)
			{
				if( strcmp( ((*UserIterator)->userName), recvbuf.message.logoutmember.userName) == 0 )
				{
					removeiterator = UserIterator;
					break;
				}
			}
			if(removeiterator != NULL)
				ClientList.remove(*removeiterator);
			break;
		}
	case P2PTRANS:
		{
			// 某个客户希望服务端向另外一个客户发送一个打洞消息
			printf("%s wants to p2p %s\n",inet_ntoa(sender.sin_addr),recvbuf.message.translatemessage.userName);
			stUserListNode node = GetUser(recvbuf.message.translatemessage.userName);
			sockaddr_in remote;
			remote.sin_family=AF_INET;
			remote.sin_port= htons(node.port); 
			remote.sin_addr.s_addr = htonl(node.ip);

			in_addr tmp;
			tmp.S_un.S_addr = htonl(node.ip);
			printf("the address is %s,and port is %d\n",inet_ntoa(tmp), node.port);

			stP2PMessage transMessage;
			transMessage.iMessageType = P2PSOMEONEWANTTOCALLYOU;
			transMessage.iStringLen = ntohl(sender.sin_addr.S_un.S_addr);
			transMessage.Port = ntohs(sender.sin_port);

			sendto(PrimaryUDP,(const char*)&transMessage, sizeof(transMessage), 0, (const sockaddr *)&remote, sizeof(remote));

			break;
		}

	case GETALLUSER:
		{
			int command = GETALLUSER;
			sendto(PrimaryUDP, (const char*)&command, sizeof(int), 0, (const sockaddr*)&sender, sizeof(sender));

			int nodecount = (int)ClientList.size();
			sendto(PrimaryUDP, (const char*)&nodecount, sizeof(int), 0, (const sockaddr*)&sender, sizeof(sender));

			for(UserList::iterator UserIterator=ClientList.begin();
				UserIterator!=ClientList.end();
				++UserIterator)
			{
				sendto(PrimaryUDP, (const char*)(*UserIterator), sizeof(stUserListNode), 0, (const sockaddr*)&sender, sizeof(sender)); 
			}
			break;
		}
				}
			}
		}

	}
	catch(Exception &e)
	{
		printf(e.GetMessage());
		return 1;
	}

	return 0;
}
