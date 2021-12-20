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

      /// @brief update the simulation window if invalid values are found
      void upgradeSimulationWindow(const std::string& , int& windowXMin, int& windowXMax, int nodeX, int& scaleX);

      // @brief It is just to ensure that the original simulation window defined is within the acceptable limit defined by [0, nodeCount] in each direction. 
      // @details If the values are not found within the acceptable limit then reset it to the nearest limiting value
      void preProcessSimulationWindow(int& windowMin, int& windowMax, int nodeCount);

      /// @brief update deltaX 
      /// @details check and update deltaX based on NodeX rather than modelling mode. This is done as it has been observed that legacy scenarios with "Both" modelling mode can also have 2x2 nodes
      double upgradeDeltaX(const std::string& modellingMode, const double& deltaX, const int nodeX);

      /// @brief update deltaY 
      /// @details check and update deltaY based on NodeY rather than modelling mode. This is done as it has been observed that legacy scenarios with "Both" modelling mode can also have 2x2 nodes
      double upgradeDeltaY(const std::string& modellingMode, const double& deltaY, const int nodeY);

      /// @brief update description 
      /// @details check and append with the existing description
      std::string upgradeDescription(const std::string&, const std::string&);

   };
}

#endif



