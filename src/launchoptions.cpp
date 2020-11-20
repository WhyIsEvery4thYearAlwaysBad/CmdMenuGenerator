#include <cstring>
#include <iostream>
#include <cstdio>
#include <filesystem>
void ShowHelp() {
	std::cout<<R"(Usage: <cvm_generate executable> <file> <args>
Arguments:
	-o= - Output directory. (Default path is "./customvoicemenu")
	/? - Help)";
}
std::filesystem::path inputfilename="";
bool usingconfig=false;
bool launchoptionfilefound=false;
bool launchoptionhelp=false;
std::filesystem::path outputdir="./customvoicemenu";
// Gets Information from launch options.
// Also validates that they actually work with a returned boolean.
// Launch Options:
// -config - Defines your config path. Removed for the time being until it becomes necessary.
// -o=<path> - Output path for folder.
// /? - Help
bool EvaluateLaunchOptions(int argc, char** argv) {
	FILE* FileExists;
	bool launchoptionsvalid=true;
	for (int i=1; i < argc; i++) {
		if (strncmp(argv[i],"-o=",3)==0) {
			// tf path cannot be blank.
			if (strcmp(argv[i],"-o=")==0/* || strcmp(argv[i],"-o=\"\"")==0*/) {
				std::cerr<<"Output path cannot be blank.\n";
				launchoptionsvalid=false;
				continue;
			}
			std::filesystem::path temp=argv[i]+3;
			const std::string invalidfilenames=". .. / // \\ \\\\";
			if (invalidfilenames.find(temp.filename().string())!=std::string::npos) {
				std::cerr<<"Bad output directory path "<<temp<<"\n";
				launchoptionsvalid=false;
				continue;
			}
			outputdir=temp.string();
		}
		else if (strcmp(argv[i],"/?")==0) {
			ShowHelp();
			launchoptionhelp=true;
		}
		else if (strchr(argv[i],'-')!=0 || strchr(argv[i],'-')!=(char*)1) {
			if (launchoptionfilefound) {
				std::cerr<<"One file at a time please!\n";
				launchoptionsvalid=false;
			}
			launchoptionfilefound=true;
			inputfilename=argv[i];
			// Validate file's existence.
			FileExists=fopen(inputfilename.string().c_str(),"r");
			if (FileExists==nullptr) {
				std::cerr<<"Input path "<<inputfilename<< " does not exist.\n";
				launchoptionsvalid=false;
			}
			fclose(FileExists);
		}
	}
	return launchoptionsvalid;
}