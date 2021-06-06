#ifndef COMMANDMENU_HPP
#define COMMANDMENU_HPP
#include <string>
#include <vector>
#include <variant>
#include "compiler.hpp"
#include "bind.hpp"

enum class CMenuDisplayType {
	NONE=0,
	CONSOLE,
	CAPTIONS
};

struct CommandMenu {
	std::string sRawName, sName;
	std::vector<std::variant<Bind, std::string> > Entries;
	CMenuDisplayType Display;
	CommandMenu();
	CommandMenu(const std::string& p_sName);
	CommandMenu(const std::string& p_sRawName, const std::string& p_sName);
	~CommandMenu();
};
#endif