#include "MDFAutomata.h"

MDFAutomata::MDFAutomata(const DFAutomata& dfa, const symbol_subsets_t& symbol_subsets_)
	: symbol_subsets(symbol_subsets_)
{
	construct_mdfa(dfa, symbol_subsets_);
}

MDFAutomata::MDFAutomata(int start_state_, const std::set<int> &final_states_, const std::vector<std::vector<int>>& transitions_,
	const symbol_subsets_t& symbol_subsets_, int num_additional_symbols_)
	: symbol_subsets(symbol_subsets_)
	, num_additional_symbols(num_additional_symbols_)
{
	start_state = start_state_;
	final_states = final_states_;
	transitions = transitions_;
}

void MDFAutomata::construct_mdfa(const DFAutomata& dfa, const symbol_subsets_t& symbol_subsets)
{
	std::vector<std::vector<int>> dfa_transitions = dfa.get_transitions();

	int num_of_tapes = symbol_subsets.size();
	int language_size = 256 + num_of_tapes;

	transitions.resize(dfa_transitions.size());
	for (int i = 0; i < dfa_transitions.size(); i++)
	{
		transitions[i] = std::vector<int>(language_size, -1);
	}

	int current_additional_state = dfa_transitions.size();
	
	for (int i = 0; i < dfa_transitions.size(); i++)
	{
		std::set<int> symbol_transition_tapes;
		for (int j = 0; j < dfa_transitions[i].size(); j++)
		{
			if (dfa_transitions[i][j] != -1)
			{
				symbol_transition_tapes.insert(get_subset_index((char)j));
			}
		}

		if (symbol_transition_tapes.size() > 1)
		{
			for (int k = 0; k < num_of_tapes; k++)
			{
				transitions.push_back(std::vector<int>(language_size, -1));
			}
			for (int j = 0; j < dfa_transitions[i].size(); j++)
			{
				if (dfa_transitions[i][j] != -1)
				{
					int subset_index = get_subset_index((char)j);
					transitions[i][256 + subset_index] = subset_index + current_additional_state;
					transitions[subset_index + current_additional_state][j] = dfa_transitions[i][j];
				}
			}
			current_additional_state += num_of_tapes;
		}
		else
		{
			for (int j = 0; j < dfa_transitions[i].size(); j++)
			{
				transitions[i][j] = dfa_transitions[i][j];
			}
		}
	}

	start_state = dfa.get_start_state();
	final_states = dfa.get_final_states();
	num_additional_symbols = num_of_tapes;
}

int MDFAutomata::get_subset_index(char c)
{
	for (int j = 0; j < symbol_subsets.size(); j++)
	{
		if (symbol_subsets[j].find(c) != symbol_subsets[j].end())
		{
			return j;
		}
	}
	return -1;
}

MDFAutomata MDFAutomata::join(const MDFAutomata& first, const MDFAutomata& second, int& first_start_state, int& second_start_state)
{
	int num_states_first = first.transitions.size();
	int num_states_second = second.transitions.size();

	int num_additional_symbols = std::max(first.num_additional_symbols, second.num_additional_symbols);
	int language_size = 256 + num_additional_symbols;

	std::vector<std::vector<int>> transitions(num_states_first + num_states_second);
	for (int i = 0; i < transitions.size(); i++)
	{
		transitions[i] = std::vector<int>(language_size, -1);
	}

	for (int i = 0; i < num_states_first; i++)
	{
		for (int j = 0; j < first.transitions[i].size(); j++)
		{
			transitions[i][j] = first.transitions[i][j];
		}
	}

	for (int i = 0; i < num_states_second; i++)
	{
		for (int j = 0; j < second.transitions[i].size(); j++)
		{
			if (second.transitions[i][j] != -1)
			{
				transitions[num_states_first + i][j] = second.transitions[i][j] + num_states_first;
			}
		}
	}

	std::set<int> final_states = first.final_states;
	auto it = second.final_states.begin();
	auto it_end = second.final_states.end();

	for (; it != it_end; ++it)
	{
		final_states.insert(*it + num_states_first);
	}

	first_start_state = first.start_state;
	second_start_state = second.start_state + num_states_first;

	return MDFAutomata(-1, final_states, transitions, first.symbol_subsets, num_additional_symbols);
}

