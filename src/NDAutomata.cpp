#include "NDAutomata.h"

#include <stack>
#include <queue>
#include <ctype.h>
#include <algorithm>

int priority(char c) {
	switch (c) {
	case '*': return 3;
	case '.': return 2;
	case '+': return 1;
	default: return 0;
	}
}

std::string regexp_to_posfix(const std::string& regexp)
{
	std::string postfix = "";
	std::stack<char> op;
	char c;
	for (unsigned int i = 0; i < regexp.size(); i++)
	{
		if (isalpha(regexp[i]))
		{
			postfix += regexp[i];
		}
		else if (regexp[i] == '(')
		{
			op.push(regexp[i]);
		}
		else if (regexp[i] == ')')
		{
			while (op.top() != '(') {
				postfix += op.top();
				op.pop();
			}
			op.pop();
		}
		else {
			while (!op.empty()) {
				c = op.top();
				if (priority(c) >= priority(regexp[i])) {
					postfix += op.top();
					op.pop();
				}
				else break;
			}
			op.push(regexp[i]);
		}
	}
	while (!op.empty())
	{
		postfix += op.top();
		op.pop();
	}
	return postfix;
}

std::string insert_concat(const std::string& regexp) {
	std::string ret = "";
	char c, c2;
	for (unsigned int i = 0; i < regexp.size(); i++) {
		c = regexp[i];
		if (i + 1 < regexp.size()) {
			c2 = regexp[i + 1];
			ret += c;
			if (c != '('&&c2 != ')'&&c != '+'&&c2 != '+'&&c2 != '*') {
				ret += '.';
			}
		}
	}
	ret += regexp[regexp.size() - 1];
	return ret;
}

NDAutomata::NDAutomata(const std::string& regexp)
{
	init(regexp);
}


NDAutomata::~NDAutomata()
{
}

void NDAutomata::add_state(int state)
{
	states.insert(state);
}

void NDAutomata::add_transition(int from_state, int to_state, char c)
{
	transitions[from_state][to_state].push_back(c);
}

void NDAutomata::init(const std::string& regexp)
{
	std::string posfix_regexp = regexp_to_posfix(insert_concat(regexp));
	int last_state = 0;
	std::stack<std::pair<int, int>> state_stack;

	for (int i = 0; i < posfix_regexp.size(); i++)
	{
		if (isalpha(posfix_regexp[i]))
		{
			character(posfix_regexp[i], state_stack, last_state);
			language.insert(posfix_regexp[i]);
		}
		else if (posfix_regexp[i] == '*')
		{
			kleene_star(state_stack, last_state);
		}
		else if (posfix_regexp[i] == '.') {
			concatenation(state_stack, last_state);
		}
		else if (posfix_regexp[i] == '+')
		{
			regex_union(state_stack, last_state);
		}
	}
	start_state = state_stack.top().first;
	final_state = state_stack.top().second;
}

void NDAutomata::character(char c, std::stack<std::pair<int, int>>& state_stack, int& last_state)
{
	int state_1 = ++last_state;
	int state_2 = ++last_state;

	add_state(state_1);
	add_state(state_2);
	add_transition(state_1, state_2, c);
	state_stack.push(std::make_pair(state_1, state_2));
}

void NDAutomata::kleene_star(std::stack<std::pair<int, int>>& state_stack, int& last_state)
{
	std::pair<int, int> one = state_stack.top();
	state_stack.pop();

	int state_1 = ++last_state;
	int state_2 = ++last_state;

	add_state(state_1);
	add_state(state_2);

	add_transition(state_1, one.first, 0);
	add_transition(state_1, state_2, 0);
	add_transition(one.second, state_2, 0);
	add_transition(one.second, one.first, 0);

	state_stack.push(std::make_pair(state_1, state_2));
}

void NDAutomata::concatenation(std::stack<std::pair<int, int>>& state_stack, int& last_state)
{
	std::pair<int, int> two = state_stack.top();
	state_stack.pop();
	std::pair<int, int> one = state_stack.top();
	state_stack.pop();

	std::map<int, std::vector<char> > state_transitions = transitions[two.first];
	states.erase(two.first);
	transitions.erase(two.first);
	transitions[one.second] = state_transitions;

	state_stack.push(std::make_pair(one.first, two.second));
}

