//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "CtcModelConverter.h"

//utilities
#include "LogHandler.h"

using namespace mbapi;

std::string Prograde::CtcModelConverter::upgradeTectonicFlag(const double & Age, const double & Min)
{
   std::string tectonicFlag;
   if (Age > Min)
      tectonicFlag = "Active Rifting";
   else if (Age == 0.0)
      tectonicFlag = "Flexural Basin";
   else
      tectonicFlag = "Passive Margin";
   return tectonicFlag;
}