int MDFAutomata::get_num_additional_symbols()
{
	return num_additional_symbols;
}

bool MDFAutomata::congruence_builder(int first_start_state, int second_start_state, std::set<std::vector<int>> &first_coordinates, std::set<std::vector<int>> &second_coordinates)
{
	int number_of_symbols = 0;
	auto symbol_subset_it = symbol_subsets.begin();
	auto symbol_subset_it_end = symbol_subsets.end();

	std::vector<int> language;

	for (; symbol_subset_it != symbol_subset_it_end; ++symbol_subset_it)
	{
		number_of_symbols += symbol_subset_it->size();
		language.insert(language.end(), symbol_subset_it->begin(), symbol_subset_it->end());
	}
	number_of_symbols += num_additional_symbols;

	for (int i = 0; i < num_additional_symbols; i++)
	{
		language.push_back(256 + i);
	}

	typedef std::vector<int> coordinate_t;
	typedef std::set<int> eq_class_t;
	typedef std::map<eq_class_t, std::set<coordinate_t>> equivalance_classes_t;
	typedef std::map<eq_class_t, std::vector<coordinate_t>> current_states_t;

	equivalance_classes_t equivalance_classes;

	eq_class_t final_states(this->final_states.begin(), this->final_states.end());
	eq_class_t non_final_states;
	for (int i = 0; i < transitions.size(); i++)
	{
		if (final_states.find(i) == final_states.end())
		{
			non_final_states.insert(i);
		}
	}

	coordinate_t zero_coordinate(number_of_symbols, 0);
	std::set<coordinate_t> zero_coordinates;
	zero_coordinates.insert(zero_coordinate);

	equivalance_classes[final_states] = zero_coordinates;
	equivalance_classes[non_final_states] = std::set<coordinate_t>();

	current_states_t current_states;
	current_states[final_states].push_back(zero_coordinate);

	for (int stage = 0; stage <= transitions.size(); stage++)
	{
		current_states_t next_states;

		auto current_states_it = current_states.begin();
		auto current_states_it_end = current_states.end();

		for (; current_states_it != current_states_it_end; ++current_states_it)
		{
			auto language_it = language.begin();
			auto language_it_end = language.end();

			for (; language_it != language_it_end; ++language_it)
			{
				equivalance_classes_t new_equivalance_classes;

				auto equivalance_classes_it = equivalance_classes.begin();
				auto equivalance_classes_it_end = equivalance_classes.end();

				for (; equivalance_classes_it != equivalance_classes_it_end; ++equivalance_classes_it)
				{
					eq_class_t divided_state = divide(equivalance_classes_it->first, current_states_it->first, *language_it);
					if (divided_state.size() != 0)
					{
						std::set<coordinate_t> divided_coordinates;
						auto current_coordinates_it = current_states_it->second.begin();
						auto current_coordinates_it_end = current_states_it->second.end();

						for (; current_coordinates_it != current_coordinates_it_end; ++current_coordinates_it)
						{
							divided_coordinates.insert(get_divide_coordinate(*current_coordinates_it, *language_it));
						}

						std::vector<coordinate_t>& next_states_coordinates = next_states[divided_state];
						next_states_coordinates.insert(next_states_coordinates.end(), divided_coordinates.begin(), divided_coordinates.end());

						if (new_equivalance_classes.find(divided_state) == new_equivalance_classes.end())
						{
							new_equivalance_classes[divided_state] = equivalance_classes_it->second;
						}
						new_equivalance_classes[divided_state].insert(divided_coordinates.begin(), divided_coordinates.end());

						if (divided_state.size() != equivalance_classes_it->first.size())
						{
							eq_class_t rest_eq_class;
							auto equivalance_class_state_it = equivalance_classes_it->first.begin();
							auto equivalance_class_state_it_end = equivalance_classes_it->first.end();

							for (; equivalance_class_state_it != equivalance_class_state_it_end; ++equivalance_class_state_it)
							{
								if (divided_state.find(*equivalance_class_state_it) == divided_state.end())
								{
									rest_eq_class.insert(*equivalance_class_state_it);
								}
							}

							new_equivalance_classes[rest_eq_class].insert(equivalance_classes_it->second.begin(),
								equivalance_classes_it->second.end());
						}
					}
					else
					{
						new_equivalance_classes[equivalance_classes_it->first].insert(equivalance_classes_it->second.begin(),
							equivalance_classes_it->second.end());
					}
				}

				equivalance_classes = new_equivalance_classes;
			}
		}

		current_states = next_states;
	}
	
	auto equivalance_classes_it = equivalance_classes.begin();
	auto equivalance_classes_it_end = equivalance_classes.end();

	for (; equivalance_classes_it != equivalance_classes_it_end; ++equivalance_classes_it)
	{
		if (equivalance_classes_it->first.find(first_start_state) != equivalance_classes_it->first.end())
		{
			first_coordinates = std::set<std::vector<int>>(equivalance_classes_it->second);
		}
		if (equivalance_classes_it->first.find(second_start_state) != equivalance_classes_it->first.end())
		{
			second_coordinates = std::set<std::vector<int>>(equivalance_classes_it->second);
		}
	}

	return first_coordinates == second_coordinates;
}

