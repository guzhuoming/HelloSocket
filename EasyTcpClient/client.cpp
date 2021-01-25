#include "EasyTcpClient.hpp"

#include<thread>
#pragma warning(disable:4996) //scanf报错
using namespace std;

void cmdThread(EasyTcpClient* client)
{

	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			client->Close();
			printf("退出cmdThread线程\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "gzm");
			strcpy(login.PassWord, "gzmmm");
			client->SendData(&login);
			//Sleep(1000);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "gzm");
			client->SendData(&logout);
			//Sleep(1000);
		}
		else {
			printf("不支持的命令\n");
		}
	}
}
int main()
{
	EasyTcpClient client;
	//client.InitSocket();
	client.Connect("192.168.182.1", 4567);

	// 启动UI线程
	thread t1(cmdThread, &client);
	t1.detach();

	/*Login login;
	strcpy(login.userName, "gzm");
	strcpy(login.PassWord, "gzmmm");*/
	while (client.isRun())
	{
		client.OnRun();
		//printf("空闲时间处理其他业务..\n");
		
		//client.SendData(&login);
	}
	client.Close();

	printf("已退出。\n");
	getchar();
	return 0;
}
