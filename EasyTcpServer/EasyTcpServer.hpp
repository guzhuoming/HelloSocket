#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#include<stdio.h>
#include<vector>
#include"MessageHeader.hpp"
#pragma warning(disable:4996) //scanf报错

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
	//绑定ip和端口号
	int Bind(const char* ip, unsigned short port)
	{
		//if (INVALID_SOCKET == _sock) {
		//	InitSocket();
		//}
		// 2 bind 绑定用于接受客户端连接的网络端口
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
			printf("错误，绑定网络端口失败<%d>...\n", port);
		}
		else {
			printf("绑定网络端口<%d>成功...\n", port);
		}
		return ret;
	}
	//监听端口号
	int Listen(int n)
	{
		// 3 listen 监听网络端口
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			printf("Socket=<%d>错误，监听网络端口失败...\n", _sock);
		}
		else {
			printf("Socket=<%d>监听网络端口成功...\n", _sock);
		}
		return ret;
	}
	//接受客户端连接
	int Accept() 
	{
		// 4 accept 等待接受客户端连接
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
			printf("Socket=<%d>错误，接收到无效客户端SOCKET...\n", (int)_sock);
		}
		else {

			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			g_clients.push_back((int)_cSock);
			printf("Socket=<%d>新客户端加入：socket = %d, IP = %s \n", (int)_sock, (int)_cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return _cSock;
	}
	//关闭Socket
	void Close() 
	{
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
			for (int n = g_clients.size() - 1; n >= 0; n--)
			{
				closesocket(g_clients[n]);
			}

			// 8 关闭套接字closesocket
			closesocket(_sock);
			//------------------------
			//清除Windows socket环境
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
	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			// 伯克利socket
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
			// nfds 是一个整数值，是指fd_set集合中所有描述符(socket)的范围，而不是数量，
			// 即所有文件描述符的最大值+1，在windows中这个参数可以写0
			timeval t = { 1, 0 };
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret < 0)
			{
				printf("select任务结束。\n");
				Close();
				return false;
			}
			// 判断描述符(socket)是否在集合中
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
			//printf("空闲时间处理其他业务..\n");
			return true;
		}
		return false;
		
	}
	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//接收数据 处理粘包 拆分包
	int RecvData(SOCKET _cSock)
	{
		// 缓冲，接收数据
		char szRecv[4096] = {};
		// 5 接收客户端数据
		int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0)
		{
			printf("客户端<Socket=%d>已退出，任务结束。\n", _cSock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(_cSock, header);
		return 0;
	}
	//响应网络消息
	virtual void OnNetMsg(SOCKET _cSock, DataHeader* header)
	{
		switch (header->cmd) {
			case CMD_LOGIN:
			{
				Login* login = (Login*)header;
				printf("收到客户端<Socket=%d>命令：CMD_LOGIN, 数据长度：%d , userName = %s , PassWord = %s\n", _cSock, login->dataLength, login->userName, login->PassWord);
				// 忽略判断用户密码是否正确的过程
				LoginResult ret;
				send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
			}
			break;
			case CMD_LOGOUT:
			{
				Logout* logout = (Logout*)header;
				printf("收到客户端<Socket=%d>命令：CMD_LOGOUT, 数据长度：%d , userName = %s\n", _cSock, logout->dataLength, logout->userName);
				// 忽略判断用户密码是否正确的过程
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
	//发送指定Socket数据
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