#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>

#include <chrono>
using namespace std::chrono;

#include "NDAutomata.h"

void parce_options(int argc, char* argv[], std::string& regex, DFAutomata::symbol_subsets_t& symbol_subsets, 
	std::string& filename, std::string& string_to_match)
{
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-regex") == 0)
		{
			if (i + 1 < argc)
			{
				regex = argv[i + 1];
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
		else if(strcmp(argv[i], "-filename") == 0)
		{
			if (i + 1 < argc)
			{
				filename = argv[i + 1];
				i++;
			}
		}
		else if (strcmp(argv[i], "-string_to_match") == 0)
		{
			if (i + 1 < argc)
			{
				string_to_match = argv[i + 1];
				i++;
			}
		}
	}
}

int main(int argc, char* argv[])
{
	std::string filename;
	std::string string_to_match;
	std::string regex;
	DFAutomata::symbol_subsets_t symbol_subsets;

	parce_options(argc, argv, regex, symbol_subsets, filename, string_to_match);

	if (regex.empty())
	{
		std::cout << "-regex is not specified" << std::endl;
		return 1;
	}
	if (filename.empty() && string_to_match.empty())
	{
		std::cout << "Neighter -filename nor -string_to_match are specified" << std::endl;
		return 1;
	}
	if (!filename.empty() && !string_to_match.empty())
	{
		std::cout << "Both -filename and -string_to_match are specified. Only one of them should be specified" << std::endl;
		return 1;
	}
	if (symbol_subsets.empty())
	{
		std::cout << "-symbol_subset is not specified" << std::endl;
		return 1;
	}


	NDAutomata nda(regex);
	DFAutomata dfa = nda.construct_dfa();

	if (!filename.empty())
	{
		std::ifstream file(filename);
		string_to_match = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	}
	
	auto start = high_resolution_clock::now();

	std::vector<std::string> matched_words = dfa.match_word(string_to_match, symbol_subsets);

	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);

	std::cout << "Time taken by function: " << duration.count() << " microseconds" << std::endl;
	std::cout << "Matched words count: " << matched_words.size() << std::endl;

	return 0;
}

