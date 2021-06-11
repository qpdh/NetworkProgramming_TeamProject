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
// 송신자를 제외한 모든 클라이언트에게 보내기
void SendMessageOtherClients(SOCKET sock, DWORD bytesTrans, char* messageBuffer);
// 송신자를 포함한 모든 클라이언트에게 보내기
void SendMessageAllClients(DWORD bytesTrans, char* messageBuffer);
//함수 선언
void commandCompare(SocketScore sock, vector<string> commandSplit);
int joinRoom(SocketScore sock, vector<string> commandSplit);

//점수 체크 및 출력
void CheckScore();
//점수 증가
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

	printf("%d : 쓰레드 수\n", (int)sysInfo.dwNumberOfProcessors);

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

		cout << "현재 접속자 수 : " << vectorSOCKET.size()+1 << endl;
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

		// vector 클라이언트 소켓 추가
		vectorSOCKET.push_back(hClntSock);

		// 화면 초기화
		char tmp5[14] = "[Server] /cls";
		SendMessageAllClients(14, tmp5);

		// 방 정보 보내기
		string strRoomInfo = PrintRoomInfo();
		char* charRoomInfo = new char[strRoomInfo.length()];
		strcpy(charRoomInfo, strRoomInfo.c_str());

		SendMessageAllClients(strRoomInfo.length(), charRoomInfo);

		// &info->overlapped  info 전체 주소 전달
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
			
			// 클라이언트가 접속 종료시 처리
			if (bytesTrans == 0) {
				closesocket(socket.sock);
				free(handleInfo);
				free(ioInfo);
				auto it = find(vectorSOCKET.begin(), vectorSOCKET.end(), socket);
				vectorSOCKET.erase(it);
				continue;
			}


			// 송신자를 제외한 모두에게 보내기
			//char* charMessageFromClient = new char[ioInfo->wsaBuf.len];
			char* charMessageFromClient = new char[strlen(ioInfo->buffer)];
			strcpy(charMessageFromClient, ioInfo->buffer);
			free(ioInfo);

			///////////////////////////
			//입력 값 검증
			//명령어 인가?
			// 클라이언트가 보낸 메시지 
			// string strMessageFromClient
			//
			charMessageFromClient[bytesTrans] = 0;
			string strMessageFromClient = charMessageFromClient;

			// 이름 부분 자르기
			strMessageFromClient = strMessageFromClient.substr(strMessageFromClient.find(" ") + 1);
			//cout << "클라이언트로 부터 메시지 : " << strMessageFromClient << endl;

			if (strMessageFromClient[0] == '/') {
				cout << "클라이언트로 부터 명령어 입력입니다." << endl;
				// 명령어 분리하기
				// 
				// 2번째 공백 위치
				// ex /test1 test2 -> test
				istringstream CommandSpliter(strMessageFromClient);
				vector<string> commandSplit;
				string stringBuffer;
				while (getline(CommandSpliter, stringBuffer, ' ')) {
					commandSplit.push_back(stringBuffer);
				}
				cout << "입력된 명령어 : " << commandSplit.at(0) << endl;


				//중간 함수부분
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

// 송신자를 제외한 모든 클라이언트에게 보내기
void SendMessageOtherClients(SOCKET sock, DWORD bytesTrans, char* messageBuffer) {
	//모두에게 보내기

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


// 송신자를 포함한 모든 클라이언트에게 보내기
void SendMessageAllClients(DWORD bytesTrans, char* messageBuffer) {
	//모두에게 보내기

	for (int i = 0; i < vectorSOCKET.size(); i++) {
		LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));

		ioInfo->wsaBuf.len = bytesTrans;
		ioInfo->wsaBuf.buf = messageBuffer;
		ioInfo->rwMode = WRITE;
		WSASend(vectorSOCKET.at(i).sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
	}
}


//명령어 함수 구현
void commandCompare(SocketScore sockets, vector<string> commandSplit) {
	if (commandSplit.at(0) == "/help")
		cout << "help 입력" << endl;
	else if (commandSplit.at(0) == "/join") {
		cout << "join 입력" << endl;
		joinRoom(sockets, commandSplit);	//방 입장
	}
	else if (commandSplit.at(0) == "/q" || commandSplit.at(0) == "/Q")
		cout << "q or Q 입력" << endl;
	else if (commandSplit.at(0) == "/ready")
		cout << "ready 입력" << endl;
	else if (commandSplit.at(0) == "/start")
		cout << "start 입력" << endl;
}


//방 입장 확인
int joinRoom(SocketScore sock, vector<string> commandSplit) {
	int roomNum = stoi(commandSplit.at(1));

	///////////////////////////////
	// 방 꽉채우기 위한 테스트용 //
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
		cout << roomNum << "방 접속" << endl;
	}


	else {
		// 방이 꽉 찼는가?
		if (vectorRoom.at(roomNum).size() == MAX_ROOM_CLNTNUM) { //클라이언트에게 방이 꽉 찼다고 보냄
			LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			char msg[] = "해당 방이 가득 찼습니다";
			ioInfo->wsaBuf.len = strlen(msg);
			ioInfo->wsaBuf.buf = msg;
			ioInfo->rwMode = WRITE;
			WSASend(sock.sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
		}

	}
	return 0;
}