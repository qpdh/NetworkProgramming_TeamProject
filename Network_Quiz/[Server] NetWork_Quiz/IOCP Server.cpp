#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <WinSock2.h>
#include <Windows.h>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

#pragma comment(lib,"ws2_32.lib")

#define BUF_SIZE 1024
#define READ 3
#define WRITE 5
#define MAX_ROOM_CLNTNUM 4
class SocketScore {
public:
	SOCKET socket;
	string name; // �г���
	int score = 0; // ����
	bool ready = false;

	SocketScore(SOCKET sock) {
		this->socket = sock;
	}
	int operator== (SocketScore socketScore) {
		if (this->socket == socketScore.socket) {
			return 1;
		}
		return 0;
	}
};

typedef struct {
	SOCKET hClntSock;
	SOCKADDR_IN clntAdr;

}PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

typedef struct {
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	char buffer[BUF_SIZE];
	int rwMode;
}PER_IO_DATA, * LPPER_IO_DATA;



DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO);

static int nThreadNum = 1;
char tmp5[14] = "[Server] /cls";

vector<SocketScore> socketVector;

// �۽��ڸ� ������ ��� Ŭ���̾�Ʈ���� ������
void SendMessageOtherClients(SOCKET sock, DWORD bytesTrans, char* messageBuffer);
// �۽��ڸ� ������ ��� Ŭ���̾�Ʈ���� ������
void SendMessageAllClients(DWORD bytesTrans, char* messageBuffer);
// ��ɾ� ��(/ + ��ɾ�)
void commandCompare(SOCKET sock, vector<string> commandSplit);
// /help �޽��� �۽�
void PrintCommand(SOCKET sock);

int main() {

	WSADATA wsaData;
	HANDLE hComPort;
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	LPPER_HANDLE_DATA handleInfo;

	SOCKET hServSock;
	SOCKADDR_IN servAdr;

	DWORD recvBytes, flags = 0;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&sysInfo);

	for (int i = 0; i < (int)sysInfo.dwNumberOfProcessors; i++) {
		_beginthreadex(NULL, 0, (_beginthreadex_proc_type)EchoThreadMain, (LPVOID)hComPort, 0, NULL);
	}

	//printf("%d : ������ ��\n", (int)sysInfo.dwNumberOfProcessors);

	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(20000);

	bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr));
	listen(hServSock, 5);

	while (1) {
		SOCKET hClntSock;
		SOCKADDR_IN clntAdr;
		int addrLen = sizeof(clntAdr);

		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &addrLen);

		//cout << "���� ������ �� : " << vectorSOCKET.size()+1 << endl;
		//PrintRoomInfo();

		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAdr), &clntAdr, addrLen);

		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);

		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->rwMode = READ;

		// vector Ŭ���̾�Ʈ ���� �߰�
		socketVector.push_back(SocketScore(hClntSock));

		//// ȭ�� �ʱ�ȭ
		//SendMessageAllClients(14, tmp5);
		//Sleep(100);

		// &info->overlapped  info ��ü �ּ� ����
		WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, &recvBytes, &flags, &(ioInfo->overlapped), NULL);
	}
	return 0;

}

