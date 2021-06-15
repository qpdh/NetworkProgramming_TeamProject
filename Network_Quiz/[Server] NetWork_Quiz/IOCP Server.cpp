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
#define MAX_ROOM 5
#define MAX_ROOM_CLNTNUM 4

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

class{ //�������� �ʿ��ؼ� Ŭ���̾�Ʈ ���� ������ ��� ����ü�� �ʿ���
	SOCKET sock;
	string name; //�г���
	int i = 0; //�� ���� 0:����, 1 ~ n : ����
}CLNT_INFO;

DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO);

static int nThreadNum = 1;
char tmp5[14] = "[Server] /cls";

vector<SOCKET> vectorSOCKET;
vector<vector<SOCKET>> vectorRoom;

// �� ��ȣ ���
string PrintRoomInfo();
// �۽��ڸ� ������ ��� Ŭ���̾�Ʈ���� ������
void SendMessageOtherClients(SOCKET sock, DWORD bytesTrans, char* messageBuffer);
// �۽��ڸ� ������ ��� Ŭ���̾�Ʈ���� ������
void SendMessageAllClients(DWORD bytesTrans, char* messageBuffer);
// ���ӹ濡 �ִ� ��� Ŭ���̾�Ʈ���� �޽��� ������
void SendMessageRoomAllClients(SOCKET sock);
// ��ɾ� ��(/ + ��ɾ�)
void commandCompare(SOCKET sock, vector<string> commandSplit);
// �� ����
void joinRoom(SOCKET sock, vector<string> commandSplit);
// ��ɾ� ���� ���(����)
void printCommand(SOCKET sock);
// ��ɾ� ���� ���(���ӹ�)
void printRoomCommand(SOCKET sock);

int main() {
	for (int i = 0; i < MAX_ROOM; i++) {
		vectorRoom.push_back(vector<SOCKET>());
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
		vectorSOCKET.push_back(hClntSock);

		// ȭ�� �ʱ�ȭ
		SendMessageAllClients(14, tmp5);
		Sleep(100);

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
				int i = 0;
				auto it = find(vectorSOCKET.begin(), vectorSOCKET.end(), sock);
				if (it != vectorSOCKET.end()) { //���濡 �ִ� ���
					closesocket(sock);
					free(handleInfo);
					free(ioInfo);
					vectorSOCKET.erase(it);
				}
				else { //���ӹ濡 �ִ� ���
					while (true) {
						auto it = find(vectorRoom[i].begin(), vectorRoom[i].end(), sock);
						if (it != vectorRoom[i].end()) {
							vectorRoom[i].erase(it);
							break;
						}
						i++;
					}
				}
				// ȭ�� �ʱ�ȭ
				SendMessageAllClients(14, tmp5);

				// �� ���� ������
				string strRoomInfo = PrintRoomInfo();
				char* charRoomInfo = new char[strRoomInfo.length()];
				strcpy(charRoomInfo, strRoomInfo.c_str());

				SendMessageAllClients(strRoomInfo.length(), charRoomInfo);
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
				// 
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

	for (int i = 0; i < vectorSOCKET.size(); i++) {
		if (memcmp(&sock, &vectorSOCKET.at(i), sizeof(SOCKET)) != 0) {
			LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));

			ioInfo->wsaBuf.len = bytesTrans;
			ioInfo->wsaBuf.buf = messageBuffer;
			ioInfo->rwMode = WRITE;
			WSASend(vectorSOCKET.at(i), &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);

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
			WSASend(vectorSOCKET.at(i), &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
	}
}

void SendMessageRoomAllClients(SOCKET sock, int i) {
	;
}

// �� ��ȣ�� ���
string PrintRoomInfo() {
	string roomStr="";
	for (int i = 0; i < MAX_ROOM; i++) {
		roomStr += to_string(i + 1) + "�� �� || �ο� �� " + to_string(vectorRoom.at(i).size())+"/4\n";

		//cout << i + 1 << "�� �� || �ο� �� " << vectorRoom.at(i).size() << "/4" << endl;
	}
	cout << roomStr << endl;
	return roomStr;
}

//��ɾ� ��(/ + ��ɾ�)
void commandCompare(SOCKET sock, vector<string> commandSplit) {
	LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
	memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
	ioInfo->rwMode = WRITE;
	
	int i = 0;
	auto it = find(vectorSOCKET.begin(), vectorSOCKET.end(), sock);

	if (it != vectorSOCKET.end()) { //����
		if (commandSplit.at(0) == "/help") //����
			printCommand(sock);
		else if (commandSplit.at(0) == "/join") //�� ����
			joinRoom(sock, commandSplit);
		else if (commandSplit.at(0) == "/q" || commandSplit.at(0) == "/Q") { //����
			int i = 0;
			auto it = find(vectorSOCKET.begin(), vectorSOCKET.end(), sock);
			char msg[] = "���� ����";
			ioInfo->wsaBuf.len = strlen(msg);
			ioInfo->wsaBuf.buf = msg;
			WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);

			// ȭ�� �ʱ�ȭ
			SendMessageAllClients(14, tmp5);

			// �� ���� ������
			string strRoomInfo = PrintRoomInfo();
			char* charRoomInfo = new char[strRoomInfo.length()];
			strcpy(charRoomInfo, strRoomInfo.c_str());

			SendMessageAllClients(strRoomInfo.length(), charRoomInfo);
		}
		else {
			char msg[] = "��ɾ Ȯ�����ּ���";
			ioInfo->wsaBuf.len = strlen(msg);
			ioInfo->wsaBuf.buf = msg;
			WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
		}
	}
	else { //���ӹ�
		if (commandSplit.at(0) == "/help") //����
			printRoomCommand(sock);
		else if (commandSplit.at(0) == "/ready") //�غ�
			cout << "ready �Է�" << endl;
		else if (commandSplit.at(0) == "/start") //����(���常)
			cout << "start �Է�" << endl;
		else if (commandSplit.at(0) == "/q" || commandSplit.at(0) == "/Q"){ //�泪����
			while (true) {
				auto it = find(vectorRoom[i].begin(), vectorRoom[i].end(), sock);
				if (it != vectorRoom[i].end()) {
					vectorSOCKET.push_back(sock);
					vectorRoom[i].erase(it);
					break;
				}
				i++;
			}
			// ȭ�� �ʱ�ȭ
			SendMessageAllClients(14, tmp5);

			// �� ���� ������
			string strRoomInfo = PrintRoomInfo();
			char* charRoomInfo = new char[strRoomInfo.length()];
			strcpy(charRoomInfo, strRoomInfo.c_str());

			SendMessageAllClients(strRoomInfo.length(), charRoomInfo);
		}
		else {
			char msg[] = "��ɾ Ȯ�����ּ���";
			ioInfo->wsaBuf.len = strlen(msg);
			ioInfo->wsaBuf.buf = msg;
			WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
		}
	}
}

