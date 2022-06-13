#pragma once

#include <string>
#include <vector>

//Small library to group the functions used to create casa commands from wizard targets and interpret these commands in casa.
//This allows for unit testing that the translation is done consistently.

namespace casaCmdInterface
{

//This function is used to create consistent target (wizards) and observable (casa) identifiers,
//such that the casa output can be easily linked to wizard targets.
std::string stringVecToStringWithNoSpaces(std::vector<std::string> strings, std::string delimiter);

std::string casaCommandFromStrVec(std::vector<std::string> stringVec);
std::vector<std::string> stringVecFromCasaCommand(const std::string& cmdLine, std::string& cmdID, int lineNum = -1);

}; //casaCmdInterface

