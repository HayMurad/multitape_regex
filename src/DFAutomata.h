#pragma once
#include <set>
#include <map>
#include <vector>
#include <deque>
#include <queue>
#include <set>

class DFAutomata
{
public:
	typedef std::vector<std::deque<std::pair<char, int>>> tapes_t;
	typedef std::vector<std::set<char>> symbol_subsets_t;

public:
	DFAutomata();
	DFAutomata(int start_state, const std::set<int> &final_states, 
		const std::map<int, std::map<int, char> >& transitions);
	~DFAutomata();

	void minimize();

	std::vector<std::string> match_word(const std::string& matching_string, const symbol_subsets_t& symbol_subsets);
	bool match_full_word(const std::string& matching_string, const symbol_subsets_t& symbol_subsets);
	bool match_full_word(std::vector<std::deque<char>>& tapes, const symbol_subsets_t& symbol_subsets);

	std::vector<std::vector<int>> get_transitions() const;
	int get_start_state() const;
	std::set<int> get_final_states() const;

private:
	void match_in_the_begining_from_state(int state, tapes_t& tapes, const std::string &matching_string, int matching_string_index,
		int& counter, int last_count, const symbol_subsets_t& symbol_subsets, int& result_index);
	void match_in_the_begining_from_state_non_recursive(int state, tapes_t& tapes, const std::string &matching_string, int matching_string_index,
		int& counter, int last_count, const symbol_subsets_t& symbol_subsets, int& result_index);
	int get_tape_index(char c, const symbol_subsets_t& symbol_subsets);

	bool match_full_word_from_state(int state, std::vector<std::deque<char>>& tapes);
	void get_tapes_from_word(const std::string& word, std::vector<std::deque<char>>& tapes, const symbol_subsets_t& symbol_subsets);

protected:
	std::vector<std::vector<int>> transitions;
	int start_state;
	std::set<int> final_states;
};

