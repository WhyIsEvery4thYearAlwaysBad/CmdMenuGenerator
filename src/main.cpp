#include <regex>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <codecvt>
#include <map>
#include <cstdio>
#include <windows.h>
#include <locale>
#include "lex.hpp"
#include "token.hpp"
#include "commandmenu.hpp"
#include "compiler.hpp"
#include "launchoptions.hpp"
extern std::deque<Parser::MenuToken*> CMenuTokens;
extern std::deque<Token> Tokens; 
extern std::deque<Token> ErrorTokens; // 
std::deque<CommandMenu> CMenuContainer;
std::map<std::string,std::string> KVMap={
	{"linger_time","0"},
	{"predisplay_time","0.25"},
	{"format","$(nkey). $(str)<cr>"},
	{"consolemode","false"},
	{"resetkeys","bind 1 slot1; bind 2 slot2; bind 3 slot3; bind 4 slot4; bind 5 slot5; bind 6 slot6; bind 7 slot7; bind 8 slot8; bind 9 slot9; bind 0 slot10"}
};
std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> convert;
// From launch options
extern std::filesystem::path InputFilePath;
extern std::filesystem::path sOutputDir;
int main(int argc, char** argv) {
	unsigned short iBindCount=0u;
	if (!EvaluateLaunchOptions(argc,argv)) return -1;
	std::string Line, InFileContent; 
	std::ifstream inputf(InputFilePath,std::ios_base::binary);
	while (std::getline(inputf,Line)) {
		InFileContent+=Line+'\n';
	}
	// If tokenization and parsing process failed, then error out and return.
	if (!Lexer::Tokenize(InFileContent) 
		|| !Parser::ParseTokens()) {
		return -1;
	}
	// Use the parsed tokens to form CMenus.
	ParseMenuTokens(iBindCount);
	// Directories
	std::filesystem::create_directories(sOutputDir.string()+"/cfg/cmenu");
	// Main CFG file that initializes our CMenus.
	std::ofstream InitRoutineFile(sOutputDir.string()+"/cfg/cmenu_initialize.cfg");
	// Console mode makes the Voicemenu use the console for displaying binds, instead of captions.
	bool bConsoleModeTrue=(KVMap["bConsoleModeTrue"].find("true")!=std::string::npos);
	if (bConsoleModeTrue) {
		InitRoutineFile<<R"(alias _cmenu.nullkeys "alias _cmenu.1 ; alias _cmenu.2 ; alias _cmenu.3 ; alias _cmenu.4 ; alias _cmenu.5 ; alias _cmenu.6 ; alias _cmenu.7 ; alias _cmenu.8 ; alias _cmenu.9 ; alias _cmenu.0"
alias cmenu.exitmenu "developer 0; )"<<KVMap["resetkeys"]<<"; cmenu.on_exitmenu\"";
		InitRoutineFile<<R"(
alias _cmenu.menusettings "developer 1; clear; bind 1 _cmenu.1; bind 2 _cmenu.2; bind 3 _cmenu.3; bind 4 _cmenu.4; bind 5 _cmenu.5; bind 6 _cmenu.6; bind 7 _cmenu.7; bind 8 _cmenu.8; bind 9 _cmenu.9; bind 0 _cmenu.0"
alias cmenu.on_exitmenu ;
alias cmenu.on_page_exit ;
cmenu.exitmenu
)";
	std::string CMenuCFGPath="";
	unsigned long iToggleNumber=0u;
	for (auto CMenu=CMenuContainer.begin(); CMenu!=CMenuContainer.end(); CMenu++) {
			unsigned long segmentnumber=0u;
			CMenuCFGPath=sOutputDir.string()+"/cfg/$cmenu_"+CMenu->sRawName+".cfg";
			std::ofstream CMenuCFG(CMenuCFGPath);
			// Write to cfg
			CMenuCFG<<"_cmenu.menusettings\n_cvm.nullkeys\n";
			for (auto& kbind : CMenu->binds) {
				if (kbind.bToggleBind) {
					InitRoutineFile<<"alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _cmenu.toggle_"<<std::to_string(iToggleNumber)<<"_0\n";
					InitRoutineFile<<"alias _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<"_0\n";
					for (unsigned char sti=0u; sti < kbind.NameContainer.size(); sti++) {
						InitRoutineFile<<"alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string(sti)<<'\"'<<kbind.CmdStrContainer.at(sti)<<"; alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string((sti+1)%kbind.NameContainer.size())<<"; alias _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string((sti+1)%kbind.NameContainer.size())<<"\"\n";
						InitRoutineFile<<"alias _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string(sti)<<"\"echo "<<kbind.NameContainer.at(sti)<<"\"\n";
					}
					CMenuCFG<<"_#cmenu.toggle_"<<std::to_string(iToggleNumber)<<'\n';
					CMenuCFG<<"alias _cmenu."<<std::to_string(kbind.cKey)<<" \"_cmenu.toggle_"<<std::to_string(iToggleNumber)<<"\"\n";
					iToggleNumber++;
				}
				else {
					CMenuCFG<<"echo "<<kbind.NameContainer.front()<<'\n'<<"alias _cmenu."<<std::to_string(kbind.cKey)<<" \""<<kbind.CmdStrContainer.front()<<"\"\n";;
				}
			}
		}
	}
	else {
	InitRoutineFile<<R"(closecaption 1
cc_lang commandmenu
alias _cmenu.nullkeys "alias _cmenu.1 ; alias _cmenu.2 ; alias _cmenu.3 ; alias _cmenu.4 ; alias _cmenu.5 ; alias _cmenu.6 ; alias _cmenu.7 ; alias _cmenu.8 ; alias _cmenu.9; alias _cmenu.0"
alias cmenu.exitmenu "cc_emit _#cmenu.clear_screen; )"<<KVMap["resetkeys"]<<"; cc_linger_time "<<KVMap["linger_time"]<<"; cc_predisplay_time "<<KVMap["predisplay_time"]<<"; cmenu.on_exitmenu\"";
	InitRoutineFile<<R"(
alias _cmenu.menusettings "cc_linger_time 10000; cc_predisplay_time 0; bind 1 _cmenu.1; bind 2 _cmenu.2; bind 3 _cmenu.3; bind 4 _cmenu.4; bind 5 _cmenu.5; bind 6 _cmenu.6; bind 7 _cmenu.7; bind 8 _cmenu.8; bind 9 _cmenu.9; bind 0 _cmenu.0"
alias cmenu.on_exitmenu ;
alias cmenu.on_page_exit ;
cmenu.exitmenu
)";
	// Conversion to UCS-2.
	std::locale utf16(std::locale::classic(),new std::codecvt_utf16<wchar_t, 0xffff, std::little_endian>);
	// Create the captions directory once.
	if (!std::filesystem::exists(sOutputDir.string()+"/resource")) std::filesystem::create_directories(sOutputDir.string()+"/resource");
	std::wofstream CMenuCaptionFile(sOutputDir.string()+"/resource/closecaption_commandmenu.txt",std::ios_base::binary);
	CMenuCaptionFile.imbue(utf16);
	CMenuCaptionFile<<(wchar_t)0xFEFF; // BOM.
	CMenuCaptionFile<<convert.from_bytes("\"lang\"\n{\n\t\"Language\" \"commandmenu\"\n\t\"Tokens\"\n\t{\n\t\t\"_#cmenu.clear_screen\" \"<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	\"\n");
	
	std::string CMenuCFGPath; // String path for each CMenu CFG.
	unsigned long iToggleNumber=0u;
	for (auto CMenu=CMenuContainer.begin(); CMenu!=CMenuContainer.end(); CMenu++) {
		unsigned int iTBindSegmentNum=0u;
		CMenuCFGPath=sOutputDir.string()+"/cfg/$cmenu_"+CMenu->sRawName+".cfg";
		std::ofstream CMenuCFG(CMenuCFGPath);
		/* Create the actual binds needed for the declared binds. */
		CMenuCFG<<"_cmenu.menusettings\ncc_emit _#cmenu.clear_screen\ncc_emit _#cmenu."+CMenu->sRawName+'\n';
		{
			unsigned int t_ToggleNum=iToggleNumber;
			for (auto kbind=CMenu->binds.begin(); kbind!=CMenu->binds.end(); kbind++) {
				if (kbind->bToggleBind==true) {
					CMenuCFG<<"_#cmenu.toggle_"+std::to_string(t_ToggleNum)<<'\n';
					t_ToggleNum++;
				}
				
				if (kbind!=CMenu->binds.begin() && kbind->bToggleBind==false && (kbind-1)->bToggleBind==true) {
					CMenuCFG<<"cc_emit _#cmenu."<<CMenu->sRawName<<"_seg_"<<std::to_string(iTBindSegmentNum)<<'\n';
					iTBindSegmentNum++;
				}
			}
			iTBindSegmentNum=0u;
		}
		CMenuCFG<<"_cmenu.nullkeys\n";
		/* Write captions for our binds. */
		for (auto kbind=CMenu->binds.begin(); kbind!=CMenu->binds.end(); kbind++) {
			if (kbind==CMenu->binds.begin() && kbind->bToggleBind!=true)
				CMenuCaptionFile<<convert.from_bytes("\t\t\"_#cmenu."+CMenu->sRawName+"\" \"");
			if (kbind->bToggleBind==true) {
				InitRoutineFile<<"alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _cmenu.toggle_"<<std::to_string(iToggleNumber)<<"_0\n";
				InitRoutineFile<<"alias _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<"_0\n";
				for (unsigned char sti=0u; sti < kbind->NameContainer.size(); sti++) {
					InitRoutineFile<<"alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string(sti)<<"\"alias _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string((sti+1)%kbind->NameContainer.size())<<';'<<kbind->CmdStrContainer.at(sti)<<"; alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string((sti+1)%kbind->NameContainer.size())<<"\"\n";
					InitRoutineFile<<"alias _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string(sti)<<"\"cc_emit _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string(sti)<<"\"\n";
					CMenuCaptionFile<<convert.from_bytes("\t\t\"_#cmenu.toggle_"+std::to_string(iToggleNumber)+'_'+std::to_string(sti)+"\" \""+kbind->NameContainer.at(sti)+"\"\n");
				}
				CMenuCFG<<"alias _cmenu."<<std::to_string(kbind->cKey)<<" \"_cmenu.toggle_"<<std::to_string(iToggleNumber)<<"\"\n";
				iToggleNumber++;
			}
			else {
				if (kbind!=CMenu->binds.begin() && (kbind-1)->bToggleBind==true) {
					CMenuCaptionFile<<convert.from_bytes("\t\t\"_#cmenu."+CMenu->sRawName+"_seg_"+std::to_string(iTBindSegmentNum)+"\" \"");
					iTBindSegmentNum++;
				}
				CMenuCaptionFile<<convert.from_bytes(kbind->NameContainer.front());
				if (kbind==CMenu->binds.end()-1 || (kbind+1)->bToggleBind==true) CMenuCaptionFile<<L"\"\n";
				CMenuCFG<<"alias _cmenu."<<std::to_string(kbind->cKey)<<" \""<<kbind->CmdStrContainer.at(0)<<"\"\n";
			}
		}
	}
	CMenuCaptionFile<<"\t}\n}";
	CMenuCaptionFile.close();
	}
	InitRoutineFile<<"\necho [Command Menu Generator] Initialized command menus!";
	InitRoutineFile.close();
	//Done!
	std::cout<<std::to_string(iBindCount)<<" binds compiled.\n";
}
