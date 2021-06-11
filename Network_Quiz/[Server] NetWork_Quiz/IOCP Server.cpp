#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <Windows.h>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include "RoomManagement.h"
#include "QuizClass.h"

#pragma comment(lib,"ws2_32.lib")

#define MAX_ROOM 5
#define WRITE 5
#define BUF_SIZE 1024
#define READ 3
#define MAX_ROOM_CLNTNUM 4

using namespace std;

string PrintRoomInfo();
// �۽��ڸ� ������ ��� Ŭ���̾�Ʈ���� ������
void SendMessageOtherClients(SOCKET sock, DWORD bytesTrans, char* messageBuffer);
// �۽��ڸ� ������ ��� Ŭ���̾�Ʈ���� ������
void SendMessageAllClients(DWORD bytesTrans, char* messageBuffer);
//�Լ� ����
void commandCompare(SocketScore sock, vector<string> commandSplit);
int joinRoom(SocketScore sock, vector<string> commandSplit);

//���� üũ �� ���
void CheckScore();
//���� ����
//void IncreseScore(SocketScore sock);


DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO);

static int nThreadNum = 1;

int main() {
	for (int i = 0; i < MAX_ROOM; i++) {
		vectorRoom.push_back(vector<SocketScore>());
	}

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

	printf("%d : ������ ��\n", (int)sysInfo.dwNumberOfProcessors);

	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(20000);

	bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr));
	listen(hServSock, 5);

	while (1) {
		SocketScore hClntSock;
		SOCKADDR_IN clntAdr;
		int addrLen = sizeof(clntAdr);

		hClntSock.sock = accept(hServSock, (SOCKADDR*)&clntAdr, &addrLen);

		cout << "���� ������ �� : " << vectorSOCKET.size()+1 << endl;
		PrintRoomInfo();

		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClntSock = hClntSock.sock;
		memcpy(&(handleInfo->clntAdr), &clntAdr, addrLen);

		CreateIoCompletionPort((HANDLE)hClntSock.sock, hComPort, (DWORD)handleInfo, 0);

		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->rwMode = READ;

		// vector Ŭ���̾�Ʈ ���� �߰�
		vectorSOCKET.push_back(hClntSock);

		// ȭ�� �ʱ�ȭ
		char tmp5[14] = "[Server] /cls";
		SendMessageAllClients(14, tmp5);

		// �� ���� ������
		string strRoomInfo = PrintRoomInfo();
		char* charRoomInfo = new char[strRoomInfo.length()];
		strcpy(charRoomInfo, strRoomInfo.c_str());

		SendMessageAllClients(strRoomInfo.length(), charRoomInfo);

		// &info->overlapped  info ��ü �ּ� ����
		WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, &recvBytes, &flags, &(ioInfo->overlapped), NULL);
	}
	return 0;

}

DWORD WINAPI EchoThreadMain(LPVOID pComPort) {
	HANDLE hComPort = (HANDLE)pComPort;
	SocketScore socket;
	DWORD bytesTrans;
	LPPER_HANDLE_DATA handleInfo;
	LPPER_IO_DATA ioInfo;
	DWORD flags = 0;

	int nNum = nThreadNum;
	nThreadNum++;

	while (1) {
		
		GetQueuedCompletionStatus(hComPort, &bytesTrans, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
		socket.sock = handleInfo->hClntSock;

		if (ioInfo->rwMode == READ) {
			printf("%d : received!\n", nNum);
			
			// Ŭ���̾�Ʈ�� ���� ����� ó��
			if (bytesTrans == 0) {
				closesocket(socket.sock);
				free(handleInfo);
				free(ioInfo);
				auto it = find(vectorSOCKET.begin(), vectorSOCKET.end(), socket);
				vectorSOCKET.erase(it);
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
				cout << "Ŭ���̾�Ʈ�� ���� ��ɾ� �Է��Դϴ�." << endl;
				// ��ɾ� �и��ϱ�
				// 
				// 2��° ���� ��ġ
				// ex /test1 test2 -> test
				istringstream CommandSpliter(strMessageFromClient);
				vector<string> commandSplit;
				string stringBuffer;
				while (getline(CommandSpliter, stringBuffer, ' ')) {
					commandSplit.push_back(stringBuffer);
				}
				cout << "�Էµ� ��ɾ� : " << commandSplit.at(0) << endl;


				//�߰� �Լ��κ�
				commandCompare(socket, commandSplit);
			}

			else {
				SendMessageOtherClients(socket.sock, bytesTrans, charMessageFromClient);
			}



			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));

			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;

			WSARecv(socket.sock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
		}
		else {
			puts("message sent!");
			free(ioInfo);
		}

	}
	return 0;
}

