#pragma once

#include <iostream>
#include <string>
#include <fstream>
using namespace std;

int main() {
	fstream fs;
	fs.open("quiztest.csv",ios::in);

	

	while (!fs.eof()) {
		string quizBuffer[4];
		for (int i = 0; i < sizeof(quizBuffer)/sizeof(string); i++) {
			getline(fs, quizBuffer[i], ';');
			//cout << quizBuffer[i] << endl;
		}
		cout << "문제 번호 : " << quizBuffer[0] << endl;
		cout << "문제 : " << quizBuffer[1] << endl;
		cout << "정답 : " << quizBuffer[2] << endl;
		cout << "힌트 : " << quizBuffer[3] << endl;

	}
	fs.close();
	return 0;

}
