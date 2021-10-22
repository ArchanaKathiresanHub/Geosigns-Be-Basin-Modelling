//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lasSectionReader.h"

#include <sstream>

namespace casaWizard
{

LASSectionReader::LASSectionReader(const std::vector<std::string>& section, WellData& welldata, ImportOptions& importOptions) :
  section_{section},
  welldata_{welldata},
  importOptions_{importOptions}
{
}

std::vector<std::string> LASSectionReader::splitLine(const std::string& line) const
{
  // Format of every LAS line (except in the ~A section):
  // MNEM .UNIT      DATA                 :DESCRIPTION OF MNEMONIC
  std::vector<std::string> splittedLine;

  std::string todo = line;
  std::vector<char> delimiters = {'.', ' ', ':'};
  for (const char delimiter : delimiters)
  {
    if (todo.find(delimiter) == std::string::npos)
    {
      return {};
    }

    const std::string beforeDelimiter = todo.substr(0, todo.find(delimiter));
    splittedLine.push_back(trim(beforeDelimiter));
    todo = todo.substr(todo.find(delimiter) + 1);
  }

  splittedLine.push_back(trim(todo));

  return splittedLine;
}

bool LASSectionReader::lineInvalid(const std::vector<std::string>& splittedLine) const
{
  //   0    1         2                               3
  // MNEM .UNIT      DATA                 :DESCRIPTION OF MNEMONIC
  //                  ^
  //                 DATA part of the line is empty, so splittedLine[2].empty()
  //                 or line does not have right size at all (splittedLine.size() != 4)
  return (splittedLine.size() != 4 || splittedLine[2].empty());
}

std::string LASSectionReader::trim(const std::string& input) const
{
  return trimLeft(trimRight(input));
}

std::string LASSectionReader::trimLeft(const std::string& input) const
{
  std::string trimCharacters = " \n\r\t\f\v";
  return input.find_first_not_of(trimCharacters) == std::string::npos ? "" : input.substr(input.find_first_not_of(trimCharacters));
}

std::string LASSectionReader::trimRight(const std::string& input) const
{
  std::string trimCharacters = " \n\r\t\f\v";
  return input.find_last_not_of(trimCharacters) == std::string::npos ? "" : input.substr(0, input.find_last_not_of(trimCharacters) + 1);
}


} // namespace casaWizard
