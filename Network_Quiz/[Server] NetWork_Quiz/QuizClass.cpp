#pragma once
#include "QuizClass.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>
#include <random>
#include <sstream>

using namespace std;

vector<Quiz> quizVector;

void ReadCSV() {
	fstream fs;
	fs.open("quiztest.csv",ios::in);

	while (!fs.eof()) {
		string quizBuffer[4];
		string lineString;
		for (int i = 0; i < sizeof(quizBuffer)/sizeof(string); i++) {
			getline(fs, lineString, '\n');
			istringstream CommandSpliter(lineString);
			for (int i = 0; i < 4; i++) {

				getline(CommandSpliter, quizBuffer[i], ',');
			}

		}

		quizVector.push_back(Quiz(quizBuffer[0], quizBuffer[1], quizBuffer[2], quizBuffer[3]));

	}
	fs.close();
}

Quiz StartQuiz() {
	srand(time(NULL));

	int randomIndex = rand() % quizVector.size();
	cout << randomIndex << endl;
	
	return quizVector.at(randomIndex);
}
