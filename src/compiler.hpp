#ifndef COMPILER_HPP
#define COMPILER_HPP
#define MAX_TOGGLE_STATES 512
#include <string>
#include <iostream>
#include "token.hpp"
void ParseMenuTokens(unsigned short& p_iBindCount, unsigned char& p_bUsedDisplayFlags);
namespace Parser {
	// For handling multi Tokens.
	enum CMenuTokenType {MENU_BIND=0, MENU_TOGGLE_BIND, DECLARE_CMENU, END_CMENU, KV_SET};
	struct MenuToken {
		CMenuTokenType Type;
		MenuToken() {}
		virtual ~MenuToken() {}
	};
	struct KVToken : public MenuToken {
		std::string Key;
		std::string Value;
		KVToken() {}
		KVToken(const std::string& ident, const std::string& value)
		: Key(ident), Value(value) {
			Type=CMenuTokenType::KV_SET;
		}
	};
	struct BindToken : public MenuToken {
		bool bNoExit=false, bFormatted=true;
		std::string sName;
		std::string sCmdStr;
		// pagebind - Whether a bind refers to a command menu bind.
		BindToken() {}
		BindToken(const std::string& p_sName, const std::string& p_sCmdStr, const bool& p_bNoExit, const bool& p_bFormatted)
		: sName(p_sName), sCmdStr(p_sCmdStr), bNoExit(p_bNoExit), bFormatted(p_bFormatted) {
			Type=CMenuTokenType::MENU_BIND;
		}
	};
	struct ToggleBindToken : public MenuToken {
		bool bNoExit=false, bFormatted=true;
		unsigned short ToggleStates=0u;
		std::string NameContainer[MAX_TOGGLE_STATES];
		std::string CmdStrContainer[MAX_TOGGLE_STATES];
		ToggleBindToken() {}
		ToggleBindToken(const std::string p_names[MAX_TOGGLE_STATES], const std::string p_CmdStrContainer[MAX_TOGGLE_STATES], unsigned short p_iToggleStates, const bool& p_bNoExit, const bool& p_bFormatted)
		: ToggleStates(p_iToggleStates), bNoExit(p_bNoExit), bFormatted(p_bFormatted)
		{
			for (unsigned short i=0; i < MAX_TOGGLE_STATES; i++) {
				NameContainer[i]=p_names[i];
				NameContainer[i].shrink_to_fit();
			}
			for (unsigned short i=0; i < MAX_TOGGLE_STATES; i++) {
				CmdStrContainer[i]=p_CmdStrContainer[i];
				CmdStrContainer[i].shrink_to_fit();
			}
			Type=CMenuTokenType::MENU_TOGGLE_BIND;
		}
	};
	struct CMenuToken : public MenuToken {
		std::string sName;
		bool bFormatted=true;
		CMenuToken() {}
		CMenuToken(const std::string& sName, const bool& p_bFormatted)
		: sName(sName), bFormatted(p_bFormatted) {
			Type=CMenuTokenType::DECLARE_CMENU;
		}
	};
	struct CMenuEndToken : public MenuToken {
		CMenuEndToken() {
			Type=CMenuTokenType::END_CMENU;
		}
	};
	bool ParseTokens();
}
#endif