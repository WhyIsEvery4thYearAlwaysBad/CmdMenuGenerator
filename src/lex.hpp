#include <string>
// Convert to a safer string format for file and caption names. (Yes the duplicate input string is intentional.)
std::string formatRaw(std::string p_sInStr);
namespace Lexer {
	// Tokenize any string into needed Tokens for parsing.
	bool Tokenize(const std::string_view& str);
}