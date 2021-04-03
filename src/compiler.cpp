#include "Tokens.hpp"
#include "binds.hpp"
#include "commandmenu.hpp"
#include "compiler.hpp"
#include <iostream>
#include <string>
#include <deque>
#include <map>
#include <stack>
std::size_t iLineNum=1u;
std::size_t iLineColumn=1u;
extern std::map<std::string,std::string> KVMap;
#define Error(error) ErrorTokens.push_back(Token(0u,0u,TokenType::COMPILER_ERROR,error))
std::deque<Token> TokenContainer;
std::deque<Token> ErrorTokens;
std::deque<Parser::MenuToken*> CMenuTokens;
extern std::deque<CommandMenu> CMenuContainer; // Made in main.cpp

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
		if (i<p_sInStr.length()-1 && (!isascii(p_sInStr.at(i)) || !isascii(p_sInStr.at(i+1)))) {
			p_sInStr.erase(i,2);
			i--; 
			continue;
		}
		if (isspace(p_sInStr.at(i)) && p_sInStr.at(i)!='\0') p_sInStr.at(i)='_';
		if (isupper(p_sInStr.at(i))) p_sInStr.at(i)=tolower(p_sInStr.at(i)); 
	}
	return p_sInStr;
}
// Checks if character is usable in a nonterminal.
bool IsIdentChar(const char& c) {
	if (isalnum(c)
	|| c=='#' 
	|| c=='.'
	|| c=='_') return true;
	else return false;
}
// Convert string to token stream
bool Tokenize(const std::string& p_sInStr) {
	bool bErrorsFound=false;
	std::string t_sStrTemp;
	for (std::size_t i=0; i < p_sInStr.length(); )
	{
		if (TokenContainer.size()>0) TokenContainer.back().sValue.shrink_to_fit();
		// EOF Token
		if (i==p_sInStr.length()-1) {
			TokenContainer.push_back(Token(p_sInStr.length()-1,iLineNum,TokenType::END_OF_FILE,""));
			TokenContainer.back().sValue.shrink_to_fit();
			break;
		}
		// Nonterminal
		if (IsIdentChar(p_sInStr.at(i))) {
			TokenContainer.push_back(Token(iLineColumn,iLineNum,TokenType::IDENTIFIER,""));
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
			continue;
		}
		switch (p_sInStr.at(i))
		{
		// comments
		case '/':
			// Line comments
			if (p_sInStr.at(i+1)=='/') {
				for (auto t=p_sInStr.begin()+i; t!=p_sInStr.end(); t++, i++) {
					if (*t=='\t') iLineColumn+=5-(iLineColumn%4==0 ? 4 : iLineColumn%4);
					else iLineColumn++;
					if (*t=='\n' || *t=='\r') break;
				}
			}
			/* Block comments */
			else if (p_sInStr.at(i+1)=='*') { //
				std::size_t tempi=i, templinecolumn=iLineColumn, templinenumber=iLineNum;
				for (auto t=p_sInStr.begin()+i; t!=p_sInStr.end(); t++, tempi++) {
					if (*t=='\t') templinecolumn+=5-(iLineColumn%4==0 ? 4 : iLineColumn%4);
					else templinecolumn++;
					if (*t=='\n') {
						if (*(t-1)!='\r') templinecolumn=1u;
						templinenumber++;
					}
					else if (*t=='\r') {
						templinecolumn=1u;
						if (*(t+1)!='\n') templinenumber++;
					}
					if (*t=='*' && *(t+1)=='/') {
						i=tempi+2;
						iLineColumn=templinecolumn+2;
						iLineNum=templinenumber+2;
						break;
					}
					if (t==p_sInStr.end()-1) {
						Error("error: Unclosed comment. ("+std::to_string(iLineNum)+':'+std::to_string(iLineColumn)+')');
						i=tempi;
						break;
					}
				}
			}
			else {
				TokenContainer.push_back(Token(iLineColumn,iLineNum,TokenType::UNDEFINED,"/"));
				iLineColumn++;
				i++;
			}
			break;
		// strings
		case '\"':
			{
			std::size_t tempcol=iLineColumn,templn=iLineNum;
			// New lines or carriage returns cannot be in strings. (I don't mean the '\r' or '\n' character.)
			for (i++, iLineColumn++; i < p_sInStr.length(); i++, iLineColumn++) {
				if (p_sInStr.at(i)=='\r') {
					Error("error: Missing a quote at (");
					ErrorTokens.back().iLineNum=iLineNum;
					ErrorTokens.back().iLineColumn=iLineColumn;
					ErrorTokens.back().sValue+=ErrorTokens.back().GetFileLoc()+")";
					// 
					TokenContainer.push_back(Token(iLineColumn,iLineNum,TokenType::STRING,""));
					t_sStrTemp="";
					iLineColumn=1u;
					if (p_sInStr.at(i+1)!='\n') {
						iLineNum++;
						i++;
					}
					break;
				}
				else if (p_sInStr.at(i)=='\n') {
					Error("error: Missing a quote at (");
					ErrorTokens.back().iLineNum=iLineNum;
					ErrorTokens.back().iLineColumn=iLineColumn;
					ErrorTokens.back().sValue+=ErrorTokens.back().GetFileLoc()+")";
					// 
					TokenContainer.push_back(Token(iLineColumn,iLineNum,TokenType::STRING,""));
					t_sStrTemp="";
					iLineNum++;
					if (p_sInStr.at(i-1)!='\r') {
						iLineColumn=1u;
						i++;
					}
					break;
				}
				else if (i==p_sInStr.length()-1){
					Error("error: Missing a quote at (");
					ErrorTokens.back().sValue+=ErrorTokens.back().GetFileLoc()+")";
					t_sStrTemp="";
					break;
				}
				else if (p_sInStr.at(i)=='\"') {
					TokenContainer.push_back(Token(tempcol,templn,TokenType::STRING,t_sStrTemp));
					t_sStrTemp="";
					break;
				}
				else {
					if (p_sInStr.at(i)=='\t') iLineColumn+=5-(iLineColumn%4==0 ? 4 : iLineColumn%4);
					t_sStrTemp+=p_sInStr.at(i);
				}
			}
			i++; // Starts at the ending quote if this doesn't exist.
			iLineColumn++;
			}
			break;
		//terminals
		case '=':
			TokenContainer.push_back(Token(iLineColumn,iLineNum,TokenType::EQUALS,"="));
			i++;
			iLineColumn++;
			break;
		case '|':
			TokenContainer.push_back(Token(iLineColumn,iLineNum,TokenType::VBAR,"|"));
			i++;
			iLineColumn++;
			break;
		case '{':
			TokenContainer.push_back(Token(iLineColumn,iLineNum,TokenType::LCBRACKET,"{"));
			i++;
			iLineColumn++;
			break;
		case '}':
			TokenContainer.push_back(Token(iLineColumn,iLineNum,TokenType::RCBRACKET,"}"));
			i++;
			iLineColumn++;
			break;
		//spaces and colon
		case '\t':
			iLineColumn+=5-(iLineColumn%4==0 ? 4 : iLineColumn%4);
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
			TokenContainer.push_back(Token(iLineColumn,iLineNum,TokenType::UNDEFINED,""));
			TokenContainer.back().sValue.push_back(p_sInStr.at(i));
			iLineColumn++;
			i++;
			break;
		}
	}
	if (ErrorTokens.size()>=1) bErrorsFound=true;
	return !bErrorsFound;
}
namespace Parser {
	unsigned int depth=0u;
	bool bEOFFound=false, bErrorsFound=false;
	bool ParseTokens() {
		bool bNoExit=false, bFormatted=true;
		for (auto token=TokenContainer.begin(); token!=TokenContainer.end(); ) {
			switch (token->Type) {
				case TokenType::NOEXIT:
					if (bNoExit==true) Error("error: Duplicate modifier \"NOEXIT\". ("+token->GetFileLoc()+')');
					else bNoExit=true;
					token++;
					break;
				case TokenType::NOFORMAT:
					if (bFormatted==false) Error("error: Duplicate modifier \"NOFORMAT\". ("+token->GetFileLoc()+')');
					else bFormatted=false;
					token++;
					break;
				case TokenType::TOGGLE: 
					{ // Check for toggle bind.
						std::string namelist[MAX_TOGGLE_STATES];
						std::string cmdlist[MAX_TOGGLE_STATES];
						unsigned short i=1;
						if (depth<=0) 
							Error("error: Toggle bind must be set in a command menu. ("+token->GetFileLoc()+')');
						if ((token+i)->Type!=TokenType::BIND) 
							Error("Expected \'BIND\' ("+(token+i)->GetFileLoc()+")");
						else i++;
						while ((token+i)->Type==TokenType::STRING)
						{
							if (i % 2 == 0) namelist[(i-2)/2]=(token+i)->sValue;
							else if (i % 2 == 1) cmdlist[(i-2)/2]=(token+i)->sValue;
							i++;
						}
						if (i-1<=1 || i % 2!=0)  // Even amount of strings indicate that the toggle bind has names and cmdstrs
							Error("error: Expected string! ("+(token+i)->GetFileLoc()+")");
						if ((token+i)->Type!=TokenType::VBAR) 
							Error("error: Expected '|' ("+(token+i-1)->GetFileLoc()+")");
						CMenuTokens.push_back(new Parser::ToggleBindToken(namelist,cmdlist,static_cast<unsigned short>((i-2)/2),bNoExit,bFormatted));
						if (bNoExit==true) bNoExit=false;
						token+=i;
					}
					break;
				case TokenType::BIND: // Check for bind.
				{
					unsigned short i=1u;
					if (depth<=0) 
						Error("error: Bind must be set in a commandmenu. ("+token->GetFileLoc()+')');
					if ((token+i)->Type!=TokenType::STRING) 
						Error("error: Expected string. ("+(token+i)->GetFileLoc()+")");
					else {
						i++;
						if ((token+i)->Type!=TokenType::STRING) 
							Error("error: Expected string. ("+(token+i)->GetFileLoc()+")");
						else i++;
					}
					if ((token+i)->Type!=TokenType::VBAR) 
						Error("error: Expected '|' ("+(token+i)->GetFileLoc()+")");
					else i++;
					CMenuTokens.push_back(new Parser::BindToken((token+1)->sValue, (token+2)->sValue,bNoExit,bFormatted));
					bNoExit=false, bFormatted=true;
					token+=i;
				}
					break;
				case TokenType::STRING: // Check for new commandmenu.
				{
					unsigned short i=1u;
					if ((token+i)->Type!=TokenType::LCBRACKET) 
						Error("error: Expected '{' ("+(token+i)->GetFileLoc()+")");
					else if (bNoExit==true) {
						Error("error: Expected a bind. ("+token->GetFileLoc()+')');
						bNoExit=false;
					}
					CMenuTokens.push_back(new Parser::CMenuToken(token->sValue,bFormatted));
					bFormatted=true;
					token+=i;
					depth++;
				}
				break;
				case TokenType::IDENTIFIER: // Check for set keymaps
				{
					unsigned short i=1u;
					// Maybe they were trying to make a commandmenu…
					if ((token+i)->Type==TokenType::LCBRACKET) {
						token++;
						break;
					};
					
					if ((token+i)->Type!=TokenType::EQUALS) {
						Error("error: Expected '='! ("+(token+i)->GetFileLoc()+")");
						TokenContainer.insert(token+i,Token((token+i)->iLineColumn,(token+i)->iLineNum,TokenType::EQUALS,"="));
					}
					if ((token+i+1)->Type!=TokenType::LCBRACKET) i++;
					if ((token+i)->Type!=TokenType::STRING || (token+i+1)->Type==TokenType::LCBRACKET) {
						Error("error: Expected string! ("+(token+i)->GetFileLoc()+")");
						TokenContainer.insert(token+i,Token((token+i)->iLineColumn,(token+i)->iLineNum,TokenType::STRING,""));
					}
					if ((token+i+1)->Type!=TokenType::LCBRACKET) i++;
					CMenuTokens.push_back(new Parser::KVToken(token->sValue,(token+2)->sValue));
					token+=i;
				}
				break;
				case TokenType::LCBRACKET:
					depth++;
					if (token>TokenContainer.begin() && (token-1)->Type!=TokenType::STRING) {
						Error("error: Expected string! ("+(token-1)->GetFileLoc()+")");
						depth--;
					}
					token++;
				break;
				case TokenType::RCBRACKET:
					depth--;
					if (depth==UINT32_MAX) {
						Error("error: Stray '}' ("+token->GetFileLoc()+")");
						depth++;
					}
					CMenuTokens.push_back(new Parser::PageEndToken());
					token++;
					break;
				case TokenType::END_OF_FILE:
					bEOFFound=true;
					token=TokenContainer.end();
					break;
				case TokenType::UNDEFINED: 
					Error("error: Unrecognized token '"+token->sValue+"' ("+token->GetFileLoc()+")");
					token++;
					break;
				default:
					token++;
					break;
		}
		}
		if (bEOFFound==false) std::cout<<"warning: EOF not found!\n";
		if (ErrorTokens.size()>=1) bErrorsFound=true;
		return !bErrorsFound;
	}
}

