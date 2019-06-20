//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_FRACTURE_PRESSURE_MODEL_CONVERTER_H
#define PROGRADE_FRACTURE_PRESSURE_MODEL_CONVERTER_H

#include "cmbAPI.h"
//cmbAPI
#include "FracturePressureManager.h"
//DataAccess
#include "ProjectHandle.h"

namespace Prograde
{
   /// @class FracturePressureModelConverter Converts bottom boundary models as well as crust/mantle property model
   class  FracturePressureModelConverter {

   public:

      FracturePressureModelConverter() = default;
      FracturePressureModelConverter(const FracturePressureModelConverter &) = delete;
      FracturePressureModelConverter& operator=(const FracturePressureModelConverter &) = delete;

      ~FracturePressureModelConverter() = default;

      /// @brief update fracture pressure models in project3d file 
      /// @details check and update fracture pressure model if it uses None model
      std::string upgradeFracturePressureModel();
      
      };
}

#endif


