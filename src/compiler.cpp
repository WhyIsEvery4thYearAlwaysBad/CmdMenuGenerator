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
	int iCMenuDepth=0; // Records how nested is a CMenu.
	bool bEOFFound=false, bErrorsFound=false;
	bool ParseTokens() {
		bool bNoExit=false, bFormatted=true;
		for (auto token=TokenContainer.begin(); token!=TokenContainer.end(); ) {
			switch (token->Type) {
				case TokenType::NOEXIT:
					if (bNoExit==true) ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Duplicate modifier \"NOEXIT\"."));
					else bNoExit=true;
					token++;
					break;
				case TokenType::NOFORMAT:
					if (bFormatted==false) ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Duplicate modifier \"NOFORMAT\"."));
					else bFormatted=false;
					token++;
					break;
				case TokenType::TOGGLE: 
					{ // Check for toggle bind.
						std::string NameList[MAX_TOGGLE_STATES];
						std::string CmdList[MAX_TOGGLE_STATES];
						unsigned short i=1;
						if (iCMenuDepth<=0) 
							ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Toggle bind must be set in a command menu."));
						if ((token+i)->Type!=TokenType::BIND) 
							ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Expected \'BIND\' keyword here."));
						else i++;
						while ((token+i)->Type==TokenType::STRING)
						{
							if (i % 2 == 0) NameList[(i-2)/2]=(token+i)->sValue;
							else if (i % 2 == 1) CmdList[(i-2)/2]=(token+i)->sValue;
							i++;
						}
						if (i-1<=1 || i % 2!=0)  // Even amount of strings indicate that the toggle bind has names and cmdstrs
							ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Expected string here."));
						if ((token+i)->Type!=TokenType::VBAR) 
							ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,(token+i)->GetFileLoc()+"error: Expected '|'."));
						if (!bErrorsFound) CMenuTokens.push_back(new Parser::ToggleBindToken(NameList,CmdList,static_cast<unsigned short>((i-2)/2),bNoExit,bFormatted));
						bNoExit=false, bFormatted=true;
						token+=i;
					}
					break;
				case TokenType::BIND: // Check for bind.
				{
					unsigned short i=1u;
					if (iCMenuDepth<=0) 
						ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Bind must be set in a command menu."));
					if ((token+i)->Type!=TokenType::STRING) 
						ErrorTokens.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,(token+i)->GetFileLoc()+": error: Expected a string."));
					else {
						i++;
						if ((token+i)->Type!=TokenType::STRING) 
							ErrorTokens.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,(token+i)->GetFileLoc()+": error: Expected a string."));
						else i++;
					}
					if ((token+i)->Type!=TokenType::VBAR) 
						ErrorTokens.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,(token+i)->GetFileLoc()+": error: Expected '|'."));
					else i++;
					if (!bErrorsFound) CMenuTokens.push_back(new Parser::BindToken((token+1)->sValue, (token+2)->sValue,bNoExit,bFormatted));
					bNoExit=false, bFormatted=true;
					token+=i;
				}
					break;
				case TokenType::STRING: // Check for new commandmenu.
				{
					unsigned short i=1u;
					if (bNoExit==true) {
						ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: 'NOEXIT' modifier cannot be used with CMenus."));
						bNoExit = false;
					}
					if ((token+i)->Type!=TokenType::LCBRACKET) 
						ErrorTokens.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,(token+i)->GetFileLoc()+": error: Expected '{' here."));
					if (!bErrorsFound) CMenuTokens.push_back(new Parser::CMenuToken(token->sValue,bFormatted));
					bFormatted=true;
					token+=i;
				}
				break;
				case TokenType::IDENTIFIER: // Check for set keymaps
				{
					unsigned short i=1u;
					if ((token+i)->Type!=TokenType::EQUALS) {
						ErrorTokens.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,(token+i)->GetFileLoc()+": error: Expected a '=' here."));
					} 
					else i++;
					if ((token+i)->Type!=TokenType::STRING) {
						ErrorTokens.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,(token+i)->GetFileLoc()+": error: Expected a string here."));
					}
					else i++;
					if (!bErrorsFound) CMenuTokens.push_back(new Parser::KVToken(token->sValue,(token+2)->sValue));
					token+=i;
				}
				break;
				case TokenType::LCBRACKET:
					iCMenuDepth++;
					if (token>TokenContainer.begin() && (token-1)->Type!=TokenType::STRING) {
						ErrorTokens.push_back(Token((token-1)->iLineNum,(token-1)->iLineColumn,TokenType::COMPILER_ERROR,(token-1)->GetFileLoc()+": error: Expected a string here."));
						iCMenuDepth--;
					}
					token++;
				break;
				case TokenType::RCBRACKET:
					iCMenuDepth--;
					if (iCMenuDepth<0) {
						ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Stray '}'"));
						iCMenuDepth++;
					}
					if (!bErrorsFound) CMenuTokens.push_back(new Parser::CMenuEndToken());
					token++;
					break;
				case TokenType::END_OF_FILE:
					bEOFFound=true;
					token=TokenContainer.end();
					break;
				case TokenType::UNDEFINED: 
					ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,token->GetFileLoc()+": error: Unrecognized character '"+token->sValue+"'."));
					token++;
					break;
				default:
					token++;
				break;
			}
		}
		if (bEOFFound==false) std::cout<<"warning: EOF not found!\n";
		if (ErrorTokens.size()>=1) bErrorsFound=true;
		return bErrorsFound;
	}
}

/* Convert menu TokenContainer into something useful.
	* p_iBindCount (unsigned short) stores how many binds were counted
	* p_bUsedDisplayFlags (a flag) stores what display types were used.
		- 3rd bit: Is the "none" display method used?
		- 2nd bit: Is the "console" display method used?
		- 1st bit: Is the "captions" display method used?
*/

void ParseMenuTokens(unsigned short& p_iBindCount, unsigned char& p_bUsedDisplayFlags) {
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
				
				// Store what display types were used in p_bUsedDisplayFlags.
				if (KVMap["display"]=="none") p_bUsedDisplayFlags |= 1;
				else if (KVMap["display"]=="console") p_bUsedDisplayFlags |= 2;
				else if (KVMap["display"]=="caption") p_bUsedDisplayFlags |= 4;
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