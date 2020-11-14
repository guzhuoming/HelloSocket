#define WIN_32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#include<Windows.h>
#include<stdio.h>

#pragma comment(lib, "ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_ERROR
};

struct DataHeader
{
	short dataLength;
	short cmd;
};

struct Login: public DataHeader
{
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};

struct LoginResult: public DataHeader
{
	LoginResult() {
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct Logout: public DataHeader
{
	Logout() {
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutResult: public DataHeader
{
	LogoutResult() {
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};

int main()
{
	//启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	//------------------

	//-- 用Socket API建立简易TCP服务端
	// 1 建立一个socket套接字
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock)
	{
		printf("错误，建立Socket失败...\n");
	}
	else {
		printf("成功，建立Socket成功...\n");
	}
	// 2 bind 绑定用于接受客户端连接的网络端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	//bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("错误，绑定网络端口失败...\n");
	}
	else {
		printf("绑定网络端口成功...\n");
	}
	// 3 listen 监听网络端口
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		printf("错误，监听网络端口失败...\n");
	}
	else {
		printf("监听网络端口成功...\n");
	}
	// 4 accept 等待接受客户端连接
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;
	//char msgBuf[] = "Hello, I'm Server.";

	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock)
	{
		printf("错误，接收到无效客户端SOCKET...\n");
	}
	printf("新客户端加入：socket = %d, IP = %s \n", _cSock, inet_ntoa(clientAddr.sin_addr));

	char _recvBuf [128] = {};
	while (true)
	{	
		DataHeader header = {};
		// 5 接收客户端数据
		int nLen = recv(_cSock, (char*)&header, sizeof(header), 0);
		if (nLen <= 0) 
		{	
			printf("客户端已退出，任务结束。");
			break;
		}
		switch (header.cmd) {
			case CMD_LOGIN:
			{
				Login login = {};
				recv(_cSock, (char*)&login + sizeof(DataHeader), sizeof(Login)-sizeof(DataHeader), 0);
				printf("收到命令：CMD_LOGIN, 数据长度：%d , userName = %s , PassWord = %s\n", login.dataLength, login.userName, login.PassWord);
				// 忽略判断用户密码是否正确的过程
				LoginResult ret;
				send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
			}
			break;
			case CMD_LOGOUT:
			{
				Logout logout = {};
				recv(_cSock, (char*)&logout + sizeof(DataHeader), sizeof(Logout) - sizeof(DataHeader), 0);
				printf("收到命令：CMD_LOGOUT, 数据长度：%d , userName = %s\n", logout.dataLength, logout.userName);
				// 忽略判断用户密码是否正确的过程
				LoginResult ret;
				send(_cSock, (char*)&ret, sizeof(ret), 0);
			}
			break;
			default:
				header.cmd = CMD_ERROR;
				header.dataLength = 0;
				send(_cSock, (char*)&header, sizeof(header), 0);

			break;
		}
	}

	// 8 关闭套接字closesocket
	closesocket(_sock);
	//------------------------
	//清除Windows socket环境
	WSACleanup();
	printf("已退出，任务结束。");
	getchar();
	return 0;
}
