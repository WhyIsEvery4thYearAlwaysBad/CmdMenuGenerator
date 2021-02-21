#include <cstring>
#include <iostream>
#include <cstdio>
#include <filesystem>
void ShowHelp() {
	std::cout<<R"(Usage: <cvm_generate executable> <file> <args>
Arguments:
	-o, --output-dir	Output directory for the main files; Default path is "./customvoicemenu"
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
// -o, --output-dir <path> - Output path for folder with CFGs and Captions.
// /? - Help
bool EvaluateLaunchOptions(int argc, char** argv) {
	FILE* FileExists;
	bool launchoptionsvalid=true;
	for (int i=1; i < argc; i++) {
		if (!strcmp(argv[i],"-o") || !strcmp(argv[i],"--output-dir")) {
			if ((i+1) < argc) outputdir=argv[i+1];
			else {
				printf("error: Missing output path. [%s]\n",argv[i]);
				launchoptionsvalid=false;
				continue;
			}
			i++;
		}
		else if (strcmp(argv[i],"/?")==0) launchoptionhelp=true;
		else if (strchr(argv[i],'-')==nullptr) {
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