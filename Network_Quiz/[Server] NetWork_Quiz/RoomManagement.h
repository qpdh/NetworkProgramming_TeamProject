#include "ScoreManagement.h"
using namespace std;
#define MAX_ROOM 5

vector<vector<SocketScore>> vectorRoom;

//�� ��ȣ ���
class RoomManagement {
	int manCount;
	SocketScore* sockets;
public:
	void isFill();
	string PrintRoomInfo();
	//void SendMessageOneClients(SOCKET sock, DWORD bytesTrans, char* messageBuffer);
	//void SendMessageOtherClients(SOCKET sock, DWORD bytesTrans, char* messageBuffer);
	//void SendMessageAllClients(SOCKET sock, DWORD bytesTrans, char* messageBuffer);
	//void getQuiz();
	//string ShowAnswer();
};

string PrintRoomInfo() {
	string roomStr = "";
	for (int i = 0; i < MAX_ROOM; i++) {
		roomStr += to_string(i + 1) + "�� �� || �ο� �� " + to_string(vectorRoom.at(i).size()) + "/4\n";

		//cout << i + 1 << "�� �� || �ο� �� " << vectorRoom.at(i).size() << "/4" << endl;
	}
	cout << roomStr << endl;
	return roomStr;
}

