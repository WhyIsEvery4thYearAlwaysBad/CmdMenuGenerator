#include "structs.hpp"
extern std::vector<std::pair<Page,unsigned char> > pages;
std::string Format(std::string str) {
	for (unsigned long long i=0; i < str.length(); i++) {
		if (isspace(str.at(i))) str.at(i)='_';
		else if (str.at(i)=='<' && str.find('>',i)!=std::string::npos) {str.erase(i,str.find('>',i)-i+1);i--;}
		else if (!isalnum(str.at(i))) {str.erase(i,1);i--;}
		else if (isupper(str.at(i))) str.at(i)=tolower(str.at(i)); 
	}
	return str;
}
// Gets It's parent page pos
unsigned int GetParentPage(const unsigned int& p_i) {
	unsigned int i=p_i;
	for (; i > 0; i--) {
		if (pages.at(i).second==pages.at(p_i).second-1) break;
	}
	return i;
}