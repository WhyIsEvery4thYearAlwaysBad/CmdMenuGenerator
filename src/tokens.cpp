#include "tokens.hpp"
#include "lex.hpp"
#include <deque>
#include <map>
#define GetNextChar(str,i,c) str.find(c,i+1)-i
extern std::map<std::string,std::string> keymap;
std::deque<Token> tokens;
// Conver string to token stream
void Tokenize(const std::string& str, unsigned char& depthval) {
	unsigned long long terminatorlocation=0ul;
	for (unsigned int i=0; i < str.length();) 
	{
		switch (str.at(i))
		{
		// comments
		case '/':
			if (str.at(i+1)=='/') i=str.find('\n',i+1);
			break;
		// strings
		case '"':
			tokens.push_back({Token_type::STRING,str.substr(i+1,GetNextChar(str,i,'"')-1)});
			// printf("STR: %s\n",tokens.back().val.c_str());
			i+=GetNextChar(str,i,'"')+1;
			break;
		//terminals
		case '|':
			tokens.push_back({Token_type::TERMINAL,"|"});
			i++;
			break;
		case '{':
			tokens.push_back({Token_type::TERMINAL,"{"});
			depthval++;
			i++;
			break;
		case '}':
			tokens.push_back({Token_type::TERMINAL,"}"});
			depthval--;
		//spaces and colon
		case ' ':
		case '\t':
		case '\v':
		case '\f':
		case '\r':
		case '\n':
			i++;
			break;
		// key map interpertor
		default:
			// Only stop if it hits a colon, space, or double quote.
			for (terminatorlocation=i; terminatorlocation < str.length(); terminatorlocation++) {
				if (isspace(str.at(terminatorlocation))||str.at(terminatorlocation)=='"') break;
			}
			tokens.push_back({Token_type::NONTERMINAL,str.substr(i,terminatorlocation-i)});
			i=terminatorlocation;
			break;
		}
	}
}

void ParseTokens(std::vector<std::pair<Page,unsigned char> >& pages, const bool& isconcise, unsigned char& depthval) {
	unsigned char nkeyit=0; 
	unsigned char nkeys[256]={1};
	unsigned int pageiter=0;
	// Container of already used binds
	std::string bindlist="";
	std::string pagenamelist="";

	// Grammatical Rules:
	// #<nonterminal>: <nonterminal|string> - New Key map
	// BIND <STRING> <STRING> - New Bind
	// TOGGLE <STRING> <STRING>.. | - Toggle Bind
	// STRING { - New Page
	for (std::size_t i=0; i < tokens.size();) {
		// #<nonterminal>: <nonterminal|string> - New Key map
		// if (tokens.at(i).val.front()=='#' && tokens.at(i).type==Token_type::NONTERMINAL && (tokens.at(i+1).type==Token_type::NONTERMINAL || tokens.at(i+1).type==Token_type::STRING)) {
		if (StreamIsKeymap(i)) {
			// Remove colons.
			if (tokens.at(i).val.back()==':') tokens.at(i).val.pop_back();
			if (tokens.at(i+1).type==Token_type::STRING) keymap[tokens.at(i).val]='"'+tokens.at(i+1).val+'"';
			else keymap[tokens.at(i).val]=tokens.at(i+1).val;
			i+=2;
		}
		// <STRING> <STRING> - New Bind
		// else if (tokens.at(i).type==Token_type::STRING && tokens.at(i+1).type==Token_type::STRING) {
		else if (StreamIsBind(i)) {
			std::string tempformat=Format(tokens.at(i).val);
			// Add to a list if already found.
			unsigned short duplicate_num=1;
			if (bindlist.find(tempformat)!=std::string::npos) {
				while (bindlist.find(tempformat+'_'+std::to_string(duplicate_num))!=std::string::npos) {
					duplicate_num++;
				}
				tempformat+='_'+std::to_string(duplicate_num);
			}
			bindlist+=tempformat;
			pages.at(pageiter).first.binds.push_back({nkeys[nkeyit],tokens.at(i).val,tempformat,tokens.at(i+1).val+"; cvm.exitmenu"});
			// Add keys.
			nkeys[nkeyit]++;
			if ((nkeys[nkeyit]%10)==0) nkeys[nkeyit]=(nkeys[nkeyit]%10)+1;
			i+=2;
		}
		// TOGGLE <STRING> <STRING>.. | - Toggle Bind
		// else if (tokens.at(i).type==Token_type::NONTERMINAL && tokens.at(i).val=="TOGGLE" && tokens.at(i+1).type==Token_type::STRING && tokens.at(i+2).type==Token_type::STRING) {
		else if (StreamIsToggleBind(i)) {
			pages.at(pageiter).first.binds.push_back({nkeys[nkeyit],tokens.at(i+1).val,Format(tokens.at(i+1).val),tokens.at(i+2).val});
			i+=3;
			do {
				pages.at(pageiter).first.binds.back().name.push_back(tokens.at(i).val);
				pages.at(pageiter).first.binds.back().formatted_name.push_back(Format(tokens.at(i).val));
				pages.at(pageiter).first.binds.back().commandstr.push_back(tokens.at(i+1).val);
				i+=2;
			} while (tokens.at(i).type!=Token_type::TERMINAL && tokens.at(i).val!="|");
			// Prep for next bind.
			nkeys[nkeyit]++;
			if ((nkeys[nkeyit]%10)==0) nkeys[nkeyit]=(nkeys[nkeyit]%10)+1;
		}
		// STRING { - New Page
		else if (StreamIsNewPage(i)) {
			std::string tempformat=Format(tokens.at(i).val);
			// Add page name to a list if already found to prevent hash collisions.
			
			//
			unsigned short duplicate_num=1;
			if (pagenamelist.find(tempformat)!=std::string::npos) {
				while (pagenamelist.find(tempformat+'_'+std::to_string(duplicate_num))!=std::string::npos) duplicate_num++;
				tempformat.append('_'+std::to_string(duplicate_num));
			}
			pagenamelist+=tempformat;
			// Now push the page back.
			pageiter=pages.size();
			// pages.push_back({Page(tokens.at(i).val,tempformat),depthval});
			pages.push_back({Page(tokens.at(i).val,tempformat),depthval});
			// Binds the keys properly.
			if (pages.back().second!=0) {
				pages.at(GetParentPage(pages.size()-1)).first.binds.push_back(Bind(nkeys[nkeyit],tokens.at(i).val,tempformat,"exec cvm_"+tempformat));
				nkeys[nkeyit]++; nkeyit++; nkeys[nkeyit]=1;
			}
			depthval++;
			i+=2;
		}
		else if (tokens.at(i).type==Token_type::TERMINAL && tokens.at(i).val=="}") {
			depthval--, nkeyit--, pageiter=GetParentPage(pageiter), i+=1;
		}
		else if (tokens.at(i).type==Token_type::TERMINAL && tokens.at(i).val=="|") i++;
		else {
			printf("I think we found an error chief... (At Token %llu.)\n",i);
			i++;
		}
	}
}
