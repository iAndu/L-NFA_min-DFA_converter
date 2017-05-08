#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <queue>
#include <cstdlib>
#include <algorithm>

using namespace std;

struct transition {
	int this_state, next_state;
	char letter;
};

unordered_set<int> accepting_states;
vector<transition> transitions;
unordered_map<char, int> letters_map;
vector<int> **nfa_table;
vector<int> *dfa_table;
int nr_states;

void read() {
	ifstream in("../lnfa_nfa/nfa.def");
	
	char *line = new char[256];
	in.getline(line, 256);
	
	char *p = strtok(line, " ");

	while (p != NULL) {
		accepting_states.insert(atoi(p));
		p = strtok(NULL, " ");
	}

	int index = 0;

	while (!in.eof()) {
		transition x;

		in >> x.this_state >> x.letter >> x.next_state;
		if (!in.eof()) {
			transitions.push_back(x);

			if (letters_map.find(x.letter) == letters_map.end())
				letters_map[x.letter] = index++;

			nr_states = max(nr_states, x.this_state);
			nr_states = max(nr_states, x.next_state);
		}
	}
	++nr_states;

	in.close();
}

// Create NFA automata
void create_automata() {
	nfa_table = new vector<int>*[nr_states];

	for (int i = 0; i < nr_states; ++i)
		nfa_table[i] = new vector<int>[letters_map.size()];

	for (auto i = transitions.begin(); i != transitions.end(); ++i)
		nfa_table[i->this_state][letters_map[i->letter]].push_back(i->next_state);
}

// If nr contains x return true, otherwise false
bool contains(int nr, int x) {
	do {
		if (x == nr % 10)
			return true;
		nr /= 10;
	} while (nr);
	return false;
}

// Combine numbers from set to number
int set_to_nr(unordered_set<int> &m_set) {
	vector<int> sorting;
	for (auto j = m_set.begin(); j != m_set.end(); ++j)
		sorting.push_back(*j);
	sort(sorting.begin(), sorting.end());
	int nr = 0;
	for (auto j = sorting.begin(); j != sorting.end(); ++j)
		nr = nr * 10 + *j;

	return nr;
}

void nfa_to_dfa() {
	vector< unordered_set<int> > comp_states;
	queue< unordered_set<int> > que;
	unordered_set<int> state;
	unordered_map<int, int> state_map;
	int dfa_states = 1;

	// Insert starting state
	state.insert(0);
	que.push(state);
	// Insert state in compound states
	comp_states.push_back(state);
	// Initialize the DFA table
	dfa_table = new vector<int>(letters_map.size(), -1);

	int index = 0;

	// Hash the starting state with index
	state_map[0] = index++;

	while (!que.empty()) {
		state = que.front();

		// For each letter
		for (auto i = letters_map.begin(); i != letters_map.end(); ++i) {
			unordered_set<int> aux;
			// For each state in vector state
			for (auto j = state.begin(); j != state.end(); ++j)
				// Insert in aux accesable states from this state by letter 'i'
				aux.insert(nfa_table[*j][i->second].begin(), nfa_table[*j][i->second].end());

			// Search if aux is already a known compound state
			auto it = find(comp_states.begin(), comp_states.end(), aux);

			// Transform sets to numbers so they can be hashed as new states
			int nr = set_to_nr(aux);
			int now = set_to_nr(state);

			// If there are any accessable states
			if (aux.size()) {
				// And if it already exists, then just add to DFA table state nr as accessable
				// from state now by letter i
				if (it != comp_states.end()) {
					dfa_table[state_map[now]][i->second] = state_map[nr];
				}
				// Otherwise
				else {
					// Add the new compound state
					comp_states.push_back(aux);
					// And hash it
					state_map[nr] = index++;

					// Copy the DFA table to a temporary variable
					vector<int> *copy = dfa_table;
					// Asign a new amount of memory to DFA table
					dfa_table = new vector<int>[++dfa_states];

					// Fill it with -1 values
					for (int j = 0; j < dfa_states; ++j)
						for (int k = 0; k < letters_map.size(); ++k)
							dfa_table[j].push_back(-1);

					// And copy the elements from temporary variable back to DFA table
					for (int j = 0; j < dfa_states - 1; ++j)
						for (int k = 0; k < copy[j].size(); ++k)
							dfa_table[j][k] = copy[j][k];

					// Then add the new state as accesable from state now bt letter i
					dfa_table[state_map[now]][i->second] = state_map[nr];
					// And add the newly descovered state to the queue
					que.push(aux);
				}
			}
		}
		que.pop();
	}
	// Reassign the number of states in the table
	nr_states = dfa_states;

	// Recompute the accepting states
	unordered_set<int> dfa_accepting_states;
	
	for (auto i = accepting_states.begin(); i != accepting_states.end(); ++i)
		for (auto j = state_map.begin(); j != state_map.end(); ++j)
			if (contains(j->first, *i))
				dfa_accepting_states.insert(j->second);

	accepting_states = dfa_accepting_states;
}

// Print the accepting states and DFA transitions to file
void print_dfa() {
	ofstream out("dfa.def");
	for (auto i = accepting_states.begin(); i != accepting_states.end(); ++i)
		out << *i << ' ';
	out << '\n';
	for (int i = 0; i < nr_states; ++i)
		for (auto j = letters_map.begin(); j != letters_map.end(); ++j)
			if (dfa_table[i][j->second] >= 0)
				out << i << ' ' << j->first << ' ' << dfa_table[i][j->second] << '\n';
	out.close();
}

int main() {
	read();
	create_automata();
	nfa_to_dfa();
	print_dfa();

	return 0;
}