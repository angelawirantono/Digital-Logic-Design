#include<iostream>
#include<cmath>
#include<cstdlib>
#include<string>
#include<fstream>

// Linked list node & pointer definition.
typedef struct linkedlist
{
	int nodeNumber;
	char nodeChar;
	struct linkedlist* then_edge;
	struct linkedlist* else_edge;
} node;

typedef node* p_node;
// Ends here.

int stringToInt(const std::string);

// Class definition.
class directedAG
{
public:
	directedAG();

	int getIVNumber(const std::string);
	int getOFNumber(const std::string);
	char* getIVs(std::string, const int);

	void makeTree(p_node, const int, const int, const int);
	void traverseTree(const p_node, const std::string, const int);
	void printTree(std::ostream&, const p_node) const;
	void printLabel(std::ostream&, const p_node) const;

	p_node findNode(const int, const p_node, p_node*, int*);

	void ROBDDTree();
	void cutRedundant(const p_node, bool*);
	void cutIsomorphic(const p_node, bool*);
	void cutISO(bool*);

	void fix_n_nodeNum();
	void runROBDD(const p_node);

	friend std::istream& operator >> (std::istream&, directedAG&); // .pla format input
	friend std::ostream& operator << (std::ostream&, directedAG&); // .dot format output

// private:
	int n_inputVariables;
	char* inputVariables;
	// int n_outputFunctions;
	// char* outputFunctions;

	node msb;
	node binary_0;
	node binary_1;

	int* n_nodeNum;
};

int main(int argc, char* argv[])
{
	directedAG bdd;

	std::ifstream inputFile;
	inputFile.open(argv[1], std::ios::in);
	inputFile >> bdd;

	std::ofstream outputFileOBDD;
	outputFileOBDD.open(argv[2], std::ios::out);
	outputFileOBDD << bdd;

	bdd.ROBDDTree();

	std::ofstream outputFileROBDD;
	outputFileROBDD.open(argv[3], std::ios::out);
	outputFileROBDD << bdd;

	std::cout << "OBDD and ROBDD made successfully.\n";

	return 0;
}

int stringToInt(const std::string input) // Verified.
{
	if (input.size() == 1) return (int)input[0] - 48;
	else
	{
		int ans = 0;
		int multiplier = 1;

		for (int i = input.size() - 1; i >= 0; i--)
		{
			ans += (((int)input[i] - 48) * multiplier);
			multiplier *= 10;
		}

		return ans;
	}
}

directedAG::directedAG()
{
	msb.nodeNumber = 1;
	msb.then_edge = NULL;
	msb.else_edge = NULL;

	binary_0.nodeNumber = 0;
	binary_0.then_edge = NULL;
	binary_0.else_edge = NULL;
	binary_0.nodeChar = '0';

	binary_1.nodeNumber = -1;
	binary_1.then_edge = NULL;
	binary_1.else_edge = NULL;
	binary_1.nodeChar = '1';
}

int directedAG::getIVNumber(const std::string input)
{
	return stringToInt(input.substr(input.find(" ", 0) + 1, std::string::npos));
}

int directedAG::getOFNumber(const std::string input)
{
	return stringToInt(input.substr(input.find(" ", 0) + 1, std::string::npos));
}

char* directedAG::getIVs(std::string input, const int IV)
{
	char* inputVariables = new char[IV];

	for (int i = 0; i < IV; i++)
	{
		int spaceLoc = input.find(" ", 0);
		inputVariables[i] = input[spaceLoc + 1];
		input = input.substr(spaceLoc + 1, std::string::npos);
	}

	return inputVariables;
}

