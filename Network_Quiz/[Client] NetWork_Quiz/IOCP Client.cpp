//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <process.h>
//#include <WinSock2.h>
//#include <Windows.h>
//
//#pragma comment(lib, "ws2_32.lib")
//
//unsigned WINAPI sendMsg(void* arg);
//unsigned WINAPI recvMsg(void* arg);
//
//int isConnected = 0;
//
//int main() {
//	WSADATA wsaData;
//	SOCKET hSock;
//	SOCKADDR_IN sAddr;
//	HANDLE hSendThread, hRecvThread;
//
//	WSAStartup(MAKEWORD(2, 2), &wsaData);
//
//	hSock = socket(PF_INET, SOCK_STREAM, 0);
//
//	memset(&sAddr, 0, sizeof(sAddr));
//	sAddr.sin_family = AF_INET;
//	sAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
//	sAddr.sin_port = htons(30000);
//
//	connect(hSock, (SOCKADDR*)&sAddr, sizeof(sAddr));
//
//	isConnected = 1;
//
//	hSendThread = (HANDLE)_beginthreadex(NULL, 0, sendMsg, (void*)&hSock, 0, NULL);
//	hRecvThread = (HANDLE)_beginthreadex(NULL, 0, recvMsg, (void*)&hSock, 0, NULL);
//
//	WaitForSingleObject(hSendThread, INFINITE);
//	WaitForSingleObject(hRecvThread, INFINITE);
//
//	closesocket(hSock);
//	WSACleanup();
//
//	return 0;
//}
//
//unsigned WINAPI sendMsg(void* arg)
//{
//	SOCKET hSock = *((SOCKET*)arg);
//	char msg[5000];
//
//	while (isConnected) {
//		send(hSock, "Hello", sizeof(char) * 5, 0);
//		Sleep(10);
//	}
//
//	return 0;
//}
//
//unsigned WINAPI recvMsg(void* arg)
//{
//	SOCKET hSock = *((SOCKET*)arg);
//	char msg[5000];
//	int nLen;
//
//	while (isConnected) {
//		memset(msg, 0, sizeof(msg));
//		nLen = recv(hSock, msg, sizeof(msg) - 1, 0);
//
//		if (nLen == 0 || nLen == -1) {
//			isConnected = 0;
//			closesocket(hSock);
//
//			printf("Server Disconnected");
//		}
//		else {
//			printf("Receive : %s\n", msg);
//		}
//	}
//
//	return 0;
//}
