#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h> 
#include <string>
#include <iostream>
using namespace std;

#define BUF_SIZE 1024
#define NAME_SIZE 20

#pragma comment(lib, "ws2_32.lib")

unsigned WINAPI SendMsg(void* arg);
unsigned WINAPI RecvMsg(void* arg);
void ErrorHandling(const char* msg);

//char name[NAME_SIZE] = "[DEFAULT]";
string strName;
string inputName = "default nick";

char msg[BUF_SIZE];

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET hSock;
    SOCKADDR_IN servAdr;
    HANDLE hSndThread, hRcvThread;
    string nameSet;
    char IP[100];
    int PORT;

    cout << "아이피주소 설정 >> ";
    cin >> IP;
    
    while (true) {
        cout << "닉네임 입력 : ";
        cin >> inputName;
        if (inputName == "Server")
            cout << "Server로 닉네임을 설정할 수 없습니다.\n";
        else
            break;
    }
    strName = "[" + inputName + "] ";

    //sprintf(name, "[%s]", inputName);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    hSock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&servAdr, 0, sizeof(servAdr));
    servAdr.sin_family = AF_INET;
    servAdr.sin_addr.s_addr = inet_addr(IP);
    servAdr.sin_port = htons(20000);

    if (connect(hSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
        ErrorHandling("connect() error");

    cout << "접속 중..." << endl;


    char connectMsg[BUF_SIZE];
    int strLen = recv(hSock, connectMsg, BUF_SIZE - 1, 0);
    connectMsg[strLen] = 0;
    if (strLen == 0) {
        cout << "접속 실패" << endl;
        system("PAUSE");
        return -1;
    }
    else {
        cout << connectMsg << endl;
    }


    hSndThread =
        (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&hSock, 0, NULL);
    hRcvThread =
        (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&hSock, 0, NULL);





    string loginMsg = strName + "접속";
    nameSet = strName + "/name" + " " + inputName;
    send(hSock, nameSet.c_str(), nameSet.length(), 0);
    send(hSock, loginMsg.c_str(), loginMsg.length(), 0);

    WaitForSingleObject(hSndThread, INFINITE);
    WaitForSingleObject(hRcvThread, INFINITE);
    closesocket(hSock);
    WSACleanup();
    return 0;
}

unsigned WINAPI SendMsg(void* arg)   // send thread main
{
    SOCKET hSock = *((SOCKET*)arg);
    string strNameMsg;
    string msg;
    while (1)
    {
        
        getline(cin,msg);
        if (msg == "\0")
            continue;
        //cin >> msg;
        /*if (msg == "/q"|| msg == "/Q")
        {
           strNameMsg = strName + "종료";
           cout << "서버와의 연결을 종료합니다." << endl;
           exit(0);
        }*/
        
        if (msg.substr(0, 5) == "/name") {
            inputName = msg.substr(6, msg.size());
            strName = "[" + inputName + "] ";
        }
        strNameMsg = strName + msg;
        send(hSock, strNameMsg.c_str(), strNameMsg.length(), 0);
    }
    return 0;
}

unsigned WINAPI RecvMsg(void* arg)   // read thread main
{
    int hSock = *((SOCKET*)arg);
    char nameMsg[NAME_SIZE + BUF_SIZE];
    int strLen;
    while (1)
    {
        strLen = recv(hSock, nameMsg, BUF_SIZE - 1, 0);
        if (strLen == -1)
            return -1;
        nameMsg[strLen] = 0;
        if (!strcmp(nameMsg, "[Server] /cls"))
            system("cls");
        else if (!strcmp(nameMsg, "프로그램 종료"))
            exit(1);
        else {
            cout << nameMsg << endl;
            //fputs(nameMsg, stdout);
        }
    }
    return 0;
}

void ErrorHandling(const char* msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}