std::set<int> MDFAutomata::divide(const std::set<int> &eq_class, const std::set<int> &div_class, int symbol)
{
	std::set<int> result;
	auto eq_class_it = eq_class.begin();
	auto eq_class_it_end = eq_class.end();

	for (; eq_class_it != eq_class_it_end; ++eq_class_it)
	{
		auto div_class_it = div_class.begin();
		auto div_class_it_end = div_class.end();

		for (; div_class_it != div_class_it_end; ++div_class_it)
		{
			if (transitions[*eq_class_it][symbol] == *div_class_it)
			{
				result.insert(*eq_class_it);
			}
		}
	}

	return result;
}

std::vector<int> MDFAutomata::get_divide_coordinate(const std::vector<int> &coordinate, int symbol)
{
	int begin = 0, end, index = -1;
	
	auto symbol_subset_it = symbol_subsets.begin();
	auto symbol_subset_it_end = symbol_subsets.end();

	for (; symbol_subset_it != symbol_subset_it_end; ++symbol_subset_it)
	{
		if (symbol_subset_it->find(symbol) != symbol_subset_it->end())
		{
			auto symbols_it = symbol_subset_it->begin();
			auto symbols_it_end = symbol_subset_it->end();
			index = 0;
			for (; symbols_it != symbols_it_end; ++symbols_it)
			{
				if (*symbols_it == symbol)
				{
					break;
				}
				index++;
			}
			end = begin + symbol_subset_it->size();
			break;
		}
		else
		{
			begin += symbol_subset_it->size();
		}
	}

	if (index == -1)
	{
		end = begin + num_additional_symbols;
		index = symbol - 256;
	}
	

	std::vector<int> result = coordinate;
	for (int i = begin; i < end; i++)
	{
		result[i] *= 2;
	}
	result[begin + index] += 1;

	return result;
}

int remove_leading_one(int num)
{
	std::vector<int> bin_num;
	while (num)
	{
		bin_num.push_back(num % 2);
		num /= 2;
	}

	int result_num = 0;
	for (int i = bin_num.size() - 2; i >= 0; i--)
	{
		result_num *= 2;
		result_num += bin_num[i];
	}

	return result_num;
}

