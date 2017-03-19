#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>
#include <unordered_set>
#include <queue>
#include <unordered_map>

using namespace std;

struct transition
{
	int this_state, next_state;
	char letter;
};

vector<int> accepting_states;
vector<transition> transitions;
unordered_map<char, int> letters_map;
unordered_set<int> **transition_table;
int nr_states;

/*
 * CREATE_AUTOMATA *
 *******************
 * Creates a 2d vector of vectors. Each cell will contains accessable states
 * from state 'i' with letter 'j', where 'i' is line, and 'j' is column.
*/

void create_automata()
{
    transition_table = new unordered_set<int>*[nr_states];

    for (int i = 0; i < nr_states; ++i)
        transition_table[i] = new unordered_set<int>[letters_map.size()];

    // For each transition, add next_state as accessable from this_state by
    // letter it->letter (mapped)
    for (auto it = transitions.begin(); it != transitions.end(); ++it)
    {
        column_id = letters_map[it->letter];
        transition_table[it->this_state][column_id].insert(it->next_state);
    }
}

/*
 * LAMBDA_CLOSURE *
 ******************
 * Adds all available states from each state using 0 or more lambdas
*/

void lambda_closure()
{
    for (int i = 0; i < nr_states; ++i)
    {
        queue<int> que;
        vector<bool> visited(nr_states, false);

        que.push(i);
        // State 'i' is always accessable with lambda from itself
        transition_table[i][letters_map['-']].insert(i);
        while (!que.empty())
        {
            int lambda_id = letters_map['-'];
            // For each available state with lambda from state que.front()
            for (auto j = transition_table[que.front()][lambda_id].begin(); j != transition_table[que.front()][lambda_id].end(); ++j)
            {
                // If not already marked as accessable from state 'i' with lambda, add it
                if (transition_table[i][lambda_id].find(*j) == transition_table[que.front()][lambda_id].end())
                {
                    transition_table[i][lambda_id].insert(*j);
                }
                // Also add it to the queue if not already added
                if (!visited[*j])
                {
                    que.push(*j);
                    visited[*j] = true;
                }
            }
            que.pop();
        }
    }
}

/*
 * TRANSITION_FUNCTION *
 ***********************
 * Creates the NFA transition transition_table from the lambda NFA transition transition_table
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
                for (auto l_a_state = transition_table[state][lambda_id].begin();
                     l_a_state != transition_table[state][lambda_id].end(); ++l_a_state)
                {
                    // Add all states available from state *k with letter i->second to states available from
                    // state 'j' with letter i->second as well (lambda transitions followed by letter)
                    transition_table[state][letter->second].insert(transition_table[*l_a_state][letter->second].begin(),
                                                                   transition_table[*l_a_state][letter->second].end());
                    // Also add all states available using lambda from that states
                    for (auto l_a_l_state = transition_table[*l_a_state][letter->second].begin();
                         l_a_l_state != transition_table[*l_a_state][letter->second].end(); ++l_a_l_state)
                        transition_table[l_a_state][i->second].insert(transition_table[*l_a_l_state][lambda_id].begin(),
                                                              transition_table[*l_a_l_state][lambda_id].end());
                }
        }
    }
}

void lambda_to_nfa()
{
    lambda_closure();
    transition_function();
    letters_map.erase('-');
}

int main()
{
	ifstream in("project2.in");
	string line;
	getline(in, line);
	int index = 0;

    for (auto i = 0; i < line.length(); ++i)
        if (isdigit(line[i]))
            accepting_states.push_back(line[i] - '0');

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

    /*for (int i = 0; i < nr_states; ++i)
    {
        cout << i << "\n\n";

        for (auto k = letters_map.begin(); k != letters_map.end(); ++k)
        {
            cout << k->first << ": ";

            for (auto j = transition_table[i][k->second].begin(); j != transition_table[i][k->second].end(); ++j)
                cout << *j << ' ';

            cout << '\n';
        }
    }*/

	return 0;
}