void directedAG::makeTree(p_node nodeVar, const int thenEdge, const int elseEdge, const int charIdx)
{
	if (nodeVar->nodeNumber >= pow(2, n_inputVariables - 1)) return;
	else
	{
		p_node new_node1 = (p_node)malloc(sizeof(node));

		new_node1->nodeNumber = thenEdge;
		new_node1->else_edge = &binary_0;
		new_node1->then_edge = &binary_0;
		new_node1->nodeChar = inputVariables[charIdx];

		nodeVar->else_edge = new_node1;

		p_node new_node2 = (p_node)malloc(sizeof(node));

		new_node2->nodeNumber = elseEdge;
		new_node2->else_edge = &binary_0;
		new_node2->then_edge = &binary_0;
		new_node2->nodeChar = inputVariables[charIdx];

		nodeVar->then_edge = new_node2;

		makeTree(nodeVar->else_edge, thenEdge * 2, (thenEdge * 2) + 1, charIdx + 1);
		makeTree(nodeVar->then_edge, elseEdge * 2, (elseEdge * 2) + 1, charIdx + 1);
	}
}

void directedAG::traverseTree(const p_node pointer, const std::string input, const int idx)
{
	if (idx == n_inputVariables - 1)
	{
		p_node temp = &binary_0;
		if (input[idx + 2] == '1') temp = &binary_1;

		if (input[idx] == '0') pointer->else_edge = temp;
		else if (input[idx] == '1') pointer->then_edge = temp;
		else if (input[idx] == '-')
		{
			pointer->else_edge = temp;
			pointer->then_edge = temp;
		}

		return;
	}
	else
	{
		if (input[idx] == '1')
		{
			// pointer = pointer->then_edge;
			traverseTree(pointer->then_edge, input, idx + 1);
		}
		else if (input[idx] == '0')
		{
			// pointer = pointer->else_edge;
			traverseTree(pointer->else_edge, input, idx + 1);
		}
		else if (input[idx] == '-')
		{
			// pointer = pointer->else_edge;
			// pointer = pointer->then_edge;
			traverseTree(pointer->else_edge, input, idx + 1);
			traverseTree(pointer->then_edge, input, idx + 1);
		}
	}

}

void directedAG::printTree(std::ostream& os, const p_node node) const
{
	if (node->else_edge == NULL || node->then_edge == NULL) return;

	if (n_nodeNum[node->nodeNumber] == 1)
	{
		os << "   " << node->nodeNumber << " -> " << node->else_edge->nodeNumber << " [label=\"0\", style=dotted]\n";
		os << "   " << node->nodeNumber << " -> " << node->then_edge->nodeNumber << " [label=\"1\", style=solid]\n";

		n_nodeNum[node->nodeNumber] = 0;

		printTree(os, node->else_edge);
		printTree(os, node->then_edge);
	}
}

void directedAG::printLabel(std::ostream& os, const p_node node) const
{
	if (node->else_edge == NULL || node->then_edge == NULL) return;

	if (n_nodeNum[node->nodeNumber] == 1)
	{
		os << "   " << node->nodeNumber << " [label=\"" << node->nodeChar << "\"]\n";

		n_nodeNum[node->nodeNumber] = 0;

		printLabel(os, node->else_edge);
		printLabel(os, node->then_edge);
	}
}

p_node directedAG::findNode(const int nodeNum, const p_node head, p_node* subhead, int* then)
{
	// std::cout << "finding " << nodeNum << " head " << head->nodeNumber << " pw " << pow(2, n_inputVariables) << std::endl;
	if (head->nodeNumber == 0 || head->nodeNumber == pow(2, n_inputVariables)) return NULL;

	if (head->then_edge->nodeNumber == nodeNum)
	{
		// std::cout << nodeNum << " found, subhead: " << head->nodeNumber << std::endl;
		*subhead = head;
		*then = 1;
		return head->then_edge;
	}
	else if (head->else_edge->nodeNumber == nodeNum)
	{
		// std::cout << nodeNum << " found, subhead: " << head->nodeNumber << std::endl;
		*subhead = head;
		*then = 0;
		return head->else_edge;
	}
	else
	{
		if (head->then_edge->nodeNumber != 0 && head->then_edge->nodeNumber != 1 && head->else_edge->nodeNumber != 0 && head->else_edge->nodeNumber != 1)
		{
			p_node first_half = findNode(nodeNum, head->else_edge, subhead, then);
			p_node second_half = findNode(nodeNum, head->then_edge, subhead, then);

			if (first_half == NULL && second_half == NULL) return NULL;
			else if (first_half == NULL) return second_half;
			else if (second_half == NULL) return first_half;
		}
		else if (head->then_edge->nodeNumber != 0 && head->then_edge->nodeNumber != 1)
		{
			return findNode(nodeNum, head->then_edge, subhead, then);
		}
		else if (head->else_edge->nodeNumber != 0 && head->else_edge->nodeNumber != 1)
		{
			return findNode(nodeNum, head->else_edge, subhead, then);
		}
		else return NULL;

	}

}

