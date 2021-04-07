#include "DFAutomata.h"
#include <stack>

DFAutomata::DFAutomata(int start_state_, const std::set<int> &final_states_, 
	const std::map<int, std::map<int, char> >& transitions_)
	: start_state(start_state_)
	, final_states(final_states_)
{
	int max_states = 0;
	std::map<int, std::map<int, char> >::const_iterator it = transitions_.begin();
	std::map<int, std::map<int, char> >::const_iterator it_end = transitions_.end();

	for (; it != it_end; ++it)
	{
		if (it->first > max_states)
		{
			max_states = it->first;
		}
		std::map<int, char>::const_iterator map_it = it->second.begin();
		std::map<int, char>::const_iterator map_it_end = it->second.end();
		for (; map_it != map_it_end; ++map_it)
		{
			if (map_it->first > max_states)
			{
				max_states = map_it->first;
			}
		}
	}

	int language_size = 256;

	transitions.resize(max_states+1);
	for (int i = 0; i < max_states + 1; i++)
	{
		transitions[i] = std::vector<int>(language_size, -1);
	}

	it = transitions_.begin();
	it_end = transitions_.end();

	for (; it != it_end; ++it)
	{
		std::map<int, char>::const_iterator map_it = it->second.begin();
		std::map<int, char>::const_iterator map_it_end = it->second.end();
		for (; map_it != map_it_end; ++map_it)
		{
			transitions[it->first][(int)map_it->second] = map_it->first;
		}
	}

}


DFAutomata::~DFAutomata()
{
}

void DFAutomata::minimize()
{
	std::set<std::set<int>> current_partition;
	current_partition.insert(final_states);

	std::set<int> non_final_states;
	for (int i = 0; i < transitions.size(); i++)
	{
		if (final_states.find(i) == final_states.end())
		{
			non_final_states.insert(i);
		}
	}
	current_partition.insert(non_final_states);

	std::set<std::set<int>> next_partition;

	//todo

}

int DFAutomata::get_tape_index(char c, const symbol_subsets_t& symbol_subsets)
{
	for (int i = 0; i < symbol_subsets.size(); i++)
	{
		if (symbol_subsets[i].find(c) != symbol_subsets[i].end())
		{
			return i;
		}
	}
	return -1;
}

void DFAutomata::match_in_the_begining_from_state(int state, tapes_t& tapes, const std::string &matching_string, int matching_string_index,
	int& counter, int last_count, const symbol_subsets_t& symbol_subsets, int& result_index)
{
	if (this->final_states.find(state) != this->final_states.end())
	{
		bool is_matched = true;
		for (int i=0; i<tapes.size(); i++)
		{
			if (!tapes[i].empty() && tapes[i].front().second < last_count)
			{
				is_matched = false;
				break;
			}
		}

		if (is_matched)
		{
			if (last_count > result_index)
			{
				result_index = last_count;
			}
		}
	}

	int num_of_empty_tapes = 0;

	for (int i=0; i < tapes.size(); i++)
	{
		if (tapes[i].empty())
		{
			num_of_empty_tapes++;
		}
	}

	while (num_of_empty_tapes > 0)
	{
		if (matching_string_index == matching_string.size())
		{
			break;
		}
		char c = matching_string[matching_string_index];
		matching_string_index++;

		int tape_index = get_tape_index(c, symbol_subsets);
		if (tapes[tape_index].empty())
		{
			num_of_empty_tapes--;
		}
		tapes[tape_index].push_back(std::make_pair(c, counter++));
	}

	for (int i = 0; i < tapes.size(); i++)
	{
		if (!tapes[i].empty())
		{
			std::pair<char, int> current_symbol_pair = tapes[i].front();
			int next_state = this->transitions[state][(int)current_symbol_pair.first];
			if (next_state != -1)
			{
				tapes[i].pop_front();
				match_in_the_begining_from_state(next_state, tapes, matching_string, matching_string_index, counter,
					std::max(current_symbol_pair.second, last_count), symbol_subsets, result_index);
				tapes[i].push_front(current_symbol_pair);
			}
		}
	}
}

