#include <cassert>
#include <string>
#include <deque>
#include <map>
#include <iomanip>
#include <stack>
#include <variant>
#include "compiler.hpp"
#include "lex.hpp"
#include "token.hpp"
#include "bind.hpp"
#include "commandmenu.hpp"

extern std::map<std::string,std::string> KVMap;
std::deque<Token> TokenContainer;
std::deque<Token> ErrorTokens;
// Raw pointer bad, std::variant good.
typedef std::variant<Parser::MenuToken, Parser::KVToken, Parser::BindToken, Parser::ToggleBindToken, Parser::CMenuToken, Parser::CMenuEndToken> MenuToken_t;
std::deque<MenuToken_t> CMenuTokens;

extern std::deque<CommandMenu> CMenuContainer; // Made in main.cpp
extern std::vector<std::string> UsedKeys; // made in main.cpp

namespace Parser {
	Parser::MenuToken::MenuToken() {
		
	}
	
	Parser::MenuToken::~MenuToken() {
		
	}
	
	Parser::KVToken::KVToken() {
		
	}
	
	Parser::KVToken::KVToken(const std::string& ident, const std::string& value)
	: Key(ident), Value(value) {
		fAttribs=0;
	}
	
	Parser::BindToken::BindToken() {}
	
	Parser::BindToken::BindToken(const std::string& p_sName, const std::string& p_sCmdStr, const char& p_fAttributeFlag)
	: sName(p_sName), sCmdStr(p_sCmdStr) {fAttribs = p_fAttributeFlag;}
	
	Parser::BindToken::BindToken(const std::string& p_sKey, const std::string& p_sName, const std::string& p_sCmdStr, const char& p_fAttributeFlag)
	: sKey(p_sKey), sName(p_sName), sCmdStr(p_sCmdStr) {fAttribs = p_fAttributeFlag;		}
	
	Parser::BindToken::~BindToken() {}
	
	Parser::ToggleBindToken::ToggleBindToken() {}
	
	Parser::ToggleBindToken::ToggleBindToken(const std::string p_names[MAX_TOGGLE_STATES], const std::string p_CmdStrContainer[MAX_TOGGLE_STATES], unsigned short p_iToggleStates, const char& p_fAttributeFlag)
	: ToggleStates(p_iToggleStates)
	{
		for (unsigned short i=0; i < MAX_TOGGLE_STATES; i++) {
			NameContainer[i]=p_names[i];
			NameContainer[i].shrink_to_fit();
		}
		for (unsigned short i=0; i < MAX_TOGGLE_STATES; i++) {
			CmdStrContainer[i]=p_CmdStrContainer[i];
			CmdStrContainer[i].shrink_to_fit();
		}
		
		fAttribs = p_fAttributeFlag;
	}
	
	Parser::ToggleBindToken::ToggleBindToken(const std::string& p_sKey, const std::string p_names[MAX_TOGGLE_STATES], const std::string p_CmdStrContainer[MAX_TOGGLE_STATES], unsigned short p_iToggleStates, const char& p_fAttributeFlag)
	: ToggleStates(p_iToggleStates), sKey(p_sKey)
	{
		for (unsigned short i=0; i < MAX_TOGGLE_STATES; i++) {
			NameContainer[i]=p_names[i];
			NameContainer[i].shrink_to_fit();
		}
		for (unsigned short i=0; i < MAX_TOGGLE_STATES; i++) {
			CmdStrContainer[i]=p_CmdStrContainer[i];
			CmdStrContainer[i].shrink_to_fit();
		}
		fAttribs = p_fAttributeFlag;
	}
	
	Parser::ToggleBindToken::~ToggleBindToken() {}
	
	Parser::CMenuToken::CMenuToken() {}
	
	Parser::CMenuToken::CMenuToken(const std::string& sName, const char& p_fAttributeFlag)
	: sName(sName) {
		fAttribs = p_fAttributeFlag;
	}

	Parser::CMenuToken::CMenuToken(const std::string& p_sKey, const std::string& sName, const char& p_fAttributeFlag)
	: sName(sName), sKey(p_sKey) {
		fAttribs = p_fAttributeFlag;
	}
	
	Parser::CMenuToken::~CMenuToken() {}
	
	Parser::CMenuEndToken::CMenuEndToken() {}
	