// Convert menu TokenContainer into something useful.
void ParseMenuTokens(unsigned short& p_iBindCount) {
	std::deque<CommandMenu> CMenuStack;
	std::stack<unsigned char> NumKeyStack;
	// Variable is here to reduce the amount of goddamn downcasts I would have to do.
	Parser::CMenuToken CurrentCMenu;
	for (auto token = CMenuTokens.begin(); token != CMenuTokens.end(); token++) {
		switch ((*token)->Type)
		{
		case Parser::CMenuTokenType::KV_SET:
			{
				Parser::KVToken temp=static_cast<Parser::KVToken&>(**token);
				KVMap.insert_or_assign(temp.Key,temp.Value);
			}
			break;
		case Parser::CMenuTokenType::DECLARE_CMENU:
			{
				//For binds to CMenuContainer.
				CurrentCMenu=static_cast<Parser::CMenuToken&>(**token);
				std::size_t iDuplicateNumber=0llu;
				// Form duplicates if formatted name is already taken.
				if (CMenuStack.size()>0) for (auto p=CMenuStack.end(); p!=CMenuStack.begin();)
				{
					p--; // This has to be here instead of in the for declaration, otherwise if there is one commandmenu in the stack, it would not be checked for duplication.
					if (formatRaw(p->sName)==formatRaw(CurrentCMenu.sName)) iDuplicateNumber++;
				}
				for (auto& p : CMenuContainer)
				{
					if (formatRaw(p.sName)==formatRaw(CurrentCMenu.sName)) iDuplicateNumber++;
				}
				CMenuStack.push_front(CommandMenu(CurrentCMenu.sName));
				if (iDuplicateNumber>0) CMenuStack.front().sRawName+='_'+std::to_string(iDuplicateNumber);
				if (CMenuStack.size()>1) {
					if (iDuplicateNumber>0) (CMenuStack.begin()+1)->binds.push_back(Bind(NumKeyStack.top(),Parser::BindToken(CurrentCMenu.sName,"exec $cmenu_"+formatRaw(CurrentCMenu.sName)+'_'+std::to_string(iDuplicateNumber),true,CurrentCMenu.bFormatted)));
					else (CMenuStack.begin()+1)->binds.push_back(Bind(NumKeyStack.top(),Parser::BindToken(CurrentCMenu.sName,"exec $cmenu_"+formatRaw(CurrentCMenu.sName),true,CurrentCMenu.bFormatted)));
				}
				NumKeyStack.push(1u);
			}
			break;
		case Parser::CMenuTokenType::END_CMENU:
			// Warning:
			if (CMenuStack.front().binds.size()>10) std::cout<<"Warning: More than ten binds in CMenu \'"<<CMenuStack.front().sName<<"\'!\n";
			CMenuContainer.push_back(CMenuStack.front());
			CMenuStack.pop_front();
			NumKeyStack.pop();
			if (!NumKeyStack.empty()) {
				NumKeyStack.top()=(NumKeyStack.top()+1)%10;
			}
			break;
		case Parser::CMenuTokenType::MENU_BIND:
			CMenuStack.front().binds.push_back(Bind(NumKeyStack.top(),static_cast<Parser::BindToken&>(**token)));
			NumKeyStack.top()=(NumKeyStack.top()+1)%10;
			p_iBindCount++;
			break;
		case Parser::CMenuTokenType::MENU_TOGGLE_BIND:
			CMenuStack.front().binds.push_back(Bind(NumKeyStack.top(),static_cast<Parser::ToggleBindToken&>(**token)));
			NumKeyStack.top()=(NumKeyStack.top()+1)%10;
			p_iBindCount++;
			break;
		default:
			break;
		}
	}
}