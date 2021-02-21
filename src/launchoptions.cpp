#include <cstring>
#include <iostream>
#include <cstdio>
#include <filesystem>
void ShowHelp() {
	std::cout<<R"(Usage: <cvm_generate executable> <file> <args>
Arguments:
	-o, --output-dir	Output directory for the main files; Default path is "./customvoicemenu"
	-h, --help  		Display this menu.)";
}
std::filesystem::path inputfilename="";
bool usingconfig=false;
bool launchoptionfilefound=false;
std::filesystem::path outputdir="./customvoicemenu";
// Gets Information from launch options.
// Also validates that they actually work with a returned boolean.
// Launch Options:
// -config - Defines your config path. Removed for the time being until it becomes necessary.
// -o, --output-dir <path> - Output path for folder with CFGs and Captions.
// -h, --help - Display this text.
bool EvaluateLaunchOptions(int argc, char** argv) {
	FILE* FileExists;
	bool launchoptionsvalid=(argc<=1 ? false : true);
	bool helpran=false;
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
		else if ((!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help")) && !helpran) {
			ShowHelp();
			launchoptionsvalid=false;
			helpran=true;
		}
		else if (!launchoptionfilefound) {
			launchoptionfilefound=true;
			inputfilename=argv[i];
			// Validate file's existence.
			FileExists=fopen(inputfilename.string().c_str(),"r");
			if (FileExists==nullptr) {
				std::cerr<<"error: Filepath "<<inputfilename.string()<<" is invalid.\n";
				launchoptionsvalid=false;
			}
			fclose(FileExists);
		}
	}
	if (launchoptionfilefound!=true) {
		std::cerr<<"error: No input file found.\n";
		launchoptionsvalid=false;
	}
	return launchoptionsvalid;
}