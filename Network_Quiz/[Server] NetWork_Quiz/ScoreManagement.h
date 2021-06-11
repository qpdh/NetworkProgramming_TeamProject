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
	//������ ���߸� if() {}
	//�ش� ������ ������ 1 ����
	//������ 5�� �����ϸ�
};

vector <SocketScore> vectorSOCKET;

//������ ���� üũ(���)
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
