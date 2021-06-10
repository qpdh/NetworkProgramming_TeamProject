#pragma once

#include <string>
using namespace std;



class QuizManagement {
	int categoryNumber;
	string quizType;
	int numOfQuiz;

public:
	QuizManagement(int categoryNumber, string quizType, int numOfQuiz) {
		this->categoryNumber = categoryNumber;
		this->quizType = quizType;
		this->numOfQuiz = numOfQuiz;

	}
	
};

class Quiz {
	string quizNumber;
	string problem;
	string answer;
	string comment;
public:
	Quiz(string quizNumber, string problem, string answer, string comment) {
		this->quizNumber = quizNumber;
		this->problem = problem;
		this->answer = answer;
		this->comment = comment;
	}

	void printTest() {
		cout << quizNumber << " " << problem << " " << answer << " " << comment << endl;
	}

	char* getProblem() {
		char* quizProblem = new char[problem.length()+1];
		strcpy(quizProblem, problem.c_str());
		return quizProblem;
	}
};

void ReadCsv();
Quiz StartQuiz();