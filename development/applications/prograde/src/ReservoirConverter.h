//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_RESERVOIR_CONVERTER_H
#define PROGRADE_RESERVOIR_CONVERTER_H

//cmbAPI
#include "ReservoirManager.h"

namespace Prograde
{
   /// @class ReservoirConverter Converts BPA1 layerwise reservoir fields into BPA2 global values
   class ReservoirConverter {

   public:

      ReservoirConverter() = default;
      ReservoirConverter(const ReservoirConverter &) = delete;
      ReservoirConverter& operator=(const ReservoirConverter &) = delete;
      ~ReservoirConverter() = default;

      /// @brief Upgrades the corresponding ReservoirIoTbl      
      void trapCapacityLogic(const double valueProject3d, double & globalValue);
      void blockingPermeabilityLogic(const size_t resId, const double valueProject3d, double & globalValue);
      void bioDegradIndLogic(const int valueProject3d, int & globalValue);
      void oilToGasCrackingIndLogic(const int valueProject3d, int & globalValue);
      void blockingIndLogic(const int valueProject3d, int & globalValue);
   };
}

#endif
