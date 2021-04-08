#include <iostream>
#include <string>
#include <deque>
#include <map>
#include <stack>
#include "compiler.hpp"
#include "lex.hpp"
#include "token.hpp"
#include "bind.hpp"
#include "commandmenu.hpp"

extern std::map<std::string,std::string> KVMap;
std::deque<Token> TokenContainer;
std::deque<Token> ErrorTokens;
std::deque<Parser::MenuToken*> CMenuTokens;
extern std::deque<CommandMenu> CMenuContainer; // Made in main.cpp


namespace Parser {
	unsigned int depth=0u;
	bool bEOFFound=false, bErrorsFound=false;
	bool ParseTokens() {
		bool bNoExit=false, bFormatted=true;
		for (auto token=TokenContainer.begin(); token!=TokenContainer.end(); ) {
			switch (token->Type) {
				case TokenType::NOEXIT:
					if (bNoExit==true) TokenContainer.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Duplicate modifier \"NOEXIT\"."));
					else bNoExit=true;
					token++;
					break;
				case TokenType::NOFORMAT:
					if (bFormatted==false) TokenContainer.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Duplicate modifier \"NOFORMAT\"."));
					else bFormatted=false;
					token++;
					break;
				case TokenType::TOGGLE: 
					{ // Check for toggle bind.
						std::string namelist[MAX_TOGGLE_STATES];
						std::string cmdlist[MAX_TOGGLE_STATES];
						unsigned short i=1;
						if (depth<=0) 
							TokenContainer.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Toggle bind must be set in a command menu."));
						if ((token+i)->Type!=TokenType::BIND) 
							TokenContainer.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Expected \'BIND\' keyword here."));
						else i++;
						while ((token+i)->Type==TokenType::STRING)
						{
							if (i % 2 == 0) namelist[(i-2)/2]=(token+i)->sValue;
							else if (i % 2 == 1) cmdlist[(i-2)/2]=(token+i)->sValue;
							i++;
						}
						if (i-1<=1 || i % 2!=0)  // Even amount of strings indicate that the toggle bind has names and cmdstrs
							TokenContainer.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Expected string here."));
						if ((token+i)->Type!=TokenType::VBAR) 
							TokenContainer.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,(token+i)->GetFileLoc()+"error: Expected '|'."));
						CMenuTokens.push_back(new Parser::ToggleBindToken(namelist,cmdlist,static_cast<unsigned short>((i-2)/2),bNoExit,bFormatted));
						if (bNoExit==true) bNoExit=false;
						token+=i;
					}
					break;
				case TokenType::BIND: // Check for bind.
				{
					unsigned short i=1u;
					if (depth<=0) 
						TokenContainer.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Bind must be set in a command menu."));
					if ((token+i)->Type!=TokenType::STRING) 
						TokenContainer.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,(token+i)->GetFileLoc()+": error: Expected string."));
					else {
						i++;
						if ((token+i)->Type!=TokenType::STRING) 
							TokenContainer.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,(token+i)->GetFileLoc()+": error: Expected string."));
						else i++;
					}
					if ((token+i)->Type!=TokenType::VBAR) 
						TokenContainer.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,(token+i)->GetFileLoc()+": error: Expected '|'."));
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
						TokenContainer.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,(token+i)->GetFileLoc()+": error: Expected '{' here."));
					else if (bNoExit==true) {
						TokenContainer.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Expected a bind."));
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
					if ((token+i)->Type!=TokenType::EQUALS) {
						TokenContainer.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,(token+i)->GetFileLoc()+": error: Expected a '=' here."));
						TokenContainer.insert(token+i,Token((token+i)->iLineColumn,(token+i)->iLineNum,TokenType::EQUALS,"="));
					}
					if ((token+i+1)->Type!=TokenType::LCBRACKET) i++;
					if ((token+i)->Type!=TokenType::STRING || (token+i+1)->Type==TokenType::LCBRACKET) {
						TokenContainer.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,(token+i)->GetFileLoc()+": error: Expected a string here."));
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
						TokenContainer.push_back(Token((token-1)->iLineNum,(token-1)->iLineColumn,TokenType::COMPILER_ERROR,(token-1)->GetFileLoc()+": error: Expected a string here."));
						depth--;
					}
					token++;
				break;
				case TokenType::RCBRACKET:
					depth--;
					if (depth==UINT32_MAX) {
						TokenContainer.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Stray '}'"));
						depth++;
					}
					CMenuTokens.push_back(new Parser::CMenuEndToken());
					token++;
					break;
				case TokenType::END_OF_FILE:
					bEOFFound=true;
					token=TokenContainer.end();
					break;
				case TokenType::UNDEFINED: 
					TokenContainer.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Unrecognized character '"+token->sValue+"'."));
					token++;
					break;
				case TokenType::COMPILER_ERROR:
					std::cout<<token->sValue<<'\n';
				break;
				default:
					token++;
				break;
			}
		}
		if (bEOFFound==false) std::cout<<"warning: EOF not found!\n";
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