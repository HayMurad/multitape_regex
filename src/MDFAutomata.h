#pragma once
#include "DFAutomata.h"
#include <set>
#include <map>
class MDFAutomata :
	public DFAutomata
{
public:
	MDFAutomata(int start_state_, const std::set<int> &final_states_,
		const std::vector<std::vector<int>>& transitions_, const symbol_subsets_t& symbol_subsets_, int num_additional_symbols_);
	MDFAutomata(const DFAutomata& dfa, const symbol_subsets_t& symbol_subsets);
	static MDFAutomata join(const MDFAutomata& first, const MDFAutomata& second, int& first_start_state, int& second_start_state);
	bool congruence_builder(int first_start_state, int second_start_state, 
		std::set<std::vector<int>> &first_coordinates, std::set<std::vector<int>> &second_coordinates);
	int get_num_additional_symbols();
	static std::string get_word_from_coordinate(std::deque<int> coordinate, const symbol_subsets_t& symbol_subsets);
	static double distance_of_words(const std::string& word1, const std::string& word2, const symbol_subsets_t& symbol_subsets);

private:
	static std::vector<int> get_word_c_for_symbol(const std::string& word, char c, const symbol_subsets_t& symbol_subsets);
	static std::vector<std::vector<int>> get_word_c_vector(const std::string& word, const symbol_subsets_t& symbol_subsets);


	void construct_mdfa(const DFAutomata& dfa, const symbol_subsets_t& symbol_subsets);
	int get_subset_index(char c);

	std::set<int> divide(const std::set<int> &eq_class, const std::set<int> &div_class, int symbol);
	std::vector<int> get_divide_coordinate(const std::vector<int> &coordinate, int symbol);


private:
	symbol_subsets_t symbol_subsets;
	int num_additional_symbols;
};