	Parser::CMenuEndToken::~CMenuEndToken() {}
	
	/* Parses the tokens from the lexer
		Returns true if parsing was successful; false if it wasn't.
	*/
	
	bool ParseTokens() {
		// TODO: At some point convert the array into an actual parse tree. 
		/* `fParserStateFlag` records the states of the parser. 
			1st bit - PARSER_STATE_FORMATTED - Default: 1
			2nd bit - PARSER_STATE_NOEXIT - Default: 0
			…
			7th bit - EOF found? - Default: 0
			8th bit - Error found? - Default: 0
		*/
		char fParserStateFlag = PARSER_STATE_FORMATTED;
		int iCMenuDepth=0; // Records how nested is a CMenu.
		// Check if there have already been errors and keep that in mind.
		if (ErrorTokens.size()>=1) fParserStateFlag |= PARSER_STATE_ERRORS_FOUND;
		for (auto token=TokenContainer.begin(); token!=TokenContainer.end(); ) {
			switch (token->Type) {
				case TokenType::NOEXIT:
					if (fParserStateFlag & PARSER_STATE_NOEXIT) ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,"error: Duplicate modifier \"NOEXIT\"."));
					else fParserStateFlag |= PARSER_STATE_NOEXIT;
					token++;
					break;
				case TokenType::NOFORMAT:
					if (!(fParserStateFlag & PARSER_STATE_FORMATTED)) ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,"error: Duplicate modifier \"NOFORMAT\"."));
					else fParserStateFlag &= ~PARSER_STATE_FORMATTED;
					token++;
					break;
				case TokenType::TOGGLE: 
					{
						std::string NameList[MAX_TOGGLE_STATES];
						std::string CmdList[MAX_TOGGLE_STATES];
						unsigned short i=1;
						if (iCMenuDepth<=0) 
							ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,"error: Toggle bind must be set in a command menu."));
						if ((token+i)->Type!=TokenType::BIND)
							ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,"error: Expected \'BIND\' keyword here."));
						else i++;
						while ((token+i)->Type == TokenType::STRING)
						{
							if (i % 2 == 0) NameList[(i-2)/2]=(token+i)->sValue;
							else if (i % 2 == 1) CmdList[(i-2)/2]=(token+i)->sValue;
							i++;
						}
						if (i-1<=1 || i % 2 != 0)  // Even amount of strings indicate that the toggle bind has names and cmdstrs
							ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,"error: Expected string here."));
						if ((token+i)->Type != TokenType::VBAR) 
							ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,(token+i)->GetFileLoc()+"error: Expected '|'."));
						if (!(fParserStateFlag & PARSER_STATE_ERRORS_FOUND)) 
							CMenuTokens.push_back(Parser::ToggleBindToken(NameList, CmdList, static_cast<unsigned short>((i-2)/2), ((fParserStateFlag & PARSER_STATE_FORMATTED) ? CMTOKATTRIB_FORMATTED : 0) | ((fParserStateFlag & PARSER_STATE_NOEXIT) ? CMTOKATTRIB_NOEXIT : 0)));
						// Bind already set so reset the modifier flag.
						fParserStateFlag &= ~(PARSER_STATE_NOEXIT | PARSER_STATE_BIND_KEYSET);
						token += i;
					}
					break;
				case TokenType::BIND: // Check for bind.
					{
						unsigned short i=1u;
						if (iCMenuDepth<=0) 
							ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,"error: Bind must be set in a command menu."));
						if ((token+i)->Type!=TokenType::STRING) 
							ErrorTokens.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,"error: Expected a string."));
						else {
							i++;
							if ((token+i)->Type != TokenType::STRING)
								ErrorTokens.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,"error: Expected a string."));
							else i++;
						}
						if ((token+i)->Type != TokenType::VBAR)
							ErrorTokens.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,"error: Expected '|'."));
						else i++;
						if (!(fParserStateFlag & PARSER_STATE_ERRORS_FOUND))
							CMenuTokens.push_back(Parser::BindToken((token+1)->sValue, (token+2)->sValue, ((fParserStateFlag & PARSER_STATE_FORMATTED) ? CMTOKATTRIB_FORMATTED : 0) | ((fParserStateFlag & PARSER_STATE_NOEXIT) ? CMTOKATTRIB_NOEXIT : 0)));
						// Bind already set so reset the modifier flag.
						fParserStateFlag &= ~PARSER_STATE_NOEXIT;
						fParserStateFlag |= PARSER_STATE_FORMATTED;
						token+=i;
					}
					break;
				case TokenType::STRING: // Check for new command menu.
				{
					unsigned short i=1u;
					// CMenus will never run cmenu.exitmenu so NOEXIT is redundant.
					if (fParserStateFlag & PARSER_STATE_NOEXIT) {
						ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,"error: 'NOEXIT' modifier cannot be used with CMenus."));
						fParserStateFlag &= ~CMTOKATTRIB_NOEXIT;
					}
					if ((token+i)->Type!=TokenType::LCBRACKET) 
						ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,"error: Expected '{' here."));
					if (!(fParserStateFlag & PARSER_STATE_ERRORS_FOUND)) 
						CMenuTokens.push_back(Parser::CMenuToken(token->sValue,((fParserStateFlag & PARSER_STATE_FORMATTED) ? CMTOKATTRIB_FORMATTED : 0) | CMTOKATTRIB_NOEXIT));
					fParserStateFlag |= CMTOKATTRIB_FORMATTED;
					token+=i;
				}
				break;
				case TokenType::IDENTIFIER: // Check for set keymaps
				{
					// Key Values should never have modifiers.
					if (!(fParserStateFlag & PARSER_STATE_FORMATTED) || (fParserStateFlag & PARSER_STATE_NOEXIT) || (fParserStateFlag & PARSER_STATE_BIND_KEYSET)) {
						ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,"error: Modifiers can only be applied to CMenus, binds, or toggle binds."));
					}
					unsigned short i=1u;
					if ((token+i)->Type!=TokenType::EQUALS) {
						ErrorTokens.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,"error: Expected a '=' here."));
					} 
					else i++;
					if ((token+i)->Type!=TokenType::STRING) {
						ErrorTokens.push_back(Token((token+i)->iLineNum,(token+i)->iLineColumn,TokenType::COMPILER_ERROR,"error: Expected a string here."));
					}
					else i++;
					if (!(fParserStateFlag & PARSER_STATE_ERRORS_FOUND)) CMenuTokens.push_back(Parser::KVToken(token->sValue,(token+2)->sValue));
					token+=i;
				}
				break;
				case TokenType::LCBRACKET:
					iCMenuDepth++;
					if (token>TokenContainer.begin() && (token-1)->Type!=TokenType::STRING) {
						ErrorTokens.push_back(Token((token-1)->iLineNum,(token-1)->iLineColumn,TokenType::COMPILER_ERROR,": error: Expected a string here."));
						iCMenuDepth--;
					}
					token++;
				break;
				case TokenType::RCBRACKET:
					iCMenuDepth--;
					if (iCMenuDepth<0) {
						ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,"error: Stray '}'"));
						iCMenuDepth++;
					}
					if (!(fParserStateFlag & PARSER_STATE_ERRORS_FOUND)) CMenuTokens.push_back(Parser::CMenuEndToken());
					token++;
					break;
				case TokenType::END_OF_FILE:
					// End parsing immediately.
					fParserStateFlag |= PARSER_STATE_EOF_FOUND;
					token=TokenContainer.end();
					break;
				case TokenType::UNDEFINED: 
					ErrorTokens.push_back(Token(token->iLineNum,token->iLineColumn,TokenType::COMPILER_ERROR,"error: Unrecognized character \'"+token->sValue+"\'."));
					token++;
					break;
				default:
					token++;
				break;
			}
		}
		if (!(fParserStateFlag & PARSER_STATE_EOF_FOUND)) std::cout<<"warning: EOF not found!\n";
		if (ErrorTokens.size()>=1) fParserStateFlag |= PARSER_STATE_ERRORS_FOUND;
		return !(fParserStateFlag >> 7 /* Errors Found? */);
	}
}

