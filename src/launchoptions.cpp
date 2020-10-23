#include <cstring>
#include <iostream>
#include <cstdio>
#include <filesystem>
char* inputfilename="";
char* ModName="";
bool usingconfig=false;
bool launchoptionfilefound=false;
bool launchoptionhelp=false;
extern std::filesystem::path TFdirectory;
// Gets Information from launch options.
// Also validates that they actually work with a returned boolean.
// Launch Options:
// -config=<path> - Defines your config path.
// -tf-path=<path> - Defines the tf path.
bool EvaluateLaunchOptions(int argc, char** argv) {
	FILE* FileExists;
	bool launchoptionsvalid=true;
	for (int i=1; i < argc; i++) {
		if (strncmp(argv[i],"-config=",8)==0) {
			if (usingconfig==true) {
				std::cerr<<"One config or the other.\n";
				launchoptionsvalid=false;
			}
			usingconfig=true;
			// Leave if it's blank
			if (strcmp(argv[i],"-config=")==0) {
				std::cerr<<"The mod path is supposed to be HERE! -config=<---\n";
				launchoptionsvalid=false;
			}
			ModName=argv[i]+8;
			FileExists=fopen(ModName,"r");
			if (FileExists==nullptr) {
				if (strcmp(argv[i],".vpk")==strlen(argv[i])-3) std::cerr<<"VPK file "<<ModName<< "does not exist.\n";
				else std::cerr<<"Folder "<<ModName<< " does not exist.\n";
				launchoptionsvalid=false;
				fclose(FileExists);	
			}
			fclose(FileExists);
		}
		else if (strncmp(argv[i],"-tf-path=",9)==0) {
			// tf path cannot be blank.
			if (strcmp(argv[i],"-tf-path=")==0) {
				std::cerr<<"TF path cannot be blank.\n";
				launchoptionsvalid=false;
			}
			TFdirectory=argv[i]+9;
			if (TFdirectory.string().back()!='/') TFdirectory+='/';
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