std::vector<std::string> DFAutomata::match_word(const std::string& matching_string, symbol_subsets_t& symbol_subsets)
{
	std::vector<std::string> matched_words;
	int matching_string_index = 0;
	tapes_t tapes(symbol_subsets.size());
	while (matching_string_index < matching_string.size())
	{
		for (int i = 0; i < tapes.size(); i++)
		{
			while (!tapes[i].empty())
			{
				tapes[i].pop_back();
			}
		}

		int result_index = -2;
		int counter = 0;
		match_in_the_begining_from_state_non_recursive(this->start_state, tapes, matching_string, matching_string_index,
			counter, -1, symbol_subsets, result_index);

		if (result_index == -2)
		{
			matching_string_index++;
		}
		else {
			int matched_word_size = result_index + 1;
			matched_words.push_back(matching_string.substr(matching_string_index, matched_word_size));

			if (matched_word_size == 0) {
				matching_string_index++;
			}
			else {
				matching_string_index += matched_word_size;
			}
		}
	}

	return matched_words;
}


void DFAutomata::match_in_the_begining_from_state_non_recursive(int state, tapes_t& tapes, const std::string &matching_string, int matching_string_index,
	int& counter, int last_count, const symbol_subsets_t& symbol_subsets, int& result_index)
{

	struct stack_entry_t
	{
		int state;
		int last_count;
		std::pair<char, int> prev_symbol_pair;
		int prev_tape_index;

		stack_entry_t(int state_, int last_count_, const std::pair<char, int>& prev_symbol_pair_, int prev_tape_index_)
			: state(state_)
			, last_count(last_count_)
			, prev_symbol_pair(prev_symbol_pair_)
			, prev_tape_index(prev_tape_index_)
		{}
	};

	std::stack<stack_entry_t> function_stack;
	stack_entry_t init_stack_entry(state, last_count, std::make_pair(0, 0), -1);
	function_stack.push(init_stack_entry);

	while (!function_stack.empty()) {
		stack_entry_t stack_entry = function_stack.top();

		if (this->final_states.find(stack_entry.state) != this->final_states.end())
		{
			bool is_matched = true;
			for (int i = 0; i < tapes.size(); i++)
			{
				if (!tapes[i].empty() && tapes[i].front().second < stack_entry.last_count)
				{
					is_matched = false;
					break;
				}
			}

			if (is_matched)
			{
				if (stack_entry.last_count > result_index)
				{
					result_index = stack_entry.last_count;
				}
			}
		}

		int num_of_empty_tapes = 0;

		for (int i = 0; i < tapes.size(); i++)
		{
			if (tapes[i].empty())
			{
				num_of_empty_tapes++;
			}
		}

		while (num_of_empty_tapes > 0)
		{
			if (matching_string_index == matching_string.size())
			{
				break;
			}
			char c = matching_string[matching_string_index];
			matching_string_index++;

			int tape_index = get_tape_index(c, symbol_subsets);
			if (tapes[tape_index].empty())
			{
				num_of_empty_tapes--;
			}
			tapes[tape_index].push_back(std::make_pair(c, counter++));
		}

		bool remove_entry = true;
		for (int i = stack_entry.prev_tape_index+1; i < tapes.size(); i++)
		{
			if (!tapes[i].empty())
			{
				std::pair<char, int> current_symbol_pair = tapes[i].front();
				int next_state = this->transitions[stack_entry.state][(int)current_symbol_pair.first];
				if (next_state != -1)
				{
					remove_entry = false;
					if (stack_entry.prev_tape_index != -1)
					{
						tapes[stack_entry.prev_tape_index].push_front(stack_entry.prev_symbol_pair);
					}
					tapes[i].pop_front();

					function_stack.top().prev_symbol_pair = current_symbol_pair;
					function_stack.top().prev_tape_index = i;
					function_stack.push(stack_entry_t(next_state, std::max(current_symbol_pair.second, stack_entry.last_count), std::make_pair(0, 0), -1));
					break;
				}
			}
		}

		if (remove_entry)
		{
			function_stack.pop();
		}

	}
}
