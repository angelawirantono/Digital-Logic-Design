#include<iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>
#include<map>
#include<set>
#include <iterator>
#include <array>
using namespace std;

vector<pair<string*, vector<array<string*, 3>>>> termList;
map<string, int> token_id;
set<string> existState;
vector<string*> token;
char tmp[128];

int main(int argc, char* argv[]) {
	/*string inputFile = "input.kiss";
	string inputDot = "input.dot";
	string outputKiss = "output.kiss";
	string outputDot = "output.dot";*/

	string inputFile = argv[1];
	string outputKiss = argv[2];
	string outputDot = argv[3];

	ifstream inputF;

	inputF.open(inputFile);

	if (inputF.fail()) {
		cout << "Read File Fail\n"; exit(1);
	}
	else cout << "Read File success\n";
	string line = "";
	int inputCmdCount = -1;
	int outputCmdCount = -1;
	int termCount, stateCount, count = 0;
	char reset;
	int numOfTerm = 0;
	while (getline(inputF, line)) {
		stringstream temp(line);
		if (line[0] == '.') {
			string word = "";
			while (temp >> word) {
				if (word == ".start_kiss")continue;
				else if (word == ".end_kiss")break;
				else if (word == ".i")temp >> inputCmdCount;
				else if (word == ".o")temp >> outputCmdCount;
				else if (word == ".p")temp >> termCount;
				else if (word == ".s")temp >> stateCount;
				else if (word == ".r")temp >> reset;
				else {
					cout << "Error! invalid input\n"; exit(1);
				}
			}
		}
		else {
			string* state[4];
			string stateTemp[4];
			string* input, * curState, * nextState, * output;
			for (int i = 0; i < 4; i++) {
				temp >> stateTemp[i];
				/*if (i == 0)*input = *state[i];
				else if (i == 1)*curState = *state[i];
				else if (i == 2)*nextState = *state[i];
				else {
					*output = *state[i];
				*/
			}

			for (int i = 0; i < 4; i++) {
				if (token_id.count(stateTemp[i])) {
					state[i] = token[token_id[stateTemp[i]]];
				}
				else {
					int id = token.size();
					token_id[stateTemp[i]] = id;
					token.push_back(new string(stateTemp[i]));
					state[i] = token[id];
				}
			}
			input = state[0];
			curState = state[1];
			nextState = state[2];
			output = state[3];
		
#ifdef TEST
			cout << "Term " << numOfTerm + 1 << endl;
			cout << "curState: " << *curState << endl;
			cout << "nextState: " << *nextState << endl;
			cout << "in out: " << *input << " " << *output << endl << endl;
#endif // TEST

			numOfTerm++;

			existState.insert(*curState);
			existState.insert(*nextState);

			array < string*, 3 >nextTerm = { nextState, input, output };
			bool exist = false;
			for (int i = 0; i < termList.size(); i++) {
				if (termList[i].first == curState) {
					exist = 1;
					termList[i].second.push_back(nextTerm);
				}
			}
			if (exist == false) {
				termList.push_back(make_pair(curState, vector<array<string*, 3>>(1, nextTerm)));
			}
		}
	}
	inputF.close();

#ifdef TEST
	//DOT input
	ofstream inputD;
	inputD.open(inputDot);
	if (inputD.fail()) {
		cout << "Input Dot failed\n";
		exit(1);
	}
	else cout << "Input DOT successfully created\n";
	inputD << "digraph STG {" << endl;
	inputD << "  rankdir=LR;\n" << endl;
	inputD << "  INIT [shape=point];" << endl;
	for (auto it : existState) {
		sprintf(tmp,
			"  %s [label=\"%s\"];\n", it.c_str(), it.c_str()
		);
		inputD << tmp;
	}
	inputD << endl;
	inputD << "  INIT -> " << reset << ";" << endl;
	for (auto it : termList) {
		string curState = *it.first;
		for (auto jt : it.second) {
			string nextState = *jt[0], input = *jt[1], output = *jt[2];
			sprintf(tmp,
				"  %s -> %s [label=\"%s/%s\"];\n", curState.c_str(), nextState.c_str(), input.c_str(), output.c_str()
			);
			inputD << tmp;
		}
	}
	inputD << "}";
	inputD.close();
#endif // TEST

	for (auto it : termList) {
		auto& term = it.second;
		sort(term.begin(), term.end(), [](const array<string*, 3>& a, const array<string*, 3>& b) {
			return stoi(*a[1]) < stoi(*b[1]);
			});
	}
	bool ok = true;
	do {
		ok = true;
		for (auto it = termList.begin(); it != termList.end(); it++) {
			for (auto jt = it + 1; jt != termList.end();) {
				auto& aTerm = it->second, & bTerm = jt->second;
				bool isEqual = false;
				if (aTerm.size() == bTerm.size()) {
					isEqual = true;
					for (int i = 0; i < aTerm.size(); i++) {
						for (int j = 0; j < 3; j++) {
							if (*aTerm[i][j] != *bTerm[i][j]) {
								isEqual = false; break;
							}
							if (!isEqual) break;
						}
						if (!isEqual) break;
					}
				}
				if (isEqual) {
					ok = false;
					termCount -= bTerm.size();
					stateCount--;
					existState.erase(*jt->first);
					(*jt->first) = (*it->first);
					jt = termList.erase(jt);
					continue;
				}
				jt++;
			}
		}
	} while (!ok);

	//KISS output
	ofstream outputK;
	outputK.open(outputKiss);
	if (outputK.fail()) {
		cout << "Output Kiss failed\n";
		exit(1);
	}
	else cout << "Output Kiss successfully created\n";
	outputK << ".start_kiss" << endl;
	outputK << ".i " << inputCmdCount << endl;
	outputK << ".o " << outputCmdCount << endl;
	outputK << ".p " << termCount << endl;
	outputK << ".s " << stateCount << endl;
	outputK << ".r " << reset << endl;
	for (auto it : termList) {
		for (auto jt : it.second) {
			outputK << *jt[1];
			outputK << " " << *it.first;
			outputK << " " << *jt[0];
			outputK << " " << *jt[2];
			outputK << endl;
		}
	}
	outputK << ".end_kiss";
	outputK.close();

	//DOT output
	ofstream outputD;
	outputD.open(outputDot);
	if (outputD.fail()) {
		cout << "Output Dot failed\n";
		exit(1);
	}
	else cout << "Output DOT successfully created\n";
	outputD << "digraph STG {" << endl;
	outputD << "  rankdir=LR;" << endl;
	outputD << endl;
	outputD << "  INIT [shape=point];" << endl;
	for (auto it : existState) {
		sprintf(tmp,
			"  %s [label=\"%s\"];\n", it.c_str(), it.c_str()
		);
		outputD << tmp;
	}
	outputD << endl;
	outputD << "  INIT -> " << reset << ";" << endl;
	for (auto it : termList) {
		string curState = *it.first;
		for (auto jt : it.second) {
			string nextState = *jt[0], input = *jt[1], output = *jt[2];
			sprintf(tmp,
				"  %s -> %s [label=\"%s/%s\"];\n", curState.c_str(), nextState.c_str(), input.c_str(), output.c_str()
			);
			outputD << tmp;
		}
	}
	outputD << "}";
	outputD.close();
}