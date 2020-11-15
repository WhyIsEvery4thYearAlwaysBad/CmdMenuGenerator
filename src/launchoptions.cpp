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
const char* inputfilename="";
const char* ModName="";
bool usingconfig=false;
bool launchoptionfilefound=false;
bool launchoptionhelp=false;
std::string outputdir="./customvoicemenu";
// Gets Information from launch options.
// Also validates that they actually work with a returned boolean.
// Launch Options:
// -config - Defines your config path.
// -o=<path> - Output path for folder.
// /? - Help
bool EvaluateLaunchOptions(int argc, char** argv) {
	FILE* FileExists;
	bool launchoptionsvalid=true;
	for (int i=1; i < argc; i++) {
		if (strncmp(argv[i],"-o=",3)==0) {
			// tf path cannot be blank.
			if (strcmp(argv[i],"-o=")==0 || strcmp(argv[i],"-o=\"\"")==0) {
				std::cerr<<"Output path cannot be blank.\n";
				launchoptionsvalid=false;
			}
			std::filesystem::path temp=outputdir;
			const std::string invalidfilenames=". .. / // \\ \\\\";
			if (invalidfilenames.find(temp.filename().string())!=std::string::npos) {
				std::cerr<<"Bad output directory path \""<<outputdir<<"\"\n";
				launchoptionsvalid=false;
			}
			outputdir=argv[i]+(argv[i][3]=='"' ? 4 : 3);
			if (outputdir.back()=='"') outputdir.pop_back();
		}
		else if (strcmp(argv[i],"/?")==0) {
			ShowHelp();
			launchoptionhelp=true;
		}
		else if (strchr(argv[i],'-')!=0||strchr(argv[i],'-')!=(char*)1) {
			if (launchoptionfilefound) {
				std::cerr<<"One file at a time please!\n";
				launchoptionsvalid=false;
			}
			launchoptionfilefound=true;
			inputfilename=argv[i];
			// Validate file's existence.
			FileExists=fopen(inputfilename,"r");
			if (FileExists==nullptr) {
				std::cerr<<"Input path \""<<inputfilename<< "\" does not exist.\n";
				fclose(FileExists);
				launchoptionsvalid=false;
			}
			fclose(FileExists);
		}
	}
	return launchoptionsvalid;
}