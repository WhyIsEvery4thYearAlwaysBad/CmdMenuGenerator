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
#include "commandmenu.hpp"
#include "compiler.hpp"
#include "launchoptions.hpp"
extern std::deque<Parser::MenuToken*> menutokens;
extern std::deque<Token> tokens; // {CommandMenu, Depth}
extern std::deque<Token> errors; // 
std::deque<std::pair<CommandMenu,unsigned char> > cmenus; // {CommandMenu, Depth}
std::map<std::string,std::string> keymap={
	{"linger_time","0"},
	{"predisplay_time","0.25"},
	{"format","$(nkey). $(str)<cr>"},
	{"consolemode","false"},
	{"resetkeys","bind 1 slot1; bind 2 slot2; bind 3 slot3; bind 4 slot4; bind 5 slot5; bind 6 slot6; bind 7 slot7; bind 8 slot8; bind 9 slot9; bind 0 slot10"}
};
std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> convert;
// From launch options
extern std::filesystem::path inputfilename;
extern std::filesystem::path outputdir;
int main(int argc, char** argv) {
	unsigned short bindcount=0u;
	if (!EvaluateLaunchOptions(argc,argv)) return -1;
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
	std::filesystem::create_directories(outputdir.string()+"/cfg/cmenu");
	// Main exec.
	std::ofstream exec(outputdir.string()+"/cfg/cmenu_initialize.cfg");
	// Console mode makes the Voicemenu use the console for displaying binds, instead of captions.
	bool consolemode=(keymap["consolemode"].find("true")!=std::string::npos);
	if (consolemode) {
		exec<<R"(alias _cmenu.nullkeys "alias _cmenu.1 ; alias _cmenu.2 ; alias _cmenu.3 ; alias _cmenu.4 ; alias _cmenu.5 ; alias _cmenu.6 ; alias _cmenu.7 ; alias _cmenu.8 ; alias _cmenu.9 ; alias _cmenu.0"
alias cmenu.exitmenu "developer 0; )"<<keymap["resetkeys"]<<"; cmenu.on_exitmenu\"";
		exec<<R"(
alias _cmenu.menusettings "developer 1; clear; bind 1 _cmenu.1; bind 2 _cmenu.2; bind 3 _cmenu.3; bind 4 _cmenu.4; bind 5 _cmenu.5; bind 6 _cmenu.6; bind 7 _cmenu.7; bind 8 _cmenu.8; bind 9 _cmenu.9; bind 0 _cmenu.0"
alias cmenu.on_exitmenu ;
alias cmenu.on_page_exit ;
cmenu.exitmenu
)";
	std::string cfgpath="";
	std::size_t cmi=0u;
	unsigned long togglenumber=0u;
	for (auto commandmenu=cmenus.begin(); commandmenu!=cmenus.end(); commandmenu++, cmi++) {
			unsigned long segmentnumber=0u;
			cfgpath=outputdir.string()+"/cfg/$cmenu_"+commandmenu->first.formatted_title+".cfg";
			std::ofstream cfgfile(cfgpath);
			// Write to cfg
			cfgfile<<"_cmenu.menusettings\n_cvm.nullkeys\n";
			for (auto& kbind : commandmenu->first.binds) {
				if (kbind.istogglebind) {
					exec<<"alias _cmenu.toggle_"<<std::to_string(togglenumber)<<" _cmenu.toggle_"<<std::to_string(togglenumber)<<"_0\n";
					exec<<"alias _#cmenu.toggle_"<<std::to_string(togglenumber)<<" _#cmenu.toggle_"<<std::to_string(togglenumber)<<"_0\n";
					for (unsigned char sti=0u; sti < kbind.name.size(); sti++) {
						exec<<"alias _cmenu.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string(sti)<<'\"'<<kbind.cmdstr.at(sti)<<"; alias _cmenu.toggle_"<<std::to_string(togglenumber)<<" _cmenu.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string((sti+1)%kbind.name.size())<<"; alias _#cmenu.toggle_"<<std::to_string(togglenumber)<<" _#cmenu.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string((sti+1)%kbind.name.size())<<"\"\n";
						exec<<"alias _#cmenu.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string(sti)<<"\"echo "<<kbind.name.at(sti)<<"\"\n";
					}
					cfgfile<<"_#cmenu.toggle_"<<std::to_string(togglenumber)<<'\n';
					cfgfile<<"alias _cmenu."<<std::to_string(kbind.numberkey)<<" \"_cmenu.toggle_"<<std::to_string(togglenumber)<<"\"\n";
					togglenumber++;
				}
				else {
					cfgfile<<"echo "<<kbind.name.front()<<'\n'<<"alias _cmenu."<<std::to_string(kbind.numberkey)<<" \""<<kbind.cmdstr.front()<<"\"\n";;
				}
			}
		}
	}
	else {
	exec<<R"(closecaption 1
cc_lang commandmenu
alias _cmenu.nullkeys "alias _cmenu.1 ; alias _cmenu.2 ; alias _cmenu.3 ; alias _cmenu.4 ; alias _cmenu.5 ; alias _cmenu.6 ; alias _cmenu.7 ; alias _cmenu.8 ; alias _cmenu.9; alias _cmenu.0"
alias cmenu.exitmenu "cc_emit _#cmenu.clear_screen; )"<<keymap["resetkeys"]<<"; cc_linger_time "<<keymap["linger_time"]<<"; cc_predisplay_time "<<keymap["predisplay_time"]<<"; cmenu.on_exitmenu\"";
	exec<<R"(
alias _cmenu.menusettings "cc_linger_time 10000; cc_predisplay_time 0; bind 1 _cmenu.1; bind 2 _cmenu.2; bind 3 _cmenu.3; bind 4 _cmenu.4; bind 5 _cmenu.5; bind 6 _cmenu.6; bind 7 _cmenu.7; bind 8 _cmenu.8; bind 9 _cmenu.9; bind 0 _cmenu.0"
alias cmenu.on_exitmenu ;
alias cmenu.on_page_exit ;
cmenu.exitmenu
)";
	// Conversion to UCS-2.
	std::locale utf16(std::locale::classic(),new std::codecvt_utf16<wchar_t, 0xffff, std::little_endian>);
	// Create the captions directory once.
	if (!std::filesystem::exists(outputdir.string()+"/resource")) std::filesystem::create_directories(outputdir.string()+"/resource");
	std::wofstream captionfile(outputdir.string()+"/resource/closecaption_commandmenu.txt",std::ios_base::binary);
	captionfile.imbue(utf16);
	captionfile<<(wchar_t)0xFEFF; // BOM.
	captionfile<<convert.from_bytes("\"lang\"\n{\n\t\"Language\" \"commandmenu\"\n\t\"Tokens\"\n\t{\n\t\t\"_#cmenu.clear_screen\" \"<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	\"\n");
	std::string cfgpath;
	std::size_t cmi=0u;
	unsigned long togglenumber=0u;
	for (auto commandmenu=cmenus.begin(); commandmenu!=cmenus.end(); commandmenu++, cmi++) {
		unsigned long segmentnumber=0u;
		cfgpath=outputdir.string()+"/cfg/$cmenu_"+commandmenu->first.formatted_title+".cfg";
		std::ofstream cfgfile(cfgpath);
		// Write to cfg
		cfgfile<<"_cmenu.menusettings\ncc_emit _#cmenu.clear_screen\ncc_emit _#cmenu."+commandmenu->first.formatted_title+'\n';
		{
			unsigned long temptogglenumber=togglenumber;
			for (auto kbind=commandmenu->first.binds.begin(); kbind!=commandmenu->first.binds.end(); kbind++) {
				if (kbind->istogglebind==true) {
					cfgfile<<"_#cmenu.toggle_"+std::to_string(temptogglenumber)<<'\n';
					temptogglenumber++;
				}
				
				if (kbind!=commandmenu->first.binds.begin() && kbind->istogglebind==false && (kbind-1)->istogglebind==true) {
					cfgfile<<"cc_emit _#cmenu."<<commandmenu->first.formatted_title<<"_seg_"<<std::to_string(segmentnumber)<<'\n';
					segmentnumber++;
				}
			}
			segmentnumber=0u;
		}
		cfgfile<<"_cmenu.nullkeys\n";
		// Write captions
		for (auto kbind=commandmenu->first.binds.begin(); kbind!=commandmenu->first.binds.end(); kbind++) {
			if (kbind==commandmenu->first.binds.begin() && kbind->istogglebind!=true)
				captionfile<<convert.from_bytes("\t\t\"_#cmenu."+commandmenu->first.formatted_title+"\" \"");
			if (kbind->istogglebind==true) {
				exec<<"alias _cmenu.toggle_"<<std::to_string(togglenumber)<<" _cmenu.toggle_"<<std::to_string(togglenumber)<<"_0\n";
				exec<<"alias _#cmenu.toggle_"<<std::to_string(togglenumber)<<" _#cmenu.toggle_"<<std::to_string(togglenumber)<<"_0\n";
				for (unsigned char sti=0u; sti < kbind->name.size(); sti++) {
					exec<<"alias _cmenu.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string(sti)<<"\"alias _#cmenu.toggle_"<<std::to_string(togglenumber)<<" _#cmenu.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string((sti+1)%kbind->name.size())<<';'<<kbind->cmdstr.at(sti)<<"; alias _cmenu.toggle_"<<std::to_string(togglenumber)<<" _cmenu.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string((sti+1)%kbind->name.size())<<"\"\n";
					exec<<"alias _#cmenu.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string(sti)<<"\"cc_emit _#cmenu.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string(sti)<<"\"\n";
					captionfile<<convert.from_bytes("\t\t\"_#cmenu.toggle_"+std::to_string(togglenumber)+'_'+std::to_string(sti)+"\" \""+kbind->name.at(sti)+"\"\n");
				}
				cfgfile<<"alias _cmenu."<<std::to_string(kbind->numberkey)<<" \"_cmenu.toggle_"<<std::to_string(togglenumber)<<"\"\n";
				togglenumber++;
			}
			else {
				if (kbind!=commandmenu->first.binds.begin() && (kbind-1)->istogglebind==true) {
					captionfile<<convert.from_bytes("\t\t\"_#cmenu."+commandmenu->first.formatted_title+"_seg_"+std::to_string(segmentnumber)+"\" \"");
					segmentnumber++;
				}
				captionfile<<convert.from_bytes(kbind->name.front());
				if (kbind==commandmenu->first.binds.end()-1 || (kbind+1)->istogglebind==true) captionfile<<L"\"\n";
				cfgfile<<"alias _cmenu."<<std::to_string(kbind->numberkey)<<" \""<<kbind->cmdstr.at(0)<<"\"\n";
			}
		}
	}
	captionfile<<"\t}\n}";
	captionfile.close();
	}
	exec<<"\necho [Command Menu Generator] Initialized command menus!";
	exec.close();
	//Done!
	std::cout<<std::to_string(bindcount)<<" binds compiled.\n";
}
