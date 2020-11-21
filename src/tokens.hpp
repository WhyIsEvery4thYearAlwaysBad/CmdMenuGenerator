#ifndef TOKENS_HPP
#define TOKENS_HPP
#include <string>
enum TokenType {
	IDENTIFIER=0,
	STRING,
	EQUALS, // =
	LCBRACKET, // {
	RCBRACKET, // }
	VBAR, // |
	BIND, // BIND
	TOGGLE, // TOGGLE
	FILEEND, // the end!
	ERR, // ah crepe!
	UNDEFINED
};
class Token {
	public:
	unsigned char type=TokenType::UNDEFINED;
	std::size_t location=1u; // Location
	std::size_t linenumber=1u; // Location from newline
	std::string val;
	
	Token();
	Token(const std::size_t& loc, const std::size_t& ln, const unsigned char t, const std::string& v);
	~Token();
	
	std::string inline GetFileLoc() {
		return std::to_string(linenumber) + ":" + std::to_string(location);
	}
};
#endif