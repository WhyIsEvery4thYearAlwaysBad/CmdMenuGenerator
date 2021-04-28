#ifndef BINDS_HPP
#define BINDS_HPP
#include <vector>
#include "compiler.hpp"
class Bind {
public:
	bool bToggleBind=false;
	std::string sKey="";
	std::vector<std::string> RawNameContainer /* <---- For caption identification.*/, NameContainer, /* <---- For captions */ CmdStrContainer;
	Bind();
	Bind(const std::string& p_sKey, const Parser::BindToken& p_Token);
	Bind(const std::string& p_sKey, const Parser::ToggleBindToken& p_Token);
	Bind(const std::string& p_sKey, const std::string& p_sRawName, const std::string& p_sCmdStr);
	Bind(const std::string& p_sKey, const std::string& p_sRawName, const std::string& p_sName, const std::string& p_sCmdStr);
	~Bind();
};
#endif