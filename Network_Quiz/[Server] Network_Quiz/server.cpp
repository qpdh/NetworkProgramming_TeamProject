#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h> 

#define BUF_SIZE 100
#define MAX_CLNT 256

unsigned WINAPI HandleClnt(void* arg);
void SendMsg(char* msg, int len);

int clntCnt = 0;

// Thread���� ���� Clinet Socket ����
SOCKET clntSocks[MAX_CLNT];

// ������ ����ȭ�� ���� Mutex
HANDLE hMutex;

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAdr, clntAdr;
	int clntAdrSz;
	HANDLE  hThread;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	hMutex = CreateMutex(NULL, FALSE, NULL);
	hServSock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(20000);

	bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr));

	listen(hServSock, 5);

	while (1)
	{
		clntAdrSz = sizeof(clntAdr);
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &clntAdrSz);

		// Socket ����ȭ
		WaitForSingleObject(hMutex, INFINITE);
		//Socket�� ����� ���� Socket �迭�� �߰�
		clntSocks[clntCnt++] = hClntSock;
		ReleaseMutex(hMutex);

		hThread =
			(HANDLE)_beginthreadex(NULL, 0, HandleClnt, (void*)&hClntSock, 0, NULL);
		printf("Connected client IP: %s:%d \n", inet_ntoa(clntAdr.sin_addr), htons(clntAdr.sin_port));
	}
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

unsigned WINAPI HandleClnt(void* arg)
{
	SOCKET hClntSock = *((SOCKET*)arg);
	int strLen = 0, i;
	char msg[BUF_SIZE];

	// Ŭ���̾�Ʈ�κ��� �޼����� ���� �� ó�� ����
	while ((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != 0)
		SendMsg(msg, strLen);

	// Ŭ���̾�Ʈ ���� ������ ����ȭ
	WaitForSingleObject(hMutex, INFINITE);
	for (i = 0; i < clntCnt; i++)   // remove disconnected client
	{
		if (hClntSock == clntSocks[i])
		{
			while (i++ < clntCnt - 1)
				clntSocks[i] = clntSocks[i + 1];
			break;
		}
	}
	clntCnt--;
	ReleaseMutex(hMutex);
	closesocket(hClntSock);
	return 0;
}

// ���� �� ��� Ŭ���̾�Ʈ�� �޼��� ����
void SendMsg(char* msg, int len)   // send to all
{
	int i;

	// �޼��� ���� ����ȭ
	WaitForSingleObject(hMutex, INFINITE);
	for (i = 0; i < clntCnt; i++)
		send(clntSocks[i], msg, len, 0);

	ReleaseMutex(hMutex);
}