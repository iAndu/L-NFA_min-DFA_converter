#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>
#include <unordered_set>
#include <queue>
#include <unordered_map>
#include <algorithm>

typedef unsigned int u_int;

using namespace std;

struct transition
{
	int this_state, next_state;
	char letter;
};

unordered_set<int> accepting_states;
vector<transition> transitions;
unordered_map<char, int> letters_map;
unordered_set<int> **nfa_table;
int nr_states;
int dfa_states;
vector< pair<string, int> > labels;

/*
 * RECOMPUTE_FINAL_STATES *
 **************************
 * Adds the states that can access a final state by a lambda movement as final states.
*/

void recompute_final_states()
{
	int lambda_id = letters_map['-'];
	for (int state = 0; state < nr_states; ++state)
		for (auto f_state = accepting_states.begin(); f_state != accepting_states.end(); ++f_state)
			if (nfa_table[state][lambda_id].find(*f_state) != nfa_table[state][lambda_id].end())
				accepting_states.insert(state);
}

/*
 * CREATE_AUTOMATA *
 *******************
 * Creates a 2d vector of vectors. Each cell will contains accessable states
 * from state 'i' with letter 'j', where 'i' is line, and 'j' is column.
*/

void create_automata()
{
    nfa_table = new unordered_set<int>*[nr_states];

    for (int i = 0; i < nr_states; ++i)
        nfa_table[i] = new unordered_set<int>[letters_map.size()];

    // For each transition, add next_state as accessable from this_state by
    // letter it->letter (mapped)
    for (auto it = transitions.begin(); it != transitions.end(); ++it)
    {
        int column_id = letters_map[it->letter];
        nfa_table[it->this_state][column_id].insert(it->next_state);
    }
}

/*
 * LAMBDA_CLOSURE *
 ******************
 * Adds all available states from each state using 0 or more lambdas
*/

void lambda_closure()
{
    for (int state = 0; state < nr_states; ++state)
    {
        queue<int> que;
        vector<bool> visited(nr_states, false);

        que.push(state);
        int lambda_id = letters_map['-'];

        // State 'i' is always accessable with lambda from itself
        nfa_table[state][lambda_id].insert(state);
        while (!que.empty())
        {
            // For each available state with lambda from state que.front()
            for (auto l_a_state = nfa_table[que.front()][lambda_id].begin();
                 l_a_state != nfa_table[que.front()][lambda_id].end(); ++l_a_state)
            {
                // If not already marked as accessable from state 'i' with lambda, add it
                if (nfa_table[state][lambda_id].find(*l_a_state) == nfa_table[que.front()][lambda_id].end())
                {
                    nfa_table[state][lambda_id].insert(*l_a_state);
                }
                // Also add it to the queue if not already added
                if (!visited[*l_a_state])
                {
                    que.push(*l_a_state);
                    visited[*l_a_state] = true;
                }
            }
            que.pop();
        }
    }
}

/*
 * TRANSITION_FUNCTION *
 ***********************
 * Creates the NFA transition nfa_table from the lambda NFA transition nfa_table
*/

void transition_function()
{
    // For each letter in the alphabet
    for (auto letter = letters_map.begin(); letter != letters_map.end(); ++letter)
    {
        int lambda_id = letters_map['-'];
        // If it's not lambda
        if (letter->first != '-')
        {
            // For each state in automata
            for (int state = 0; state < nr_states; ++state)
                // For each accessable state from state 'j' with lambda
                for (auto l_a_state = nfa_table[state][lambda_id].begin();
                     l_a_state != nfa_table[state][lambda_id].end(); ++l_a_state)
                {
                    // Add all states available from state *k with letter i->second to states available from
                    // state 'j' with letter i->second as well (lambda transitions followed by letter)
                    nfa_table[state][letter->second].insert(nfa_table[*l_a_state][letter->second].begin(),
                                                                   nfa_table[*l_a_state][letter->second].end());
                    // Also add all states available using lambda from that states
                    for (auto l_a_l_state = nfa_table[*l_a_state][letter->second].begin();
                         l_a_l_state != nfa_table[*l_a_state][letter->second].end(); ++l_a_l_state)

                        nfa_table[state][letter->second].insert(nfa_table[*l_a_l_state][lambda_id].begin(),
                                                                      nfa_table[*l_a_l_state][lambda_id].end());
                }
        }
    }
}

void lambda_to_nfa()
{
    lambda_closure();
    recompute_final_states();
    transition_function();
    
    int lambda_id = letters_map['-'];

    for (int i = 0; i < nr_states; ++i)
    {
    	unordered_set<int> *table = &nfa_table[i][lambda_id];
    	table->erase(table->begin(), table->end());
    }

    letters_map.erase('-');
}

void print_transitions()
{
    ofstream out("nfa.def");
    for (auto i = accepting_states.begin(); i != accepting_states.end(); ++i)
        out << *i << ' ';
    out << '\n';

    for (int i = 0; i < nr_states; ++i)
        for (auto j = letters_map.begin(); j != letters_map.end(); ++j)
            for (auto k = nfa_table[i][j->second].begin(); k != nfa_table[i][j->second].end(); ++k)
                out << i << ' ' << j->first << ' ' << *k << '\n';     
    out << '\n';
    out.close();   
}

int main()
{
	ifstream in("lambda_nfa.def");
	string line;
	getline(in, line);
	int index = 0;

    for (u_int i = 0; i < line.length(); ++i)
        if (isdigit(line[i]))
            accepting_states.insert(line[i] - '0');

    unordered_set<int> states;
    while (!in.eof())
    {
        transition x;
        in >> x.this_state >> x.letter >> x.next_state;

        /* Mapping the letters with unique indexes. First letter will be mapped as 0,
         * second letter as 1 and so on
        */
        if (letters_map.find(x.letter) == letters_map.end())
            letters_map[x.letter] = index++;

        transitions.push_back(x);
        states.insert(x.this_state);
        states.insert(x.next_state);
    }

    in.close();
    nr_states = states.size();

    create_automata();
    lambda_to_nfa();

    print_transitions();

    for (auto i = accepting_states.begin(); i != accepting_states.end(); ++i)
    	cout << *i << ' ';

	return 0;
}
