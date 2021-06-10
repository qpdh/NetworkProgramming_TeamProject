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
	int quizNumber;
	string problem;
	string answer;
	string comment;
public:
	Quiz(int quizNumber, string problem, string answer, string comment) {
		this->quizNumber = quizNumber;
		this->problem = problem;
		this->answer = answer;
		this->comment = comment;
	}
};
