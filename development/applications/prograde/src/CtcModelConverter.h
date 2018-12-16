//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_CTC_MODEL_CONVERTER_H
#define PROGRADE_CTC_MODEL_CONVERTER_H

//cmbAPI
#include "CtcManager.h"

namespace Prograde
{
   /// @class CtcModelConverter Converts bottom boundary models as well as crust/mantle property model
   class  CtcModelConverter {

   public:

      CtcModelConverter() = default;
      CtcModelConverter(const CtcModelConverter &) = delete;
      CtcModelConverter& operator=(const CtcModelConverter &) = delete;
      ~CtcModelConverter() = default;


      /// @brief Updates the TectonicFlag of CTCRiftingHistoryIoTbl
      /// @details Sets the tectonic context of the formation for each age/timestep  
      std::string upgradeTectonicFlag(const double & Age, const double & Min);
   };
}

#endif

