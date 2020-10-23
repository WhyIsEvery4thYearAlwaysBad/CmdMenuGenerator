#include "lex.hpp"
#include "tokens.hpp"
#include <deque>
extern std::deque<Token> tokens;

// Grammatical Rules:
// <STRING> <STRING> - New Bind
bool StreamIsBind(const std::size_t& iterator) {
	if (tokens.at(iterator).type==Token_type::STRING && tokens.at(iterator+1).type==Token_type::STRING) {
		return true;
	}
	else return false;
}

// #<nonterminal>: <nonterminal|string> - New Key map
bool StreamIsKeymap(const std::size_t& iterator) {
	if (tokens.at(iterator).val.front()=='#'
	&& tokens.at(iterator).type==Token_type::NONTERMINAL
	&& (tokens.at(iterator+1).type==Token_type::NONTERMINAL
	|| tokens.at(iterator+1).type==Token_type::STRING)) return true;
	else return false;
}

// TOGGLE <STRING> <STRING>.. | - Toggle Bind
bool StreamIsToggleBind(const std::size_t& iterator) {
	if (tokens.at(iterator).type==Token_type::NONTERMINAL && tokens.at(iterator).val=="TOGGLE") {
		for (unsigned long i=iterator+1; i < tokens.size(); i+=2) {
			if (tokens.at(i).type==Token_type::STRING && tokens.at((i+1)%tokens.size()).type==Token_type::STRING) ;
			else if (tokens.at(i).type==Token_type::TERMINAL && tokens.at(i).val=="|") return true;
		}
	}
	return false;
}

// STRING { - New Page
bool StreamIsNewPage(const std::size_t& iterator) {
	if (tokens.at(iterator).type==Token_type::STRING && tokens.at(iterator+1).type==TERMINAL && tokens.at(iterator+1).val=="{") {
		return true;
	}
	else return false;
}