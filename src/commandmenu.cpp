#include "commandmenu.hpp"
#include "compiler.hpp"
#include "lex.hpp"

CommandMenu::CommandMenu() {}

CommandMenu::CommandMenu(const std::string& p_sName)
	: sRawName(formatRaw(p_sName)), sName(p_sName) {}
	
CommandMenu::CommandMenu(const std::string& p_sRawName, const std::string& p_sName)
: sRawName(p_sRawName), sName(p_sName) {}

CommandMenu::~CommandMenu() {}