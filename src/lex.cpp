#include <string>
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
	for (unsigned long long i=0; i < p_sInStr.length(); i++) {
		if (p_sInStr.at(i)=='<' && p_sInStr.find('>',i)!=std::string::npos) {
			p_sInStr.erase(i,(p_sInStr.find('>',i+1)-i)+1);
			if (i>0) i--;
		}
		// remove punctuation
		if (ispunct(p_sInStr.at(i))) {
			p_sInStr.erase(i,1);
			i--;
			continue;
		}
		// and non-ascii characters
		if (i<p_sInStr.length()-1 && (!isascii(p_sInStr.at(i)) || !isascii(p_sInStr.at(i+1))) && (p_sInStr.at(i) != '\0' && p_sInStr.at(i+1) != '\0')) {
			p_sInStr.erase(i,2);
			i--; 
			continue;
		}
		if (isspace(p_sInStr.at(i)) && p_sInStr.at(i)!='\0') p_sInStr.at(i)='_';
		if (isupper(p_sInStr.at(i))) p_sInStr.at(i)=tolower(p_sInStr.at(i)); 
	}
	return p_sInStr;
}


namespace Lexer {
	// Checks if character is usable in a nonterminal.
	bool IsIdentChar(const char& c) {
		if (isalnum(c)
		|| c=='#' 
		|| c=='.'
		|| c=='_') return true;
		else return false;
	}
	// Convert string to token stream. Returns true if successful and false if an error occurs.
	bool Tokenize(const std::string& p_sInStr) {
		bool bErrorsFound=false, bInBlockComment=false;
		std::string t_sStrTemp;
		for (std::size_t i=0; i < p_sInStr.length(); )
		{
			if (bInBlockComment) {
				if (p_sInStr.find("*/",i)==std::string::npos) {
					ErrorTokens.push_back(Token(iLineNum,iLineColumn,TokenType::COMPILER_ERROR,std::to_string(iLineNum)+':'+std::to_string(iLineColumn)+": error: Block quote has no ending sequence ('*/')."));
					bErrorsFound=true;
					i=p_sInStr.length();
					break;
				}
				else if (p_sInStr.at(i)=='*' && p_sInStr.at(i+1)=='/') {
					bInBlockComment=false;	
					i+=2;
					iLineColumn+=2;
					continue;
				}
				else switch (p_sInStr.at(i)) {
					//spaces and colon
					case '\t':
						iLineColumn += 4 - (iLineColumn % 4);
						i++;
					break;
					case ' ':
						iLineColumn++;
						i++;
					break;
					case '\r':
						iLineColumn=1u;
						if ((i+1)<p_sInStr.length() && p_sInStr.at(i+1)!='\n') iLineNum++;
						i++;
					break;
					case '\v':
						i++;
						iLineNum++;
					break;
					case '\n':
						iLineNum++;
						if (i>0 && p_sInStr.at(i-1)!='\r') iLineColumn=1u;
						i++;
					break;
					default:
						i++;
						iLineColumn++;
					break;
				}
			}
			else {
				if (TokenContainer.size()>0) TokenContainer.back().sValue.shrink_to_fit();
				// EOF Token
				if (i == p_sInStr.length() - 1) {
					TokenContainer.push_back(Token(iLineNum,p_sInStr.length()-1,TokenType::END_OF_FILE,""));
					TokenContainer.back().sValue.shrink_to_fit();
					break;
				}
				// Nonterminal
				if (IsIdentChar(p_sInStr.at(i))) {
					TokenContainer.push_back(Token(iLineNum,iLineColumn,TokenType::IDENTIFIER,""));
					while (IsIdentChar(p_sInStr.at(i)))
					{
						TokenContainer.back().sValue.push_back(p_sInStr.at(i));
						i++;
						iLineColumn++;
					}
					// Check for terminals.
					if (TokenContainer.back().sValue=="TOGGLE") TokenContainer.back().Type=TokenType::TOGGLE;
					else if (TokenContainer.back().sValue=="BIND") TokenContainer.back().Type=TokenType::BIND;
					else if (TokenContainer.back().sValue=="NOEXIT") TokenContainer.back().Type=TokenType::NOEXIT;
					else if (TokenContainer.back().sValue=="NOFORMAT") TokenContainer.back().Type=TokenType::NOFORMAT;
					else if (TokenContainer.back().sValue=="KEY") TokenContainer.back().Type=TokenType::KEY;
					continue;
				}
				switch (p_sInStr.at(i))
				{
				// comments
				case '/':
					// Line comments
					if (p_sInStr.at(i+1)=='/') {
						for (auto t=p_sInStr.begin()+i; t!=p_sInStr.end(); t++, i++) {
							if (*t=='\t') iLineColumn += 4 - (iLineColumn % 4);
							else iLineColumn++;
							if (*t=='\n' || *t=='\r') break;
						}
					}
					/* Block comments */
					else if (p_sInStr.at(i+1)=='*') { //
						bInBlockComment=true;
						i+=2;
						iLineColumn+=2;
					}
					else {
						TokenContainer.push_back(Token(iLineNum,iLineColumn,TokenType::UNDEFINED,"/"));
						iLineColumn++;
						i++;
					}
					break;
				// strings
				case '\"':
					{
					// New lines or carriage returns cannot be in strings. (I don't mean the '\r' or '\n' character.)
					for (i++, iLineColumn++; i < p_sInStr.length(); i++, iLineColumn++) {
						if (p_sInStr.at(i)=='\r') {
							ErrorTokens.push_back(Token(iLineNum,iLineColumn,TokenType::COMPILER_ERROR,std::to_string(iLineNum)+':'+std::to_string(iLineColumn)+": error: Ending '\"' is missing."));
							bErrorsFound=true;
							TokenContainer.push_back(Token(iLineNum,iLineColumn,TokenType::STRING,""));
							t_sStrTemp="";
							iLineColumn=1u;
							if (p_sInStr.at(i+1)!='\n') {
								iLineNum++;
								i++;
							}
							break;
						}
						else if (p_sInStr.at(i)=='\n') {
							ErrorTokens.push_back(Token(iLineNum,iLineColumn,TokenType::COMPILER_ERROR,std::to_string(iLineNum)+':'+std::to_string(iLineColumn)+": error: Ending '\"' is missing."));
							bErrorsFound=true;
							TokenContainer.push_back(Token(iLineNum,iLineColumn,TokenType::STRING,""));
							t_sStrTemp="";
							iLineNum++;
							if (p_sInStr.at(i-1)!='\r') {
								iLineColumn=1u;
								i++;
							}
							break;
						}
						else if (i==p_sInStr.length()-1){
							ErrorTokens.push_back(Token(iLineNum,iLineColumn,TokenType::COMPILER_ERROR,std::to_string(iLineNum)+':'+std::to_string(iLineColumn)+": error: Ending '\"' is missing."));
							bErrorsFound=true;
							t_sStrTemp="";
							break;
						}
						else if (p_sInStr.at(i)=='\"') {
							TokenContainer.push_back(Token(iLineNum,iLineColumn,TokenType::STRING,t_sStrTemp));
							t_sStrTemp="";
							break;
						}
						else {
							if (p_sInStr.at(i)=='\t') iLineColumn += 4 - (iLineColumn % 4);
							t_sStrTemp+=p_sInStr.at(i);
						}
					}
					i++; // Starts at the ending quote if this doesn't exist.
					iLineColumn++;
					}
					break;
				//terminals
				case '=':
					TokenContainer.push_back(Token(iLineNum,iLineColumn,TokenType::EQUALS,"="));
					i++;
					iLineColumn++;
					break;
				case '|':
					TokenContainer.push_back(Token(iLineNum,iLineColumn,TokenType::VBAR,"|"));
					i++;
					iLineColumn++;
					break;
				case '{':
					TokenContainer.push_back(Token(iLineNum,iLineColumn,TokenType::LCBRACKET,"{"));
					i++;
					iLineColumn++;
					break;
				case '}':
					TokenContainer.push_back(Token(iLineNum,iLineColumn,TokenType::RCBRACKET,"}"));
					i++;
					iLineColumn++;
					break;
				//spaces and colon
				case '\t':
					iLineColumn += 4 - (iLineColumn % 4);
					i++;
					break;
				case ' ':
					iLineColumn++;
					i++;
					break;
				case '\r':
					iLineColumn=1u;
					if (i+1==p_sInStr.length() || p_sInStr.at(i+1)!='\n') iLineNum++;
					i++;
					break;
				case '\v':
					i++;
					iLineNum++;
					break;
				case '\n':
					iLineNum++;
					if (i>0 && p_sInStr.at(i-1)!='\r') iLineColumn=1u;
					i++;
					break;
				default:
					ErrorTokens.push_back(Token(iLineNum,iLineColumn,TokenType::UNDEFINED,""));
					ErrorTokens.back().sValue.push_back(p_sInStr.at(i));
					iLineColumn++;
					i++;				
					break;
				}
			}
		}
		return !bErrorsFound;
	}
}