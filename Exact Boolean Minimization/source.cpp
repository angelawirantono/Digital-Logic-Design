#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <cmath>
using namespace std;

int inputVertex = 0;
vector<string> input;

int commandSOP = 0, commandX;
vector<string> sop;
vector<string> x;
set<int> productTerms,
dontCareTerms;

// Data of Quine-VMcCluskey algorithm
struct minterm{
	string binary = "";
	set<int> contentMin;
	bool used = false;
};
vector<vector<minterm> > oneNumber;

// Data of Petrick's method
vector<minterm> asterisk;
struct sopTerm{
	int sopIndex = 0;
	vector<int> ptr;
};
vector<sopTerm> petrick;
vector<int> expand;
bool first = true;

struct answer {
	vector<int> fewestTerms;
	int totalTerm = 0;
	int	totalLiteral = 0;
}minimalist;

// Save output amount and Vertexs' name
int outputVertex = 0;
vector<string> output;

void recursivePetrick(int);

int main(int argc, char* argv[]) {
	//string inputFile = "test.pla",
	//outputFile = "output.pla";

	string inputFile = argv[1];
	string outputFile = argv[2];

	//read .pla file
	ifstream inputF;
	inputF.open(inputFile);
	if (inputF.fail()) {
		cout << "Read File Fail\n"; exit(1);
	}

	string line = "";
	while (getline(inputF, line)) {
		stringstream temp(line);
		string word = "";
		while (temp >> word) {
			if (word == ".i")temp >> inputVertex;
			else if (word == ".o")temp >> outputVertex;
			else if (word == ".ilb") {
				for (int i = 0; i < inputVertex; i++) {
					temp >> word;
					input.push_back(word);
				}
			}
			else if (word == ".ob") {
				for (int i = 0; i < outputVertex; i++) {
					temp >> word;
					output.push_back(word);
				}
			}
			else if (word == ".p") {
				int totalCmd = 0;
				temp >> totalCmd;
				for (int i = 0; i < totalCmd; i++) {
					getline(inputF, line);
					temp.clear();
					temp.str(line);
					string term = "";
					char answer = '\0';
					temp >> term;
					temp >> answer;
					if (answer == '1') {
						commandSOP++; sop.push_back(term);
					}
					else if (answer == '-') {
						commandX++; x.push_back(term);
					}
				} 
			}
			else if (word == ".e") {
#ifdef TEST
				cout << "end of file\n";
#endif // TEST
			}
			else {
				cout << "Error! invalid input\n"; exit(1);
			}
		}
		temp.str("");
	}
	inputF.close();

	//process
	//find SOP terms
	for (int i = 0; i < commandSOP; i++){
		vector<int> num;
		for (int j = 0, k = (inputVertex - 1); j < inputVertex; j++, k--){
			if (sop[i][j] == '1'){
				if (j == 0)num.push_back(pow(2, k));
				else {
					for (int a = 0; a < num.size(); a++)num[a] += pow(2, k);
				}
			}
			else if (sop[i][j] == '0'){
				if (j == 0)num.push_back(0);
			}
			else if (sop[i][j] == '-'){
				if (j == 0){
					num.push_back(0);
					num.push_back(pow(2, k));
				}
				else{
					int amount = num.size();
					for (int a = 0; a < amount; a++){
						num.push_back(num[a] + pow(2, k));
					}
				}
			}
		}
		for (int a = 0; a < num.size(); a++){
			productTerms.insert(num[a]);
		}
	}
	// Find the don't care terms' indices
	for (int i = 0; i < commandX; i++){
		vector<int> number;
		for (int l = 0, w = (inputVertex - 1); l < inputVertex; l++, w--){
			if (x[i][l] == '1'){
				if (l == 0)number.push_back(pow(2, w));
				else {
					for (int a = 0; a < number.size(); a++)number[a] += pow(2, w);
				}
			}
			else if (x[i][l] == '0'){
				if (l == 0)number.push_back(0);
			}
			else if (x[i][l] == '-'){
				if (l == 0){
					number.push_back(0);
					number.push_back(pow(2, w));
				}
				else{
					int amount = number.size();
					for (int a = 0; a < amount; a++){
						number.push_back(number[a] + pow(2, w));
					}
				}
			}
		}
		for (int a = 0; a < number.size(); a++){
			dontCareTerms.insert(number[a]);
		}
	}

#ifdef TEST
	set<int>::iterator itSet;
	cout << "Product: ";
	for (itSet = productTerms.begin(); itSet != productTerms.end(); itSet++)cout << *itSet << " ";
	cout << endl;
	cout << "Don\'t care: ";
	for (itSet = dontCareTerms.begin(); itSet != dontCareTerms.end(); itSet++)cout << *itSet << " ";
	cout << endl;
#endif // TEST

	// Classify 1's number
	// Create vector of 0 ~ inputVertex
	for (int i = 0; i <= inputVertex; i++){
		vector<minterm> temp;
		oneNumber.push_back(temp);
	}
	// Convert Decimal to Integer and find number of 1
	// SOP
	set<int>::iterator itOne;
	for (itOne = productTerms.begin(); itOne != productTerms.end(); itOne++){
		int numberOfOne = 0;
		stringstream stream;
		// Decimal to Binary
		for (int d = inputVertex - 1; d >= 0; d--)
		{
			stream << ((*itOne >> d) % 2);
			if ((*itOne >> d) % 2)
				numberOfOne++;
		}
		minterm temp;
		stream >> temp.binary;
		temp.contentMin.insert(*itOne);
		oneNumber[numberOfOne].push_back(temp);
	}
	// Don't care
	for (itOne = dontCareTerms.begin(); itOne != dontCareTerms.end(); itOne++){
		int numberOfOne = 0;
		stringstream stream;
		// Decimal to Binary
		for (int d = inputVertex - 1; d >= 0; d--){
			stream << ((*itOne >> d) % 2);
			if ((*itOne >> d) % 2)numberOfOne++;
		}
		minterm temp;
		stream >> temp.binary;
		temp.contentMin.insert(*itOne);
		oneNumber[numberOfOne].push_back(temp);
	}

#ifdef TEST
	for (int n = 0; n <= inputVertex; n++)
	{
		cout << n << endl;
		for (int c = 0; c < oneNumber[n].size(); c++)
		{
			cout << oneNumber[n][c].binary << " "
				<< *oneNumber[n][c].contentMin.begin() << endl;
		}
		cout << endl;
	}
#endif // TEST

	// Quine-McCluskey algorithm
	bool finish = false;
	while (!finish){
#ifdef TEST
		cout << "oneNumber\'s Size: " << oneNumber.size() << endl;
#endif // TEST
		for (int o = 0, on = 1; o < oneNumber.size() - 1; o++, on++){
			vector<minterm> replace;
			// Compare o and o+1 content minterm
			for (int i = 0; i < oneNumber[o].size(); i++){
				for (int in = 0; in < oneNumber[on].size(); in++){
#ifdef TEST
					cout << oneNumber[o][i].binary << " v.s. " << oneNumber[on][in].binary << endl;
#endif // TEST
					// Check whether only one different
					bool onlyOne = false;
					int diffPosition = 0;
					for (int b = 0; b < inputVertex; b++){
						if (oneNumber[o][i].binary[b] != oneNumber[on][in].binary[b]){
							if (onlyOne == false){
								onlyOne = true;
								diffPosition = b;
							}
							else{
								onlyOne = false;
								break;
							}
						}
					}
					// Only one different
					if (onlyOne){
						oneNumber[o][i].used = true;
						oneNumber[on][in].used = true;
						// Push into replacement vector
						minterm temp;
						temp = oneNumber[o][i];
						// Change the index of the binary representation
						temp.binary[diffPosition] = '-';
						// Check whether replace already has the binary representation
						bool exist = false;
						for (int r = 0; r < replace.size(); r++){
							if (replace[r].binary == temp.binary)exist = true;
						}
						if (!exist){
							// Combine two contentMin
							set<int>::iterator c;
							for (c = oneNumber[on][in].contentMin.begin(); c != oneNumber[on][in].contentMin.end(); c++){
								temp.contentMin.insert(*c);
							}
							// Reset used;
							temp.used = false;
							replace.push_back(temp);
						}
					}
				}
				if (oneNumber[o][i].used == false){
					asterisk.push_back(oneNumber[o][i]);
				}
			}
			// Kill all of o's minterm
			oneNumber[o].clear();
			// Reset the minterm with replace
			for (int r = 0; r < replace.size(); r++){
				oneNumber[o].push_back(replace[r]);
			}
		}
		// Check whether the least number of 1 has unused
		vector<minterm> least;
		for (int l = 0; l < oneNumber[oneNumber.size() - 1].size(); l++){
			if (oneNumber[oneNumber.size() - 1][l].used == false){
				minterm temp = oneNumber[oneNumber.size() - 1][l];
				bool exist = false;
				for (int lr = 0; lr < least.size(); lr++){
					if (least[lr].binary == temp.binary)exist = true;
				}
				if (!exist){
					temp.used = false;
					least.push_back(temp);
				}
			}
		}
		// Combine least with asterisk
		for (int l = 0; l < least.size(); l++){
			asterisk.push_back(least[l]);
		}
		// Clear least
		least.clear();
		// Delete the least number of 1
		oneNumber.erase(oneNumber.end() - 1);
		// Check whether all oneNumber's content is Clear
		finish = true;
		for (int o = 0; o < oneNumber.size(); o++){
			if (oneNumber[o].size() != 0)finish = false;
		}
#ifdef TEST
		cout << "\nTemparary Output:\n";
		for (int o = 0; o < oneNumber.size(); o++){
			cout << o << endl;
			for (int i = 0; i < oneNumber[o].size(); i++){
				cout << oneNumber[o][i].binary;
				set<int>::iterator it;
				for (it = oneNumber[o][i].contentMin.begin(); it != oneNumber[o][i].contentMin.end(); it++){
					cout << " " << *it;
				}
				cout << endl;
			}
		}
		cout << endl;
#endif // TEST
	}
	// Clear the oneNumber data
	oneNumber.clear();
#ifdef TEST
	cout << "clear: " << oneNumber.size() << endl;
#endif // TEST

	// Petrick's method
	set<int>::iterator it;
	// Set the petrick table
	for (it = productTerms.begin(); it != productTerms.end(); it++){
		sopTerm temp;
		temp.sopIndex = *it;
		petrick.push_back(temp);
	}

	for (int a = 0; a < asterisk.size(); a++){
		int petrickC = 0;
		for (it = asterisk[a].contentMin.begin(); it != asterisk[a].contentMin.end(); it++){
			bool find = false,
				isDontCare = false;
			set<int>::iterator dc;
			// Kill the don't care item
			for (dc = dontCareTerms.begin(); dc != dontCareTerms.end(); dc++){
				if (*it == *dc)isDontCare = true;
			}
			while (!find && !isDontCare){
				if (petrick[petrickC].sopIndex == *it){
					petrick[petrickC].ptr.push_back(a);
					find = true;
				}
				petrickC++;
			}
		}
	}
#ifdef TEST
	for (int p = 0; p < petrick.size(); p++){
		std::cout.setf(std::ios::right);
		if (p == 0){
			cout << "\nAsterisk:";
			for (int a = 0; a < asterisk.size(); a++){
				cout << std::setw(6) << asterisk[a].binary;
			}
			cout << endl;
		}
		cout << std::setw(9) << petrick[p].sopIndex;
		for (int a = 0; a < asterisk.size(); a++){
			bool found = false;
			cout << std::setw(6);
			for (int i = 0; i < petrick[p].ptr.size(); i++){
				if (petrick[p].ptr[i] == a)found = true;
			}
			if (found)cout << 'X';
			else cout << ' ';
		}
		cout << endl;
	}
	cout << endl;
#endif // TEST
	// Use recursive solve petrick
	recursivePetrick(0);
#ifdef TEST
	cout << minimalist.fewestTerms.size() << endl;
	for (int i = 0; i < minimalist.fewestTerms.size(); i++)
	{
		cout << minimalist.fewestTerms[i];
	}
	cout << endl << endl;
#endif // TEST

	// Output
	ofstream outputF;
	outputF.open(outputFile);
	if (outputF.fail()){
		cout << "Output file opening failed\n";
		exit(1);
	}
	outputF << ".i " << inputVertex << endl;
	outputF << ".o " << outputVertex << endl;
	outputF << ".ilb";
	for (int i = 0; i < inputVertex; i++)outputF << " " << input[i];
	outputF << endl;
	outputF << ".ob";
	for (int o = 0; o < outputVertex; o++)outputF << " " << output[o];
	outputF << endl;
	outputF << ".p " << minimalist.fewestTerms.size() << endl;

	for (int i = 0; i < minimalist.fewestTerms.size(); i++){
		string command = asterisk[minimalist.fewestTerms[i]].binary;
		outputF << command << " 1\n";
	}
	outputF << ".e\n";
	outputF.close();
	cout << "Total number of terms: " << minimalist.totalTerm << endl;
	cout << "Total number of literals: " << minimalist.totalLiteral << endl;

	return 0;
}
// Brute-force attack
void recursivePetrick(int level){
	if (level == petrick.size()){
		set<int> simplify;
		for (int e = 0; e < expand.size(); e++)simplify.insert(expand[e]);
		if (first || simplify.size() <= minimalist.fewestTerms.size()){
			int tempLiteral = 0;
			set<int>::iterator it;
			// Find literal
			for (it = simplify.begin(); it != simplify.end(); it++){
				for (int l = 0; l < asterisk[*it].binary.length(); l++){
					if (asterisk[*it].binary[l] != '-')
						tempLiteral++;
				}
			}
			// Whether find smaller answer
			if (first || (tempLiteral < minimalist.totalLiteral)){
				minimalist.fewestTerms.clear();
				for (it = simplify.begin(); it != simplify.end(); it++){
					minimalist.fewestTerms.push_back(*it);
				}
				// terms: Amount of the essential prime implicants of the function
				minimalist.totalTerm = minimalist.fewestTerms.size();
				// literals: Amount of 0 & 1 in the implicants
				minimalist.totalLiteral = tempLiteral;
			}
			first = false;
		}
		return;
	}
	else {
		for (int ptrC = 0; ptrC < petrick[level].ptr.size(); ptrC++){
			expand.push_back(petrick[level].ptr[ptrC]);
			recursivePetrick(level + 1);
			expand.pop_back();
		}
	}
}
