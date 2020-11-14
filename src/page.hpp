#ifndef PAGE_HPP
#define PAGE_HPP
#include <string>
#include <vector>
#include <deque>
#include "binds.hpp"
#include "compiler.hpp"
struct Page;
std::deque<Parser::MenuToken*>::iterator GetParentPageToken(const std::deque<Parser::MenuToken*>& cont, std::deque<Parser::MenuToken*>::iterator it, const unsigned char& age);
std::size_t GetParentPage(const std::deque<std::pair<Page, unsigned char> >& cont, std::size_t p_i, const unsigned char& age);
struct Page {
	std::string title;
	std::string formatted_title;
	std::vector<Bind> binds;

	Page() {}
	Page(const std::string& p_title)
	: title(p_title), formatted_title(Format(p_title)) {}
	Page(const std::string& p_title, const std::string& p_ftitle)
	: title(p_title), formatted_title(p_ftitle) {}
	~Page() {}
};
#endif