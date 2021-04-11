#ifndef BINDS_HPP
#define BINDS_HPP
#include <vector>
#include "compiler.hpp"
class Bind {
public:
	bool bToggleBind=false;
	unsigned char cKey=0;
	std::vector<std::string> RawNameContainer /* <---- For caption identification.*/, NameContainer, /* <---- For captions */ CmdStrContainer;
	Bind();
	Bind(const unsigned char& p_cKey, const Parser::BindToken& p_Token);
	Bind(const unsigned char& p_cKey, const Parser::ToggleBindToken& p_Token);
	Bind(const unsigned char& p_cKey, const std::string& p_sRawName, const std::string& p_sCmdStr);
	Bind(const unsigned char& p_cKey, const std::string& p_sRawName, const std::string& p_sName, const std::string& p_sCmdStr);
	~Bind();
};
#endif