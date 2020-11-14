#include "tokens.hpp"
#include "compiler.hpp"
#include <deque>
#include <iostream>
#include <map>
#define SPACE (' ' || '\r' || '\t' || '\v' || '\n')
#define GetPrevChar(str,i,c) str.rfind(c,i-1)
#define GetNextChar(str,i,c) str.find(c,i+1)
extern std::map<std::string,std::string> keymap;
extern std::deque<Token> tokens;