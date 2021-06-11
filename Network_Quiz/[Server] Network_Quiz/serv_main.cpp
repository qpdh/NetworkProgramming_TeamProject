////#include <iostream>
////#include <winsock2.h>
////using namespace std;
////
////#define PACKET_SIZE 1024
////SOCKET skt, client_sock;
////
////int main() {
////	WSADATA wsa;
////	WSAStartup(MAKEWORD(2, 2), &wsa);
////
////	skt = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
////
////	SOCKADDR_IN addr = {};
////	addr.sin_family = AF_INET;
////	addr.sin_port = htons(4444);
////	addr.sin_addr.s_addr = htonl(INADDR_ANY);
////
////	bind(skt, (SOCKADDR*)&addr, sizeof(addr));
////	listen(skt, SOMAXCONN);
////
////	SOCKADDR_IN client = {};
////	int client_size = sizeof(client);
////	ZeroMemory(&client, client_size);
////	client_sock = accept(skt, (SOCKADDR*)&client, &client_size);
////
////	closesocket(client_sock);
////	closesocket(skt);
////	WSACleanup();
////}