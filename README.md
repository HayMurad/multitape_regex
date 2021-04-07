# multitape_regex_match

Multitape regex match is a program that finds matches for specified multitape regular expression in sperifid file or string. 
The program outputs number of microseconds the algorithm ran and number of words matched.

Arguments:

-regex  - regular expression pattern

-symbol_subset  - space seperated symbols of subset (multiple can be specified)

-filename  - file to find matches from

-string_to_match  - string to find matches from

Example:

./regex_match -regex a+bc -symbol_subset a b -symbol_subset c -string_to_match aabbcc
