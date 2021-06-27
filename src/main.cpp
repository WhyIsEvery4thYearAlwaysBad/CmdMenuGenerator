#define NDEBUG
#include <regex>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <codecvt>
#include <algorithm>
#include <map>
#include <cstdio>
#include <locale>
#include "lex.hpp"
#include "token.hpp"
#include "commandmenu.hpp"
#include "parser.hpp"
#include "launchoptions.hpp"

#define CMENU_KEY_ALIAS_LENGTH 9 /* length of "_cmenu.k_" */
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
	// Evaluate launch options. Exit program if ran with --help or launch options were invalid.
	char Status = EvaluateLaunchOptions(argc, argv);
	if (Status != 0) return -(Status < 0);
	// Get content from input file.
	std::string Line, InFileContent; 
	std::ifstream inputf(InputFilePath,std::ios_base::binary);
	while (std::getline(inputf,Line)) {
		InFileContent+=Line+'\n';
	}
	// If tokenization and parsing process failed then error out and return. Also get the compiled bind count.
	unsigned long long iBindCount=0u;
	unsigned char bUsedDisplayFlags = 0; // Flag used to mark if and what display types were used.
	std::string init_file_user_code = ""; // Code inserted via ``.
	if (!Lexer::Tokenize(InFileContent) 
		|| !Parser::ParseTokens()
		|| !ParseMenuTokens(iBindCount, bUsedDisplayFlags, init_file_user_code)) {
		std::for_each(ErrorTokens.cbegin(),ErrorTokens.cend(),[](const Token& e){
			std::cout << std::to_string(e.iLineNum) << ':' << std::to_string(e.iLineColumn) << ": " << e.sValue << '\n';
		});
		return -1;
	}
	// Now start creating the neccessary CFG and Captions files for CMenus.
	std::filesystem::create_directories(sOutputDir.string()+"/cfg");
	// Main CFG file that initializes our CMenus.
	std::ofstream InitRoutineFile(sOutputDir.string()+"/cfg/cmenu_initialize.cfg");	
	InitRoutineFile<<"cc_lang commandmenu\nalias _cmenu.nullkeys \"";
	// Output the null keys alias.
	std::for_each(UsedKeys.cbegin(), UsedKeys.cend(), 
	[&InitRoutineFile](const std::string_view key){
		InitRoutineFile<<"alias _cmenu.k_"<<((key.length() + CMENU_KEY_ALIAS_LENGTH > 32 ) ? key.substr(0, key.length() - CMENU_KEY_ALIAS_LENGTH) : key) << ';';
	});
	InitRoutineFile<<R"("
alias cmenu.exitmenu"_cmenu.exitmenu;cc_emit _#cmenu.clear_screen;)"<<"cc_linger_time "<<KVMap["linger_time"]<<";cc_predisplay_time "<<KVMap["predisplay_time"]<<';'<<KVMap["resetkeys"]<<"; cmenu.on_exitmenu\"";
	InitRoutineFile<<R"(
