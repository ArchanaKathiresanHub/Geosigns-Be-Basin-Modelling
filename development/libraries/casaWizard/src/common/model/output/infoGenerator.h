//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <string>

namespace casaWizard
{

class InfoGenerator
{
public:
   InfoGenerator();
   virtual ~InfoGenerator() = default;

   virtual void generateInfoTextFile() = 0;
   virtual void loadProjectReader(const std::string& projectFileName) = 0;

   void setFileName(const std::string& fileName);

protected:
   void writeTextToFile();
   void addHeader(const std::string& headerTitle);
   void addOption(const std::string& option, const std::string& optionValue);
   void addSectionSeparator();

private:
   void addHeaderSeparator();

   std::string infoText_;
   std::string fileName_;
};

}
