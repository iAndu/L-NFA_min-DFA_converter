#include <fstream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cstring>
#include <iostream>
#include <queue>

using namespace std;

struct transition {
	int this_state, next_state;
	char letter;
};

unordered_set<int> accepting_states;
vector<transition> transitions;
unordered_map<char, int> letters_map;
vector<int> *dfa_table;
unordered_set<int> states;
unordered_set<int> *min_states;
vector<int> *min_dfa_table;
unordered_set<int> min_accepting_states;
int nr_states;
int min_nr_states;

void read() {
	ifstream in("../nfa_dfa/dfa.def");

	char *line = new char[256];
	in.getline(line, 256);
	
	char *p = strtok(line, " ");


	while (p != NULL) {
		accepting_states.insert(atoi(p));
		states.insert(atoi(p));
		p = strtok(NULL, " ");
	}

	int index = 0;
	while (!in.eof()) {
		transition x;

		in >> x.this_state >> x.letter >> x.next_state;
		states.insert(x.this_state);
		states.insert(x.next_state);
		if (!in.eof()) {
			transitions.push_back(x);

			if (letters_map.find(x.letter) == letters_map.end())
				letters_map[x.letter] = index++;
		}
	}

	nr_states = states.size();
}

void create_automata() {
	dfa_table = new vector<int>[nr_states];
	for (int i = 0; i < nr_states; ++i)
		dfa_table[i].resize(nr_states, -1);

	for (auto i = transitions.begin(); i != transitions.end(); ++i)
		dfa_table[i->this_state][letters_map[i->letter]] = i->next_state;
}

void minimize() {
	vector<bool> min_dfa[nr_states];
	for (int i = 0; i < nr_states; ++i)
		min_dfa[i].resize(nr_states, false);

	// Distinguish accepting states from non-accepting states
	for (int i = 0; i < nr_states; ++i)
		if (accepting_states.find(i) == accepting_states.end())
			for (auto j = accepting_states.begin(); j != accepting_states.end(); ++j)
				min_dfa[i][*j] = min_dfa[*j][i] = true;

	bool sw;
	/* While there are still changes being made in table, check for each pair of
	 * unmarked states if they go by same letter to a pair of distinguished states
	*/
	do {
		sw = false;

		for (int i = 0; i < nr_states; ++i)
			for (int j = i + 1; j < nr_states; ++j)
				if (!min_dfa[i][j])
					for (auto k = letters_map.begin(); k != letters_map.end(); ++k)
						if (dfa_table[i][k->second] == -1 && dfa_table[j][k->second] == -1)
							continue;
						else if ((dfa_table[i][k->second] == -1 && dfa_table[j][k->second] != -1) || 
							(dfa_table[i][k->second] != -1 && dfa_table[j][k->second] == -1) ||
							(min_dfa[dfa_table[i][k->second]][dfa_table[j][k->second]] == true)) {
							min_dfa[i][j] = min_dfa[j][i] = true;
							sw = true;
							break;
						}
	} while(sw);
	// Computes the minimized states
	min_states = new unordered_set<int>[nr_states];
	unordered_set<int> aux;
	int index = 0;
	sw = false;
	for (int i = 1; i < nr_states; ++i)
		for (int j = 0; j < i; ++j)
			if (!min_dfa[i][j]) {
				int k;
				for (k = 0; k < index; ++k)
					if (min_states[k].find(i) != min_states[k].end() || min_states[k].find(j) != min_states[k].end()) {
						aux.insert(i);
						aux.insert(j);
						min_states[k].insert(i);
						min_states[k].insert(j);
						break;
					}
				if (k == index) {
					aux.insert(i);
					aux.insert(j);
					min_states[index].insert(i);
					min_states[index++].insert(j);
				}
			}

	// Add the distinguished states to the minimized states
	for (auto i = states.begin(); i != states.end(); ++i)
		if (aux.find(*i) == aux.end())
			min_states[index++].insert(*i);

	// Construct the minimized DFA
	min_dfa_table = new vector<int>[index];
	for (int i = 0; i < index; ++i)
		min_dfa_table[i].resize(index, -1);
	
	for (int i = 0; i < index; ++i)
		for (auto j = letters_map.begin(); j != letters_map.end(); ++j) {
			int st = dfa_table[*(min_states[i].begin())][j->second];
			for (int k = 0; k < index; ++k)
				if (min_states[k].find(st) != min_states[k].end()) {
					min_dfa_table[i][j->second] = k;
					break;
				}
		}

	// New accepting states
	for (int i = 0; i < index; ++i)
		if (accepting_states.find(*(min_states[i].begin())) != accepting_states.end())
			min_accepting_states.insert(i);

	min_nr_states = index;
}

// Remove unaccesable states from minimized DFA
void remove_unacc_states() {
	vector<int> grades(min_nr_states, 0);
	// Compute the inner grade of each state
	for (int i = 0; i < min_nr_states; ++i)
		for (auto j = letters_map.begin(); j != letters_map.end(); ++j)
			if (min_dfa_table[i][j->second] != -1 && min_dfa_table[i][j->second] != i)
				++grades[min_dfa_table[i][j->second]];

	queue<int> aux;
	// Push all unaccesable states in queue
	for (auto i = grades.begin(); i != grades.end(); ++i)
		if (!(*i)) {
			int state = i - grades.begin();
			aux.push(state);
		}

	while(!aux.empty()) {
		int state = aux.front();
		for (int j = 0; j < min_nr_states; ++j)
				for (auto k : letters_map) 
					if (j == state || min_dfa_table[j][k.second] == state) {
						// Decrement the inner grade of accesable states and push them in queue
						// if they become unaccesable as well
						if (min_dfa_table[j][k.second] != -1 && min_dfa_table[j][k.second] != 0 &&
							!--grades[min_dfa_table[j][k.second]])
							aux.push(min_dfa_table[j][k.second]);
						min_dfa_table[j][k.second] = -1;
					}
		aux.pop();
	}
}

void dead_state(int state, vector<bool> &viz) {
	viz[state] = true;
	for (auto i : letters_map)
		if (min_dfa_table[state][i.second] >= 0 && !viz[min_dfa_table[state][i.second]])
			dead_state(min_dfa_table[state][i.second], viz);
}

void remove_dead_states() {
	vector<bool> viz(min_nr_states);
	for (int i = 0; i < min_nr_states; ++i) {
		for (auto j : viz)
			j = false;
		// DFS from state i
		dead_state(i, viz);
		// Check if any accepting state was reached
		bool sw = false;
		for (auto j = min_accepting_states.begin(); !sw && j != min_accepting_states.end(); ++j)
			if (viz[*j])
				sw = true;
		// If it is a dead state
		if (!sw)
			for (int k = 0; k < min_nr_states; ++k)
				for (auto l : letters_map)
					if (k == i || min_dfa_table[k][l.second] == i)
						min_dfa_table[k][l.second] = -1;
	}
}

void print() {
	ofstream out("min_dfa.def");
	for (auto i = min_accepting_states.begin(); i != min_accepting_states.end(); ++i)
		out << *i << ' ';
	out << '\n';

	for (int i = 0; i < min_nr_states; ++i)
		for (auto j = letters_map.begin(); j != letters_map.end(); ++j) 
			if (min_dfa_table[i][j->second] != -1)
				out << i << ' ' << j->first << ' ' << min_dfa_table[i][j->second] << '\n';
	out.close();	
}

int main() {
	read();
	create_automata();
	minimize();
	remove_unacc_states();
	remove_dead_states();
	print();

	return 0;
}