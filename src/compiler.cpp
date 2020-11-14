#include "tokens.hpp"
#include "binds.hpp"
#include "page.hpp"
#include "compiler.hpp"
#include <iostream>
#include <string>
#include <deque>
#include <map>
#include <stack>
#define MAX_ALIAS_NAME 32
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
		if (i>str.length()) break;
		if (str.at(i)=='<' && str.find('>',i)!=std::string::npos) {
			str.erase(i,(str.find('>',i+1)-i)+1);
			if (i>0) i--;
		}
		// remove punctuation and non-ascii characters
		if (ispunct(str.at(i)) || !isascii(str.at(i))) {
			str.erase(i,1);
			if (i>0) i--;
		}
		if (isspace(str.at(i))) str.at(i)='_';
		if (isupper(str.at(i))) str.at(i)=tolower(str.at(i)); 
		// Aliases can only have 32 characters. oops
		if (i>=MAX_ALIAS_NAME-1) str.erase(i,str.length()-i);
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
	unsigned char depthval=0u;
	std::size_t templinecolumn=1u;
	std::string strtemp;
	for (unsigned int i=0; i < str.length(); )
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
			if (str.at(i)=='\n') linenumber++;
			if (str.at(i)=='\t') linecolumn+=4;
			// Check for terminals.
			if (tokens.back().val=="TOGGLE") tokens.back().type=TokenType::TOGGLE;
			if (tokens.back().val=="BIND") tokens.back().type=TokenType::BIND;
			continue;
		}
		switch (str.at(i))
		{
		// comments
		case '/':
			if (str.at(i+1)=='/') {
				for (auto t=str.begin()+i; t!=str.end(); t++, i++) {
					if (*t=='\t') linecolumn+=4;
					else linecolumn++;
					if (*t=='\n') break;
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
			templinecolumn=linecolumn;
			// New lines cannot be in strings. (I don't mean the '\n' character.)
			for (i++, linecolumn++; i < str.length(); i++, linecolumn++) {
				if (str.at(i)=='\n') {
					Error("error: Missing a quote at (");
					errors.back().linenumber=linenumber;
					errors.back().location=linecolumn;
					errors.back().val+=errors.back().GetFileLoc()+")";
					// 
					tokens.push_back(Token(linecolumn,linenumber,TokenType::STRING,""));
					strtemp="";
					linecolumn=1u;
					linenumber++;
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
					if (str.at(i)=='\t') linecolumn+=4;
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
			tokens.push_back(Token(linecolumn,linenumber,TokenType::LCBRACKET,"|"));
			depthval++;
			i++;
			linecolumn++;
			break;
		case '}':
			tokens.push_back(Token(linecolumn,linenumber,TokenType::RCBRACKET,"}"));
			depthval--;
			i++;
			linecolumn++;
			break;
		//spaces and colon
		case '\n':
			linenumber++;
			linecolumn=1u;
			i++;
			break;
		case '\t':
			linecolumn+=4;
			i++;
			break;
		case ' ':
		case '\v':
		case '\f':
		case '\r':
			i++;
			linecolumn++;
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
// TODO : ADD AST
namespace Parser {
	unsigned int depth=0u;
	bool eoffound=false;
	bool errorsfound=false;
	bool ParseTokens() {
		for (auto t=tokens.begin(); t!=tokens.end(); ) {
			if (t->type==TokenType::TOGGLE) { // Check for toggle bind.
				std::string namelist[MAX_TOGGLE_STATES];
				std::string cmdlist[MAX_TOGGLE_STATES];
				if ((t+1)->type!=TokenType::BIND) {
					Error("Expected \"BIND\". ("+(t+1)->GetFileLoc()+")");
				}
				unsigned int i=2;
				while ((t+i)->type==TokenType::STRING)
				{
					if (i % 2 == 0) namelist[(i-2)/2]=(t+i)->val;
					else if (i % 2 == 1) cmdlist[(i-2)/2]=(t+i)->val;
					i++;
				}
				if (i % 2!=0) { // Even amount of strings indicate that the toggle bind has names and cmdstrs
					Error("error: Uneven amount of strings! ("+(t+i)->GetFileLoc()+")");
				}
				if (i<=1) {
					Error("error: Expected ≥2 strings. ("+(t+i)->GetFileLoc()+")");
				}
				if ((t+i)->type!=TokenType::VBAR) {
					Error("error: Expected '|'. ("+(t+i-1)->GetFileLoc()+")");
				}
				menutokens.push_back(new Parser::ToggleBindToken(namelist,cmdlist,(i-2)/2));
				t+=i;
			}
			else if (t->type==TokenType::BIND) { // Check for bind.
				if ((t+1)->type!=TokenType::STRING) {
					Error("error: Expected name string. ("+(t+1)->GetFileLoc()+")");
				}
				if ((t+2)->type!=TokenType::STRING) {
					Error("error: Expected command string. ("+(t+2)->GetFileLoc()+")");
				}
				if ((t+3)->type!=TokenType::VBAR) {
					Error("error: Expected '|'. ("+(t+3)->GetFileLoc()+")");
				}
				menutokens.push_back(new Parser::BindToken((t+1)->val, (t+2)->val));
				t+=4;
			}
			else if (t->type==TokenType::STRING) { // Check for new page.
				if ((t+1)->type!=TokenType::LCBRACKET) {
					Error("error: Expected '{'. ("+t->GetFileLoc()+")");
				}
				menutokens.push_back(new Parser::PageToken(t->val,depth));
				depth++;
				t+=2;
			}
			else if (t->type==TokenType::IDENTIFIER) { // Check for set keymaps
				if ((t+1)->type!=TokenType::EQUALS) {
					Error("error: Expected '='. ("+(t+1)->GetFileLoc()+")");
				}
				if ((t+2)->type!=TokenType::STRING) {
					Error("error: Expected string. ("+(t+2)->GetFileLoc()+")");
				}
				menutokens.push_back(new Parser::KVToken(t->val,(t+2)->val));
				t+=3;
			}
			else if (t->type==TokenType::RCBRACKET) {
				depth--;
				if (depth==UINT32_MAX) {
					Error("Stray '}'. ("+t->GetFileLoc()+")");
					depth++;
				}
				menutokens.push_back(new Parser::PageEndToken());
				t++;
			}
			else if (t->type==TokenType::FILEEND) {
				eoffound=true;
				break;
			}
			else if (t->type==TokenType::UNDEFINED) {
				Error("error: Unrecognized token '"+t->val+"' ("+t->GetFileLoc()+")");
				t++;
			}
			else {
				t++;
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
			Parser::SetKV(static_cast<Parser::KVToken&>(**t));
			break;
		case Parser::MenuTokenType::MENU_NEW_PAGE:
			{
				bool bindpagecreated=false;
				//For binds to pages.
				tpage=static_cast<Parser::PageToken&>(**t);
				std::size_t i=0llu;
				pagestack.push_front({Page(tpage.Name),tpage.depth});	
				if (pagestack.size()>=2) (pagestack.begin()+1)->first.binds.push_back(Bind(nkeystack.top(),tpage.Name,"exec _cvm_page_"+Format(tpage.Name)));
				for (auto& t : pagestack)
				{
					if (t.first.formatted_title==pagestack.front().first.formatted_title) {
						i++;
					}
				}
				for (auto& t : pages)
				{
					if (t.first.formatted_title==pagestack.front().first.formatted_title) {
						i++;
					}
				}
				if (i>=2) {
					pagestack.front().first.formatted_title+='_'+std::to_string(i-1);
					if (pagestack.size()>=2) (pagestack.begin()+1)->first.binds.back().cmdstr.front()+='_'+std::to_string(i-1);
				}
			}
			nkeystack.push(1u);
			break;
		case Parser::MenuTokenType::MENU_END_PAGE:
			pages.push_front(pagestack.front());
			pagestack.pop_front();
			nkeystack.pop();
			if (!nkeystack.empty()) nkeystack.top()++;
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

// Set key value pair.
void Parser::SetKV(const Parser::KVToken& t) {
	keymap.insert_or_assign(t.Key,t.Value);
}