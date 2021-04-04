#ifndef COMMANDMENU_HPP
#define COMMANDMENU_HPP
#include <string>
#include <vector>
#include <deque>
#include "compiler.hpp"
#include "bind.hpp"

struct CommandMenu {
	std::string sRawName, sName;
	std::vector<Bind> binds;

	CommandMenu();
	CommandMenu(const std::string& p_sName);
	CommandMenu(const std::string& p_sRawName, const std::string& p_sName);
	~CommandMenu();
};
#endif