void NDAutomata::regex_union(std::stack<std::pair<int, int>>& state_stack, int& last_state)
{
	std::pair<int, int> two = state_stack.top();
	state_stack.pop();
	std::pair<int, int> one = state_stack.top();
	state_stack.pop();

	int state1 = ++last_state;
	int state2 = ++last_state;

	add_state(state1);
	add_state(state2);
	add_transition(state1, one.first, 0);
	add_transition(state1, two.first, 0);
	add_transition(one.second, state2, 0);
	add_transition(two.second, state2, 0);
	state_stack.push(std::make_pair(state1, state2));
}

std::vector<int> NDAutomata::get_next_states(int state, char c)
{
	std::vector<int> next_states;
	std::map<int, std::vector<char> >::iterator it = transitions[state].begin();
	std::map<int, std::vector<char> >::iterator it_end = transitions[state].end();

	for (; it != it_end; ++it)
	{
		if (std::find(it->second.begin(), it->second.end(), c) != it->second.end())
		{
			next_states.push_back(it->first);
		}
	}

	return next_states;
}

std::set<int> NDAutomata::get_next_states(const std::set<int>& states, char c)
{
	std::set<int> next_states;
	std::vector<int> next_states_for_state;

	std::set<int>::const_iterator it = states.begin();
	std::set<int>::const_iterator it_end = states.end();

	for (; it != it_end; ++it)
	{
		next_states_for_state = get_next_states(*it, c);
		next_states.insert(next_states_for_state.begin(), next_states_for_state.end());
	}
	
	return next_states;
}

void NDAutomata::get_epsilion_closure(std::set<int>& states)
{
	std::queue<int> state_queue;
	std::set<int>::const_iterator it = states.begin();
	std::set<int>::const_iterator it_end = states.end();

	for (; it != it_end; ++it)
	{
		state_queue.push(*it);
	}

	while (!state_queue.empty())
	{
		int state = state_queue.front();
		state_queue.pop();
		std::vector<int> next_states = get_next_states(state, 0);

		for (int i = 0; i < next_states.size(); i++)
		{
			if (states.insert(next_states[i]).second)
			{
				state_queue.push(next_states[i]);
			}
		}
	}
}

DFAutomata NDAutomata::construct_dfa()
{
	std::map<std::set<int>, int> dstates;
	std::map<int, std::map<int, char> > dfa_transitions;
	std::queue<std::set<int>> state_queue;

	std::set<int> current_states;
	std::set<int> next_states;
	current_states.insert(start_state);
	get_epsilion_closure(current_states);

	int state_num = 0;
	state_queue.push(current_states);
	dstates[current_states] = state_num++;

	while (!state_queue.empty())
	{
		current_states = state_queue.front();
		state_queue.pop();

		int current_states_num = dstates[current_states];

		std::set<char>::const_iterator it = language.begin();
		std::set<char>::const_iterator it_end = language.end();
		for (; it != it_end; ++it)
		{
			next_states = get_next_states(current_states, *it);
			if (!next_states.empty()) {
				get_epsilion_closure(next_states);

				int next_states_num;
				if (dstates.find(next_states) == dstates.end())
				{
					state_queue.push(next_states);
					next_states_num = state_num++;
					dstates[next_states] = next_states_num;
					
				}
				else {
					next_states_num = dstates[next_states];
				}
				dfa_transitions[current_states_num][next_states_num] = *it;
			}
		}
	}

	int dfa_start_state;
	std::set<int> dfa_final_states;

	std::map<std::set<int>, int>::iterator dit = dstates.begin();
	std::map<std::set<int>, int>::iterator dit_end = dstates.end();

	for (; dit != dit_end; ++dit)
	{
		if (dit->first.find(start_state) != dit->first.end())
		{
			dfa_start_state = dit->second;
		}
		if (dit->first.find(final_state) != dit->first.end())
		{
			dfa_final_states.insert(dit->second);
		}
	}

	return DFAutomata(dfa_start_state, dfa_final_states, dfa_transitions);
}

