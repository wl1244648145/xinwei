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
	//�ļ���
	CString myname;
	myname.Format("%s",nameph[fino]);
	CFile myFile;
	//���ļ�
	myFile.Open(myname, Cfile::modeRead | Cfile::typeBinary|Cfile::shareDenyNone); 
	//����ָ��λ�á�
	myFile.Seek(seek,Cfile::begin);
	char m_buf[SIZE];
	int len2;
	int len1;
	len1=len;
	//��ʼ���գ�ֱ�����������ļ�
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
	//�ļ���Ϣ
	fileinfo* fiinfo;
	//���ջ���
	char* m_buf;
	m_buf=new char[100];
	//���������������û��׽���
	SOCKET  pthis=(SOCKET)lpparam;
	//����������Ϣ
	int aa=readn(pthis,m_buf,100);
	//����д�ͷ���
	if(aa<0){
		closesocket (pthis);
		return -1;
	}
	//�Ѵ�������ϢתΪ������ļ���Ϣ
	fiinfo=(fileinfo*)m_buf;
	//CString aaa;
	//����ͻ���˵ʲô
	switch(fiinfo->type)
	{
		//��Ҫ���ļ���Ϣ
	case 0:
		//���ļ�
		aa=sendn(pthis,(char*)zmfile,1080);
		//�д�
		if(aa<0){ 
			closesocket (pthis);
			return -1;
		}
		//����Ϣ��������
		aaa="�յ�LIST����\n";
		//AfxGetMainWnd()->SendMessageToDescendants(WM_AGE1,(LPARAM)aaa.GetBuffer(0),1);
		printf(aaa);
		break;
		//��׼�����ˣ����Դ��ļ��� 

	case 2:
		//���ļ���Ϣ��������
		//aaa.Format("%s  �ļ�������%s\n",zmfile[fiinfo->fileno].name,nameph[fiinfo->fileno]);
		//AfxGetMainWnd()->SendMessageToDescendants(WM_AGE1,(LPARAM)aaa.GetBuffer(0),1);
		printf("\r\n �ļ������� \r\n");	
		//���ļ���������
		readfile(pthis,fiinfo->seek,fiinfo->len,fiinfo->fileno);
		//��������˵ʲô 

	default:
		//aaa="����Э�����\n";
		//AfxGetMainWnd()->SendMessageToDescendants(WM_AGE1,(LPARAM)aaa.GetBuffer(0),1);
		printf("\r\n ����Э�����\r\n");
		break;
	} 

	return 0;
}

DWORD WINAPI listenthread(LPVOID lpparam)
{ 

//���������������׽���
	SOCKET  pthis=(SOCKET)lpparam;
	//��ʼ����
	int rc=listen(pthis,30);
	//��������ʾ��Ϣ
	if(rc<0)
	{
		//CString aaa;
		//aaa="listen����\n";
		////AfxGetMainWnd()->SendMessageToDescendants(WM_AGE1,(LPARAM)aaa.GetBuffer(0),1);
		//AfxGetMainWnd()->SendMessageToDescendants(WM_ACTIVATE,(LPARAM)aaa.GetBuffer(0),1);

		//aaa.ReleaseBuffer();
		return 0;
	}
	//����ѭ���������յ������׽���
	while(1)
	{
		//�½�һ���׽��֣����ڿͻ���
		SOCKET s1;
		s1=accept(pthis,NULL,NULL);

		//��������������������Ϣ
		//CString aa;
		//aa="һ�����룡\n";
		////AfxGetMainWnd()->SendMessageToDescendants(WM_AGE1,(LPARAM)aa.GetBuffer(0),1);
		//AfxGetMainWnd()->SendMessageToDescendants(WM_ACTIVATE,(LPARAM)aa.GetBuffer(0),1);
		//aa.ReleaseBuffer();
		DWORD dwthread;
		//�����û��߳�
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

		// ��ʼ��ѭ��.
		// ��ѭ���������漸������:
		// һ:��ȡ�ͻ��˵�½�͵ǳ���Ϣ,��¼�ͻ��б�
		// ��:ת���ͻ�p2p����
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
			//  ������û�����Ϣ��¼���û��б���
			printf("has a user login : %s\n", recvbuf.message.loginmember.userName);
			stUserListNode *currentuser = new stUserListNode();
			strcpy(currentuser->userName, recvbuf.message.loginmember.userName);
			currentuser->ip = ntohl(sender.sin_addr.S_un.S_addr);
			currentuser->port = ntohs(sender.sin_port);

			ClientList.push_back(currentuser);

			// �����Ѿ���½�Ŀͻ���Ϣ
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
			// ���˿ͻ���Ϣɾ��
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
			// ĳ���ͻ�ϣ�������������һ���ͻ�����һ������Ϣ
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
