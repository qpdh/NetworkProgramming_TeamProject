#define WRITE 5
#define BUF_SIZE 1024
#include <string>
using namespace std;

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

class SocketScore {
public:
	SOCKET sock;
	int score;
	bool operator==(SocketScore sock2) {
		if (memcmp(&this->sock, &sock2.sock, sizeof(SOCKET))) {
			return (*this == sock2);
		}
		return !(*this == sock2);
	}
private:
	//정답을 맞추면 if() {}
	//해당 소켓의 점수를 1 증가
	//점수가 5에 도달하면
};

vector <SocketScore> vectorSOCKET;

//접속자 점수 체크(출력)
void CheckScore() {
	char* messageBuffer = NULL;
	DWORD bytesTrans;
	for (int i = 0; i < vectorSOCKET.size(); i++) {
		LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		string scoreTmp = to_string(vectorSOCKET.at(i).score);
		bytesTrans = sizeof(scoreTmp) / sizeof(char*);
		ioInfo->wsaBuf.len = bytesTrans;
		ioInfo->wsaBuf.buf = (char*)scoreTmp.c_str();
		ioInfo->rwMode = WRITE;
		WSASend((vectorSOCKET.at(i)).sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
	}
}
