#define WIN_32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#include<Windows.h>
#include<stdio.h>
#pragma warning(disable:4996) //scanf����

#pragma comment(lib, "ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_ERROR
};

struct DataHeader
{
	short dataLength;
	short cmd;
};

struct Login
{
	char userName[32];
	char PassWord[32];
};

struct LoginResult
{
	int result;
};

struct Logout
{
	char userName[32];
};

struct LogoutResult
{
	int result;
};

int main()
{
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	//------------------

	//-- ��Socket API��������TCP�ͻ���
	// 1 ����һ��socket�׽���
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock)
	{
		printf("���󣬽���Socketʧ��...\n");
	}
	else {
		printf("�ɹ�������Socket�ɹ�...\n");
	}
	// 2 ���ӷ����� connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//host to net unsigned short
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("�������ӷ�����ʧ��...\n");
	}
	else {
		printf("���ӷ������ɹ�...\n");
	}
	while (true)
	{
		// 3 ������������
		char cmdBuf[128] = {};
		scanf("%s", cmdBuf);
		// 4 ������������
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("�յ�exit������������");
			break;
		}
		else if ( 0== strcmp(cmdBuf, "login")){
			Login login = {"gzm", "gzmmm"};
			DataHeader dh = {sizeof(login), CMD_LOGIN};
			// 5 �������������������
			send(_sock, (const char*)&dh, sizeof(dh), 0);
			send(_sock, (const char*)&login, sizeof(login), 0);
			// ���շ��������ص�����
			DataHeader retHeader = {};
			LoginResult loginRet = {};
			recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginResult: %d \n", loginRet.result);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout = { "gzm" };
			DataHeader dh = { sizeof(logout), CMD_LOGOUT };
			// 5 �������������������
			send(_sock, (const char*)&dh, sizeof(dh), 0);
			send(_sock, (const char*)&logout, sizeof(logout), 0);
			// ���շ��������ص�����
			DataHeader retHeader = {};
			LoginResult logoutRet = {};
			recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char*)&logoutRet, sizeof(logoutRet), 0);
			printf("LogoutResult: %d \n", logoutRet.result);
		}
		else {
			printf("��֧�ֵ�������������롣\n");
		}
	}
	
	// 7 �ر��׽��� closesocket
	closesocket(_sock);
	//------------------------
	//���Windows socket����
	WSACleanup();
	getchar();
	return 0;
}
