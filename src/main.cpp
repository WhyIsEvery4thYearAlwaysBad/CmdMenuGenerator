#include <regex>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <codecvt>
#include <map>
#include <cstdio>
#include <windows.h>
#include <locale>
#include <vector>
#include "tokens.hpp"
#include "launchoptions.hpp"
#define GetNextChar(str,i,c) str.find(c,i+1)-i
std::vector<std::pair<Page,unsigned char> > pages; // {Page, Depth}
const std::string badfpcharacters="|?\"\\:*<>";
std::map<std::string,std::string> keymap={{"#cvm.defaultcolor",""},{"#cvm.boldbydefault","false"},{"#cvm.italicizebydefault","false"},{"#cvm.resetkeys","\"bind 1 slot1; bind 2 slot2; bind 3 slot3; bind 4 slot4; bind 5 slot5; bind 6 slot6; bind 7 slot7; bind 8 slot8; bind 9 slot9; bind 0 slot10\""},{"#cvm.resetkeys.scout",""},{"#cvm.resetkeys.soldier",""},{"#cvm.resetkeys.pyro",""},{"#cvm.resetkeys.demoman",""},{"#cvm.resetkeys.heavy",""},{"#cvm.resetkeys.engineer",""},{"#cvm.resetkeys.medic",""},{"#cvm.resetkeys.sniper",""},{"#cvm.resetkeys.spy",""}};
std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> convert;
// From launch options
extern const char* inputfilename;
extern const char* ModName;
extern bool usingconfig;
extern bool launchoptionfilefound;
extern bool launchoptionhelp;
extern std::string outputdir;
int main(int argc, char** argv) {
	if (!EvaluateLaunchOptions(argc,argv)) return -1;
	if (argc<=1 || launchoptionhelp==true || launchoptionfilefound==false) {
		ShowHelp();
		return -1;
	}
	
	bool overrideexists=false;
	// The real thing
	bool concise=false;
	std::string Line, InFileContent; 
	std::ifstream inputf(inputfilename,std::ios_base::binary);
	unsigned char depth=0;
	while (std::getline(inputf,Line)) {
		InFileContent+=Line+'\n';
	}
	std::cout<<"Tokenizing Stream.\n";
	Tokenize(InFileContent,depth);
	std::cout<<"Tokenized Stream. Parsing Tokens.\n";
	unsigned long bindcount=0u;
	if (!ParseTokens(pages,concise,depth,bindcount)) return -1;
	std::cout<<"Parsed Tokens. ("<<bindcount<<" binds compiled.) \nCreating main cfg.\n";
	if (depth!=0) {std::cout<<"Missing a brace somewhere...";return -1;}
	// Directories
	std::filesystem::create_directories(outputdir+"/cfg");
	// For a fix relating to multiple menus.
	std::ofstream multimenu_fix(outputdir+"/cfg/cvm_multimenu_fix.cfg");
	// Main exec.
	std::ofstream exec(outputdir+"/cfg/activate_cvm.cfg");
	exec<<R"(closecaption 1
cc_lang customvoicemenu
alias _cvm.nullkeys "alias cvm.1 ; alias cvm.2 ; alias cvm.3 ; alias cvm.4 ; alias cvm.5 ; alias cvm.6 ; alias cvm.7 ; alias cvm.8 ; alias cvm.9"
alias _cvm.exitmenu "cc_linger_time 0; cc_emit #cvm.clear_screen; alias +cvm.opencvm _cvm.cvmstate0; _cvm.resetkeys"
alias +cvm.opencvm _cvm.cvmstate0
alias -cvm.opencvm ;
alias _cvm.cvmstate0 "_cvm.openmenu; alias -cvm.opencvm alias +cvm.opencvm _cvm.cvmstate1; bind 1 cvm.1; bind 2 cvm.2; bind 3 cvm.3; bind 4 cvm.4; bind 5 cvm.5; bind 6 cvm.6; bind 7 cvm.7; bind 8 cvm.8; bind 9 cvm.9"
alias _cvm.cvmstate1 "_cvm.exitmenu; alias -cvm.opencvm alias +cvm.opencvm _cvm.cvmstate0"
_cvm.exitmenu
alias _cvm.resetkeys )"<<keymap["#cvm.resetkeys"]<<'\n';
	{
		std::size_t i=0;
		for (auto& page : pages) {
		exec<<"alias cvm.menu_"<<page.first.formatted_title<<" \"_cvm.mmenu_fix"<<i<<"; alias _cvm.openmenu _cvm.menu_"<<page.first.formatted_title<<"\"\n";
		exec<<"alias _cvm.menu_"<<page.first.formatted_title<<" \"exec cvm_"<<page.first.formatted_title<<"\""<<'\n';
		multimenu_fix<<"alias _cvm.mmenu_fix"<<i<<"\"alias +cvm.opencvm _cvm.cvmstate0\"\n";
		i++;
		}
	}
	exec << "alias cvm.openmenu ;\n";
	// Write override reset keys to the class configs if possible. If -config is present, print to console.
	if (usingconfig==true) {
		if (overrideexists==true) for (auto& s : keymap) {
			std::cout<<"Enter the reset key overrides into your class cfgs:\n";
			if (s.first.find("#cvm.resetkeys.")==0 && s.second!="") {
				std::cout<<"alias "<<s.first.c_str()<<' '<<s.second.c_str()<<'\n';
			}
		}
	}
	else {
		if (overrideexists==true) for (auto& s : keymap) {
			if (s.first.substr(0,14)=="#cvm.resetkeys." && s.second!="") {
				std::ofstream classcfg(outputdir+"/cfg/"+s.first.substr(14,s.first.length()-14)+".cfg");
				classcfg<<"alias "<<s.first<<' '<<s.second<<'\n';
			}
		}
	}
	// Conversion to UCS-2.
	std::locale utf16(std::locale::classic(),new std::codecvt_utf16<wchar_t, 0xffff, std::little_endian>);
	std::cout<<"Done. Creating caption file.\n";
	// Create the captions directory once.
	if (!std::filesystem::exists(outputdir+"captions")) std::filesystem::create_directories(outputdir+"captions/resource");
	std::wofstream captionfile(outputdir+"captions/resource/closecaption_customvoicemenu.txt",std::ios_base::binary);
	captionfile.imbue(utf16);
	captionfile<<(wchar_t)0xFEFF; // BOM.
	captionfile<<"\"lang\"\n{\n\t\"Language\" \"customvoicemenu\"\n\t\"Tokens\"\n\t{\n\t\t\"#cvm.clear_screen\" \"<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	\"\n";
	unsigned long togglenumber=0u;
	std::string cfgpath;
	std::cout<<"Done. Creating CFGs and Captions.\n";
	// Macro to reduce the method path names.
	#define GetBind(iter) pages.at(pi).first.binds.at(iter)
	for (unsigned int pi=0; pi < pages.size(); pi++) {
		cfgpath=outputdir+"/cfg/cvm_"+pages.at(pi).first.formatted_title+".cfg";
		// If a page exists with the same name, append a number to the end.
		std::ofstream cfgfile(cfgpath);
		// Write to cfg
		cfgfile<<"cc_emit #cvm.clear_screen\ncc_emit #cvm."+pages.at(pi).first.formatted_title+'\n';
		unsigned long temptogglenumber=togglenumber;
		for (std::size_t i=0; i < pages.at(pi).first.binds.size(); i++) {
			if (GetBind(i).name.size()>1) {
				temptogglenumber++;
				cfgfile<<"#cvm.toggle_"+std::to_string(temptogglenumber)+'\n';
			}
		}
		cfgfile<<"_cvm.nullkeys\n";
		// Write captions
		if (pages.at(pi).first.binds.front().name.size()<2) {
			captionfile<<convert.from_bytes("\t\t\"#cvm."+pages.at(pi).first.formatted_title+"\" \"");
			if (keymap["#cvm.italicizedbydefault"].find("true")!=std::string::npos) captionfile<<"<I>";
			if (keymap["#cvm.boldbydefault"].find("true")!=std::string::npos) captionfile<<"<B>";
			captionfile<<convert.from_bytes(keymap["#cvm.defaultcolor"]);
		}
		for (unsigned int i=0; i < pages.at(pi).first.binds.size(); i++) {
			if (GetBind(i).name.size()<2) {
				if (i!=0 && pages.at(pi).first.binds.at(i-1).name.size()>1) captionfile<<convert.from_bytes("\t\t\"#cvm."+GetBind(i).formatted_name.at(0)+"\" \"");
				captionfile<<convert.from_bytes(std::to_string(GetBind(i).numberkey)+". "+GetBind(i).name.at(0));
				captionfile<<((i==pages.at(pi).first.binds.size()-1 || pages.at(pi).first.binds.at(i+1).name.size()>1) ? L"" : L"<cr>");
				if (GetBind(i).name.at(0).find("<clr:")!=std::string::npos) captionfile<<convert.from_bytes(keymap["#cvm.defaultcolor"]);
				captionfile<<((i==pages.at(pi).first.binds.size()-1 || pages.at(pi).first.binds.at(i+1).name.size()>1) ? L"\"\n" : L"");
				// Write to cfg
				cfgfile<<"alias cvm."<<std::to_string(GetBind(i).numberkey)<<" \""+GetBind(i).commandstr.at(0)+"\"\n";
			}
			else {
				togglenumber++;
				std::string togglename="cvm.toggle_"+std::to_string(togglenumber);
				std::string togglestatename;
				for (std::size_t subi=0; subi<GetBind(i).name.size(); subi++) {
				captionfile<<convert.from_bytes("\t\t\"#cvm."+GetBind(i).formatted_name.at(subi)+"\"\t\"");
				if (keymap["#cvm.boldbydefault"].find("true")!=std::string::npos) captionfile<<"<B>";
				if (keymap["#cvm.italicizedbydefault"].find("true")!=std::string::npos) captionfile<<"<I>";
				captionfile<<convert.from_bytes(keymap["#cvm.defaultcolor"]+std::to_string(GetBind(i).numberkey)+". "+GetBind(i).name.at(subi)+"\"\n");
				// Generate toggle
				// alias cvm.toggle_#_# commandstr#
				// alias cvm.toggle_# cvm.toggle_#_1
				// alias #cvm.toggle_# #cvm.toggle_#_1
				togglestatename=togglename+'_'+std::to_string(subi);
				exec<<"alias "+togglestatename+" \""+GetBind(i).commandstr.at(subi)+"; alias "+togglename+' '+togglename+'_'+std::to_string((subi+1) % GetBind(i).name.size())<<"; alias #"+togglename+" #"+togglename+'_'+std::to_string((subi+1) % GetBind(i).name.size())+"\"\n";
				exec<<"alias #"+togglestatename+" \"cc_emit #cvm."+GetBind(i).formatted_name.at(subi)+"\"\n";
				}
				exec<<"alias "+togglename+' '+togglename+"_0\n";
				exec<<"alias #"+togglename+" #"+togglename+"_0\n";
				cfgfile<<"alias cvm."<<std::to_string(GetBind(i).numberkey)<<" \""+togglename;
				for (std::size_t subi=0; subi<GetBind(i).name.size(); subi++) {
				if (GetBind(i).commandstr.at(subi).find("exec cvm_")==std::string::npos) {
					cfgfile<<"; cvm.exitmenu\"\n";
					break;
				}
				if (subi==GetBind(i).name.size()-1) cfgfile<<"\"\n";
				}
			}
		}
		// FIX for pressing buttons
		cfgfile<<"\nexec cvm_multimenu_fix\nalias _cvm.mmenu_fix"<<pi<<" ;";
		std::cout<<pages.at(pi).first.formatted_title<<'\n';
	}
	captionfile<<"\t}\n}";
	exec.close(), multimenu_fix.close(), captionfile.close();
	//Done!
	std::cout<<"Done! Now just insert exec activate_cvm into autoexec, compile the captions, and convert "<<std::filesystem::path(outputdir).filename()<<" to a vpk.\n";
}
