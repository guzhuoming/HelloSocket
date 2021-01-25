#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#include<stdio.h>
#include<vector>
#include"MessageHeader.hpp"
#pragma warning(disable:4996) //scanf����

#ifdef _WIN32
#define WIN_32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#include<Windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif
using namespace std;

class EasyTcpServer {
private:
	SOCKET _sock;
	vector<SOCKET> g_clients;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	// ��ʼ��socket
	void InitSocket()
	{
		// ���� WinSock2.x ����
#ifdef _WIN32
	//����Windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif

		// 1 ����һ��socket�׽���
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>�رվ�����...\n", _sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (INVALID_SOCKET == _sock)
		{
			printf("���󣬽���Socketʧ��...\n");
		}
		else {
			printf("�ɹ�������Socket=<%d>�ɹ�...\n", _sock);
		}
	}
	//��ip�Ͷ˿ں�
	int Bind(const char* ip, unsigned short port)
	{
		//if (INVALID_SOCKET == _sock) {
		//	InitSocket();
		//}
		// 2 bind �����ڽ��ܿͻ������ӵ�����˿�
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(4567);//host to net unsigned short
#ifdef _WIN32
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}		
#else
		if (ip) {
			_sin.sin_addr.s_addr = inet_addr(ip);
	}
		else {
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif
	//bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
		{
			printf("���󣬰�����˿�ʧ��<%d>...\n", port);
		}
		else {
			printf("������˿�<%d>�ɹ�...\n", port);
		}
		return ret;
	}
	//�����˿ں�
	int Listen(int n)
	{
		// 3 listen ��������˿�
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			printf("Socket=<%d>���󣬼�������˿�ʧ��...\n", _sock);
		}
		else {
			printf("Socket=<%d>��������˿ڳɹ�...\n", _sock);
		}
		return ret;
	}
	//���ܿͻ�������
	int Accept() 
	{
		// 4 accept �ȴ����ܿͻ�������
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
		if (INVALID_SOCKET == _cSock)
		{
			printf("Socket=<%d>���󣬽��յ���Ч�ͻ���SOCKET...\n", (int)_sock);
		}
		else {

			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			g_clients.push_back((int)_cSock);
			printf("Socket=<%d>�¿ͻ��˼��룺socket = %d, IP = %s \n", (int)_sock, (int)_cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return _cSock;
	}
	//�ر�Socket
	void Close() 
	{
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
			for (int n = g_clients.size() - 1; n >= 0; n--)
			{
				closesocket(g_clients[n]);
			}

			// 8 �ر��׽���closesocket
			closesocket(_sock);
			//------------------------
			//���Windows socket����
			WSACleanup();
#else
			for (int n = g_clients.size() - 1; n >= 0; n--)
			{
				close(g_clients[n]);
			}
			close(_sock);
#endif
		}
	}
	//����������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			// ������socket
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExp;
			// qinglijihe 
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			// jiangmiaoshufu jiarujihe 
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);
			SOCKET	maxSock = _sock;
			for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
				FD_SET(g_clients[n], &fdRead);
				if (maxSock < g_clients[n])
				{
					maxSock = g_clients[n];
				}
			}
			// nfds ��һ������ֵ����ָfd_set����������������(socket)�ķ�Χ��������������
			// �������ļ������������ֵ+1����windows�������������д0
			timeval t = { 1, 0 };
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret < 0)
			{
				printf("select���������\n");
				Close();
				return false;
			}
			// �ж�������(socket)�Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(g_clients[n], &fdRead))
				{
					if (-1 == RecvData(g_clients[n]))
					{
						auto iter = g_clients.begin();// std::vector<SOCKET>::iterator iter = g_clients.begin();
						if (iter != g_clients.end())
						{
							g_clients.erase(iter);
						}
					}
				}
			}
			//printf("����ʱ�䴦������ҵ��..\n");
			return true;
		}
		return false;
		
	}
	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//�������� ����ճ�� ��ְ�
	int RecvData(SOCKET _cSock)
	{
		// ���壬��������
		char szRecv[4096] = {};
		// 5 ���տͻ�������
		int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0)
		{
			printf("�ͻ���<Socket=%d>���˳������������\n", _cSock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(_cSock, header);
		return 0;
	}
	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET _cSock, DataHeader* header)
	{
		switch (header->cmd) {
			case CMD_LOGIN:
			{
				Login* login = (Login*)header;
				printf("�յ��ͻ���<Socket=%d>���CMD_LOGIN, ���ݳ��ȣ�%d , userName = %s , PassWord = %s\n", _cSock, login->dataLength, login->userName, login->PassWord);
				// �����ж��û������Ƿ���ȷ�Ĺ���
				LoginResult ret;
				send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
			}
			break;
			case CMD_LOGOUT:
			{
				Logout* logout = (Logout*)header;
				printf("�յ��ͻ���<Socket=%d>���CMD_LOGOUT, ���ݳ��ȣ�%d , userName = %s\n", _cSock, logout->dataLength, logout->userName);
				// �����ж��û������Ƿ���ȷ�Ĺ���
				LoginResult ret;
				send(_cSock, (char*)&ret, sizeof(ret), 0);
			}
			break;
			default:
			{
				DataHeader header = { 0, CMD_ERROR };
				send(_cSock, (char*)&header, sizeof(header), 0);
			}
			break;
		}
		
	}
	//����ָ��Socket����
	int SendData(SOCKET _cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	int SendDataToAll(DataHeader* header)
	{
		if (isRun() && header)
		{
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				SendData(g_clients[n], header);
			}
		}
		return SOCKET_ERROR;
	}
};
#endif