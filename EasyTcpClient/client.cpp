#define WIN_32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifdef _WIN32
#include<WinSock2.h>
#include<Windows.h>
#else
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

#include<stdio.h>
#include<thread>
#pragma warning(disable:4996) //scanf����
using namespace std;
#pragma comment(lib, "ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

struct DataHeader
{
	short dataLength;
	short cmd;
};

struct Login : public DataHeader
{
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};

struct LoginResult : public DataHeader
{
	LoginResult() {
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct Logout : public DataHeader
{
	Logout() {
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutResult : public DataHeader
{
	LogoutResult() {
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};

struct NewUserJoin : public DataHeader
{
	NewUserJoin() {
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		scok = 0;
	}
	int scok;
};

int processor(SOCKET _cSock)
{
	// ���壬��������
	char szRecv[4096] = {};
	// 5 ���տͻ�������
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("��������Ͽ����ӣ����������\n");
		return -1;
	}
	switch (header->cmd) {
	case CMD_LOGIN_RESULT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* login = (LoginResult*)szRecv;
		printf("�յ��������Ϣ��CMD_LOGIN_RESULT, ���ݳ��ȣ�%d\n", login->dataLength);
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LogoutResult* logout = (LogoutResult*)szRecv;
		printf("�յ��������Ϣ��CMD_LOGOUT_RESULT, ���ݳ��ȣ�%d\n", logout->dataLength);
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		NewUserJoin* userJoin = (NewUserJoin*)szRecv;
		printf("�յ��������Ϣ��CMD_NEW_USER_JOIN, ���ݳ��ȣ�%d\n", userJoin->dataLength);

	}
	default:
	{
		DataHeader header = { 0, CMD_ERROR };
		send(_cSock, (char*)&header, sizeof(header), 0);
	}
	break;
	}
	return 0;
}
bool g_bRun = true;
void cmdThread(SOCKET _sock)
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("�˳�cmdThread�߳�\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "gzm");
			strcpy(login.PassWord, "gzmmm");
			send(_sock, (const char*)&login, sizeof(Login), 0);
			//Sleep(1000);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "gzm");
			send(_sock, (const char*)&logout, sizeof(Logout), 0);
			//Sleep(1000);
		}
		else {
			printf("��֧�ֵ�����\n");
		}
	}
}
int main()
{
#ifdef _WIN32
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
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
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#else
	_sin.sin_addr.s_addr = inet_addr("192.168.182.1");
#endif
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("�������ӷ�����ʧ��...\n");
	}
	else {
		printf("���ӷ������ɹ�...\n");
	}

	// �����߳�
	thread t1(cmdThread, _sock);
	t1.detach();
	while (g_bRun)
	{
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		timeval t{ 1, 0 };
		int ret = select(_sock, &fdReads, 0, 0, &t);
		if (ret < 0)
		{
			printf("select�������1\n");
			break;
		}
		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);
			if (-1 == processor(_sock))
			{
				printf("select�������2\n");
				break;
			}
		}
		//printf("����ʱ�䴦������ҵ��..\n");

	}

	// 7 �ر��׽��� closesocket

#ifdef _WIN32
	closesocket(_sock);
	//------------------------
	//���Windows socket����
	WSACleanup();
#else
	close(_sock);
#endif
	getchar();
	return 0;
}
