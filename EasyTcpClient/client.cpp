#include "EasyTcpClient.hpp"

#include<thread>
#pragma warning(disable:4996) //scanf����
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
			printf("�˳�cmdThread�߳�\n");
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
			printf("��֧�ֵ�����\n");
		}
	}
}
int main()
{
	EasyTcpClient client;
	//client.InitSocket();
	client.Connect("192.168.182.1", 4567);

	// ����UI�߳�
	thread t1(cmdThread, &client);
	t1.detach();

	/*Login login;
	strcpy(login.userName, "gzm");
	strcpy(login.PassWord, "gzmmm");*/
	while (client.isRun())
	{
		client.OnRun();
		//printf("����ʱ�䴦������ҵ��..\n");
		
		//client.SendData(&login);
	}
	client.Close();

	printf("���˳���\n");
	getchar();
	return 0;
}
