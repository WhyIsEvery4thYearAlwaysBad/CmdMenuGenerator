#include <cstring>
#include <iostream>
#include <cstdio>
#include <filesystem>

void ShowHelp() {
	std::cout<<R"(Usage: <cmg executable> <file> <args>
Arguments:
	-o, --output-dir	Output directory for the main files; Default path is "./customvoicemenu"
	-h, --help  		Display this menu.
)";
}

std::filesystem::path InputFilePath="";
bool bInputFileSet=false;
std::filesystem::path sOutputDir="./customvoicemenu";
/* Gets Information from launch options.
 Also validates that they actually work with a returned byte. 
	-1 - Exit with fail.
	0 - Continue with success.
	1 - Exit with sucess.
 Launch Options:
 -config - Defines your config path. Removed for the time being until it becomes necessary.
 -o, --output-dir <path> - Output path for folder with CFGs and Captions.
 -h, --help - Display this text. */
char EvaluateLaunchOptions(int p_iArgc, char** p_szArgv) {
	FILE* FileExists;
	char Status = (p_iArgc <= 1 ? -1 : 0);
	bool bHelpLaunchOPFound=false;
	for (int i = 1; i < p_iArgc; i++) {
		if (!strcmp(p_szArgv[i], "-o") || !strcmp(p_szArgv[i], "--output-dir")) {
			if ((i + 1) < p_iArgc) sOutputDir = p_szArgv[i+1];
			else {
				printf("error: Missing output path. [%s]\n", p_szArgv[i]);
				Status = -1;
				continue;
			}
			i++;
		}
		else if ((!strcmp(p_szArgv[i], "-h") || !strcmp(p_szArgv[i], "--help")) && Status != 1) {
			ShowHelp();
			Status = 1;
		}
		else if (!bInputFileSet) {
			bInputFileSet=true;
			InputFilePath=p_szArgv[i];
			// Validate file's existence.
			FileExists=fopen(InputFilePath.string().c_str(),"r");
			if (FileExists==nullptr) {
				std::cerr<<"error: Filepath "<<InputFilePath.string()<<" is invalid.\n";
				Status = -1;
			}
			fclose(FileExists);
		}
	}
	if (!bInputFileSet && Status != 1) {
		std::cerr << "error: No input file found.\n";
		Status = -1;
	}
	return Status;
}