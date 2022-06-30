#include <string>
using namespace std;
#pragma once

class TextOutput {
public:
	static void OutputText(std::string const& details, std::string const& state, std::string const& largeIcon, std::string const& largeText, std::string const& smallIcon, std::string const& smallText);
};

extern string gameName;
extern int FailRandom;
extern bool TO_StateHasChanged;