#include <string>
#include <vector>
std::string Format(std::string str);
unsigned int GetParentPage(const unsigned int& p_i);
struct Bind {
	unsigned char numberkey=0;
	std::vector<std::string> name, formatted_name, /* <---- For captions */ commandstr;
	Bind() {}
	Bind(const unsigned char& p_nkey, const std::string& p_name, const std::string& p_fname, const std::string& p_cmdstr)
	: numberkey(p_nkey) {
		name.push_back(p_name);
		formatted_name.push_back(p_fname);
		commandstr.push_back(p_cmdstr);
	}
	~Bind() {}
};
struct Page {
	std::string title;
	std::string formatted_title;
	std::vector<Bind> binds;

	Page() {}
	Page(const std::string& p_title, const std::string& p_ftitle)
	: title(p_title), formatted_title(p_ftitle) {}
	~Page() {}
};