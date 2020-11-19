#ifndef COMPILER_HPP
#define COMPILER_HPP
#define MAX_TOGGLE_STATES 512
#include <string>
#include <iostream>
#include "tokens.hpp"
class Token;
bool StreamIsBind(const Token* t);
bool StreamIsKeymap(const std::size_t& iterator);
bool StreamIsToggleBind(const std::size_t& iterator);
bool StreamIsNewPage(const std::size_t& iterator);
bool Tokenize(const std::string& str);
void MenuCreate(unsigned short& bindcount);
// Convert to a safer string format for file and caption names.
std::string Format(std::string str);
namespace Parser {
	// For handling multi tokens.
	enum MenuTokenType {MENU_BIND=0, MENU_TOGGLE_BIND, MENU_NEW_PAGE, MENU_END_PAGE, KV_SET};
	struct MenuToken {
		MenuTokenType type;
		MenuToken() {}
		virtual ~MenuToken() {}
	};
	struct KVToken : public MenuToken {
		std::string Key;
		std::string Value;
		KVToken() {}
		KVToken(const std::string& ident, const std::string& value)
		: Key(ident), Value(value) {
			type=MenuTokenType::KV_SET;
		}
	};
	struct BindToken : public MenuToken {
		std::string name;
		std::string cmdstr;
		// pagebind - Whether a bind refers to a page bind.
		BindToken() {}
		BindToken(const std::string& p_name, const std::string& p_cmdstr)
		: name(p_name), cmdstr(p_cmdstr) {
			type=MenuTokenType::MENU_BIND;
		}
	};
	struct ToggleBindToken : public MenuToken {
		unsigned short states=0u;
		std::string names[MAX_TOGGLE_STATES];
		std::string cmdstrs[MAX_TOGGLE_STATES];
		ToggleBindToken() {}
		ToggleBindToken(const std::string p_names[MAX_TOGGLE_STATES], const std::string p_cmdstrs[MAX_TOGGLE_STATES], unsigned short p_states)
		: states(p_states)
		{
			for (unsigned short i=0; i < MAX_TOGGLE_STATES; i++) {
				names[i]=p_names[i];
				names[i].shrink_to_fit();
			}
			for (unsigned short i=0; i < MAX_TOGGLE_STATES; i++) {
				cmdstrs[i]=p_cmdstrs[i];
				cmdstrs[i].shrink_to_fit();
			}
			type=MenuTokenType::MENU_TOGGLE_BIND;
		}
	};
	struct PageToken : public MenuToken {
		std::string Name;
		unsigned char depth=0u;
		PageToken() {}
		PageToken(const std::string& name, const unsigned char& p_depth)
		: Name(name), depth(p_depth) {
			type=MenuTokenType::MENU_NEW_PAGE;
		}
	};
	struct PageEndToken : public MenuToken {
		PageEndToken() {
			type=MenuTokenType::MENU_END_PAGE;
		}
	};
	bool ParseTokens();
};
#endif