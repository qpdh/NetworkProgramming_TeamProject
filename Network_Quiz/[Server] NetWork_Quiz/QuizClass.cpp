#pragma once
#include "QuizClass.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>
#include <random>
using namespace std;

vector<Quiz> quizVector;

void ReadCSV() {
	fstream fs;
	fs.open("quiztest.csv",ios::in);

	while (!fs.eof()) {
		string quizBuffer[4];
		for (int i = 0; i < sizeof(quizBuffer)/sizeof(string); i++) {
			getline(fs, quizBuffer[i], ';');
			//cout << quizBuffer[i] << endl;
		}
		//cout << "문제 번호 : " << quizBuffer[0] << endl;
		//cout << "문제 : " << quizBuffer[1] << endl;
		//cout << "정답 : " << quizBuffer[2] << endl;
		//cout << "힌트 : " << quizBuffer[3] << endl;

		quizVector.push_back(Quiz(quizBuffer[0], quizBuffer[1], quizBuffer[2], quizBuffer[3]));

	}
	fs.close();

	//for (int i = 0; i < quizVector.size(); i++) {
	//	quizVector.at(i).printTest();
	//}



}

Quiz StartQuiz() {
	srand(time(NULL));

	int randomIndex = rand() % quizVector.size();
	cout << randomIndex << endl;
	
	return quizVector.at(randomIndex);
}
