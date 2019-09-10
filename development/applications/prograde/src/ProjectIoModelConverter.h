//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_PROJECTIO_MODEL_CONVERTER_H
#define PROGRADE_PROJECTIO_MODEL_CONVERTER_H

#include "cmbAPI.h"
//cmbAPI
#include "ProjectDataManager.h"
//DataAccess
#include "ProjectHandle.h"

namespace Prograde
{
   /// @class ProjectIoModelConverter Converts ProjectIoTbl data 
   class  ProjectIoModelConverter {

   public:

      ProjectIoModelConverter() = default;
      ProjectIoModelConverter(const ProjectIoModelConverter&) = delete;
      ProjectIoModelConverter& operator=(const ProjectIoModelConverter&) = delete;

      ~ProjectIoModelConverter() = default;

      /// @brief update number of nodes in x-direction of origin 
      /// @details Update the number of x-nodes depending upon modellingMode if out-of-range values are found.
	  //Also update the windowing size as needed.
      int upgradeNodeX(const std::string&, const int&, const int&, int&);

      /// @brief update number of nodes in y-direction of origin 
      /// @details Update the number of y-nodes depending upon modellingMode if out-of-range values are found 
	  //Also update the windowing size as needed.
      int upgradeNodeY(const std::string&, const int&, const int&, int&);

      /// @brief update deltaX 
      /// @details check and update deltaX for 1d scenario if default values of BPA2 is not used
      double upgradeDeltaX(const std::string&, const double&);

      /// @brief update deltaY 
      /// @details check and update deltaY for 1d scenario if default values of BPA2 is not used
      double upgradeDeltaY(const std::string&, const double&);

      /// @brief update modelling mode 
      /// @details check and update modelling mode for 1d/both scenario as BPA2 does not have these modes
      std::string upgradeModellingMode(const std::string&);

      /// @brief update description 
      /// @details check and append with the existing description
      std::string upgradeDescription(const std::string&, const std::string&);

   };
}

#endif



