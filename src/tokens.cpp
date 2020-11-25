#include "tokens.hpp"
#include "compiler.hpp"
#include <deque>
#include <iostream>
#include <map>
#define GetPrevChar(str,i,c) str.rfind(c,i-1)
#define GetNextChar(str,i,c) str.find(c,i+1)
extern std::map<std::string,std::string> keymap;
extern std::deque<Token> tokens;

Token::Token() {}
Token::Token(const std::size_t& loc, const std::size_t& ln, const unsigned char t, const std::string& v)
: location(loc), linenumber(ln), type(t), val(v) {

}
Token::~Token() {}