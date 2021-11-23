/* Shawn Tripathy and Jose Delgado
 * CSF Assignment 05
 * Calc.cpp
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <map>
#include <sstream> 
#include <string>
#include <stack>
#include <mutex>
#include "calc.h"
//#include "pthread.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::stoi;
using std::vector;
using std::map;
using std::stringstream;
using std::stack;
using std::pair;
using std::mutex;




struct Calc {
	private:
		map<string, int> variables;
		std::mutex mutex;
		pthread_mutex_t lock;
	public:
	// public member functions

	int evalExpr(const std::string &expr, int &result) {
		vector <string> v = tokenize(expr);
		
		if ((v.size() == 1)) { // just a variable or a number
			if (isValid(v.at(0), variables)) { 
				pthread_mutex_lock(&lock);
				int number = getNumber(v.at(0), variables);
				
				result = number;
				pthread_mutex_lock(&lock);
				return 1;
			}
			return 0;
		}
		else if ((v.size() == 3) ) {
			if ((isValid(v.at(0), variables)) && (isValid(v.at(2), variables))) { //number op number
				if (v.at(1) == "+") {
					pthread_mutex_lock(&lock);
					int number = getNumber(v.at(0), variables) + getNumber(v.at(2), variables);
					
					result = number;
					pthread_mutex_unlock(&lock);
					return 1;
				}
				else if (v.at(1) == "-") {
					pthread_mutex_lock(&lock);
					int number = getNumber(v.at(0), variables) - getNumber(v.at(2), variables);
					
					result = number;
					pthread_mutex_unlock(&lock);
					return 1;
				}
				else if (v.at(1) == "*") {
					pthread_mutex_lock(&lock);
					int number = getNumber(v.at(0), variables) * getNumber(v.at(2), variables);
					
					result = number;
					pthread_mutex_unlock(&lock);
					return 1;
				}
				else if ((v.at(1) == "/") && (v.at(2) != "0")) {
					pthread_mutex_lock(&lock);
					int number = getNumber(v.at(0), variables) / getNumber(v.at(2), variables);
					
					result = number;
					pthread_mutex_unlock(&lock);
					return 1;
				}
				return 0;
			}
			else if ((v.at(1) == "=") && (isValid(v.at(2), variables))) { //var = number
				pthread_mutex_lock(&lock);
				int number = getNumber(v.at(2), variables);
				string variable = v.at(0);
				saveVariable(variable, variables, number);
				
				result = number;
				pthread_mutex_unlock(&lock);
				return 1;
			}
			return 0;
		}
		else if ((v.size() == 5 && !(isNumber(v.at(0))) && (v.at(1) == "="))) {
			if ((isValid(v.at(2), variables)) && (isValid(v.at(4), variables))) { //var = number op number
				pthread_mutex_lock(&lock);
				int num1 = getNumber(v.at(2), variables);
				int num2 = getNumber(v.at(4), variables);
				pthread_mutex_unlock(&lock);
				if (v.at(3) == "+") {
					pthread_mutex_lock(&lock);
					int number  = num1 + num2;
					string variable = v.at(0);
					saveVariable(variable, variables, number);
					
					result = number;
					pthread_mutex_unlock(&lock);
					return 1;
				}
				else if (v.at(3) == "-") {
					pthread_mutex_lock(&lock);
					int number  = num1 - num2;
					string variable = v.at(0);
					saveVariable(variable, variables, number);
					
					result = number;
					pthread_mutex_unlock(&lock);
					return 1;
				}
				else if (v.at(3) == "*") {
					pthread_mutex_lock(&lock);
					int number  = num1 * num2;
					string variable = v.at(0);
					saveVariable(variable, variables, number);
					
					result = number;
					pthread_mutex_unlock(&lock);
					return 1;
				}
				else if ((v.at(3) == "/") && (v.at(4) != "0")) {
					pthread_mutex_lock(&lock);
					int number  = num1 / num2;
					string variable = v.at(0);
					saveVariable(variable, variables, number);
					
					result = number;
					pthread_mutex_unlock(&lock);
					return 1;
				}
				return 0;
			}
			return 0;
		}
		return 0;
	}

	private:
	// private member functions
	//tokenize string input
	vector<string> tokenize(const string &expr) {
	vector<string> vec;
        stringstream s(expr);
        std::string tok;
        while (s >> tok) {
        	vec.push_back(tok);
        }
        return vec;
	}

	//function to check if string is a number
	bool isNumber(const string& s) {
		std::string::const_iterator it = s.begin();
		if (s.at(0) == '-') {
			it++;
		}
		while (it != s.end() && std::isdigit(*it)) {
			++it;    
		}
		return !s.empty() && it == s.end();
	}


	//function to checks if value of variable is known
	bool isSavedVariable(const string& variable, map<string, int> &variables) {
		map<string, int>::iterator it;
		it = variables.find(variable);
		if (it != variables.end()) {
			return true;
		}
			else {
				return false;
		}
	}

	//function to convert string operand to number
	int getNumber(const string& s, map<string, int> &variables) {
		int number = 0;
			if (isNumber(s)) {
				number  = stoi (s);
			}	
		else { //must always be save variables
			map<string, int>::iterator it = variables.find(s); 
			number = it->second;
		}
		return number;
	}


	//function to check if an operand is valid
	bool isValid(const string& s, map<string, int> &variables) {
		if (isNumber(s) || isSavedVariable(s, variables)) {
			return true;
		}
		else {
			return false;
		}
	}

	//function to save variable in library
	void saveVariable(const string& variable, map<string, int> &variables, int &number) {
		map<string, int>::iterator it;
		it = variables.find(variable);
		if (it != variables.end()) {
			it->second = number;
		}
		else {
			variables.insert(pair<string, int>(variable, number));
		}        
	}
};

extern "C" struct Calc *calc_create(void) {
	return new Calc();
}

extern "C" void calc_destroy(struct Calc *calc) {
	delete calc;
}

extern "C" int calc_eval(struct Calc *calc, const char *expr, int *result) {
	return calc->evalExpr(expr, *result);
}

