#pragma once
#include <vector>
#include <map>
#include <set>
#include <stack>

#include "DFAutomata.h"

class NDAutomata
{
public:
	NDAutomata(const std::string& regexp);
	~NDAutomata();

	DFAutomata construct_dfa();

private:
	void add_state(int state);
	void add_transition(int from_state, int to_state, char c);

	void init(const std::string& regexp);
	void character(char c, std::stack<std::pair<int, int>>& state_stack, int& last_state);
	void kleene_star(std::stack<std::pair<int, int>>& state_stack, int& last_state);
	void concatenation(std::stack<std::pair<int, int>>& state_stack, int& last_state);
	void regex_union(std::stack<std::pair<int, int>>& state_stack, int& last_state);

	void get_epsilion_closure(std::set<int>& states);
	std::vector<int> get_next_states(int state, char c);
	std::set<int> get_next_states(const std::set<int>& states, char c);

private:
	std::set<int> states;
	int start_state;
	int final_state;
	std::map<int, std::map<int, std::vector<char> > > transitions;
	std::set<char> language;
};

