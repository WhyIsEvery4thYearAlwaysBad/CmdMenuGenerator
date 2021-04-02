#include "binds.hpp"
#include "compiler.hpp"
#include <map>
extern std::map<std::string,std::string> KVMap;
Bind::Bind() {}
Bind::Bind(const unsigned char& p_cKey, const Parser::BindToken& p_Token)
: cKey(p_cKey) {
	if (p_Token.bFormatted) {
		std::string sFormatVal=KVMap["format"];
		if (sFormatVal.find("$(nkey)")!=std::string::npos) {
			std::size_t pos=sFormatVal.find("$(nkey)");
			while (pos!=std::string::npos) {
				sFormatVal.replace(pos,7,std::to_string(cKey));
				pos=sFormatVal.find("$(nkey)",pos);
			}
		}
		if (sFormatVal.find("$(str)")!=std::string::npos) {
			std::size_t pos=sFormatVal.find("$(str)");
			while (pos!=std::string::npos) {
				sFormatVal.replace(pos,6,p_Token.sName);
				pos=sFormatVal.find("$(str)",pos);
			}
		}
		NameContainer.push_back(sFormatVal);
	}
	else NameContainer.push_back(p_Token.sName);
	RawNameContainer.push_back(formatRaw(p_Token.sName));
	if (p_Token.bNoExit==true) CmdStrContainer.push_back(p_Token.sCmdStr);
	else CmdStrContainer.push_back("cmenu.exitmenu; cmenu.on_cmenu_exit; "+p_Token.sCmdStr);
}
Bind::Bind(const unsigned char& p_cKey, const Parser::ToggleBindToken& p_Token)
: bToggleBind(true), cKey(p_cKey) {
	for (unsigned short i=0; i < p_Token.ToggleStates; i++) {
		if (p_Token.bFormatted) {
			std::string sFormatVal=KVMap["format"];
			if (sFormatVal.find("$(nkey)")!=std::string::npos) {
				std::size_t pos=sFormatVal.find("$(nkey)");
				while (pos!=std::string::npos) {
					sFormatVal.replace(pos,7,std::to_string(cKey));
					pos=sFormatVal.find("$(nkey)",pos);
				}
			}
			if (sFormatVal.find("$(str)")!=std::string::npos) {
				std::size_t pos=sFormatVal.find("$(str)");
				while (pos!=std::string::npos) {
					sFormatVal.replace(pos,6,p_Token.NameContainer[i]);
					pos=sFormatVal.find("$(str)",pos);
				}
			}
			NameContainer.push_back(sFormatVal);
		}
		else NameContainer.push_back(p_Token.NameContainer[i]);
		RawNameContainer.push_back(formatRaw(p_Token.NameContainer[i]));
		if (p_Token.bNoExit==true) CmdStrContainer.push_back(p_Token.CmdStrContainer[i]);
		else CmdStrContainer.push_back("cmenu.exitmenu; cmenu.on_page_exit; "+p_Token.CmdStrContainer[i]);
	}
}
Bind::Bind(const unsigned char& p_cKey, const std::string& p_sName, const std::string& p_sCmdStr)
: cKey(p_cKey) {
	RawNameContainer.push_back(formatRaw(p_sName));
	NameContainer.push_back(p_sName);
	CmdStrContainer.push_back(p_sCmdStr);
}
Bind::Bind(const unsigned char& p_cKey, const std::string& p_sRawName, const std::string& p_sName, const std::string& p_sCmdStr)
: cKey(p_cKey) {
	NameContainer.push_back(p_sRawName);
	RawNameContainer.push_back(p_sName);
	CmdStrContainer.push_back(p_sCmdStr);
}
Bind::~Bind() {
	
}