#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>

#include <chrono>
using namespace std::chrono;

#include "NDAutomata.h"
#include "MDFAutomata.h"

void parce_options(int argc, char* argv[], std::string& regex1, std::string& regex2, DFAutomata::symbol_subsets_t& symbol_subsets)
{
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-regex1") == 0)
		{
			if (i + 1 < argc)
			{
				regex1 = argv[i + 1];
				i++;
			}
		}
		else if (strcmp(argv[i], "-regex2") == 0)
		{
			if (i + 1 < argc)
			{
				regex2 = argv[i + 1];
				i++;
			}
		}
		else if (strcmp(argv[i], "-symbol_subset") == 0)
		{
			i++;
			std::set<char> symbol_subset;
			while (i < argc)
			{
				if (argv[i][0] == '-')
				{
					i--;
					break;
				}
				symbol_subset.insert(argv[i][0]);
				i++;
			}
			if (!symbol_subset.empty()) {
				symbol_subsets.push_back(symbol_subset);
			}
		}
	}
}

int main(int argc, char* argv[])
{
	std::string regex1;
	std::string regex2;
	DFAutomata::symbol_subsets_t symbol_subsets;

	parce_options(argc, argv, regex1, regex2, symbol_subsets);

	if (regex1.empty())
	{
		std::cout << "-regex1 is not specified" << std::endl;
		return 1;
	}
	if (regex2.empty())
	{
		std::cout << "-regex2 is not specified" << std::endl;
		return 1;
	}
	if (symbol_subsets.empty())
	{
		std::cout << "-symbol_subset is not specified" << std::endl;
		return 1;
	}


	NDAutomata nda1(regex1);
	DFAutomata dfa1 = nda1.construct_dfa();

	NDAutomata nda2(regex2);
	DFAutomata dfa2 = nda2.construct_dfa();

	auto start = high_resolution_clock::now();

	MDFAutomata mdfa1(dfa1, symbol_subsets);
	MDFAutomata mdfa2(dfa2, symbol_subsets);
	int first_start_state, second_start_state;
	MDFAutomata mdfa_joined = MDFAutomata::join(mdfa1, mdfa2, first_start_state, second_start_state);
	std::set<std::vector<int>> first_coordinates;
	std::set<std::vector<int>> second_coordinates;
	bool are_equal = mdfa_joined.congruence_builder(first_start_state, second_start_state, first_coordinates, second_coordinates);

	if (are_equal)
	{
		std::cout << "Regexs are equal" << std::endl;
		return 0;
	}

	int num_additional_symbols = mdfa_joined.get_num_additional_symbols();

	std::set<std::deque<int>> first_words;
	std::set<std::deque<int>> second_words;

	auto first_coordinates_it = first_coordinates.begin();
	auto first_coordinates_it_end = first_coordinates.end();

	for (; first_coordinates_it != first_coordinates_it_end; ++first_coordinates_it)
	{
		int size = first_coordinates_it->size() - num_additional_symbols;
		first_words.insert(std::deque<int>(first_coordinates_it->begin(), first_coordinates_it->begin() + size));
	}

	auto second_coordinates_it = second_coordinates.begin();
	auto second_coordinates_it_end = second_coordinates.end();

	for (; second_coordinates_it != second_coordinates_it_end; ++second_coordinates_it)
	{
		int size = second_coordinates_it->size() - num_additional_symbols;
		second_words.insert(std::deque<int>(second_coordinates_it->begin(), second_coordinates_it->begin() + size));
	}

	std::vector<std::deque<int>> first_words_additional;

	auto second_words_it = second_words.begin();
	auto second_words_it_end = second_words.end();

	for (; second_words_it != second_words_it_end; ++second_words_it)
	{
		if (first_words.find(*second_words_it) == first_words.end())
		{
			if (dfa1.match_full_word(MDFAutomata::get_word_from_coordinate(*second_words_it, symbol_subsets), symbol_subsets))
			{
				first_words_additional.push_back(*second_words_it);
			}
		}
	}

	std::vector<std::deque<int>> second_words_additional;

	auto first_words_it = first_words.begin();
	auto first_words_it_end = first_words.end();

	for (; first_words_it != first_words_it_end; ++first_words_it)
	{
		if (second_words.find(*first_words_it) == second_words.end())
		{
			if (dfa2.match_full_word(MDFAutomata::get_word_from_coordinate(*first_words_it, symbol_subsets), symbol_subsets))
			{
				second_words_additional.push_back(*first_words_it);
			}
		}
	}

	first_words.insert(first_words_additional.begin(), first_words_additional.end());
	second_words.insert(second_words_additional.begin(), second_words_additional.end());

	double distance1 = DBL_MIN;

	auto first_word_it = first_words.begin();
	auto first_word_it_end = first_words.end();

	for (; first_word_it != first_word_it_end; ++first_word_it)
	{
		double distance = DBL_MAX;

		std::string first_word = MDFAutomata::get_word_from_coordinate(*first_word_it, symbol_subsets);

		auto second_word_it = second_words.begin();
		auto second_word_it_end = second_words.end();

		for (; second_word_it != second_word_it_end; ++second_word_it)
		{
			double d = MDFAutomata::distance_of_words(first_word, 
				MDFAutomata::get_word_from_coordinate(*second_word_it, symbol_subsets), symbol_subsets);
			if (distance > d)
			{
				distance = d;
			}
		}
		if (distance1 < distance)
		{
			distance1 = distance;
		}
	}

	double distance2 = DBL_MIN;

	auto second_word_it = second_words.begin();
	auto second_word_it_end = second_words.end();

	for (; second_word_it != second_word_it_end; ++second_word_it)
	{
		double distance = DBL_MAX;

		std::string second_word = MDFAutomata::get_word_from_coordinate(*second_word_it, symbol_subsets);

		auto first_word_it = first_words.begin();
		auto first_word_it_end = first_words.end();

		for (; first_word_it != first_word_it_end; ++first_word_it)
		{
			double d = MDFAutomata::distance_of_words(second_word,
				MDFAutomata::get_word_from_coordinate(*first_word_it, symbol_subsets), symbol_subsets);
			if (distance > d)
			{
				distance = d;
			}
		}
		if (distance2 < distance)
		{
			distance2 = distance;
		}
	}

	double distance = distance1 > distance2 ? distance1 : distance2;

	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);

	std::cout << "Time taken by function: " << duration.count() << " microseconds" << std::endl;
	std::cout << "Distance: " << distance << std::endl;
	
	return 0;
}