std::string MDFAutomata::get_word_from_coordinate(std::deque<int> coordinate, const symbol_subsets_t& symbol_subsets)
{
	std::string word;

	int begin = 0, end;
	
	for (int i = 0; i < symbol_subsets.size(); i++)
	{
		std::string word_part;
		std::vector<char> symbols(symbol_subsets[i].begin(), symbol_subsets[i].end());
		end = begin + symbols.size();
		while (true)
		{
			int index = std::max_element(coordinate.begin() + begin, coordinate.begin() + end) - coordinate.begin() - begin;
			if (coordinate[begin + index] == 0)
			{
				break;
			}
			word_part.insert(0, 1, symbols[index]);
			coordinate[begin + index] = remove_leading_one(coordinate[begin + index]);
		}
		word += word_part;
		begin += symbols.size();
	}

	return word;
}

std::vector<int> MDFAutomata::get_word_c_for_symbol(const std::string& word, char c, const symbol_subsets_t& symbol_subsets)
{
	int current_subset_index;
	for (int i = 0; i < symbol_subsets.size(); i++)
	{
		if (symbol_subsets[i].find(c) != symbol_subsets[i].end())
		{
			current_subset_index = i;
			break;
		}
	}

	std::vector<int> result;

	int prev_index = 0;
	for (int i = 0; i < word.size(); i++)
	{
		if (word[i] == c)
		{
			int count = 0;
			for (int j = prev_index; j < i; j++)
			{
				if (symbol_subsets[current_subset_index].find(word[j]) != symbol_subsets[current_subset_index].end())
				{
					count++;
				}
			}
			result.push_back(count);
			prev_index = i + 1;
		}
	}

	int count = 0;
	for (int j = prev_index; j < word.size(); j++)
	{
		if (symbol_subsets[current_subset_index].find(word[j]) != symbol_subsets[current_subset_index].end())
		{
			count++;
		}
	}
	result.push_back(count);

	return result;
}

std::vector<std::vector<int>> MDFAutomata::get_word_c_vector(const std::string& word, const symbol_subsets_t& symbol_subsets)
{
	std::vector<std::vector<int>> result_c_vector;

	for (int i = 0; i < symbol_subsets.size(); i++)
	{
		auto symbol_subset_it = symbol_subsets[i].begin();
		auto symbol_subset_it_end = symbol_subsets[i].end();

		for (; symbol_subset_it != symbol_subset_it_end; ++symbol_subset_it)
		{
			result_c_vector.push_back(get_word_c_for_symbol(word, *symbol_subset_it, symbol_subsets));
		}
	}

	return result_c_vector;
}


template <typename T>
double l2_distance(const std::vector<T> &v1, const std::vector<T> &v2)
{
	double result = 0;
	for (int i = 0; i < v1.size(); i++)
	{
		result += (v1[i] - v2[i])*(v1[i] - v2[i]);
	}
	return sqrt(result);
}

template <typename T>
double l2_norm(const std::vector<T> &v)
{
	double result = 0;
	for (int i = 0; i < v.size(); i++)
	{
		result += v[i] * v[i];
	}
	return sqrt(result);
}

double MDFAutomata::distance_of_words(const std::string& word1, const std::string& word2, const symbol_subsets_t& symbol_subsets)
{
	std::vector<std::vector<int>> c1 = get_word_c_vector(word1, symbol_subsets);
	std::vector<std::vector<int>> c2 = get_word_c_vector(word2, symbol_subsets);

	std::vector<double> m_vector;

	for (int i = 0; i < c1.size(); i++)
	{
		if (c1[i].size() < c2[i].size())
		{
			c1[i].insert(c1[i].end(), c2[i].size() - c1[i].size(), -1);
		}
		if (c1[i].size() > c2[i].size())
		{
			c2[i].insert(c2[i].end(), c1[i].size() - c2[i].size(), -1);
		}

		m_vector.push_back(l2_distance(c1[i], c2[i]));
	}

	return l2_norm(m_vector);
}