DWORD WINAPI EchoThreadMain(LPVOID pComPort) {
	HANDLE hComPort = (HANDLE)pComPort;
	SOCKET sock;
	DWORD bytesTrans;
	LPPER_HANDLE_DATA handleInfo;
	LPPER_IO_DATA ioInfo;
	DWORD flags = 0;

	int nNum = nThreadNum;
	nThreadNum++;

	while (1) {
		
		GetQueuedCompletionStatus(hComPort, &bytesTrans, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
		sock = handleInfo->hClntSock;

		if (ioInfo->rwMode == READ) {
			//printf("%d : received!\n", nNum);
			
			// exit(0) ��ȯ ��
			if (bytesTrans == 0) {
				//���濡 �ִ� ���
				auto it = find(socketVector.begin(), socketVector.end(), SocketScore(sock));
				socketVector.erase(it);
				closesocket(sock);
				free(handleInfo);
				free(ioInfo);
				continue;
			}


			

			// �۽��ڸ� ������ ��ο��� ������
			//char* charMessageFromClient = new char[ioInfo->wsaBuf.len];
			char* charMessageFromClient = new char[strlen(ioInfo->buffer)];
			strcpy(charMessageFromClient, ioInfo->buffer);
			free(ioInfo);

			///////////////////////////
			//�Է� �� ����
			//��ɾ� �ΰ�?
			// Ŭ���̾�Ʈ�� ���� �޽��� 
			// string strMessageFromClient
			//
			charMessageFromClient[bytesTrans] = 0;
			string strMessageFromClient = charMessageFromClient;

			// �̸� �κ� �ڸ���
			strMessageFromClient = strMessageFromClient.substr(strMessageFromClient.find(" ") + 1);
			//cout << "Ŭ���̾�Ʈ�� ���� �޽��� : " << strMessageFromClient << endl;

			if (strMessageFromClient[0] == '/') {
				//cout << "Ŭ���̾�Ʈ�� ���� ��ɾ� �Է��Դϴ�." << endl;
				// ��ɾ� �и��ϱ�
				// 2��° ���� ��ġ
				// ex /test1 test2 -> test
				istringstream CommandSpliter(strMessageFromClient);
				vector<string> commandSplit;
				string stringBuffer;
				while (getline(CommandSpliter, stringBuffer, ' ')) {
					commandSplit.push_back(stringBuffer);
				}
				//cout << "�Էµ� ��ɾ� : " << commandSplit.at(0) << endl;


				//�߰� �Լ��κ�
				commandCompare(sock, commandSplit);
			}

			else {
				SendMessageOtherClients(sock, bytesTrans, charMessageFromClient);
			}



			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));

			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;

			WSARecv(sock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
		}
		else {
			//puts("message sent!");
			free(ioInfo);
		}

	}
	return 0;
}
// �۽��ڸ� ������ ��� Ŭ���̾�Ʈ���� ������
void SendMessageOtherClients(SOCKET sock, DWORD bytesTrans, char* messageBuffer) {
	//��ο��� ������

	for (int i = 0; i < socketVector.size(); i++) {
		if (memcmp(&sock, &socketVector.at(i), sizeof(SOCKET)) != 0) {
			LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));

			ioInfo->wsaBuf.len = bytesTrans;
			ioInfo->wsaBuf.buf = messageBuffer;
			ioInfo->rwMode = WRITE;
			WSASend(socketVector.at(i).socket, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);

		}
	}
}

// �۽��ڸ� ������ ��� Ŭ���̾�Ʈ���� ������
void SendMessageAllClients(DWORD bytesTrans, char* messageBuffer) {
	//��ο��� ������

	for (int i = 0; i < socketVector.size(); i++) {
			LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));

			ioInfo->wsaBuf.len = bytesTrans;
			ioInfo->wsaBuf.buf = messageBuffer;
			ioInfo->rwMode = WRITE;
			WSASend(socketVector.at(i).socket, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
	}
}

//��ɾ� ��(/ + ��ɾ�)
void commandCompare(SOCKET sock, vector<string> commandSplit) {
	LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
	memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
	ioInfo->rwMode = WRITE;



	if (commandSplit.at(0) == "/help") //����
		PrintCommand(sock);
	else if (commandSplit.at(0) == "/q" || commandSplit.at(0) == "/Q") { //����
		char msg[] = "���α׷� ����";
		ioInfo->wsaBuf.len = strlen(msg);
		ioInfo->wsaBuf.buf = msg;
		WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
		exit(1);
	}
	else if (commandSplit.at(0) == "/ready") { // �غ�
		auto it = find(socketVector.begin(), socketVector.end(), SocketScore(sock));
		bool* socketReady = &it->ready;

		if (*socketReady) {
			char msg[] = "�̹� �غ� �����Դϴ�.";
			ioInfo->wsaBuf.len = strlen(msg);
			ioInfo->wsaBuf.buf = msg;
			WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
		}
		else {
			*socketReady = true;
			char msg[] = "�غ� �߽��ϴ�.";
			ioInfo->wsaBuf.len = strlen(msg);
			ioInfo->wsaBuf.buf = msg;
			WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
		}

		// ���� �������� Ȯ��
		for (int i = 0; i < socketVector.size(); i++) {
			if (socketVector.at(i).ready == false)
				return;
		}

		// ���� �ϱ�
		char msg[] = "������ �����մϴ�.";
		ioInfo->wsaBuf.len = strlen(msg);
		ioInfo->wsaBuf.buf = msg;
		SendMessageAllClients(sizeof(msg) / sizeof(char), msg);
		
	}

	else {
		char msg[] = "��ɾ Ȯ�����ּ���";
		ioInfo->wsaBuf.len = strlen(msg);
		ioInfo->wsaBuf.buf = msg;
		WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
	}

}

//��ɾ� ���� ���(/help)
void PrintCommand(SOCKET sock) {
	LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
	memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
	char msg[] = "/help : ��ɾ� ����\n/ready : ������ �濡�� �غ�\n/start : ������ ���� ����";
	ioInfo->wsaBuf.len = strlen(msg);
	ioInfo->wsaBuf.buf = msg;
	ioInfo->rwMode = WRITE;
	WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
}