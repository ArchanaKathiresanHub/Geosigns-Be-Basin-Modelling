//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "infoGenerator.h"

#include <fstream>

namespace casaWizard
{

InfoGenerator::InfoGenerator() :
  infoText_{""},
  fileName_{""}
{
}

void InfoGenerator::setFileName(const std::string &fileName)
{
  fileName_ = fileName;
}

void InfoGenerator::writeTextToFile()
{
  if (!fileName_.empty())
  {
    std::ofstream outfile;
    outfile.open(fileName_, std::ios_base::app); // append instead of overwrite
    outfile << infoText_;
    outfile.close();
  }
}

void InfoGenerator::addHeader(const std::string& headerTitle)
{
  addHeaderSeparator();
  infoText_ += headerTitle + "\n";
  addHeaderSeparator();
}

void InfoGenerator::addOption(const std::string& option, const std::string& optionValue)
{
  infoText_ += option + ": " + optionValue + "\n";
}

void InfoGenerator::addSectionSeparator()
{
  infoText_ += "###################################################\n";
}

void InfoGenerator::addHeaderSeparator()
{
  infoText_ += "---------------------------------------------------\n";
}

}
