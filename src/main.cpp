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
std::deque<std::pair<Page,unsigned char> > pages; // {Page, Depth}
std::map<std::string,std::string> keymap={
	{"linger_time","0"},
	{"predisplay_time","0.25"},
	{"format","$(nkey). $(str)"},
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
	std::cout<<"Compiling.\n";
	if (!Tokenize(InFileContent) || !Parser::ParseTokens()) {
		for (auto& e : errors) {
			std::cout<<e.val<<'\n';
		}
		return -1;
	}
	std::cout<<"Done. Creating Menus.\n";
	// Creating parts from Menu tokens.
	MenuCreate(bindcount);
	// Directories
	std::filesystem::create_directories(outputdir.string()+"/cfg");
	// Main exec.
	std::ofstream exec(outputdir.string()+"/cfg/activate_cvm.cfg");
	// Console mode makes the Voicemenu use the console for displaying binds, instead of captions.
	bool consolemode=(keymap["consolemode"].find("true")!=std::string::npos);
	if (consolemode) {
		exec<<R"(alias _cvm.nullkeys "alias _cvm.1 ; alias _cvm.2 ; alias _cvm.3 ; alias _cvm.4 ; alias _cvm.5 ; alias _cvm.6 ; alias _cvm.7 ; alias _cvm.8 ; alias _cvm.9 ; alias _cvm.0"
alias cvm.exitmenu "developer 0; )"<<keymap["resetkeys"]<<"; cvm.on_exitmenu\"";
		exec<<R"(
alias _cvm.menusettings "developer 1; clear; bind 1 _cvm.1; bind 2 _cvm.2; bind 3 _cvm.3; bind 4 _cvm.4; bind 5 _cvm.5; bind 6 _cvm.6; bind 7 _cvm.7; bind 8 _cvm.8; bind 9 _cvm.9; bind 0 _cvm.0"
cvm.exitmenu
alias cvm.on_exitmenu ;
alias cvm.on_page_exit ;
)";
	std::string cfgpath="";
	std::cout<<"Done. Creating CFGs.\n";
	std::size_t pi=0u;
	unsigned long togglenumber=0u;
	for (auto page=pages.begin(); page!=pages.end(); page++, pi++) {
			unsigned long segmentnumber=0u;
			cfgpath=outputdir.string()+"/cfg/$pageopen_"+page->first.formatted_title+".cfg";
			std::ofstream cfgfile(cfgpath);
			// Write to cfg
			cfgfile<<"_cvm.menusettings\n_cvm.nullkeys\n";
			for (auto& kbind : page->first.binds) {
				if (kbind.istogglebind) {
					exec<<"alias _cvm.toggle_"<<std::to_string(togglenumber)<<" _cvm.toggle_"<<std::to_string(togglenumber)<<"_0\n";
					exec<<"alias _#cvm.toggle_"<<std::to_string(togglenumber)<<" _#cvm.toggle_"<<std::to_string(togglenumber)<<"_0\n";
					for (unsigned char sti=0u; sti < kbind.name.size(); sti++) {
						exec<<"alias _cvm.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string(sti)<<'\"'<<kbind.cmdstr.at(sti)<<"; alias _cvm.toggle_"<<std::to_string(togglenumber)<<" _cvm.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string((sti+1)%kbind.name.size())<<"; alias _#cvm.toggle_"<<std::to_string(togglenumber)<<" _#cvm.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string((sti+1)%kbind.name.size())<<"\"\n";
						exec<<"alias _#cvm.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string(sti)<<"\"echo "<<kbind.name.at(sti)<<"\"\n";
					}
					cfgfile<<"_#cvm.toggle_"<<std::to_string(togglenumber)<<'\n';
					cfgfile<<"alias _cvm."<<std::to_string(kbind.numberkey)<<" \"_cvm.toggle_"<<std::to_string(togglenumber)<<"\"\n";
					togglenumber++;
				}
				else {
					cfgfile<<"echo "<<kbind.name.front()<<'\n'<<"alias _cvm."<<std::to_string(kbind.numberkey)<<" \""<<kbind.cmdstr.front()<<"\"\n";;
				}
			}
		}
	}
	else {
	exec<<R"(closecaption 1
cc_lang customvoicemenu
alias _cvm.nullkeys "alias _cvm.1 ; alias _cvm.2 ; alias _cvm.3 ; alias _cvm.4 ; alias _cvm.5 ; alias _cvm.6 ; alias _cvm.7 ; alias _cvm.8 ; alias _cvm.9; alias _cvm.0"
alias cvm.exitmenu "cc_emit _#cvm.clear_screen; )"<<keymap["resetkeys"]<<"; cc_linger_time "<<keymap["linger_time"]<<"; cc_predisplay_time "<<keymap["predisplay_time"]<<"; cvm.on_exitmenu\"";
	exec<<R"(
alias _cvm.menusettings "cc_linger_time 10000; cc_predisplay_time 0; bind 1 _cvm.1; bind 2 _cvm.2; bind 3 _cvm.3; bind 4 _cvm.4; bind 5 _cvm.5; bind 6 _cvm.6; bind 7 _cvm.7; bind 8 _cvm.8; bind 9 _cvm.9; bind 0 _cvm.0"
cvm.exitmenu
alias cvm.on_exitmenu ;
alias cvm.on_page_exit ;
)";
	// Conversion to UCS-2.
	std::locale utf16(std::locale::classic(),new std::codecvt_utf16<wchar_t, 0xffff, std::little_endian>);
	std::cout<<"Done. ("<<std::to_string(bindcount)<<" binds compiled.) Creating caption file.\n";
	// Create the captions directory once.
	if (!std::filesystem::exists(outputdir.string()+"captions")) std::filesystem::create_directories(outputdir.string()+"captions/resource");
	std::wofstream captionfile(outputdir.string()+"captions/resource/closecaption_customvoicemenu.txt",std::ios_base::binary);
	captionfile.imbue(utf16);
	captionfile<<(wchar_t)0xFEFF; // BOM.
	captionfile<<convert.from_bytes("\"lang\"\n{\n\t\"Language\" \"customvoicemenu\"\n\t\"Tokens\"\n\t{\n\t\t\"_#cvm.clear_screen\" \"<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	\"\n");
	std::string cfgpath;
	std::cout<<"Done. Creating CFGs and Captions.\n";
	std::size_t pi=0u;
	unsigned long togglenumber=0u;
	for (auto page=pages.begin(); page!=pages.end(); page++, pi++) {
		unsigned long segmentnumber=0u;
		cfgpath=outputdir.string()+"/cfg/$pageopen_"+page->first.formatted_title+".cfg";
		std::ofstream cfgfile(cfgpath);
		// Write to cfg
		cfgfile<<"_cvm.menusettings\ncc_emit _#cvm.clear_screen\ncc_emit _#cvm."+page->first.formatted_title+'\n';
		{
			unsigned long temptogglenumber=togglenumber;
			for (auto kbind=page->first.binds.begin(); kbind!=page->first.binds.end(); kbind++) {
				if (kbind->istogglebind==true) {
					cfgfile<<"_#cvm.toggle_"+std::to_string(temptogglenumber)<<'\n';
					temptogglenumber++;
				}
				
				if (kbind!=page->first.binds.begin() && kbind->istogglebind==false && (kbind-1)->istogglebind==true) {
					cfgfile<<"cc_emit _#cvm."<<page->first.formatted_title<<"_seg_"<<std::to_string(segmentnumber)<<'\n';
					segmentnumber++;
				}
			}
			segmentnumber=0u;
		}
		cfgfile<<"_cvm.nullkeys\n";
		// Write captions
		for (auto kbind=page->first.binds.begin(); kbind!=page->first.binds.end(); kbind++) {
			if (kbind==page->first.binds.begin() && kbind->istogglebind!=true)
				captionfile<<convert.from_bytes("\t\t\"_#cvm."+page->first.formatted_title+"\" \"");
			if (kbind->istogglebind==true) {
				exec<<"alias _cvm.toggle_"<<std::to_string(togglenumber)<<" _cvm.toggle_"<<std::to_string(togglenumber)<<"_0\n";
				exec<<"alias _#cvm.toggle_"<<std::to_string(togglenumber)<<" _#cvm.toggle_"<<std::to_string(togglenumber)<<"_0\n";
				for (unsigned char sti=0u; sti < kbind->name.size(); sti++) {
					exec<<"alias _cvm.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string(sti)<<"\"alias _#cvm.toggle_"<<std::to_string(togglenumber)<<" _#cvm.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string((sti+1)%kbind->name.size())<<';'<<kbind->cmdstr.at(sti)<<"; alias _cvm.toggle_"<<std::to_string(togglenumber)<<" _cvm.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string((sti+1)%kbind->name.size())<<"\"\n";
					exec<<"alias _#cvm.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string(sti)<<"\"cc_emit _#cvm.toggle_"<<std::to_string(togglenumber)<<'_'<<std::to_string(sti)<<"\"\n";
					captionfile<<convert.from_bytes("\t\t\"_#cvm.toggle_"+std::to_string(togglenumber)+'_'+std::to_string(sti)+"\" \""+kbind->name.at(sti)+"\"\n");
				}
				cfgfile<<"alias _cvm."<<std::to_string(kbind->numberkey)<<" \"_cvm.toggle_"<<std::to_string(togglenumber)<<"\"\n";
				togglenumber++;
			}
			else {
				if (kbind!=page->first.binds.begin() && (kbind-1)->istogglebind==true) {
					captionfile<<convert.from_bytes("\t\t\"_#cvm."+page->first.formatted_title+"_seg_"+std::to_string(segmentnumber)+"\" \"");
					segmentnumber++;
				}
				captionfile<<convert.from_bytes(kbind->name.front());
				if (kbind==page->first.binds.end()-1 || (kbind+1)->istogglebind==true) captionfile<<L"\"\n";
				else captionfile<<L"<cr>";
				cfgfile<<"alias _cvm."<<std::to_string(kbind->numberkey)<<" \""<<kbind->cmdstr.at(0)<<"\"\n";
			}
		}
	}
	captionfile<<"\t}\n}";
	captionfile.close();
	}
	exec.close();
	//Done!
	std::cout<<"Done! Now just insert exec activate_cvm into autoexec.\n";
}
