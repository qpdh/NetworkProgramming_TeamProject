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
#include "QuizClass.h"

using namespace std;

#pragma comment(lib,"ws2_32.lib")

#define BUF_SIZE 1024
#define READ 3
#define WRITE 5
#define MAX_ROOM_CLNTNUM 4

class SocketScore {
public:
	SOCKET socket;
	string name; // 닉네임
	int login = 0;
	int score; // 점수
	bool ready = false;

	SocketScore(SOCKET sock) {
		this->socket = sock;
		score = 0;
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
	const char* constBuffer;
	int rwMode;
}PER_IO_DATA, * LPPER_IO_DATA;



DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO);

static int nThreadNum = 1;
char tmp5[14] = "[Server] /cls";
bool isGameStart = false;
Quiz nowQuiz;

vector<SocketScore> socketVector;

// 송신자를 제외한 모든 클라이언트에게 보내기
void SendMessageOtherClients(SOCKET sock, DWORD bytesTrans, char* messageBuffer);
// 송신자를 포함한 모든 클라이언트에게 보내기
void SendMessageAllClients(DWORD bytesTrans, char* messageBuffer);
// 명령어 비교(/ + 명령어)
void commandCompare(SOCKET sock, vector<string> commandSplit);
// /help 메시지 송신
void PrintCommand(SOCKET sock);
// 게임 시작
void StartGame();
// 점수 출력
void PrintScore();


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

	//printf("%d : 쓰레드 수\n", (int)sysInfo.dwNumberOfProcessors);

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

		// TODO isGameStart 상태면 슬립시키기
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &addrLen);

		//cout << "현재 접속자 수 : " << vectorSOCKET.size()+1 << endl;
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

		// vector 클라이언트 소켓 추가
		socketVector.push_back(SocketScore(hClntSock));

		//// 화면 초기화
		//SendMessageAllClients(14, tmp5);
		//Sleep(100);

		// &info->overlapped  info 전체 주소 전달
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

			// exit(0) 반환 시
			if (bytesTrans == 0) {
				//대기방에 있는 경우
				auto it = find(socketVector.begin(), socketVector.end(), SocketScore(sock));
				socketVector.erase(it);
				closesocket(sock);
				free(handleInfo);
				free(ioInfo);
				continue;
			}

			// 송신자를 제외한 모두에게 보내기
			//char* charMessageFromClient = new char[ioInfo->wsaBuf.len];
			char* charMessageFromClient = new char[strlen(ioInfo->buffer)];
			strcpy(charMessageFromClient, ioInfo->buffer);
			free(ioInfo);

			//입력 값 검증
			//명령어 인가?
			// 클라이언트가 보낸 메시지 
			// string strMessageFromClient
			charMessageFromClient[bytesTrans] = 0;
			string strMessageFromClient = charMessageFromClient;

			// 이름 부분 자르기
			strMessageFromClient = strMessageFromClient.substr(strMessageFromClient.find(" ") + 1);

			if (strMessageFromClient[0] == '/') {
				//cout << "클라이언트로 부터 명령어 입력입니다." << endl;
				// 명령어 분리하기
				// 2번째 공백 위치
				// ex /test1 test2 -> test
				istringstream CommandSpliter(strMessageFromClient);
				vector<string> commandSplit;
				string stringBuffer;
				while (getline(CommandSpliter, stringBuffer, ' ')) {
					commandSplit.push_back(stringBuffer);
				}

				//중간 함수부분
				commandCompare(sock, commandSplit);
			}

			else {
				// 게임 시작인가?
				if (isGameStart) {
					string answer = nowQuiz.getAnswer();
					if (answer == strMessageFromClient) {

						// 정답 SocketScore 반환
						auto it = find(socketVector.begin(), socketVector.end(), SocketScore(sock));

						// 점수 추가
						it->score += 1;

						string stringMsg = "정답자 [" + it->name + "]";
						char* msg = new char[strlen(stringMsg.c_str())];
						strcpy(msg, stringMsg.c_str());

						SendMessageAllClients(strlen(msg), msg);

						// 5점을 달성한 사람이 있으면 게임 종료
						if (it->score == 5) {
							char msg[] = "게임 종료";
							SendMessageAllClients(strlen(msg), msg);
							// TODO 게임 종료 처리
							// 점수표 출력
							PrintScore();

							for (int i = 0; i < socketVector.size(); i++) {
								socketVector.at(i).ready = false;
								socketVector.at(i).score = 0;

							}
							isGameStart = false;
						}

						// 5점을 넘은 사람이 없으면 다음문제
						else {
							StartGame();
						}
					}
				}
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
// 송신자를 제외한 모든 클라이언트에게 보내기
void SendMessageOtherClients(SOCKET sock, DWORD bytesTrans, char* messageBuffer) {
	//모두에게 보내기

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

// 송신자를 포함한 모든 클라이언트에게 보내기
void SendMessageAllClients(DWORD bytesTrans, char* messageBuffer) {
	//모두에게 보내기

	for (int i = 0; i < socketVector.size(); i++) {
		LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));

		ioInfo->wsaBuf.len = bytesTrans;
		ioInfo->wsaBuf.buf = messageBuffer;
		ioInfo->rwMode = WRITE;
		WSASend(socketVector.at(i).socket, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
	}
}

//명령어 비교(/ + 명령어)
void commandCompare(SOCKET sock, vector<string> commandSplit) {
	LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
	memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
	ioInfo->rwMode = WRITE;

	if (commandSplit.at(0) == "/help") //설명
		PrintCommand(sock);
	else if (commandSplit.at(0) == "/q" || commandSplit.at(0) == "/Q") { //종료
		char msg[] = "프로그램 종료";
		ioInfo->wsaBuf.len = strlen(msg);
		ioInfo->wsaBuf.buf = msg;
		WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
		//exit(1);
	}
	else if (commandSplit.at(0) == "/ready") { // 준비
		auto it = find(socketVector.begin(), socketVector.end(), SocketScore(sock));
		bool* socketReady = &it->ready;


		if (*socketReady) {
			char msg[] = "이미 준비 상태입니다.";
			ioInfo->wsaBuf.len = strlen(msg);
			ioInfo->wsaBuf.buf = msg;
			WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
		}
		else {
			*socketReady = true;
			char msg[] = "준비 했습니다.";
			ioInfo->wsaBuf.len = strlen(msg);
			ioInfo->wsaBuf.buf = msg;

			WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
		}
		if (socketVector.size() > 1) {
			// 시작 가능한지 확인
			for (int i = 0; i < socketVector.size(); i++) {
				if (socketVector.at(i).ready == false)
					return;
			}
			// 시작 하기
			char msg[] = "게임을 시작합니다.";
			ioInfo->wsaBuf.len = strlen(msg);
			ioInfo->wsaBuf.buf = msg;
			SendMessageAllClients(sizeof(msg) / sizeof(char), msg);
			// startgame();
			StartGame();
			isGameStart = true;
		}
	}

	else if (commandSplit.at(0) == "/name") {
		auto it = find(socketVector.begin(), socketVector.end(), SocketScore(sock));
		if (socketVector.at(it - socketVector.begin()).login == 1) {
			//char msg[BUF_SIZE];
			//char name[BUF_SIZE];
			//const char* temp_name = socketVector.at(it - socketVector.begin()).name.c_str();
			//strcpy(name, temp_name);
			//const char* temp_name2 = commandSplit.at(1).c_str();
			//char name2[BUF_SIZE];
			//strcat(name, "이 닉네임을 ");
			//strcat(name2, "로 변경하였습니다."); 
			//strcat(msg, name);
			//strcat(msg, name2);
 			//ioInfo->wsaBuf.len = strlen(msg);
			//ioInfo->wsaBuf.buf = msg;
			//SendMessageAllClients(strlen(msg), msg);
			//WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
			
			
		}
		else {
			//char msg[BUF_SIZE];
			//const char* temp_name = commandSplit.at(1).c_str();
			//strcat(msg, temp_name);
			//strcat(msg, "님이 참가하였습니다.");
			//ioInfo->wsaBuf.len = strlen(msg);
			//ioInfo->wsaBuf.buf = msg;
			socketVector.at(it - socketVector.begin()).login = 1;
			//WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
			//SendMessageAllClients(strlen(msg), msg);
		}
		socketVector.at(it - socketVector.begin()).name = commandSplit.at(1); //이름 저장

		string b = socketVector.at(it - socketVector.begin()).name;
		cout << b << endl; //서버에서 이름 접근 가능
	}


	else {
		char msg[] = "명령어를 확인해주세요";
		ioInfo->wsaBuf.len = strlen(msg);
		ioInfo->wsaBuf.buf = msg;
		WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
	}

}

//명령어 설명 출력(/help)
void PrintCommand(SOCKET sock) {
	LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
	memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
	char msg[] = "/help : 명령어 설명\n/ready : 입장한 방에서 준비\n";

	ioInfo->wsaBuf.len = strlen(msg);
	ioInfo->wsaBuf.buf = msg;
	ioInfo->rwMode = WRITE;
	WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
}


// 게임 시작 (다음문제 출제)
void StartGame() {
	ReadCSV();
	nowQuiz = StartQuiz();

	char* msg = new char[strlen(nowQuiz.getProblem())];
	strcpy(msg, nowQuiz.getProblem());

	SendMessageAllClients(strlen(nowQuiz.getProblem()), msg);
}

// 점수표 출력
void PrintScore() {
	string msg = "이름\t점수\n";
	cout << "이름" << "\t" << "점수" << endl;
	for (int i = 0; i < socketVector.size(); i++) {
		msg += socketVector.at(i).name + "\t" + to_string(socketVector.at(i).score) + "\n";
		cout << socketVector.at(i).name << "\t" << socketVector.at(i).score << endl;
	}
	char* msg2 = new char[strlen(msg.c_str())];
	strcpy(msg2, msg.c_str());
	SendMessageAllClients(strlen(msg2), msg2);
}