void directedAG::ROBDDTree()
{
	bool ROBDD_Tree = false;
	bool noChanges = true;

	p_node start;

	while (!ROBDD_Tree)
	{
		noChanges = true;

		cutRedundant(&msb, &noChanges);
		cutIsomorphic(&msb, &noChanges);
		cutISO(&noChanges);

		cutRedundant(&msb, &noChanges);
		cutIsomorphic(&msb, &noChanges);
		cutISO(&noChanges);

		if (noChanges) ROBDD_Tree = true;
	}

	fix_n_nodeNum();
}

void directedAG::cutRedundant(const p_node subhead, bool* changes)
{
	// std::cout << "redundant check subhead: " << subhead->nodeNumber << std::endl;
	if (subhead->then_edge->then_edge == NULL || subhead->then_edge->else_edge == NULL || subhead->else_edge->then_edge == NULL || subhead->else_edge->else_edge == NULL) return;

	if (subhead->then_edge->then_edge == subhead->then_edge->else_edge)
	{
		// std::cout << "redundant: " << subhead->then_edge->nodeNumber << std::endl;
		*changes = false;
		subhead->then_edge = subhead->then_edge->then_edge;
		// std::cout << "solved " << subhead->then_edge->nodeNumber << " " << subhead->else_edge->nodeNumber << std::endl;
	}

	if (subhead->else_edge->then_edge == subhead->else_edge->else_edge)
	{
		// std::cout << "redundant: " << subhead->else_edge->nodeNumber << std::endl;
		*changes = false;
		subhead->else_edge = subhead->else_edge->else_edge;
		// std::cout << "solved " << subhead->then_edge->nodeNumber << " " << subhead->else_edge->nodeNumber << std::endl;
	}

	if (subhead->then_edge->then_edge == NULL || subhead->then_edge->else_edge == NULL) return;
	else cutRedundant(subhead->then_edge, changes);

	if (subhead->else_edge->then_edge == NULL || subhead->else_edge->else_edge == NULL) return;
	else cutRedundant(subhead->else_edge, changes);
}

void directedAG::cutIsomorphic(const p_node subhead, bool* changes)
{
	// std::cout << "iso subhead: " << subhead->nodeNumber << std::endl;
	if (subhead->then_edge->then_edge == NULL || subhead->then_edge->else_edge == NULL || subhead->else_edge->then_edge == NULL || subhead->else_edge->else_edge == NULL) return;

	// Isomorphic subtree checking
	if (subhead->then_edge->then_edge == subhead->else_edge->then_edge && subhead->then_edge->else_edge == subhead->else_edge->else_edge)
	{
		// std::cout << "isomorphic: " << subhead->then_edge->nodeNumber << " " << subhead->else_edge->nodeNumber << std::endl;
		*changes = false;
		subhead->else_edge = subhead->then_edge;
		// std::cout << "solved " << subhead->then_edge->nodeNumber << " " << subhead->else_edge->nodeNumber << std::endl;
	}

	cutIsomorphic(subhead->else_edge, changes);
	cutIsomorphic(subhead->then_edge, changes);
}

