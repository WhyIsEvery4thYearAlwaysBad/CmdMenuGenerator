#ifndef TOKENS_HPP
#define TOKENS_HPP
#include <string>
enum TokenType {
	IDENTIFIER = 0, // asd
	STRING, // " "
	RAW_STRING, // ` `
	EQUALS, // =
	LCBRACKET, // {
	RCBRACKET, // }
	VBAR, // |
	BIND, // BIND
	TOGGLE, // TOGGLE
	NOEXIT, // NOEXIT
	NOFORMAT, // NOFORMAT
	END_OF_FILE, // the end!
	COMPILER_ERROR, // ah crepe!
	UNDEFINED
};
class Token {
	public:
	unsigned char Type=TokenType::UNDEFINED;
	std::size_t iLineColumn=1u; // Location
	std::size_t iLineNum=1u; // Location from newline
	std::string sValue="";
	
	Token();
	Token(const std::size_t& p_iLineNum, const std::size_t& p_iColumn, const unsigned char p_TokenType, const std::string& p_sVal);
	~Token();
	
	std::string inline GetFileLoc() {
		return std::to_string(iLineNum) + ":" + std::to_string(iLineColumn);
	}
};
#endif