#include "Tokens.hpp"
#include "compiler.hpp"
#include <deque>
#include <iostream>
#include <map>

Token::Token() {}

Token::Token(const std::size_t& p_iLineNum, const std::size_t& p_iColumn, const unsigned char p_TokenType, const std::string& p_sVal)
: iLineNum(p_iLineNum), iLineColumn(p_iColumn), Type(p_TokenType), sValue(p_sVal) {

}
Token::~Token() {}