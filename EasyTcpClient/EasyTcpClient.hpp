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
	// 虚析构函数
	virtual ~EasyTcpClient() 
	{
		Close();
	}
	// 初始化socket
	void InitSocket()
	{
		// 启动 WinSock2.x 环境
		#ifdef _WIN32
			//启动Windows socket 2.x环境
			WORD ver = MAKEWORD(2, 2);
			WSADATA dat;
			WSAStartup(ver, &dat);
		#endif

		// 1 建立一个socket套接字
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>关闭旧连接...\n", _sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (INVALID_SOCKET == _sock)
		{
			printf("错误，建立Socket失败...\n");
		}
		else {
			printf("成功，建立Socket=<%d>成功...\n", _sock);
		}
	}
	//连接服务器
	int Connect(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		// 2 连接服务器 connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//host to net unsigned short
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		printf("<socket=%d>正在连接服务器<%s:%d>...\n", _sock, ip, port);
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("<socket=%d>错误，连接服务器<%s:%d>失败...\n", _sock, ip, port);
		}
		else {
			printf("<socket=%d>连接服务器<%s:%d>成功...\n", _sock, ip, port);
		}
		return ret;
	}
	// 关闭socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sock);
			//清除Windows socket环境
			WSACleanup();
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}

	}

	// 处理网络消息
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
				printf("<socket=%d>, select任务结束1\n", _sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock))
				{
					printf("<socket=%d>, select任务结束2\n", _sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	// 是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	// 接收数据 处理粘包 拆分包
	int RecvData(SOCKET _cSock)
	{
		// 缓冲，接收数据
		char szRecv[4096] = {};
		// 5 接收客户端数据
		int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0)
		{
			printf("<socket=%d>与服务器断开连接，任务结束。\n", _cSock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(header);
		return 0;
	}

	// 响应网络消息
	virtual void OnNetMsg(DataHeader* header)
	{

		switch (header->cmd) {
			case CMD_LOGIN_RESULT:
			{
				LoginResult* login = (LoginResult*)header;
				printf("<socket=%d>收到服务端消息：CMD_LOGIN_RESULT, 数据长度：%d\n", _sock, login->dataLength);
			}
			break;
			case CMD_LOGOUT_RESULT:
			{
				LogoutResult* logout = (LogoutResult*)header;
				printf("<socket=%d>收到服务端消息：CMD_LOGOUT_RESULT, 数据长度：%d\n", _sock, logout->dataLength);
			}
			break;
			case CMD_NEW_USER_JOIN:
			{
				NewUserJoin* userJoin = (NewUserJoin*)header;
				printf("<socket=%d>收到服务端消息：CMD_NEW_USER_JOIN, 数据长度：%d\n", _sock, userJoin->dataLength);

			}
			//default:
			//{
			//	DataHeader header = { 0, CMD_ERROR };
			//	send(_cSock, (char*)&header, sizeof(header), 0);
			//}
			//break;
		}
	}
	// 发送数据
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
