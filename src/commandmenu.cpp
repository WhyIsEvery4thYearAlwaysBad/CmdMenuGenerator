#include <map>
#include "commandmenu.hpp"
#include "compiler.hpp"
#include "lex.hpp"

extern std::map<std::string,std::string> KVMap;

CommandMenu::CommandMenu() {}

CommandMenu::CommandMenu(const std::string& p_sName)
: sRawName(formatRaw(p_sName)), sName(p_sName) {
	if (KVMap["display"]=="caption") Display=CMenuDisplayType::CAPTIONS;
	else if (KVMap["display"]=="console") Display=CMenuDisplayType::CONSOLE;
	else if (KVMap["display"]=="none") Display=CMenuDisplayType::NONE;
}
	
CommandMenu::CommandMenu(const std::string& p_sRawName, const std::string& p_sName)
: sRawName(p_sRawName), sName(p_sName) {
	if (KVMap["display"]=="caption") Display=CMenuDisplayType::CAPTIONS;
	else if (KVMap["display"]=="console") Display=CMenuDisplayType::CONSOLE;
	else if (KVMap["display"]=="none") Display=CMenuDisplayType::NONE;
	else Display=CMenuDisplayType::CAPTIONS;
}

CommandMenu::~CommandMenu() {}