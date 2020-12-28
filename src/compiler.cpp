#include "tokens.hpp"
#include "binds.hpp"
#include "page.hpp"
#include "compiler.hpp"
#include <iostream>
#include <string>
#include <deque>
#include <map>
#include <stack>
std::size_t linenumber=1u;
std::size_t linecolumn=1u;
extern std::map<std::string,std::string> keymap;
#define Error(error) errors.push_back(Token(0u,0u,TokenType::ERR,error))
#define GetPrevChar(str,i,c) str.rfind(c,i-1)
#define GetNextChar(str,i,c) str.find(c,i+1)
std::deque<Token> tokens;
std::deque<Token> errors;
std::deque<Parser::MenuToken*> menutokens;
extern std::deque<std::pair<Page,unsigned char> > pages; // Made in main.cpp

// Convert to a safer string format for file and caption names.
std::string Format(std::string str) {
	for (unsigned long long i=0; i < str.length(); i++) {
		if (str.at(i)=='<' && str.find('>',i)!=std::string::npos) {
			str.erase(i,(str.find('>',i+1)-i)+1);
			if (i>0) i--;
		}
		// remove punctuation
		if (ispunct(str.at(i))) {
			str.erase(i,1);
			i--;
			continue;
		}
		// and non-ascii characters
		if (i<str.length()-1 && (!isascii(str.at(i)) || !isascii(str.at(i+1)))) {
			str.erase(i,2);
			i--; 
			continue;
		}
		if (isspace(str.at(i))) str.at(i)='_';
		if (isupper(str.at(i))) str.at(i)=tolower(str.at(i)); 
	}
	return str;
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
bool Tokenize(const std::string& str) {
	bool errorsfound=false;
	std::string strtemp;
	for (std::size_t i=0; i < str.length(); )
	{
		if (tokens.size()>0) tokens.back().val.shrink_to_fit();
		// EOF Token
		if (i==str.length()-1) {
			tokens.push_back(Token(str.length()-1,linenumber,TokenType::FILEEND,""));
			tokens.back().val.shrink_to_fit();
			break;
		}
		// Nonterminal
		if (IsIdentChar(str.at(i))) {
			tokens.push_back(Token(linecolumn,linenumber,TokenType::IDENTIFIER,""));
			while (IsIdentChar(str.at(i)))
			{
				tokens.back().val.push_back(str.at(i));
				i++;
				linecolumn++;
			}
			// Check for terminals.
			if (tokens.back().val=="TOGGLE") tokens.back().type=TokenType::TOGGLE;
			else if (tokens.back().val=="BIND") tokens.back().type=TokenType::BIND;
			else if (tokens.back().val=="NOEXIT") tokens.back().type=TokenType::NOEXIT;
			else if (tokens.back().val=="NOFORMAT") tokens.back().type=TokenType::NOFORMAT;
			continue;
		}
		switch (str.at(i))
		{
		// comments
		case '/':
			// Line comments
			if (str.at(i+1)=='/') {
				for (auto t=str.begin()+i; t!=str.end(); t++, i++) {
					if (*t=='\t') linecolumn+=5-(linecolumn%4==0 ? 4 : linecolumn%4);
					else linecolumn++;
					if (*t=='\n' || *t=='\r') break;
				}
			}
			/* Block comments */
			else if (str.at(i+1)=='*') { //
				std::size_t tempi=i, templinecolumn=linecolumn, templinenumber=linenumber;
				for (auto t=str.begin()+i; t!=str.end(); t++, tempi++) {
					if (*t=='\t') templinecolumn+=5-(linecolumn%4==0 ? 4 : linecolumn%4);
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
						linecolumn=templinecolumn+2;
						linenumber=templinenumber+2;
						break;
					}
					if (t==str.end()-1) {
						Error("error: Unclosed comment. ("+std::to_string(linenumber)+':'+std::to_string(linecolumn)+')');
						i=tempi;
						break;
					}
				}
			}
			else {
				tokens.push_back(Token(linecolumn,linenumber,TokenType::UNDEFINED,"/"));
				linecolumn++;
				i++;
			}
			break;
		// strings
		case '\"':
			// New lines or carriage returns cannot be in strings. (I don't mean the '\r' or '\n' character.)
			for (i++, linecolumn++; i < str.length(); i++, linecolumn++) {
				if (str.at(i)=='\r') {
					Error("error: Missing a quote at (");
					errors.back().linenumber=linenumber;
					errors.back().location=linecolumn;
					errors.back().val+=errors.back().GetFileLoc()+")";
					// 
					tokens.push_back(Token(linecolumn,linenumber,TokenType::STRING,""));
					strtemp="";
					linecolumn=1u;
					if (str.at(i+1)!='\n') {
						linenumber++;
						i++;
					}
					break;
				}
				else if (str.at(i)=='\n') {
					Error("error: Missing a quote at (");
					errors.back().linenumber=linenumber;
					errors.back().location=linecolumn;
					errors.back().val+=errors.back().GetFileLoc()+")";
					// 
					tokens.push_back(Token(linecolumn,linenumber,TokenType::STRING,""));
					strtemp="";
					linenumber++;
					if (str.at(i-1)!='\r') {
						linecolumn=1u;
						i++;
					}
					break;
				}
				else if (i==str.length()-1){
					Error("error: Missing a quote at (");
					errors.back().val+=errors.back().GetFileLoc()+")";
					strtemp="";
					break;
				}
				else if (str.at(i)=='\"') {
					tokens.push_back(Token(linecolumn,linenumber,TokenType::STRING,strtemp));
					strtemp="";
					break;
				}
				else {
					if (str.at(i)=='\t') linecolumn+=5-(linecolumn%4==0 ? 4 : linecolumn%4);
					strtemp+=str.at(i);
				}
			}
			i++; // Starts at the ending quote if this doesn't exist.
			linecolumn++;
			break;
		//terminals
		case '=':
			tokens.push_back(Token(linecolumn,linenumber,TokenType::EQUALS,"="));
			i++;
			linecolumn++;
			break;
		case '|':
			tokens.push_back(Token(linecolumn,linenumber,TokenType::VBAR,"|"));
			i++;
			linecolumn++;
			break;
		case '{':
			tokens.push_back(Token(linecolumn,linenumber,TokenType::LCBRACKET,"{"));
			i++;
			linecolumn++;
			break;
		case '}':
			tokens.push_back(Token(linecolumn,linenumber,TokenType::RCBRACKET,"}"));
			i++;
			linecolumn++;
			break;
		//spaces and colon
		case '\t':
			linecolumn+=5-(linecolumn%4==0 ? 4 : linecolumn%4);
			i++;
			break;
		case ' ':
			linecolumn++;
			i++;
			break;
		case '\r':
			linecolumn=1u;
			if (i+1==str.length() || str.at(i+1)!='\n') linenumber++;
			i++;
			break;
		case '\v':
			i++;
			linenumber++;
			break;
		case '\n':
			linenumber++;
			if (i>0 && str.at(i-1)!='\r') linecolumn=1u;
			i++;
			break;
		default:
			tokens.push_back(Token(linecolumn,linenumber,TokenType::UNDEFINED,""));
			tokens.back().val.push_back(str.at(i));
			linecolumn++;
			i++;
			break;
		}
	}
	if (errors.size()>=1) errorsfound=true;
	return !errorsfound;
}
namespace Parser {
	unsigned int depth=0u;
	bool eoffound=false, errorsfound=false;
	bool ParseTokens() {
		bool noexit=false, format=true;
		for (auto t=tokens.begin(); t!=tokens.end(); ) {
			switch (t->type) {
				case TokenType::NOEXIT:
					if (noexit==true) Error("error: Duplicate modifier. ("+t->GetFileLoc()+')');
					else noexit=true;
					t++;
					break;
				case TokenType::NOFORMAT:
					if (format==false) Error("error: Duplicate modifier. ("+t->GetFileLoc()+')');
					else format=false;
					t++;
					break;
				case TokenType::TOGGLE: 
					{ // Check for toggle bind.
						std::string namelist[MAX_TOGGLE_STATES];
						std::string cmdlist[MAX_TOGGLE_STATES];
						unsigned short i=1;
						if (depth<=0) 
							Error("error: Toggle bind must be set in a page. ("+t->GetFileLoc()+')');
						if ((t+i)->type!=TokenType::BIND) 
							Error("Expected \'BIND\' ("+(t+i)->GetFileLoc()+")");
						else i++;
						while ((t+i)->type==TokenType::STRING)
						{
							if (i % 2 == 0) namelist[(i-2)/2]=(t+i)->val;
							else if (i % 2 == 1) cmdlist[(i-2)/2]=(t+i)->val;
							i++;
						}
						if (i-1<=1 || i % 2!=0)  // Even amount of strings indicate that the toggle bind has names and cmdstrs
							Error("error: Expected String! ("+(t+i)->GetFileLoc()+")");
						if ((t+i)->type!=TokenType::VBAR) 
							Error("error: Expected '|' ("+(t+i-1)->GetFileLoc()+")");
						menutokens.push_back(new Parser::ToggleBindToken(namelist,cmdlist,static_cast<unsigned short>((i-2)/2),noexit,format));
						if (noexit==true) noexit=false;
						t+=i;
					}
					break;
				case TokenType::BIND: // Check for bind.
					if (depth<=0) 
						Error("error: Bind must be set in a page. ("+t->GetFileLoc()+')');
					if ((t+1)->type!=TokenType::STRING) 
						Error("error: Expected name string. ("+(t+1)->GetFileLoc()+")");
					if ((t+2)->type!=TokenType::STRING) 
						Error("error: Expected command string. ("+(t+2)->GetFileLoc()+")");
					if ((t+3)->type!=TokenType::VBAR) 
						Error("error: Expected '|' ("+(t+3)->GetFileLoc()+")");
					menutokens.push_back(new Parser::BindToken((t+1)->val, (t+2)->val,noexit,format));
					noexit=false, format=true;
					t+=4;
					break;
				case TokenType::STRING: // Check for new page.
					if ((t+1)->type!=TokenType::LCBRACKET) 
						Error("error: Expected '{' ("+(t+1)->GetFileLoc()+")");
					else if (noexit==true) {
						Error("error: Expected a bind. ("+t->GetFileLoc()+')');
						noexit=false;
					}
					menutokens.push_back(new Parser::PageToken(t->val,depth,format));
					format=true;
					depth++;
					t+=2;
					break;
				case TokenType::IDENTIFIER: // Check for set keymaps
					if ((t+1)->type!=TokenType::EQUALS) Error("error: Expected '=' ("+(t+1)->GetFileLoc()+")");
					if ((t+2)->type!=TokenType::STRING) Error("error: Expected string ("+(t+2)->GetFileLoc()+")");
					if ((t+1)->type!=TokenType::EQUALS && (t+2)->type!=TokenType::STRING) Error("error: Unknown identifier \'"+t->val+"\' ("+t->GetFileLoc()+')');
					menutokens.push_back(new Parser::KVToken(t->val,(t+2)->val));
					t+=3;
					break;
				case TokenType::RCBRACKET:
					depth--;
					if (depth==UINT32_MAX) {
						Error("error: Stray '}' ("+t->GetFileLoc()+")");
						depth++;
					}
					menutokens.push_back(new Parser::PageEndToken());
					t++;
					break;
				case TokenType::FILEEND:
					eoffound=true;
					t=tokens.end();
					break;
				case TokenType::UNDEFINED: 
					Error("error: Unrecognized token '"+t->val+"' ("+t->GetFileLoc()+")");
					t++;
					break;
				default:
					t++;
					break;
		}
		}
		if (eoffound==false) std::cout<<"warning: EOF not found!\n";
		if (depth!=0) Error("error: You are missing a '}'!");
		if (errors.size()>=1) errorsfound=true;
		return !errorsfound;
	}
}

