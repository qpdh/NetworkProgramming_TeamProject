#pragma once

#include <string>
using namespace std;

void ReadCSV(void);


class Quiz {
	string quizNumber;
	string problem;
	string answer;
	string comment;
public:
	Quiz() {}
	Quiz(string quizNumber, string problem, string answer, string comment) {
		this->quizNumber = quizNumber;
		this->problem = problem;
		this->answer = answer;
		this->comment = comment;
	}

	//void printTest() {
	//	cout << "test" << endl;
	//	//cout << quizNumber << " " << problem << " " << answer << " " << comment << std::endl;
	//}

	const char* getProblem() {
		return problem.c_str();
	}

	const char* getAnswer() {
		return answer.c_str();
	}
};

Quiz StartQuiz(void);

