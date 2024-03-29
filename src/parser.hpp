#ifndef COMPILER_HPP
#define COMPILER_HPP
#define MAX_TOGGLE_STATES 512
#include <string>
#include <iostream>
#include "token.hpp"

/* Parser state flags*/

#define PARSER_STATE_FORMATTED (1<<0)
#define PARSER_STATE_NOEXIT (1<<1) // Have we parsed the NOEXIT modifier?
#define PARSER_STATE_BIND_KEYSET (1<<2) // Have we parsed a KEY= modifier but not used it yet?
#define PARSER_STATE_EOF_FOUND (1<<6) // Found the EOF?
#define PARSER_STATE_ERRORS_FOUND (1<<7) // Found any errors?

/* Attribute flags */
#define CMTOKATTRIB_FORMATTED (1<<0)
#define CMTOKATTRIB_NOEXIT (1<<1)
#define CMTOKATTRIB_BIND_KEYSET (1<<2)


/* Display flags */
#define FL_DISPLAY_NONE (1<<0)
#define FL_DISPLAY_CONSOLE (1<<1)
#define FL_DISPLAY_CAPTION (1<<2)

// Summary in compiler.cpp definition.
bool ParseMenuTokens(unsigned long long& p_iBindCount, unsigned char& p_bUsedDisplayFlags, std::string& init_defined_code);
namespace Parser {
	struct MenuToken {
		char fAttribs=0; // Attribute flag.
		MenuToken();
		virtual ~MenuToken();
	};
	// Key Values
	struct KVToken : public MenuToken {
		std::string Key, Value;
		KVToken();
		KVToken(const std::string& ident, const std::string& value);
		~KVToken();
	};
	// Bind
	struct BindToken : public MenuToken {
		std::string sName, sCmdStr, /* Should only be set if a KEY="" is located. ---> */ sKey=""; 
		BindToken();
		BindToken(const std::string& p_sName, const std::string& p_sCmdStr, const char& p_fAttributeFlag);
		BindToken(const std::string& p_sKey, const std::string& p_sName, const std::string& p_sCmdStr, const char& p_fAttributeFlag);
		~BindToken();
	};
	// Toggle Bind
	struct ToggleBindToken : public MenuToken {
		unsigned short ToggleStates=0u;
		std::string NameContainer[MAX_TOGGLE_STATES];
		std::string CmdStrContainer[MAX_TOGGLE_STATES];
		std::string sKey=""; // Key to bind this bind to.
		ToggleBindToken();
		ToggleBindToken(const std::string p_names[MAX_TOGGLE_STATES], const std::string p_CmdStrContainer[MAX_TOGGLE_STATES], unsigned short p_iToggleStates, const char& p_fAttributeFlag);
		ToggleBindToken(const std::string& p_sKey, const std::string p_names[MAX_TOGGLE_STATES], const std::string p_CmdStrContainer[MAX_TOGGLE_STATES], unsigned short p_iToggleStates, const char& p_fAttributeFlag);
		~ToggleBindToken();
	};
	// CMenu
	struct CMenuToken : public MenuToken {
		std::string sName;
		std::string sKey=""; // Key to bind this bind to.
		CMenuToken();
		CMenuToken(const std::string& sName, const char& p_fAttributeFlag);
		CMenuToken(const std::string& p_sKey, const std::string& sName, const char& p_fAttributeFlag);
		~CMenuToken();
	};
	
	// Raw code
	struct CodeToken : public MenuToken {
		std::string sValue;
		CodeToken();
		CodeToken(const std::string& p_sValue);
		~CodeToken();
	};
	// CMenu end
	struct CMenuEndToken : public MenuToken {
		CMenuEndToken();
		~CMenuEndToken();
	};
	bool ParseTokens();
}
#endif
