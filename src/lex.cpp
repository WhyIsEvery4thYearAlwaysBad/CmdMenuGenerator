#include <string>
#include <algorithm>
#include <deque>
#include "lex.hpp"
#include "compiler.hpp"
#include "token.hpp"

std::size_t iLineNum=1u;
std::size_t iLineColumn=1u;

extern std::deque<Token> ErrorTokens;
extern std::deque<Token> TokenContainer;
extern std::deque<Parser::MenuToken*> CMenuTokens;

// Convert to a safer string format for file and caption names.
std::string formatRaw(std::string p_sInStr) {
	// Remove punctation and unicode characters.
	p_sInStr.erase(std::remove_if(p_sInStr.begin(), p_sInStr.end(), [](char c){ return std::ispunct(c) || !isascii(c); }), p_sInStr.end());
	// and replace spaces with underscores
	std::replace(p_sInStr.begin(), p_sInStr.end(), ' ', '_');
	// replace uppercase with lower case
	std::transform(p_sInStr.begin(), p_sInStr.end(), p_sInStr.begin(), [](char c){ return std::tolower(c);});
	return p_sInStr;
}


namespace Lexer {
	// Checks if character is usable in a nonterminal.
	bool IsIdentChar(const char& c) {
		if (isalnum(c)) return true;
		else return false;
	}
	// Convert string to token stream. Returns true if successful and false if an error occurs.
	bool Tokenize(const std::string_view& p_sInStr) {
		bool bErrorsFound=false;
		std::string t_sStrTemp;
		for (auto str_it = p_sInStr.begin(); str_it < p_sInStr.end(); )
		{
			if (TokenContainer.size()>0) TokenContainer.back().sValue.shrink_to_fit();
			// Nonterminals / Identifiers
			if (std::isalnum(*str_it) || *str_it == '_') {
				auto last_identifier_char_it = std::find_if(str_it, p_sInStr.cend(), [](unsigned char c){return !(std::isalnum(c) || c == '_');});
				// Keep the line column at the initial character.
				TokenContainer.push_back(Token(iLineNum, iLineColumn, TokenType::IDENTIFIER, std::string(str_it, last_identifier_char_it)));
				iLineColumn += std::distance(str_it, last_identifier_char_it);
				// Check for terminals.
				if (TokenContainer.back().sValue=="TOGGLE") TokenContainer.back().Type=TokenType::TOGGLE;
				else if (TokenContainer.back().sValue=="BIND") TokenContainer.back().Type=TokenType::BIND;
				else if (TokenContainer.back().sValue=="NOEXIT") TokenContainer.back().Type=TokenType::NOEXIT;
				else if (TokenContainer.back().sValue=="NOFORMAT") TokenContainer.back().Type=TokenType::NOFORMAT;
				str_it = last_identifier_char_it;
				continue;
			}
			switch (*(str_it))
			{
				// comments
				case '/':
					// Line comments
					if (*(str_it + 1) == '/') {
						// Trying to capture the iterator will just implicitly convert it. WHY?
						unsigned long long lambda_char_pos = p_sInStr.length();
						std::for_each(str_it, std::find(str_it, p_sInStr.end(), '\n'), [lambda_char_pos, iLineColumn](char c) mutable -> void {
							switch (c) {
								lambda_char_pos++;
								case '\t': {iLineColumn += 4 - (lambda_char_pos % 4); break;}
								case '\n': break;
								default: iLineColumn++;
							}
						});
						iLineNum++;
						iLineColumn = 0u;
						str_it = std::find(str_it, p_sInStr.end(), '\n') + 1;
					}
					/* Block comments */
					else if (*(str_it + 1) == '*') {
						const std::string end_seq_ref = "*/";
						auto end_seq_it = std::search(str_it, p_sInStr.cend(), end_seq_ref.cbegin(), end_seq_ref.cend());
						if (end_seq_it >= p_sInStr.cend()) {
							ErrorTokens.push_back(Token(iLineNum,iLineColumn,TokenType::COMPILER_ERROR,"error: Matching '*/' sequence not found."));
							str_it = end_seq_it;
						}
						else {
							// Trying to capture the iterator will just implicitly convert it. WHY?
							unsigned long long lambda_char_pos = p_sInStr.length();
							std::for_each(str_it, end_seq_it, [lambda_char_pos, iLineColumn](char c) mutable -> void {
								switch (c) {
									lambda_char_pos++;
									case '\t': {iLineColumn += 4 - (lambda_char_pos % 4); break;}
									case '\n': {iLineColumn = 1; iLineNum++; break;}
									default: iLineColumn++;
								}
							});
							iLineNum += std::count(str_it, end_seq_it, '\n');
							str_it = end_seq_it + 2;
						}
					}
					else {
						TokenContainer.push_back(Token(iLineNum,iLineColumn,TokenType::UNDEFINED,"/"));
						iLineColumn++;
					}
					break;
				// strings
				case '`':
				case '\"':
					{
						std::string_view::iterator end_quote_it = std::find(str_it + 1, p_sInStr.end(), *str_it);
						// New lines or carriage returns cannot be in strings. (As in ASCII values 10 and 13, not the C-style escapes though those arent interpreted.)
						if (end_quote_it >= p_sInStr.end()
							|| std::any_of(str_it, end_quote_it, [](char c){return c == '\n' || c == '\r'; })) {
							ErrorTokens.push_back(Token(iLineNum,iLineColumn,TokenType::COMPILER_ERROR,"error: String not properly closed with "));
							ErrorTokens.back().sValue += *str_it + '.';
							str_it = p_sInStr.end();
							iLineColumn++;
						}
						else {
							// Line column and number for strings should be located at the beginning '"'.
							TokenContainer.push_back(Token(iLineNum, iLineColumn, (*str_it == '\"' ? TokenType::STRING : TokenType::RAW_STRING), std::string(str_it + 1, end_quote_it)));
							unsigned long long lambda_char_pos = p_sInStr.length();
							std::for_each(str_it, end_quote_it, [lambda_char_pos, iLineColumn](char c) mutable -> void {
								lambda_char_pos++;
								if (c == '\t') iLineColumn += 4 - (lambda_char_pos % 4);
								else if (!std::iscntrl(c)) iLineColumn++;
							});
							str_it = end_quote_it + 1;
						}
					}
					break;
				//terminals
				case '=':
					TokenContainer.push_back(Token(iLineNum,iLineColumn,TokenType::EQUALS,"="));
					str_it++;
					iLineColumn++;
					break;
				case '|':
					TokenContainer.push_back(Token(iLineNum,iLineColumn,TokenType::VBAR,"|"));
					str_it++;
					iLineColumn++;
					break;
				case '{':
					TokenContainer.push_back(Token(iLineNum,iLineColumn,TokenType::LCBRACKET,"{"));
					str_it++;
					iLineColumn++;
					break;
				case '}':
					TokenContainer.push_back(Token(iLineNum,iLineColumn,TokenType::RCBRACKET,"}"));
					str_it++;
					iLineColumn++;
					break;
				//spaces and colon
				case '\t':
					iLineColumn += 4 - (iLineColumn % 4);
					str_it++;
					break;
				case ' ':
					iLineColumn++;
					str_it++;
					break;
				case '\r':
				case '\v':
					str_it++;
					break;
				case '\n':
					iLineNum++;
					iLineColumn = 1u;
					str_it++;
					break;
				default:
					TokenContainer.push_back(Token(iLineNum,iLineColumn,TokenType::UNDEFINED,std::string(*str_it, 0)));
					iLineColumn++;
					str_it++;
					break;
			}
		}
		TokenContainer.push_back(Token(iLineNum,p_sInStr.length()-1,TokenType::END_OF_FILE,""));
		TokenContainer.back().sValue.shrink_to_fit();
		return !bErrorsFound;
	}
}