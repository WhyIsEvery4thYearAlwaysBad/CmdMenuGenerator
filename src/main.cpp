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
std::filesystem::path TFdirectory="C:/Program Files (x86)/Steam/steamapps/common/Team Fortress 2/tf/";
std::map<std::string,std::string> keymap={{"#cvm.defaultcolor",""},{"#cvm.boldbydefault","false"},{"#cvm.italicizebydefault","false"},{"#cvm.resetkeys","\"bind 1 slot1; bind 2 slot2; bind 3 slot3; bind 4 slot4; bind 5 slot5; bind 6 slot6; bind 7 slot7; bind 8 slot8; bind 9 slot9; bind 0 slot10\""},{"#cvm.resetkeys.scout",""},{"#cvm.resetkeys.soldier",""},{"#cvm.resetkeys.pyro",""},{"#cvm.resetkeys.demoman",""},{"#cvm.resetkeys.heavy",""},{"#cvm.resetkeys.engineer",""},{"#cvm.resetkeys.medic",""},{"#cvm.resetkeys.sniper",""},{"#cvm.resetkeys.spy",""}};
std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> convert;
extern char* inputfilename;
extern char* ModName;
extern bool usingconfig;
extern bool launchoptionfilefound;
extern bool launchoptionhelp;
int main(int argc, char** argv) {
	// Launch options
	// -config=<name> Location to mod
	if (!EvaluateLaunchOptions(argc,argv)) return -1;
	// if (launchoptionfilefound==false && argc>1) std::cerr<<"Where the hell is the file?\n";
	if (argc<=1 || launchoptionhelp==true) {
		std::cout<<"USEAGE: cvm_generator.exe <file> <args>\nArguments:\n-usingconfig\n-tf-path= - Path to tf folder.\n-config= - Path to your config. Isn't actually used, but it still must be valid.\n";
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
	printf("Tokenizing Stream.\n");
	Tokenize(InFileContent,depth);
	printf("Tokenized Stream. Parsing Tokens.\n");
	unsigned long bindcount=0u;
	if (!ParseTokens(pages,concise,depth,bindcount)) return -1;
	printf("Parsed Tokens. (%lu binds compiled.) \nCreating main cfg.\n",bindcount);
	if (depth!=0) {printf("Missing a brace somewhere...");return -1;}
	// Directories
	std::filesystem::create_directories(std::filesystem::temp_directory_path().string()+"customvoicemenu/cfg");
	// Main exec.
	std::ofstream exec(std::filesystem::temp_directory_path().string()+"customvoicemenu/cfg/activate_cvm.cfg");
	exec<<R"(closecaption 1
cc_lang customvoicemenu
alias cvm.nullkeys "alias cvm.1 ; alias cvm.2 ; alias cvm.3 ; alias cvm.4 ; alias cvm.5 ; alias cvm.6 ; alias cvm.7 ; alias cvm.8 ; alias cvm.9"
alias cvm.exitmenu "cc_linger_time 0; cc_emit #cvm.clear_screen; alias +cvm.opencvm cvm.cvmstate0; cvm.resetkeys"
alias +cvm.opencvm cvm.cvmstate0
alias -cvm.opencvm ;
alias cvm.cvmstate0 "cvm.openmenu; alias -cvm.opencvm alias +cvm.opencvm cvm.cvmstate1; bind 1 cvm.1; bind 2 cvm.2; bind 3 cvm.3; bind 4 cvm.4; bind 5 cvm.5; bind 6 cvm.6; bind 7 cvm.7; bind 8 cvm.8; bind 9 cvm.9"
alias cvm.cvmstate1 "cvm.exitmenu; alias -cvm.opencvm alias +cvm.opencvm cvm.cvmstate0"
cvm.exitmenu
alias cvm.resetkeys )"<<keymap["#cvm.resetkeys"]<<'\n';
	for (auto& page : pages) {
		exec<<"alias cvm.menu_"<<page.first.formatted_title<<" alias cvm.openmenu \"exec cvm_"<<page.first.formatted_title<<"\""<<'\n';
	}
	exec << "alias cvm.openmenu ;\n";
	// Write override reset keys to the class configs if possible. If -config= is present, print to console.
	if (usingconfig==true) {
		if (overrideexists==true) for (auto& s : keymap) {
			printf("Enter the reset key overrides into your class cfgs:\n");
			if (s.first.find("#cvm.resetkeys.")==0 && s.second!="") {
				printf("alias %s %s\n",s.first.c_str(),s.second.c_str());
			}
		}
	}
	else {
		if (overrideexists==true) for (auto& s : keymap) {
			if (s.first.substr(0,14)=="#cvm.resetkeys." && s.second!="") {
				std::ofstream classcfg(std::filesystem::temp_directory_path().string()+"customvoicemenu/cfg/"+s.first.substr(14,s.first.length()-14)+".cfg");
				classcfg<<"alias "<<s.first<<' '<<s.second<<'\n';
			}
		}
	}
	// Conversion to UCS-2.
	std::locale utf16(std::locale::classic(),new std::codecvt_utf16<wchar_t, 0xffff, std::little_endian>);
	printf("Done. Creating caption file.\n");
	// Create the captions directory once.
	if (!std::filesystem::exists(std::filesystem::temp_directory_path().string()+"customvoicemenucaptions")) std::filesystem::create_directories(std::filesystem::temp_directory_path().string()+"customvoicemenucaptions/resource");
	std::wofstream captionfile(std::filesystem::temp_directory_path().string()+"customvoicemenucaptions/resource/closecaption_customvoicemenu.txt",std::ios_base::binary);
	// std::cout<<"File loc:"<<captionfile.getloc().name()<<'\n';
	// std::cout<<"utf16 loc:"<<captionfile.getloc().name()<<'\n';
	captionfile.imbue(utf16);
	captionfile<<(wchar_t)0xFEFF; // BOM.
	captionfile<<"\"lang\"\n{\n\t\"Language\" \"customvoicemenu\"\n\t\"Tokens\"\n\t{\n\t\t\"#cvm.clear_screen\" \"<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	<cr>	\"\n";
	unsigned long togglenumber=0u;
	std::string cfgpath;
	printf("Creating CFGs and Captions.\n");
	// Macro to reduce the meth(od) path names.
	#define GetBind(iter) pages.at(pi).first.binds.at(iter)
	for (unsigned int pi=0; pi < pages.size(); pi++) {
		cfgpath=std::filesystem::temp_directory_path().string()+"customvoicemenu/cfg/cvm_"+pages.at(pi).first.formatted_title+".cfg";
		// If a page exists with the same name, append a number to the end.
		if (std::filesystem::exists(cfgpath)) {
			for (unsigned short duplicate_num=0; duplicate_num < INT16_MAX; duplicate_num++) {
				if (!std::filesystem::exists(cfgpath)) {
					cfgpath+='_'+std::to_string(duplicate_num);
					break;
				}
			}
		}
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
		cfgfile<<"cvm.nullkeys\n";
		// Write captions
		if (pages.at(pi).first.binds.front().name.size()<2) {
			captionfile<<convert.from_bytes("\t\t\"#cvm."+pages.at(pi).first.formatted_title+"\" \"<len:10000>");
			if (keymap["#cvm.italicizedbydefault"].find("true")!=std::string::npos) captionfile<<"<I>";
			if (keymap["#cvm.boldbydefault"].find("true")!=std::string::npos) captionfile<<"<B>";
			captionfile<<convert.from_bytes(keymap["#cvm.defaultcolor"]);
		}
		for (unsigned int i=0; i < pages.at(pi).first.binds.size(); i++) {
			if (GetBind(i).name.size()<2) {
				if (i!=0 && pages.at(pi).first.binds.at(i-1).name.size()>1) captionfile<<convert.from_bytes("\t\t\"#cvm."+GetBind(i).formatted_name.at(0)+"\" \"<len:10000>");
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
	}
	captionfile<<"\t}\n}";
	exec.close(), captionfile.close();
	Sleep(1000);
	printf("Done. Compiling Captions\n------------------------\n");
	// Compile the caption file.
	STARTUPINFOW si{};
	si.cb=sizeof(si);
	PROCESS_INFORMATION pi{};
	std::string exepath=argv[0];
	std::wstring captioncompilerexec=convert.from_bytes(TFdirectory.parent_path().parent_path().string()+"/bin/captioncompiler.exe closecaption_customvoicemenu.txt -game \""+TFdirectory.string()+'\"');
	std::wstring workdir=convert.from_bytes(std::filesystem::temp_directory_path().string()+"customvoicemenucaptions/resource");
	auto captioncommand=CreateProcessW(NULL,captioncompilerexec.data(),NULL,NULL,FALSE,IDLE_PRIORITY_CLASS,NULL,workdir.c_str(),&si,&pi);
	if (!captioncommand) {
		printf("Couldn't run captioncompiler.exe. Error Code: %lu\n",GetLastError());
	}
	WaitForSingleObject(pi.hProcess,INFINITE);
	if (!CloseHandle(pi.hProcess)||!CloseHandle(pi.hThread)) {
		printf("Well shit.");
	}
	Sleep(1000);
	// Move captions folder to custom directory if it already doesn't exist.
	// <rant>It's so irritating that the .dat file complies to tf/resource and not the .txt's home directory..</rant>
	if (std::filesystem::exists(TFdirectory.string()+"custom/customvoicemenucaptions/resource")) std::filesystem::remove_all(TFdirectory.string()+"custom/customvoicemenucaptions/resource");
	std::filesystem::create_directories(TFdirectory.string()+"custom/customvoicemenucaptions");
	std::filesystem::copy(std::filesystem::temp_directory_path().string()+"customvoicemenucaptions/resource",TFdirectory.string()+"custom/customvoicemenucaptions/resource");
	if (!std::filesystem::exists(std::filesystem::temp_directory_path().string()+"customvoicemenucaptions/resource/closecaption_customvoicemenu.dat")) std::filesystem::rename(TFdirectory.string()+"resource/closecaption_customvoicemenu.dat",TFdirectory.string()+"custom/customvoicemenucaptions/resource/closecaption_customvoicemenu.dat");
	// Convert the cfgs to vpk
	std::cout<<"------------------------\nDone. Compiling to VPK\n------------------------\n";
	STARTUPINFOW vpksi{};
	vpksi.cb=sizeof(vpksi);
	PROCESS_INFORMATION vpkpi{};
	std::wstring vpkexe=convert.from_bytes(TFdirectory.parent_path().parent_path().string()+"/bin/vpk.exe customvoicemenu");
	workdir=convert.from_bytes(std::filesystem::temp_directory_path().string());
	auto vpkcmd=CreateProcessW(NULL,vpkexe.data(),NULL,NULL,FALSE,IDLE_PRIORITY_CLASS,NULL,workdir.c_str(),&vpksi,&vpkpi);
	if (!vpkcmd) {
		printf("Couldn't run vpk.exe. Error Code: %lu\n",GetLastError());
	}
	WaitForSingleObject(vpkpi.hProcess,INFINITE);
	if (!CloseHandle(vpkpi.hProcess) || !CloseHandle(vpkpi.hThread)) {
		printf("Couldn't stop VPK process.");
	}
	// Move vpk to the custom folder.
	if (std::filesystem::exists(TFdirectory.string()+"custom/customvoicemenu.vpk")) std::filesystem::remove(TFdirectory.string()+"custom/customvoicemenu.vpk");
	std::filesystem::rename(std::filesystem::temp_directory_path().string()+"customvoicemenu.vpk",TFdirectory.string()+"custom/customvoicemenu.vpk");
	// Remove Temporary folders
	std::filesystem::remove_all(std::filesystem::temp_directory_path().string()+"customvoicemenu");
	std::filesystem::remove_all(std::filesystem::temp_directory_path().string()+"customvoicemenucaptions");
	//Done!
	std::cout<<"Done! Now just insert exec activate_cvm somewhere.\n";
}
