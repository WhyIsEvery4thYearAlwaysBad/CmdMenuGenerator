#ifndef BINDS_HPP
#define BINDS_HPP
#include <vector>
#include "compiler.hpp"
class Bind {
public:
	bool istogglebind=false;
	unsigned char numberkey=0;
	std::vector<std::string> name, formatted_name, /* <---- For captions */ cmdstr;
	Bind();
	Bind(const unsigned char& p_nkey, const Parser::BindToken& t);
	Bind(const unsigned char& p_nkey, const Parser::ToggleBindToken& t);
	Bind(const unsigned char& p_nkey, const std::string& p_name, const std::string& p_cmdstr);
	Bind(const unsigned char& p_nkey, const std::string& p_name, const std::string& p_fname, const std::string& p_cmdstr);
	~Bind();
};
#endif