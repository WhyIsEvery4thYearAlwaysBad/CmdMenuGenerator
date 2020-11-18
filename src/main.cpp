#define	MAX_ALIAS_NAME 32
#include <regex>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <codecvt>
#include <map>
#include <cstdio>
#include <windows.h>
#include <locale>
#include "tokens.hpp"
#include "page.hpp"
#include "compiler.hpp"
#include "launchoptions.hpp"
extern std::deque<Parser::MenuToken*> menutokens;
extern std::deque<Token> tokens; // {Page, Depth}
extern std::deque<Token> errors; // 
const std::string badfpcharacters="|?\"\\:*<>";
std::deque<std::pair<Page,unsigned char> > pages; // {Page, Depth}
std::map<std::string,std::string> keymap={{"#cvm.linger_time","0"},{"#cvm.predisplay_time","0.25"},{"#cvm.defaultcolor",""},{"#cvm.boldbydefault","false"},{"#cvm.italicizebydefault","false"},{"#cvm.resetkeys","bind 1 slot1; bind 2 slot2; bind 3 slot3; bind 4 slot4; bind 5 slot5; bind 6 slot6; bind 7 slot7; bind 8 slot8; bind 9 slot9; bind 0 slot10"},{"#cvm.resetkeys.scout",""},{"#cvm.resetkeys.soldier",""},{"#cvm.resetkeys.pyro",""},{"#cvm.resetkeys.demoman",""},{"#cvm.resetkeys.heavy",""},{"#cvm.resetkeys.engineer",""},{"#cvm.resetkeys.medic",""},{"#cvm.resetkeys.sniper",""},{"#cvm.resetkeys.spy",""}};
std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> convert;
// From launch options
extern const char* inputfilename;
extern const char* ModName;
extern bool launchoptionfilefound;
extern bool launchoptionhelp;
extern std::string outputdir;
bool overrideexists=false;
int main(int argc, char** argv) {
	unsigned short bindcount=0u;
	if (!EvaluateLaunchOptions(argc,argv)) return -1;
	if (argc<=1 || launchoptionhelp==true || launchoptionfilefound==false) {
		ShowHelp();
		return -1;
	}
	
	// bool overrideexists=false;
	std::string Line, InFileContent; 
	std::ifstream inputf(inputfilename,std::ios_base::binary);
	while (std::getline(inputf,Line)) {
		InFileContent+=Line+'\n';
	}
	if (!Tokenize(InFileContent) || !Parser::ParseTokens()) {
		for (auto& e : errors) {
			std::cout<<e.val<<'\n';
		}
		return -1;
	}
	// Creating parts from Menu tokens.
	MenuCreate(bindcount);
	// Directories
	std::filesystem::create_directories(outputdir+"/cfg");
	// For a fix relating to multiple menus.
	std::ofstream multimenu_fix(outputdir+"/cfg/_cvm_multimenu_fix.cfg");
	// Main exec.
	std::ofstream exec(outputdir+"/cfg/activate_cvm.cfg");
	exec<<R"(closecaption 1
cc_lang _customvoicemenu
alias _cvm.nullkeys "alias _cvm.1 ; alias _cvm.2 ; alias _cvm.3 ; alias _cvm.4 ; alias _cvm.5 ; alias _cvm.6 ; alias _cvm.7 ; alias _cvm.8 ; alias _cvm.9"
alias cvm.exitmenu "cc_emit _#cvm.clear_screen; alias +cvm.openmenu _cvm.cvmstate0; _cvm.resetkeys; cc_linger_time )"<<keymap["#cvm.linger_time"]<<"; cc_predisplay_time "<<keymap["#cvm.predisplay_time"]<<'\"';
	exec<<R"(
alias +cvm.openmenu _cvm.cvmstate0
alias -cvm.openmenu ;
alias _cvm.cvmstate0 "cc_linger_time 10000; cc_predisplay_time 0; _cvm.openmenu; alias -cvm.openmenu alias +cvm.openmenu _cvm.cvmstate1; bind 1 _cvm.1; bind 2 _cvm.2; bind 3 _cvm.3; bind 4 _cvm.4; bind 5 _cvm.5; bind 6 _cvm.6; bind 7 _cvm.7; bind 8 _cvm.8; bind 9 _cvm.9"
alias _cvm.cvmstate1 "cvm.exitmenu; alias -cvm.openmenu alias +cvm.openmenu _cvm.cvmstate0"
cvm.exitmenu
alias _cvm.resetkeys ")"<<keymap["#cvm.resetkeys"]<<"\"\n";
	{
		std::size_t i=0;
		for (auto& page : pages) {
			// Use files instead of aliases since the length limit hurts this program with aliases.
			std::ofstream selectmenucmd(outputdir+"/cfg/menu="+page.first.formatted_title+".cfg");
			selectmenucmd<<"_cvm.mmenu_fix"<<i<<"\nalias _cvm.openmenu \"exec _cvm_page_"<<page.first.formatted_title;
			multimenu_fix<<"alias _cvm.mmenu_fix"<<i<<"\"alias +cvm.openmenu _cvm.cvmstate0\"\n";
			i++;
		}
	}
	exec << "alias _cvm.openmenu ;\n";
	// Conversion to UCS-2.
	std::locale utf16(std::locale::classic(),new std::codecvt_utf16<wchar_t, 0xffff, std::little_endian>);
	std::cout<<"Done. Creating caption file.\n";
	// Create the captions directory once.
	if (!std::filesystem::exists(outputdir+"captions")) std::filesystem::create_directories(outputdir+"captions/resource");
	std::wofstream captionfile(outputdir+"captions/resource/closecaption__customvoicemenu.txt",std::ios_base::binary);
	captionfile.imbue(utf16);
	captionfile<<(wchar_t)0xFEFF; // BOM.
	captionfile<<"\"lang\"\n{\n\t\"Language\" \"_customvoicemenu\"\n\t\"Tokens\"\n\t{\n\t\t\"_#cvm.clear_screen\" \"<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	\"\n";
	std::string cfgpath;
	std::cout<<"Done. Creating CFGs and Captions.\n";
	std::size_t pi=0u;
	unsigned long togglenumber=0u;
	for (auto page=pages.begin(); page!=pages.end(); page++, pi++) {
		unsigned long segmentnumber=0u;
		cfgpath=outputdir+"/cfg/$pageopen_"+page->first.formatted_title+".cfg";
		// If a page exists with the same name, append a number to the end.
		std::ofstream cfgfile(cfgpath);
		// Write to cfg
		cfgfile<<"cc_emit _#cvm.clear_screen\ncc_emit _#cvm."+page->first.formatted_title+'\n';
		{
			unsigned long temptogglenumber=togglenumber;
			for (auto kbind=page->first.binds.begin(); kbind!=page->first.binds.end(); kbind++) {
				if (kbind->istogglebind==true) {
					cfgfile<<"_#cvm.toggle_"+std::to_string(temptogglenumber)<<'\n';
					temptogglenumber++;
				}
				
				if (kbind!=page->first.binds.begin() && kbind->istogglebind==false && (kbind-1)->istogglebind==true) {
					cfgfile<<"_#cvm."<<page->first.formatted_title<<"_seg_"<<std::to_string(segmentnumber)<<'\n';
					segmentnumber++;
				}
			}
			segmentnumber=0u;
		}
		cfgfile<<"_cvm.nullkeys\n";
		// Write captions
		if (page->first.binds.size()>0 && page->first.binds.front().name.size()<2) {
			captionfile<<convert.from_bytes("\t\t\"_#cvm."+page->first.formatted_title+"\" \"");
			if (keymap["#cvm.boldbydefault"].find("true")!=std::string::npos) captionfile<<"<B>";
			if (keymap["#cvm.italicizedbydefault"].find("true")!=std::string::npos) captionfile<<"<I>";
			captionfile<<convert.from_bytes(keymap["#cvm.defaultcolor"]);
		}
		for (auto kbind=page->first.binds.begin(); kbind!=page->first.binds.end(); kbind++) {
			if (kbind->istogglebind==true) {
				exec<<"alias _cvm.toggle_"<<std::to_string(togglenumber)<<" _cvm.toggle_"<<std::to_string(togglenumber)<<"_0\n";
				exec<<"alias _#cvm.toggle_"<<std::to_string(togglenumber)<<" _#cvm.toggle_"<<std::to_string(togglenumber)<<"_0\n";
				for (unsigned char sti=0u; sti < kbind->name.size(); sti++) {
					exec<<"alias _cvm.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string(sti)<<'\"'<<kbind->cmdstr.at(sti)<<"; alias _cvm.toggle_"<<std::to_string(togglenumber)<<" _cvm.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string((sti+1)%kbind->name.size())<<"; alias _#cvm.toggle_"<<std::to_string(togglenumber)<<" _#cvm.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string((sti+1)%kbind->name.size())<<"\"\n";
					exec<<"alias _#cvm.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string(sti)<<"\"cc_emit _#cvm.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string(sti)<<"\"\n";
					captionfile<<convert.from_bytes("\t\t\"_#cvm.toggle_"+std::to_string(togglenumber)+'_'+std::to_string(sti)+"\" \""+(keymap["#cvm.boldbydefault"].find("true")!=std::string::npos ? "<B>" : "")+(keymap["#cvm.italicizedbydefault"].find("true")!=std::string::npos ? "<I>" : "")+keymap["#cvm.defaultcolor"]+std::to_string(kbind->numberkey)+". "+kbind->name.at(sti)+"\"\n");
				}
				cfgfile<<"alias _cvm."<<std::to_string(kbind->numberkey)<<" \"_cvm.toggle_"<<std::to_string(togglenumber)<<"; cvm.exitmenu\"\n";
				togglenumber++;
			}
			else {
				if (kbind!=page->first.binds.begin() && (kbind-1)->istogglebind==true) {
					captionfile<<convert.from_bytes("\t\t\"_#cvm."+page->first.formatted_title+"_seg_"+std::to_string(segmentnumber)+"\" \"");
					segmentnumber++;
				}
				captionfile<<convert.from_bytes(std::to_string(kbind->numberkey)+". "+kbind->name.at(0));
				if (kbind==page->first.binds.end()-1 || (kbind+1)->istogglebind==true) captionfile<<L"\"\n";
				else captionfile<<L"<cr>";
				cfgfile<<"alias _cvm."<<std::to_string(kbind->numberkey)<<" \""<<kbind->cmdstr.at(0)<<"\"\n";
			}
		}
		// FIX for pressing buttons
		cfgfile<<"\nexec _cvm_multimenu_fix\nalias _cvm.mmenu_fix"<<page-pages.begin()<<" ;\n";
		auto temp=pi;
		if (pages.at(temp).second>0) {
			temp=GetParentPage(pages,pi,0);
			cfgfile<<"alias _cvm.mmenu_fix"<<temp<<" ;";
		}
	}
	captionfile<<"\t}\n}";
	exec.close(), multimenu_fix.close(), captionfile.close();
	//Done!
	std::cout<<"Done! Now just insert exec activate_cvm into autoexec, compile the captions, and preferably convert "<<std::filesystem::path(outputdir).filename()<<" to a vpk.\n";
}