// �۽��ڸ� ������ ��� Ŭ���̾�Ʈ���� ������
void SendMessageOtherClients(SOCKET sock, DWORD bytesTrans, char* messageBuffer) {
	//��ο��� ������

	for (int i = 0; i < vectorSOCKET.size(); i++) {
		if (memcmp(&sock, &vectorSOCKET.at(i).sock, sizeof(SOCKET)) != 0) {
			LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));

			ioInfo->wsaBuf.len = bytesTrans;
			ioInfo->wsaBuf.buf = messageBuffer;
			ioInfo->rwMode = WRITE;
			WSASend(vectorSOCKET.at(i).sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);

		}
	}
}


// �۽��ڸ� ������ ��� Ŭ���̾�Ʈ���� ������
void SendMessageAllClients(DWORD bytesTrans, char* messageBuffer) {
	//��ο��� ������

	for (int i = 0; i < vectorSOCKET.size(); i++) {
		LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));

		ioInfo->wsaBuf.len = bytesTrans;
		ioInfo->wsaBuf.buf = messageBuffer;
		ioInfo->rwMode = WRITE;
		WSASend(vectorSOCKET.at(i).sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
	}
}


//��ɾ� �Լ� ����
void commandCompare(SocketScore sockets, vector<string> commandSplit) {
	if (commandSplit.at(0) == "/help")
		cout << "help �Է�" << endl;
	else if (commandSplit.at(0) == "/join") {
		cout << "join �Է�" << endl;
		joinRoom(sockets, commandSplit);	//�� ����
	}
	else if (commandSplit.at(0) == "/q" || commandSplit.at(0) == "/Q")
		cout << "q or Q �Է�" << endl;
	else if (commandSplit.at(0) == "/ready")
		cout << "ready �Է�" << endl;
	else if (commandSplit.at(0) == "/start")
		cout << "start �Է�" << endl;
}


//�� ���� Ȯ��
int joinRoom(SocketScore sock, vector<string> commandSplit) {
	int roomNum = stoi(commandSplit.at(1));

	///////////////////////////////
	// �� ��ä��� ���� �׽�Ʈ�� //
	///////////////////////////////
	vectorRoom.at(0).push_back(sock);
	vectorRoom.at(0).push_back(sock);
	vectorRoom.at(0).push_back(sock);
	vectorRoom.at(0).push_back(sock);


	if (((roomNum > -1) && (roomNum < MAX_ROOM)) && (vectorRoom.at(roomNum).size() < MAX_ROOM_CLNTNUM)) {
		auto it = find(vectorSOCKET.begin(), vectorSOCKET.end(), sock);
		SocketScore temp = vectorSOCKET.at(it - vectorSOCKET.begin());
		vectorRoom[roomNum].push_back(temp);
		vectorSOCKET.erase(it);
		cout << roomNum << "�� ����" << endl;
	}


	else {
		// ���� �� á�°�?
		if (vectorRoom.at(roomNum).size() == MAX_ROOM_CLNTNUM) { //Ŭ���̾�Ʈ���� ���� �� á�ٰ� ����
			LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			char msg[] = "�ش� ���� ���� á���ϴ�";
			ioInfo->wsaBuf.len = strlen(msg);
			ioInfo->wsaBuf.buf = msg;
			ioInfo->rwMode = WRITE;
			WSASend(sock.sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
		}

	}
	return 0;
}