// Convert menu tokens into something useful.
void MenuCreate(unsigned short& bindcount) {
	std::deque<std::pair<Page, unsigned char> > pagestack;
	std::stack<Bind> unusedbindstack;
	std::stack<unsigned char> nkeystack;
	// Variable is here to reduce the amount of goddamn downcasts I have to do.
	Parser::PageToken tpage;
	for (auto t = menutokens.begin(); t != menutokens.end(); t++) {
		switch ((*t)->type)
		{
		case Parser::MenuTokenType::KV_SET:
			{
				Parser::KVToken temp=static_cast<Parser::KVToken&>(**t);
				keymap.insert_or_assign(temp.Key,temp.Value);
			}
			break;
		case Parser::MenuTokenType::MENU_NEW_PAGE:
			{
				//For binds to pages.
				tpage=static_cast<Parser::PageToken&>(**t);
				std::size_t i=0llu;
				// Form duplicates if formatted name is already taken.
				for (auto t=pagestack.end(); t!=pagestack.begin(); t--)
				{
					if (t->first.formatted_title==Format(tpage.Name)
					 || t->first.formatted_title==(Format(tpage.Name)+'_'+std::to_string(i))) i++;
				}
				for (auto& t : pages)
				{
					if (t.first.formatted_title==Format(tpage.Name)
					|| t.first.formatted_title==(Format(tpage.Name)+'_'+std::to_string(i))) i++;
				}
				pagestack.push_front({Page(tpage.Name),tpage.depth});	
				if (i>0) pagestack.front().first.formatted_title+='_'+std::to_string(i);
				if (pagestack.size()>1) {
					if (i>0) (pagestack.begin()+1)->first.binds.push_back(Bind(nkeystack.top(),Parser::BindToken(tpage.Name,"exec $pageopen_"+Format(tpage.Name)+'_'+std::to_string(i),true,tpage.formatted)));
					else (pagestack.begin()+1)->first.binds.push_back(Bind(nkeystack.top(),Parser::BindToken(tpage.Name,"exec $pageopen_"+Format(tpage.Name),true,tpage.formatted)));
				}
				nkeystack.push(1u);
			}
			break;
		case Parser::MenuTokenType::MENU_END_PAGE:
			// Warning:
			if (pagestack.front().first.binds.size()>9) std::cout<<"warning: More than nine binds in page \'"<<pagestack.front().first.title<<"\'\n";
			pages.push_back(pagestack.front());
			pagestack.pop_front();
			nkeystack.pop();
			if (!nkeystack.empty()) {
				nkeystack.top()%=9;
			 	nkeystack.top()++;
			}
			break;
		case Parser::MenuTokenType::MENU_BIND:
			pagestack.front().first.binds.push_back(Bind(nkeystack.top(),static_cast<Parser::BindToken&>(**t)));
			nkeystack.top()%=9;
			nkeystack.top()++;
			bindcount++;
			break;
		case Parser::MenuTokenType::MENU_TOGGLE_BIND:
			pagestack.front().first.binds.push_back(Bind(nkeystack.top(),static_cast<Parser::ToggleBindToken&>(**t)));
			nkeystack.top()%=9;
			nkeystack.top()++;
			bindcount++;
			break;
		default:
			break;
		}
	}
}