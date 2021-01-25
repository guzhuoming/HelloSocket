#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_
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
#endif // _WIN32

#include <stdio.h>
#include "MessageHeader.hpp"
class EasyTcpClient
{
	SOCKET _sock;
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	}
	// ����������
	virtual ~EasyTcpClient() 
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
	//���ӷ�����
	int Connect(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		// 2 ���ӷ����� connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//host to net unsigned short
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		printf("<socket=%d>�������ӷ�����<%s:%d>...\n", _sock, ip, port);
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("<socket=%d>�������ӷ�����<%s:%d>ʧ��...\n", _sock, ip, port);
		}
		else {
			printf("<socket=%d>���ӷ�����<%s:%d>�ɹ�...\n", _sock, ip, port);
		}
		return ret;
	}
	// �ر�socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sock);
			//���Windows socket����
			WSACleanup();
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}

	}

	// ����������Ϣ
	bool OnRun()
	{
		if (isRun()) 
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t{ 1, 0 };
			int ret = select(_sock, &fdReads, 0, 0, &t);
			if (ret < 0)
			{
				printf("<socket=%d>, select�������1\n", _sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock))
				{
					printf("<socket=%d>, select�������2\n", _sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	// �Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	// �������� ����ճ�� ��ְ�
	int RecvData(SOCKET _cSock)
	{
		// ���壬��������
		char szRecv[4096] = {};
		// 5 ���տͻ�������
		int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0)
		{
			printf("<socket=%d>��������Ͽ����ӣ����������\n", _cSock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(header);
		return 0;
	}

	// ��Ӧ������Ϣ
	virtual void OnNetMsg(DataHeader* header)
	{

		switch (header->cmd) {
			case CMD_LOGIN_RESULT:
			{
				LoginResult* login = (LoginResult*)header;
				printf("<socket=%d>�յ��������Ϣ��CMD_LOGIN_RESULT, ���ݳ��ȣ�%d\n", _sock, login->dataLength);
			}
			break;
			case CMD_LOGOUT_RESULT:
			{
				LogoutResult* logout = (LogoutResult*)header;
				printf("<socket=%d>�յ��������Ϣ��CMD_LOGOUT_RESULT, ���ݳ��ȣ�%d\n", _sock, logout->dataLength);
			}
			break;
			case CMD_NEW_USER_JOIN:
			{
				NewUserJoin* userJoin = (NewUserJoin*)header;
				printf("<socket=%d>�յ��������Ϣ��CMD_NEW_USER_JOIN, ���ݳ��ȣ�%d\n", _sock, userJoin->dataLength);

			}
			//default:
			//{
			//	DataHeader header = { 0, CMD_ERROR };
			//	send(_cSock, (char*)&header, sizeof(header), 0);
			//}
			//break;
		}
	}
	// ��������
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
private:

};

#endif // !_EasyTcpClient_hpp_
