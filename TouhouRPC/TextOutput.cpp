#include "TextOutput.h"
#include <iostream>

#include "games/TouhouBase.h"

using namespace std;

string gameName = "";
int FailRandom;
bool TO_StateHasChanged;

    void TextOutput::OutputText(std::string const& details, std::string const& state, std::string const& largeIcon, std::string const& largeText, std::string const& smallIcon, std::string const& smallText) {
        if (!gameName.empty()) {
            fstream fout("D:\\OBS_Scene\\THRPC_Output.txt", ofstream::out);
            fout << "" << gameName << "\n\n";
            fout << "" << largeText << "\n";
            fout << "" << smallText << "\n";
            fout << "" << details << "\n";
            fout << "" << state << "\n";
            fout << flush; fout.close();
        }
        else {
            printf("[WriteToFile] Skipped an update.\n");
        }
    }