/* Convert menu tokens in TokenContainer into something useful.
	* p_iBindCount (unsigned short) stores how many binds were counted
	* p_bUsedDisplayFlags (a flag) stores what display types were used. Used to optimize proper closing of CMenu displays.
		- 3rd bit: Is the "none" display method used?
		- 2nd bit: Is the "console" display method used?
		- 1st bit: Is the "captions" display method used?
		
	Returns true if conversion was successful. If not then it returns false.
*/
bool ParseMenuTokens(unsigned short& p_iBindCount, char& p_bUsedDisplayFlags) {
	// Has there been an error? BAIL.
	char t_bUsedDisplayFlags = p_bUsedDisplayFlags;
	if (ErrorTokens.size()>=1) return false;
	std::deque<CommandMenu> CMenuStack;
	std::stack<unsigned char> NumKeyStack;
	
	// This variable is in the function scope instead of the switch-case scope, because the end cmenu tokens NEED to remember the attributes of their cmenus.
	Parser::CMenuToken CurrentCMenu;
	
	for (auto token = CMenuTokens.begin(); token != CMenuTokens.end(); token++) {
		// Automatically resets the KEY KV to prevent other binds from being affected by it.
		if (token != CMenuTokens.begin() && !std::holds_alternative<Parser::KVToken>(*(token - 1))) KVMap["KEY"]="";
		if (!std::holds_alternative<Parser::KVToken>(*token)
			&& KVMap["KEY"] != "")
			std::visit([](auto&& args){
				args.fAttribs |= CMTOKATTRIB_BIND_KEYSET;
			}, *token);
		// Parse menu tokens.
		if (std::holds_alternative<Parser::KVToken>(*token))
		{
			KVMap.insert_or_assign(std::get<Parser::KVToken>(*token).Key, std::get<Parser::KVToken>(*token).Value);
			// Store what display types were used in p_bUsedDisplayFlags IF they get used.
			if (std::get<Parser::KVToken>(*token).Key == "display") {
				if (KVMap["display"]=="none") t_bUsedDisplayFlags |= FL_DISPLAY_NONE;
				else if (KVMap["display"]=="console") t_bUsedDisplayFlags |= FL_DISPLAY_CONSOLE;
				else if (KVMap["display"]=="caption") t_bUsedDisplayFlags |= FL_DISPLAY_CAPTION;
				// If the display KV isn't one of the three specified values, force it to the default value. ("caption").
				else {
					std::cout<<"warning: Unknown display type "<<quoted(KVMap["display"])<<". Falling back to caption display!\n";
					KVMap["display"]="caption";
					t_bUsedDisplayFlags |= FL_DISPLAY_CAPTION;
				}
			}
		}
		else if (std::holds_alternative<Parser::CMenuToken>(*token))
		{
			//For binds to CMenuContainer.
			CurrentCMenu = std::get<Parser::CMenuToken>(*token);
			CurrentCMenu.sKey = KVMap["KEY"];
			std::size_t iDuplicateNumber = 0llu;
			// Form duplicates if formatted name is already taken.
			if (CMenuStack.size() > 0) iDuplicateNumber += std::count_if(CMenuStack.rbegin(), CMenuStack.rend(), [&CurrentCMenu](const CommandMenu p){
				return (formatRaw(p.sName) == formatRaw(CurrentCMenu.sName));
			});
			if (CMenuContainer.size() > 1) iDuplicateNumber += std::count_if(CMenuContainer.begin(), CMenuContainer.end(), [&CurrentCMenu](const CommandMenu p){
				return (formatRaw(p.sName) == formatRaw(CurrentCMenu.sName));
			});
			else if (CMenuContainer.size() == 1) iDuplicateNumber += (formatRaw(CMenuContainer.front().sName) == formatRaw(CurrentCMenu.sName));
			CMenuStack.push_front(CommandMenu(CurrentCMenu.sName));
			if (iDuplicateNumber > 0) CMenuStack.front().sRawName+='_'+std::to_string(iDuplicateNumber);
			if (CMenuStack.size() > 1) {
				if (CurrentCMenu.fAttribs & CMTOKATTRIB_BIND_KEYSET) {
					if (iDuplicateNumber > 0)
						(CMenuStack.begin()+1)->binds.push_back(Bind(CurrentCMenu.sKey,Parser::BindToken(CurrentCMenu.sName,"exec $cmenu_"+formatRaw(CurrentCMenu.sName)+'_'+std::to_string(iDuplicateNumber),CurrentCMenu.fAttribs)));
					else 
						(CMenuStack.begin()+1)->binds.push_back(Bind(CurrentCMenu.sKey,Parser::BindToken(CurrentCMenu.sName,"exec $cmenu_"+formatRaw(CurrentCMenu.sName),CurrentCMenu.fAttribs)));
					// Add the keyname to a list of used key names if it isn't already added.
					if (std::none_of(UsedKeys.cbegin(),UsedKeys.cend(),[&CurrentCMenu](std::string_view s){ return s == CurrentCMenu.sKey; })) UsedKeys.push_back(CurrentCMenu.sKey);
				}
				else {
					if (iDuplicateNumber>0) (CMenuStack.begin()+1)->binds.push_back(Bind(std::to_string(NumKeyStack.top() % 10),Parser::BindToken(CurrentCMenu.sName,"exec $cmenu_"+formatRaw(CurrentCMenu.sName)+'_'+std::to_string(iDuplicateNumber),CurrentCMenu.fAttribs)));
					else (CMenuStack.begin()+1)->binds.push_back(Bind(std::to_string(NumKeyStack.top() % 10),Parser::BindToken(CurrentCMenu.sName,"exec $cmenu_"+formatRaw(CurrentCMenu.sName),CurrentCMenu.fAttribs)));
				}
			}
			if (!(CurrentCMenu.fAttribs & CMTOKATTRIB_BIND_KEYSET) && !NumKeyStack.empty()) NumKeyStack.top()=(NumKeyStack.top()+1);
			NumKeyStack.push(1u);
			p_bUsedDisplayFlags = t_bUsedDisplayFlags;
		}
		else if (std::holds_alternative<Parser::CMenuEndToken>(*token))
		{
			assert(CMenuStack.size() > 0);
			// if there are more than 10 number-key binds, they will overlap
			if (NumKeyStack.top()>11) std::cout<<"Warning: More than ten number-key binds in CMenu \'"<<CMenuStack.front().sName<<"\'. Some binds will overlap each other!\n";
			CMenuContainer.push_back(CMenuStack.front());
			CMenuStack.pop_front();
			assert(!NumKeyStack.empty());
			NumKeyStack.pop();
		}
		else if (std::holds_alternative<Parser::BindToken>(*token))
		{
			Parser::BindToken CurrentBindToken = std::get<Parser::BindToken>(*token);
			CurrentBindToken.sKey = KVMap["KEY"];
			assert(CMenuStack.size() > 0);
			if (!(CurrentBindToken.fAttribs & CMTOKATTRIB_BIND_KEYSET)) {
				CMenuStack.front().binds.push_back(Bind(std::to_string(NumKeyStack.top() % 10),CurrentBindToken));
				NumKeyStack.top()=(NumKeyStack.top()+1);
			}
			else {
				CMenuStack.front().binds.push_back(Bind(CurrentBindToken.sKey,CurrentBindToken));
				// Add the keyname to a list of used key names if it isn't already added.
				if (std::none_of(UsedKeys.cbegin(),UsedKeys.cend(),[&CurrentBindToken](std::string_view s){ return s == CurrentBindToken.sKey;})) UsedKeys.push_back(CurrentBindToken.sKey);
			}
			p_iBindCount++;
		}
		else if (std::holds_alternative<Parser::ToggleBindToken>(*token))
		{
			Parser::ToggleBindToken CurrentToggleBindToken = std::get<Parser::ToggleBindToken>(*token);
			CurrentToggleBindToken.sKey = KVMap["KEY"];
			assert(CMenuStack.size() > 0);				
			if (!(CurrentToggleBindToken.fAttribs & CMTOKATTRIB_BIND_KEYSET)) {
				CMenuStack.front().binds.push_back(Bind(std::to_string(NumKeyStack.top() % 10),CurrentToggleBindToken));
				assert(!NumKeyStack.empty());
				NumKeyStack.top()=(NumKeyStack.top()+1);
			}
			else {
				CMenuStack.front().binds.push_back(Bind(CurrentToggleBindToken.sKey,CurrentToggleBindToken));
				// Add the keyname to a list of used key names if it isn't already added.
				if (std::none_of(UsedKeys.cbegin(),UsedKeys.cend(),[&CurrentToggleBindToken](std::string_view s){ return s == CurrentToggleBindToken.sKey;})) UsedKeys.push_back(CurrentToggleBindToken.sKey);
			}
			p_iBindCount++;
		}
		else {
			
		}
	}
	return true;
}