#ifndef PAGE_HPP
#define PAGE_HPP
#include <string>
#include <vector>
#include <deque>
#include "binds.hpp"
#include "compiler.hpp"

struct CommandMenu {
	std::string sRawName, sName;
	std::vector<Bind> binds;

	CommandMenu();
	CommandMenu(const std::string& p_sName);
	CommandMenu(const std::string& p_sRawName, const std::string& p_sName);
	~CommandMenu();
};
#endif