void directedAG::cutISO(bool* changes)
{
	for (int i = n_inputVariables - 1; i >= 1; i--)
	{
		// std::cout << "trying to find for fixed node at lvl " << i << std::endl;

		int fixednode = pow(2, i);
		int then_fixed = -1;
		p_node subhead_fixed;
		p_node fixed = findNode(fixednode, &msb, &subhead_fixed, &then_fixed);

		while (fixed == NULL)
		{
			// std::cout << fixednode << " not found.\n";
			fixednode++;
			fixed = findNode(fixednode, &msb, &subhead_fixed, &then_fixed);
			continue;
		}

		// std::cout << "fixed node " << fixed->nodeNumber << " at level " << i << std::endl;

		for (int j = fixednode + 1; j < pow(2, i + 1); j++)
		{
			// std::cout << "trying to find " << j << std::endl;

			int then_potential = -1;
			p_node subhead_potential;
			p_node potential = findNode(j, &msb, &subhead_potential, &then_potential);

			if (potential == NULL)
			{
				// std::cout << j << " not found.\n";
				continue;
			}

			// std::cout << j << " is found\n";
			if (fixed->then_edge == potential->then_edge && fixed->else_edge == potential->else_edge)
			{
				// std::cout << "IsomorphicISO: " << fixed->nodeNumber << " " << potential->nodeNumber << std::endl;
				if (then_potential)
				{
					if (then_fixed) subhead_potential->then_edge = subhead_fixed->then_edge;
					else subhead_potential->then_edge = subhead_fixed->else_edge;
				}
				else
				{
					if (then_fixed) subhead_potential->else_edge = subhead_fixed->then_edge;
					else subhead_potential->else_edge = subhead_fixed->else_edge;
				}
				// std::cout << "fixed: " << subhead_potential->nodeNumber << " " << subhead_potential->then_edge->nodeNumber << " " << subhead_potential->else_edge->nodeNumber << std::endl;
				*changes = false;
			}
		}

	}
}

void directedAG::fix_n_nodeNum()
{
	for (int i = 1; i < pow(2, n_inputVariables); i++) n_nodeNum[i] = 0;

	runROBDD(&msb);
}

void directedAG::runROBDD(const p_node head)
{
	if (head->nodeNumber == 0 || head->nodeNumber == pow(2, n_inputVariables)) return;
	else
	{
		n_nodeNum[head->nodeNumber] = 1;

		runROBDD(head->else_edge);
		runROBDD(head->then_edge);
	}
}


std::istream& operator >> (std::istream& in, directedAG& plaInput)
{
	std::string temp;
	while (getline(in, temp))
	{
		if (temp.substr(0, temp.find(" ", 0)) == ".i")
		{
			plaInput.n_inputVariables = plaInput.getIVNumber(temp);

			plaInput.binary_1.nodeNumber = pow(2, plaInput.n_inputVariables);

			int temporary = pow(2, plaInput.n_inputVariables);
			plaInput.n_nodeNum = new int[temporary];

			plaInput.n_nodeNum[0] = 0;
			for (int i = 1; i < pow(2, plaInput.n_inputVariables); i++) plaInput.n_nodeNum[i] = 1;
		}
		else if (temp.substr(0, temp.find(" ", 0)) == ".ilb") plaInput.inputVariables = plaInput.getIVs(temp, plaInput.n_inputVariables);
		else if (temp.substr(0, temp.find(" ", 0)) == ".p")
		{
			plaInput.msb.nodeChar = plaInput.inputVariables[0];
			plaInput.makeTree(&(plaInput.msb), 2, 3, 1);

			int p = plaInput.getIVNumber(temp); // Borrowed a function lol

			for (int i = 0; i < p; i++)
			{
				getline(in, temp);
				plaInput.traverseTree(&(plaInput.msb), temp, 0);
			}

		}
		else if (temp.substr(0, temp.find(" ", 0)) == ".e") break;
	}
	return in;
}

std::ostream& operator << (std::ostream& out, directedAG& dotOutput)
{
	out << "digraph OBDD {\n";

	for (int i = 0; i < dotOutput.n_inputVariables; i++)
	{
		out << "   {rank=same ";

		for (int j = pow(2, i); j < pow(2, i + 1); j++)
		{
			if (dotOutput.n_nodeNum[j] == 1) out << j << " ";
		}

		out << "}\n";
	}

	out << std::endl;

	out << "   " << 0 << " [label=\"0\", shape=box]\n";

	dotOutput.printLabel(out, &(dotOutput.msb));

	out << "   " << pow(2, dotOutput.n_inputVariables) << " [label=\"1\", shape=box]\n\n";

	dotOutput.fix_n_nodeNum();

	dotOutput.printTree(out, &(dotOutput.msb));

	out << "}";
	return out;

}

// ends here.
