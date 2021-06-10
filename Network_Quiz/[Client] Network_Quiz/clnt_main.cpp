#include <iostream>
#include <winsock2.h>
using namespace std;

#define PACKET_SIZE 1024

SOCKET skt;

int main() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	skt = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(4444);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	while (1) {
		if (!connect(skt, (SOCKADDR*)&addr, sizeof(addr))) break;
	}

	closesocket(skt);
	WSACleanup();
}