//��ɾ� ���� ���(/help)
void printCommand(SOCKET sock) {
	LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
	memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
	char msg[] = "/help : ��ɾ� ����\n/join ���� : �ش� ���� �� ����\n/q or /Q : ����\n";
	ioInfo->wsaBuf.len = strlen(msg);
	ioInfo->wsaBuf.buf = msg;
	ioInfo->rwMode = WRITE;
	WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
}

void printRoomCommand(SOCKET sock) {
	LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
	memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
	char msg[] = "/help : ��ɾ� ����\n/q or /Q : �� ������\n/ready : ������ �濡�� �غ�\n/start : ������ ���� ����";
	ioInfo->wsaBuf.len = strlen(msg);
	ioInfo->wsaBuf.buf = msg;
	ioInfo->rwMode = WRITE;
	WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
}

//�� ����
void joinRoom(SOCKET sock, vector<string> commandSplit) {
	int roomNum = stoi(commandSplit.at(1)) - 1;
	LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
	memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
	ioInfo->rwMode = WRITE;

	///////////////////////////////
	// �� ��ä��� ���� �׽�Ʈ�� //
	///////////////////////////////
	//vectorRoom.at(0).push_back(sock);
	//vectorRoom.at(0).push_back(sock);
	//vectorRoom.at(0).push_back(sock);
	//vectorRoom.at(0).push_back(sock);


	if (((roomNum > -1) && (roomNum < MAX_ROOM)) && (vectorRoom.at(roomNum).size() < MAX_ROOM_CLNTNUM)) {
		auto it = find(vectorSOCKET.begin(), vectorSOCKET.end(), sock);
		SOCKET temp = vectorSOCKET.at(it - vectorSOCKET.begin());
		vectorRoom[roomNum].push_back(temp);
		vectorSOCKET.erase(it);
		cout << roomNum << "�� �� ����" << endl;
		/*
		LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = strlen(tmp5);
		ioInfo->wsaBuf.buf = tmp5;
		ioInfo->rwMode = WRITE;
		WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
		*/
		char msg[BUF_SIZE];
		string s = commandSplit.at(1) + "�� �濡 �����߽��ϴ�.";
		strcpy(msg, s.c_str());
		ioInfo->wsaBuf.len = strlen(msg);
		ioInfo->wsaBuf.buf = msg;
		WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);

		// ȭ�� �ʱ�ȭ
		SendMessageAllClients(14, tmp5);
		Sleep(100);

		char msg2[] = "�ش� ���� ���� á���ϴ�";
		ioInfo->wsaBuf.len = strlen(msg2);
		ioInfo->wsaBuf.buf = msg2;
		WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);

		// �� ���� ������
		string strRoomInfo = PrintRoomInfo();
		char* charRoomInfo = new char[strRoomInfo.length()];
		strcpy(charRoomInfo, strRoomInfo.c_str());

		SendMessageAllClients(strRoomInfo.length(), charRoomInfo);
	}
	

	else {
		// ���� �� á�°�?
		if (vectorRoom.at(roomNum).size() == MAX_ROOM_CLNTNUM) { //Ŭ���̾�Ʈ���� ���� �� á�ٰ� ����
			char msg[] = "�ش� ���� ���� á���ϴ�";
			ioInfo->wsaBuf.len = strlen(msg);
			ioInfo->wsaBuf.buf = msg;
			WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
		}

	}
}