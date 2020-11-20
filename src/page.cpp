#include "page.hpp"
#include "compiler.hpp"
extern std::deque<std::pair<Page,unsigned char> > pages;
std::deque<Parser::MenuToken*>::iterator GetParentPageToken(const std::deque<Parser::MenuToken*>& cont, std::deque<Parser::MenuToken*>::iterator it, const unsigned char& age) {
	for (; it!=cont.begin(); it--) {
		
		if (dynamic_cast<Parser::PageToken*>(*it)!=NULL && dynamic_cast<Parser::PageToken*>(*it)->depth==age) break;
	}
	return it;
}
std::size_t GetParentPage(const std::deque<std::pair<Page, unsigned char> >& cont, std::size_t p_i, const unsigned char& age) {
	unsigned int i=p_i;
	for (; i >= 0; i--) {
		if (cont.at(i).second==age) break;
	}
	return i;
}

Page::Page() {}

Page::Page(const std::string& p_title)
	: title(p_title), formatted_title(Format(p_title)) {}
	
Page::Page(const std::string& p_title, const std::string& p_ftitle)
: title(p_title), formatted_title(p_ftitle) {}

Page::~Page() {}