alias _cmenu.exitmenu
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
		if (CMenu->Display != CMenuDisplayType::CONSOLE && (bUsedDisplayFlags & FL_DISPLAY_CONSOLE))
			CMenuCFG << "developer 0\n";
		else if (CMenu->Display == CMenuDisplayType::CONSOLE && (bUsedDisplayFlags & FL_DISPLAY_CONSOLE))
		// Unfortunately we have to use wait because con_filter_enable doesn't immediately take effect after being changed, which causes the console display to not change.	I'll try to find a waitless patch.
			CMenuCFG << "con_filter_enable 0\nwait 10;developer 1\necho\"\necho\"\necho\"\necho\"\necho\"\necho\"\necho\"\necho\"\n";
		if (CMenu->Display != CMenuDisplayType::CAPTIONS && (bUsedDisplayFlags & FL_DISPLAY_CAPTION))
			CMenuCFG << "closecaption 0\ncc_emit _#cmenu.clear_screen\n";
		else if (CMenu->Display == CMenuDisplayType::CAPTIONS && (bUsedDisplayFlags & FL_DISPLAY_CAPTION))
			CMenuCFG << "closecaption 1\ncc_linger_time 10000\ncc_predisplay_time 0\ncc_emit _#cmenu.clear_screen\ncc_emit _#cmenu." + CMenu->sRawName + '\n';
		/* Initialize cmenu displays. */
		// Compress keynames so that it can fit in an alias
		std::for_each(UsedKeys.cbegin(), UsedKeys.cend(), 
			[&CMenuCFG](const std::string_view key){ CMenuCFG<<"bind "<<key<<" _cmenu.k_"<<((key.length() + CMENU_KEY_ALIAS_LENGTH > 32 ) ? key.substr(0, key.length() - CMENU_KEY_ALIAS_LENGTH) : key)<<'\n';
		});
		/*	The below code writes the code to display the toggle bind title based on its current state, before handling the code for binds/inline code. 
			However I'm going to experiment with writing the toggle bind display code, while writing the bind(/inline) code.
		{
			unsigned int t_ToggleNum=iToggleNumber;
			for (auto kTBind=CMenu->Entries.begin(); kTBind!=CMenu->Entries.end(); kTBind++) {
				if (std::holds_alternative<Bind>(*kTBind)) {
					Bind t_kTBind = std::get<Bind>(*kTBind);
					if (CMenu->Display != CMenuDisplayType::NONE && t_kTBind.bToggleBind==true) {
						CMenuCFG<<"_#cmenu.toggle_"+std::to_string(t_ToggleNum)<<'\n';
						t_ToggleNum++;
					}
					
					if (CMenu->Display == CMenuDisplayType::CAPTIONS && kTBind != CMenu->Entries.begin() && t_kTBind.bToggleBind==false && std::get<Bind>(*(kTBind-1)).bToggleBind==true) {
						CMenuCFG<<"cc_emit _#cmenu."<<CMenu->sRawName<<"_seg_"<<std::to_string(iTBindSegmentNum)<<'\n';
						iTBindSegmentNum++;
					}
				}
			}
			iTBindSegmentNum=0u;
		}
		*/
		CMenuCFG<<"_cmenu.nullkeys\n";
		/* Write the displays for our binds. */
		Bind t_kBind;
		// Get the last bind that exists in the current cmenu so we can properly add the ending quote in captions if there are any. We store it outside the for loop for efficiency.
		auto last_bind = std::find_if(CMenu->Entries.rbegin(), CMenu->Entries.rend(), [](const std::variant<Bind, std::string>& v) constexpr -> bool { return std::holds_alternative<Bind>(v); });
		// Write out the code for binds and stuff.
		for (auto kBind = CMenu->Entries.begin(); kBind < CMenu->Entries.end(); kBind++) {
			if (std::holds_alternative<Bind>(*kBind)) {
				t_kBind = std::get<Bind>(*kBind);
				if (CMenu->Display == CMenuDisplayType::CAPTIONS) {
					// Show the cmenu.
					if (kBind == CMenu->Entries.begin() && t_kBind.bToggleBind != true) 
						CMenuCaptionFile<<convert.from_bytes("\t\t\"_#cmenu."+CMenu->sRawName+"\" \"");
					// Show the toggle bind titles.
					if (t_kBind.bToggleBind == true) {
						InitRoutineFile << "alias _cmenu.toggle_"<<std::to_string(iToggleNumber) << " _cmenu.toggle_" << std::to_string(iToggleNumber) << "_0\n";
						InitRoutineFile << "alias _#cmenu.toggle_"<<std::to_string(iToggleNumber) << " _#cmenu.toggle_" << std::to_string(iToggleNumber) << "_0\n";
						for (unsigned char sti = 0u; sti < t_kBind.NameContainer.size(); sti++) {
							InitRoutineFile << "alias _cmenu.toggle_" << std::to_string(iToggleNumber) << '_' << std::to_string(sti) << "\"alias _#cmenu.toggle_" << std::to_string(iToggleNumber)
							<< " _#cmenu.toggle_" << std::to_string(iToggleNumber)<<'_' << std::to_string((sti+1) % t_kBind.NameContainer.size()) << ';' << t_kBind.CmdStrContainer.at(sti)
							<< "; alias _cmenu.toggle_" << std::to_string(iToggleNumber) << " _cmenu.toggle_" << std::to_string(iToggleNumber) << '_'
							<< std::to_string((sti + 1) % t_kBind.NameContainer.size()) << "\"\n";
							InitRoutineFile << "alias _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string(sti)<<"\"cc_emit _#cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string(sti)<<"\"\n";
							CMenuCaptionFile << convert.from_bytes("\t\t\"_#cmenu.toggle_"+std::to_string(iToggleNumber)+'_'+std::to_string(sti)+"\" \""+t_kBind.NameContainer.at(sti)+"\"\n");
						}
						CMenuCFG<<"_#cmenu.toggle_" << std::to_string(iToggleNumber) << '\n';
						CMenuCFG<<"alias _cmenu.k_" << ((t_kBind.sKey.length() + CMENU_KEY_ALIAS_LENGTH > 32) 
						? t_kBind.sKey.substr(0, t_kBind.sKey.length() - CMENU_KEY_ALIAS_LENGTH) : t_kBind.sKey) << "\"_cmenu.toggle_" << std::to_string(iToggleNumber) << "\"\n";
						iToggleNumber++;
					}
					else {
						// Segment the captions between binds and titles, and toggle binds.
						auto prev_bind = std::find_if(std::make_reverse_iterator(kBind), CMenu->Entries.rend(), [](const auto& v) { return std::holds_alternative<Bind>(v); });
						if (kBind > CMenu->Entries.begin()
						&& (prev_bind == CMenu->Entries.rend() ? false : std::get<Bind>(*prev_bind).bToggleBind) == true) {
							CMenuCaptionFile << convert.from_bytes("\t\t\"_#cmenu." + CMenu->sRawName + "_seg_" + std::to_string(iTBindSegmentNum) + "\" \"" + t_kBind.NameContainer.front());
							CMenuCFG << "cc_emit _#cmenu." << CMenu->sRawName << "_seg_" << std::to_string(iTBindSegmentNum) << '\n';
							iTBindSegmentNum++;
						}
						else CMenuCaptionFile << convert.from_bytes(t_kBind.NameContainer.front());
						// Write the ending double quote when we hit the last bind or the next entry is a toggle bind.
						auto next_bind = std::find_if(kBind + 1, CMenu->Entries.end(), [](const auto& v) { return std::holds_alternative<Bind>(v); });
						if (kBind == (last_bind + 1).base()
						|| (std::holds_alternative<Bind>(*next_bind) && std::get<Bind>(next_bind == CMenu->Entries.end() ? *kBind : *next_bind).bToggleBind == true))
							CMenuCaptionFile<<L"\"\n";
						CMenuCFG<<"alias _cmenu.k_"<<((t_kBind.sKey.length() + CMENU_KEY_ALIAS_LENGTH > 32) ? t_kBind.sKey.substr(0, t_kBind.sKey.length() - CMENU_KEY_ALIAS_LENGTH) : t_kBind.sKey)<<"\""<<t_kBind.CmdStrContainer.at(0)<<"\"\n";
					}
				}
				else if (CMenu->Display == CMenuDisplayType::CONSOLE) {
					if (t_kBind.bToggleBind) {
						InitRoutineFile<<"alias _cmenu.toggle_" << std::to_string(iToggleNumber) << " _cmenu.toggle_" << std::to_string(iToggleNumber) << "_0\n";
						InitRoutineFile<<"alias _#cmenu.toggle_" << std::to_string(iToggleNumber) << " _#cmenu.toggle_" << std::to_string(iToggleNumber) << "_0\n";
						for (unsigned char sti=0u; sti < t_kBind.NameContainer.size(); sti++) {
							InitRoutineFile<<"alias _cmenu.toggle_" << std::to_string(iToggleNumber) << '_' << std::to_string(sti) << '\"'
							<< t_kBind.CmdStrContainer.at(sti) << "; alias _cmenu.toggle_"<<std::to_string(iToggleNumber) << " _cmenu.toggle_"
							<< std::to_string(iToggleNumber) << '_' << std::to_string((sti+1)%t_kBind.NameContainer.size()) << "; alias _#cmenu.toggle_"
							<< std::to_string(iToggleNumber) << " _#cmenu.toggle_" << std::to_string(iToggleNumber) << '_'
							<< std::to_string((sti+1) % t_kBind.NameContainer.size()) << "\"\n";
							InitRoutineFile<<"alias _#cmenu.toggle_" << std::to_string(iToggleNumber) << '_' << std::to_string(sti)<<"\"echo " << t_kBind.NameContainer.at(sti) << "\"\n";
						}
						CMenuCFG<<"_#cmenu.toggle_" << std::to_string(iToggleNumber) << '\n';
						CMenuCFG<<"alias _cmenu.k_" << t_kBind.sKey << " \"_cmenu.toggle_" << std::to_string(iToggleNumber) << "\"\n";
						iToggleNumber++;
					}
					else {
						CMenuCFG<<"echo "<< t_kBind.NameContainer.front() << '\n' << "alias _cmenu.k_" << t_kBind.sKey << " \"" << t_kBind.CmdStrContainer.front() << "\"\n";
					}
				}
				else if (CMenu->Display == CMenuDisplayType::NONE) {
					if (t_kBind.bToggleBind) {
						InitRoutineFile<<"alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _cmenu.toggle_"<<std::to_string(iToggleNumber)<<"_0\n";
						for (unsigned char sti=0u; sti < t_kBind.NameContainer.size(); sti++) {
							InitRoutineFile<<"alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string(sti)<<'\"'<<t_kBind.CmdStrContainer.at(sti)<<"; alias _cmenu.toggle_"<<std::to_string(iToggleNumber)<<" _cmenu.toggle_"<<std::to_string(iToggleNumber)<<'_'<<std::to_string((sti+1)%t_kBind.NameContainer.size())<<"\n";
						}
						CMenuCFG<<"alias _cmenu.k_"<<t_kBind.sKey<<" \"_cmenu.toggle_"<<std::to_string(iToggleNumber)<<"\"\n";
						iToggleNumber++;
					}
					else {
						CMenuCFG<<"alias _cmenu.k_"<<t_kBind.sKey<<" \""<<t_kBind.CmdStrContainer.front()<<"\"\n";;
					}
				}
			}
			else if (std::holds_alternative<std::string>(*kBind)) {
				CMenuCFG << std::get<std::string>(*kBind) << '\n';
			}
		}
		// 
		if (CMenu->Display == CMenuDisplayType::CONSOLE && (bUsedDisplayFlags & FL_DISPLAY_CONSOLE)) 
			CMenuCFG << "con_filter_enable 1\ncon_filter_text zzzzzzzzzzzzzzzzzzz\nalias _cmenu.exitmenu \"developer 0;con_filter_enable 0;clear\"";
		else if (CMenu->Display != CMenuDisplayType::CONSOLE && (bUsedDisplayFlags & FL_DISPLAY_CONSOLE)) 
			CMenuCFG << "con_filter_enable 0\ncon_filter_text \"\"\nalias _cmenu.exitmenu";
	}
	// We're done compiling so close the file streams.
	CMenuCaptionFile << "\t}\n}";
	CMenuCaptionFile.close();
	InitRoutineFile << "echo [Command Menu Generator] Initialized command menus!\n" << init_file_user_code << '\n';
	InitRoutineFile.close();
	//Done!
	std::cout << std::to_string(iBindCount) << " binds compiled.\n";
}
