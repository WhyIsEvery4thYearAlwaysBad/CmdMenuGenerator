#include "binds.hpp"
#include "compiler.hpp"
Bind::Bind() {}
Bind::Bind(const unsigned char& p_nkey, const Parser::BindToken& t)
: numberkey(p_nkey) {
	name.push_back(t.name);
	formatted_name.push_back(Format(t.name));
	if (t.noexit==true) cmdstr.push_back(t.cmdstr);
	else cmdstr.push_back("cvm.exitmenu; cvm.on_page_exit; "+t.cmdstr);
}
Bind::Bind(const unsigned char& p_nkey, const Parser::ToggleBindToken& t)
: istogglebind(true), numberkey(p_nkey) {
	for (unsigned short i=0; i < t.states; i++) {
		name.push_back(t.names[i]);
		formatted_name.push_back(Format(t.names[i]));
		if (t.noexit==true) cmdstr.push_back(t.cmdstrs[i]);
		else cmdstr.push_back("cvm.exitmenu; cvm.on_page_exit; "+t.cmdstrs[i]);
	}
}
Bind::Bind(const unsigned char& p_nkey, const std::string& p_name, const std::string& p_cmdstr)
: numberkey(p_nkey) {
	name.push_back(p_name);
	formatted_name.push_back(Format(p_name));
	cmdstr.push_back(p_cmdstr);
}
Bind::Bind(const unsigned char& p_nkey, const std::string& p_name, const std::string& p_fname, const std::string& p_cmdstr)
: numberkey(p_nkey) {
	name.push_back(p_name);
	formatted_name.push_back(p_fname);
	cmdstr.push_back(p_cmdstr);
}
Bind::~Bind() {
	
}