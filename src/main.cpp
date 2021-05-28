#include <regex>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <codecvt>
#include <algorithm>
#include <map>
#include <cstdio>
#include <windows.h>
#include <locale>
#include "lex.hpp"
#include "token.hpp"
#include "commandmenu.hpp"
#include "compiler.hpp"
#include "launchoptions.hpp"

#define CMENU_KEY_ALIAS_LENGTH 11 /* length of "_cmenu.key_" */
extern std::deque<Token> ErrorTokens;
std::deque<CommandMenu> CMenuContainer;
std::map<std::string,std::string> KVMap={
	{"linger_time","0"},
	{"predisplay_time","0.25"},
	{"format","$(key). $(title)<cr>"},
	{"display","caption"},
	{"resetkeys","bind 1 slot1; bind 2 slot2; bind 3 slot3; bind 4 slot4; bind 5 slot5; bind 6 slot6; bind 7 slot7; bind 8 slot8; bind 9 slot9; bind 0 slot10"}
};
std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> convert;
std::vector<std::string> UsedKeys = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"}; // Stores what keys are used by binds.
// From launch options
extern std::filesystem::path InputFilePath;
extern std::filesystem::path sOutputDir;
int main(int argc, char** argv) {
	// Evaluate launch options.
	if (!EvaluateLaunchOptions(argc,argv)) return -1;
	// Get content from input file.
	std::string Line, InFileContent; 
	std::ifstream inputf(InputFilePath,std::ios_base::binary);
	while (std::getline(inputf,Line)) {
		InFileContent+=Line+'\n';
	}
	// If tokenization and parsing process failed then error out and return. Also get the compiled bind count.
	unsigned short iBindCount=0u;
	char bUsedDisplayFlags = 0; // Flag used to mark if and what display types were used.
	if (!Lexer::Tokenize(InFileContent) 
		|| !Parser::ParseTokens()
		|| !ParseMenuTokens(iBindCount, bUsedDisplayFlags)) {
		std::for_each(ErrorTokens.cbegin(),ErrorTokens.cend(),[](const Token& e){
			std::cout << std::to_string(e.iLineNum) << ':' << std::to_string(e.iLineColumn) << ' ' << e.sValue << '\n';
		});
		return -1;
	}
	// Now start creating the neccessary CFG and Captions files for CMenus.
	std::filesystem::create_directories(sOutputDir.string()+"/cfg");
	// Main CFG file that initializes our CMenus.
	std::ofstream InitRoutineFile(sOutputDir.string()+"/cfg/cmenu_initialize.cfg");	
	InitRoutineFile<<"closecaption 1\ncc_lang commandmenu\nalias _cmenu.nullkeys \"";
	// Output the null keys alias.
	std::for_each(UsedKeys.cbegin(), UsedKeys.cend(), 
	[&InitRoutineFile](const std::string_view key){
		InitRoutineFile<<"alias _cmenu.key_"<<((key.length() + CMENU_KEY_ALIAS_LENGTH > 32 ) ? key.substr(0, key.length() - CMENU_KEY_ALIAS_LENGTH) : key) << ';';
	});
	InitRoutineFile<<R"("
alias cmenu.exitmenu"cc_emit _#cmenu.clear_screen;)"<<"cc_linger_time "<<KVMap["linger_time"]<<";cc_predisplay_time "<<KVMap["predisplay_time"]<<';'<<KVMap["resetkeys"]<<"; cmenu.on_exitmenu\"";
	InitRoutineFile<<R"(
alias cmenu.on_exitmenu ;
alias cmenu.on_page_exit ;
cmenu.exitmenu
)";
	// Make caption file if any CMenu uses captions for displays.
	std::wofstream CMenuCaptionFile;
	if (bUsedDisplayFlags & FL_DISPLAY_CAPTION) {
		// Conversion to UCS-2 through *shudders* std::locale.
		std::locale utf16(std::locale::classic(),new std::codecvt_utf16<wchar_t, 0xffff, std::little_endian>);
		// Create the captions directory once.
		if (!std::filesystem::exists(sOutputDir.string()+"/resource")) std::filesystem::create_directories(sOutputDir.string()+"/resource");
		CMenuCaptionFile.open(sOutputDir.string()+"/resource/closecaption_commandmenu.txt", std::ios_base::binary);
		CMenuCaptionFile.imbue(utf16);
		CMenuCaptionFile<<(wchar_t)0xFEFF; // The Byte Order Mark.
		CMenuCaptionFile<<convert.from_bytes("\"lang\"\n{\n\t\"Language\" \"commandmenu\"\n\t\"Tokens\"\n\t{\n\t\t\"_#cmenu.clear_screen\" \"<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	\"\n");
	}
	std::string CMenuCFGPath; // String path for each CMenu CFG.
	unsigned long iToggleNumber = 0u;
	for (auto CMenu = CMenuContainer.begin(); CMenu != CMenuContainer.end(); CMenu++) {
		unsigned int iTBindSegmentNum=0u;
		CMenuCFGPath=sOutputDir.string()+"/cfg/$cmenu_"+CMenu->sRawName+".cfg";
		std::ofstream CMenuCFG(CMenuCFGPath);
		/* Disable other displays if this CMenu isnt using it and there are other CMenus that use it.*/
		if (CMenu->Display!=CMenuDisplayType::CONSOLE && (bUsedDisplayFlags & FL_DISPLAY_CONSOLE))
			CMenuCFG<<"developer 0\n";
		if (CMenu->Display!=CMenuDisplayType::CAPTIONS && (bUsedDisplayFlags & FL_DISPLAY_CAPTION))
			CMenuCFG<<"cc_emit _#cmenu.clear_screen\n";
		
		/* Create the actual binds needed for the declared binds. */
		if (CMenu->Display == CMenuDisplayType::CAPTIONS) CMenuCFG<<"cc_linger_time 10000\ncc_predisplay_time 0\ncc_emit _#cmenu.clear_screen\ncc_emit _#cmenu."+CMenu->sRawName+'\n';
		else if (CMenu->Display == CMenuDisplayType::CONSOLE) CMenuCFG<<"developer 1\nclear\n";
		// Compress keynames so that it can fit in an alias
		std::for_each(UsedKeys.cbegin(), UsedKeys.cend(), 
			[&CMenuCFG](const std::string_view key){ CMenuCFG<<"bind "<<key<<" _cmenu.key_"<<((key.length() + CMENU_KEY_ALIAS_LENGTH > 32 ) ? key.substr(0, key.length() - CMENU_KEY_ALIAS_LENGTH) : key)<<'\n';
		});
		{
			unsigned int t_ToggleNum=iToggleNumber;
			for (auto kTBind=CMenu->binds.begin(); kTBind!=CMenu->binds.end(); kTBind++) {
				if (CMenu->Display != CMenuDisplayType::NONE && kTBind->bToggleBind==true) {
					CMenuCFG<<"_#cmenu.toggle_"+std::to_string(t_ToggleNum)<<'\n';
					t_ToggleNum++;
				}
				
				if (CMenu->Display == CMenuDisplayType::CAPTIONS && kTBind!=CMenu->binds.begin() && kTBind->bToggleBind==false && (kTBind-1)->bToggleBind==true) {
					CMenuCFG<<"cc_emit _#cmenu."<<CMenu->sRawName<<"_seg_"<<std::to_string(iTBindSegmentNum)<<'\n';
					iTBindSegmentNum++;
				}
			}
			iTBindSegmentNum=0u;
		}
		CMenuCFG<<"_cmenu.nullkeys\n";
		/* Write the displays for our binds. */
		for (auto kBind = CMenu->binds.begin(); kBind != CMenu->binds.end(); kBind++) {
			if (CMenu->Display==CMenuDisplayType::CAPTIONS) {
				if (kBind==CMenu->binds.begin() && kBind->bToggleBind!=true) 
					CMenuCaptionFile<<convert.from_bytes("\t\t\"_#cmenu."+CMenu->sRawName+"\" \"");
				if (kBind->bToggleBind==true) {
					InitRoutineFile<<"alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _cmenu.toggle_"<<std::to_string(iToggleNumber)<<"_0\n";
					InitRoutineFile<<"alias _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<"_0\n";
					for (unsigned char sti=0u; sti < kBind->NameContainer.size(); sti++) {
						InitRoutineFile<<"alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string(sti)<<"\"alias _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string((sti+1)%kBind->NameContainer.size())<<';'<<kBind->CmdStrContainer.at(sti)<<"; alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string((sti+1)%kBind->NameContainer.size())<<"\"\n";
						InitRoutineFile<<"alias _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string(sti)<<"\"cc_emit _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string(sti)<<"\"\n";
						CMenuCaptionFile<<convert.from_bytes("\t\t\"_#cmenu.toggle_"+std::to_string(iToggleNumber)+'_'+std::to_string(sti)+"\" \""+kBind->NameContainer.at(sti)+"\"\n");
					}
					CMenuCFG<<"alias _cmenu.key_"<<((kBind->sKey.length() + CMENU_KEY_ALIAS_LENGTH > 32) ? kBind->sKey.substr(0, kBind->sKey.length() - CMENU_KEY_ALIAS_LENGTH) : kBind->sKey)<<"\"_cmenu.toggle_"<<std::to_string(iToggleNumber)<<"\"\n";
					iToggleNumber++;
				}
				else {
					if (kBind!=CMenu->binds.begin() && (kBind-1)->bToggleBind==true) {
						CMenuCaptionFile<<convert.from_bytes("\t\t\"_#cmenu."+CMenu->sRawName+"_seg_"+std::to_string(iTBindSegmentNum)+"\" \"");
						iTBindSegmentNum++;
					}
					CMenuCaptionFile<<convert.from_bytes(kBind->NameContainer.front());
					if (kBind==CMenu->binds.end()-1 || (kBind+1)->bToggleBind==true) CMenuCaptionFile<<L"\"\n";
					CMenuCFG<<"alias _cmenu.key_"<<((kBind->sKey.length() + CMENU_KEY_ALIAS_LENGTH > 32) ? kBind->sKey.substr(0, kBind->sKey.length() - CMENU_KEY_ALIAS_LENGTH) : kBind->sKey)<<"\""<<kBind->CmdStrContainer.at(0)<<"\"\n";
				}
			}
			else if (CMenu->Display==CMenuDisplayType::CONSOLE) {
				if (kBind->bToggleBind) {
					InitRoutineFile<<"alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _cmenu.toggle_"<<std::to_string(iToggleNumber)<<"_0\n";
					InitRoutineFile<<"alias _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<"_0\n";
					for (unsigned char sti=0u; sti < kBind->NameContainer.size(); sti++) {
						InitRoutineFile<<"alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string(sti)<<'\"'<<kBind->CmdStrContainer.at(sti)<<"; alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string((sti+1)%kBind->NameContainer.size())<<"; alias _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string((sti+1)%kBind->NameContainer.size())<<"\"\n";
						InitRoutineFile<<"alias _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string(sti)<<"\"echo "<<kBind->NameContainer.at(sti)<<"\"\n";
					}
					CMenuCFG<<"_#cmenu.toggle_"<<std::to_string(iToggleNumber)<<'\n';
					CMenuCFG<<"alias _cmenu."<<kBind->sKey<<" \"_cmenu.toggle_"<<std::to_string(iToggleNumber)<<"\"\n";
					iToggleNumber++;
				}
				else {
					CMenuCFG<<"echo "<<kBind->NameContainer.front()<<'\n'<<"alias _cmenu."<<kBind->sKey<<" \""<<kBind->CmdStrContainer.front()<<"\"\n";;
				}
			}
			else if (CMenu->Display==CMenuDisplayType::NONE) {
				if (kBind->bToggleBind) {
					InitRoutineFile<<"alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _cmenu.toggle_"<<std::to_string(iToggleNumber)<<"_0\n";
					for (unsigned char sti=0u; sti < kBind->NameContainer.size(); sti++) {
						InitRoutineFile<<"alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string(sti)<<'\"'<<kBind->CmdStrContainer.at(sti)<<"; alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string((sti+1)%kBind->NameContainer.size())<<"\n";
					}
					CMenuCFG<<"alias _cmenu."<<kBind->sKey<<" \"_cmenu.toggle_"<<std::to_string(iToggleNumber)<<"\"\n";
					iToggleNumber++;
				}
				else {
					CMenuCFG<<"alias _cmenu."<<kBind->sKey<<" \""<<kBind->CmdStrContainer.front()<<"\"\n";;
				}
			}
		}
	}
	// We're done compiling so close the file streams.
	CMenuCaptionFile<<"\t}\n}";
	CMenuCaptionFile.close();
	InitRoutineFile<<"\necho [Command Menu Generator] Initialized command menus!";
	InitRoutineFile.close();
	//Done!
	std::cout<<std::to_string(iBindCount)<<" binds compiled.\n";
}
