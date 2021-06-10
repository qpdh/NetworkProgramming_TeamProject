#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h> 
#include <iostream>
using namespace std;

#define BUF_SIZE 100
#define MAX_CLNT 4

unsigned WINAPI HandleClnt(void* arg);
void SendMsg(char* msg, int len);
void ReadyCheck(int hClntcnt);

int clntCnt = 0;

// Thread에서 사용될 Clinet Socket 정보
SOCKET clntSocks[MAX_CLNT];

// 소켓의 동기화를 위한 Mutex
HANDLE hMutex;

#pragma comment(lib, "ws2_32.lib")

bool clntReady[MAX_CLNT] = { true,false,false,false};

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

	listen(hServSock, 3);

	while(clntCnt<MAX_CLNT)
	{
		clntAdrSz = sizeof(clntAdr);
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &clntAdrSz);
		
		// Socket 동기화
		WaitForSingleObject(hMutex, INFINITE);

		//Socket이 연결된 이후 Socket 배열에 추가
		clntSocks[clntCnt++] = hClntSock;
		ReleaseMutex(hMutex);
		cout << "현재 접속자 수" << clntCnt << endl;
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

	// 임계영역 //////////////
	int hClntCount = clntCnt-1;
	//////////////////////////
	int strLen = 0, i;
	char msg[BUF_SIZE];

	switch (hClntCount) {
	case 0:
		send(hClntSock, "방장입니다.", BUF_SIZE, 0);
		break;
	default:
		msg[0] = hClntCount+1+'0';
		msg[1] = 0;
		strcat(msg, "번째 참가자");
		cout << msg << endl;
		send(hClntSock, msg, BUF_SIZE, 0);
		break;
	}

	// 클라이언트로부터 메세지를 받은 후 처리 과정
	while ((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != 0) {
		msg[strLen] = 0;

		if (strcmp(msg, "[DEFAULT] start\n") == 0) {
			cout << "start 입력됨" << endl;
			ReadyCheck(clntCnt);
			
		}

		else if (strcmp(msg, "[DEFAULT] ready\n") == 0) {
			cout << "ready 입력됨" << endl;
			cout << hClntCount << "번째 유저 준비 완료" << endl;
			clntReady[hClntCount] = true;
		}

		else
			SendMsg(msg, strLen);
	}

	// 클라이언트 연결 해제의 동기화
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

// 접속 된 모든 클라이언트의 메세지 전송
void SendMsg(char* msg, int len)   // send to all
{
	int i;

	// 메세지 전송 동기화
	WaitForSingleObject(hMutex, INFINITE);
	for (i = 0; i < clntCnt; i++)
		send(clntSocks[i], msg, len, 0);

	ReleaseMutex(hMutex);
}

void ReadyCheck(int hClntcnt) {
	for (int i = 0; i < clntCnt; i++) {
		if (!clntReady[i]) {
			WaitForSingleObject(hMutex, INFINITE);
			for (i = 0; i < clntCnt; i++)
				send(clntSocks[i], "모든 유저가 준비되지 않았습니다.", BUF_SIZE, 0);

			ReleaseMutex(hMutex);
			return;

		}
	}
	WaitForSingleObject(hMutex, INFINITE);
	for (int i = 0; i < clntCnt; i++)
		send(clntSocks[i], "게임 시작", BUF_SIZE, 0);

	ReleaseMutex(hMutex);

}