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

	const char* getProblem() {
		return problem.c_str();
	}

	const char* getAnswer() {
		return answer.c_str();
	}
};

void ReadCsv();
Quiz StartQuiz();