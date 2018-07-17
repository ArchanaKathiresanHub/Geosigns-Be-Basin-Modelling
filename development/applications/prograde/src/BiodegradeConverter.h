//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_BIODEGARDE_CONVERTER_H
#define PROGRADE_BIODEGARDE_CONVERTER_H

//cmbAPI
#include "BiodegradeManager.h"

namespace Prograde
{
   /// @class 
   class BiodegradeConverter {

   public:

      BiodegradeConverter() = default;
      BiodegradeConverter(const BiodegradeConverter &) = delete;
      BiodegradeConverter& operator=(const BiodegradeConverter &) = delete;
      ~BiodegradeConverter() = default;

      /// @brief update biodegradation constants in project3d file (BioDegradIoTbl)
      /// @details check and update biodegradation constants if it deviates from default value or deviates from valid range
      void upgradeBioConstants(const std::string & BioConsName, const double BioConsValue, double & BioConsFromP3dFile);
      
   };
}

#endif
