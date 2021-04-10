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
	{"display","caption"},
	{"resetkeys","bind 1 slot1; bind 2 slot2; bind 3 slot3; bind 4 slot4; bind 5 slot5; bind 6 slot6; bind 7 slot7; bind 8 slot8; bind 9 slot9; bind 0 slot10"}
};
std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> convert;
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
	// If tokenization and parsing process failed then error out and return.
	if (Lexer::Tokenize(InFileContent) 
		|| Parser::ParseTokens()) {
		
		for (auto& e : ErrorTokens) {
			std::cout << e.sValue << '\n';
		}
		return -1;
	}
	// Use the parsed tokens to form CMenus. Also get the compiled bind count.
	unsigned short iBindCount=0u;
	unsigned char bUsedDisplayFlags=0; // Flag used to mark if and what display types were used.
	ParseMenuTokens(iBindCount,bUsedDisplayFlags);
	/*
	display="caption" - Makes cmenus use the caption for bind displays
	display="console" - Makes cmenus use the console for bind displys
	display="none" - Disables bind display for future CMenus. 
	First trim the display KV to remove spaces and make the value lowercase.
	*/
	if (KVMap["display"] != "caption" && KVMap["display"] != "console" && KVMap["display"]!="none") 
		for (unsigned long long i=0; i < KVMap["display"].length(); i++) {
			if (isspace(KVMap["display"].at(i))==true) KVMap["display"].erase(i,1);
			KVMap["display"].at(i)=tolower(KVMap["display"].at(i));
		}
	
	// Then if the display KV isn't one of the three specified values, force it to default ("caption").
	if (KVMap["display"] != "caption" && KVMap["display"] != "console" && KVMap["display"]!="none") {
		std::cout<<"warning: Unknown display type \""<<KVMap["display"]<<"\". Falling back to caption display!\n";
		KVMap["display"]="caption";
	}
	// Now start creating the neccessary CFG and Captions files for CMenus.
		// Directories
		std::filesystem::create_directories(sOutputDir.string()+"/cfg/cmenu");
	// Main CFG file that initializes our CMenus.
	std::ofstream InitRoutineFile(sOutputDir.string()+"/cfg/cmenu_initialize.cfg");	
	InitRoutineFile<<R"(closecaption 1
cc_lang commandmenu
alias _cmenu.nullkeys"alias _cmenu.1 ; alias _cmenu.2 ; alias _cmenu.3 ; alias _cmenu.4 ; alias _cmenu.5 ; alias _cmenu.6 ; alias _cmenu.7 ; alias _cmenu.8 ; alias _cmenu.9; alias _cmenu.0"
alias cmenu.exitmenu"cc_emit _#cmenu.clear_screen;)"<<"cc_linger_time "<<KVMap["linger_time"]<<";cc_predisplay_time "<<KVMap["predisplay_time"]<<';'<<KVMap["resetkeys"]<<"; cmenu.on_exitmenu\"";
	InitRoutineFile<<R"(
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
		/* Disable other displays if this CMenu isnt using it and there are other CMenus that use it.*/
		
		if (CMenu->Display!=CMenuDisplayType::CONSOLE && (bUsedDisplayFlags & 2))
			CMenuCFG<<"developer 0\n";
		if (CMenu->Display!=CMenuDisplayType::CAPTIONS && (bUsedDisplayFlags & 4))
			CMenuCFG<<"cc_emit _#cmenu.clear_screen\n";
		
		/* Create the actual binds needed for the declared binds. */
		if (CMenu->Display==CMenuDisplayType::CAPTIONS) CMenuCFG<<"cc_linger_time 10000\ncc_predisplay_time 0\ncc_emit _#cmenu.clear_screen\ncc_emit _#cmenu."+CMenu->sRawName+'\n';
		else if (CMenu->Display==CMenuDisplayType::CONSOLE) CMenuCFG<<"developer 1\nclear\n";
		CMenuCFG<<"bind 1 _cmenu.1\nbind 2 _cmenu.2\nbind 3 _cmenu.3\nbind 4 _cmenu.4\nbind 5 _cmenu.5\nbind 6 _cmenu.6\nbind 7 _cmenu.7\nbind 8 _cmenu.8\nbind 9 _cmenu.9\nbind 0 _cmenu.0\n";
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
		/* Write captions for our binds. */
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
					CMenuCFG<<"alias _cmenu."<<std::to_string(kBind->cKey)<<"\"_cmenu.toggle_"<<std::to_string(iToggleNumber)<<"\"\n";
					iToggleNumber++;
				}
				else {
					if (kBind!=CMenu->binds.begin() && (kBind-1)->bToggleBind==true) {
						CMenuCaptionFile<<convert.from_bytes("\t\t\"_#cmenu."+CMenu->sRawName+"_seg_"+std::to_string(iTBindSegmentNum)+"\" \"");
						iTBindSegmentNum++;
					}
					CMenuCaptionFile<<convert.from_bytes(kBind->NameContainer.front());
					if (kBind==CMenu->binds.end()-1 || (kBind+1)->bToggleBind==true) CMenuCaptionFile<<L"\"\n";
					CMenuCFG<<"alias _cmenu."<<std::to_string(kBind->cKey)<<"\""<<kBind->CmdStrContainer.at(0)<<"\"\n";
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
					CMenuCFG<<"alias _cmenu."<<std::to_string(kBind->cKey)<<" \"_cmenu.toggle_"<<std::to_string(iToggleNumber)<<"\"\n";
					iToggleNumber++;
				}
				else {
					CMenuCFG<<"echo "<<kBind->NameContainer.front()<<'\n'<<"alias _cmenu."<<std::to_string(kBind->cKey)<<" \""<<kBind->CmdStrContainer.front()<<"\"\n";;
				}
			}
			else if (CMenu->Display==CMenuDisplayType::NONE) {
				if (kBind->bToggleBind) {
					InitRoutineFile<<"alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _cmenu.toggle_"<<std::to_string(iToggleNumber)<<"_0\n";
					for (unsigned char sti=0u; sti < kBind->NameContainer.size(); sti++) {
						InitRoutineFile<<"alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string(sti)<<'\"'<<kBind->CmdStrContainer.at(sti)<<"; alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string((sti+1)%kBind->NameContainer.size())<<"\n";
					}
					CMenuCFG<<"alias _cmenu."<<std::to_string(kBind->cKey)<<" \"_cmenu.toggle_"<<std::to_string(iToggleNumber)<<"\"\n";
					iToggleNumber++;
				}
				else {
					CMenuCFG<<"alias _cmenu."<<std::to_string(kBind->cKey)<<" \""<<kBind->CmdStrContainer.front()<<"\"\n";;
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
