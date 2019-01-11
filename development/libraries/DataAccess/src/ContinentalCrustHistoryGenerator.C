//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Interface/ContinentalCrustHistoryGenerator.h"

//std
#include <limits>
#include <stdexcept>

//utilities
#include "LogHandler.h"
#include "NumericFunctions.h"

//DataAccess
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "Interface/Snapshot.h"
#include "Interface/CrustFormation.h"
#include "Interface/Surface.h"
#include "Interface/PaleoFormationProperty.h"
#include "Interface/ProjectHandle.h"
#include "Interface/ApplicationGlobalOperations.h"

//DataModel
#include "AbstractValidator.h"

using namespace DataAccess::Interface;

void ContinentalCrustHistoryGenerator::createCrustThickness(const BottomBoundaryConditions mode)
{
   switch (mode)
   {
   case FIXED_BASEMENT_TEMPERATURE:
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::SECTION) <<
         "Initialyze crust for Basic Crust Thinning History bottom boundary conditions";
      createFixedBasementTemperatureCrust();
      break;
   case ADVANCED_LITHOSPHERE_CALCULATOR:
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::SECTION) <<
         "Initialyze crust for Advanced Lithospheric Calculator (ALC) bottom boundary conditions";
      createAdvancedLithosphereCalculatorCrust();
      break;
   case MANTLE_HEAT_FLOW:
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::SECTION) <<
         "Initialyze crust for Heat Flow History bottom boundary conditions";
      createMantleHeatFlowCrust();
      break;
   default:
      throw std::invalid_argument("Unknown bottom boundary condition mode");
   }
}

void ContinentalCrustHistoryGenerator::createFixedBasementTemperatureCrust()
{
   m_crustThicknessHistory.reallocate(&m_activityGrid);
   PaleoFormationPropertyList* crustThicknesses = m_crust.getPaleoThicknessHistory();

   for (auto & crustThicknesse : *crustThicknesses)
   {
      const auto* thicknessInstance = dynamic_cast<const PaleoFormationProperty*>(crustThicknesse);
      const auto* thicknessMap = dynamic_cast<const GridMap*>(thicknessInstance->getMap(CrustThinningHistoryInstanceThicknessMap));
      const double age = thicknessInstance->getSnapshot()->getTime();

      thicknessMap->retrieveData(true);

      for (unsigned int i = thicknessMap->firstI(); i <= thicknessMap->lastI(); ++i)
      {
         for (unsigned int j = thicknessMap->firstJ(); j <= thicknessMap->lastJ(); ++j)
         {
            if (m_validator.isValid(i, j))
            {
               m_crustThicknessHistory(i, j).AddPoint(age, thicknessMap->getValue(i, j));
            }
         }
      }
      
      thicknessMap->restoreData(false, true);

   }

   delete crustThicknesses;
}


void ContinentalCrustHistoryGenerator::createAdvancedLithosphereCalculatorCrust()
{
   PaleoFormationPropertyList* crustThicknesses = m_crust.getPaleoThicknessHistory();

   m_crustThicknessHistory.reallocate(&m_activityGrid);

   double localInitialCrustThickness = numeric_limits<double>::lowest();
   double localMaximumCrustThickness = numeric_limits<double>::lowest();
   const double basinAge = m_crust.getTopSurface()->getSnapshot()->getTime();

   const double oldestMapAge = dynamic_cast<const PaleoFormationProperty*>(*(crustThicknesses->rbegin()))
                               ->getSnapshot()->getTime();

   bool flag = false;
   for (auto & crustThicknesse : *crustThicknesses)
   {
      const auto* thicknessInstance = dynamic_cast<const PaleoFormationProperty*>(crustThicknesse);
      const auto* thicknessMap = dynamic_cast<const GridMap*>(thicknessInstance->getMap(CrustThinningHistoryInstanceThicknessMap));
      const double age = thicknessInstance->getSnapshot()->getTime();

      thicknessMap->retrieveData(true);
      
      for (unsigned int i = thicknessMap->getGrid()->firstI(true); i <= static_cast<unsigned int>(thicknessMap->getGrid()->lastI(true)); ++i)
      {
         for (unsigned int j = thicknessMap->getGrid()->firstJ(true); j <= static_cast<unsigned int>(thicknessMap->getGrid()->lastJ(true)); ++j)
         {
            if (m_validator.isValid(i, j))
            {
               double currentThickness = thicknessMap->getValue(i, j);
               m_crustThicknessHistory(i, j).AddPoint(age, currentThickness);
               if (age == basinAge)
               {
                  flag = true;
                  //dataAccess lib
                  localInitialCrustThickness = NumericFunctions::Maximum(
                     localInitialCrustThickness, currentThickness);
               }
               if (age == oldestMapAge)
               {
                  localMaximumCrustThickness = NumericFunctions::Maximum(
                     localMaximumCrustThickness, currentThickness);
               }
            }
         }
      }
      
      thicknessMap->restoreData(false, true);
   }

   double initialCrustalThickness = m_operations.maximum(localInitialCrustThickness);

   if (initialCrustalThickness < 0)
   {
      if (flag)
      {
         LogHandler(LogHandler::WARNING_SEVERITY) << "Initial crustal thickness is negative";
      }
      else
      {
         LogHandler(LogHandler::WARNING_SEVERITY) << "Initial crustal thickness is not defined at the age of basin";
      }

      initialCrustalThickness = m_operations.maximum(localMaximumCrustThickness);

      if (initialCrustalThickness < 0)
      {
         throw std::runtime_error("Basin_Warning: Could not determine the initial crustal thickness");
      }
      else
      {
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP) << "setting InitialCrustalThickness to " <<
            initialCrustalThickness;
      }
   }
   else
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP) << "InitialCrustalThickness = " <<
         initialCrustalThickness;
   }
   m_initialCrustalThickness = initialCrustalThickness;

   // clean memory
   delete crustThicknesses;
}

void ContinentalCrustHistoryGenerator::createMantleHeatFlowCrust()
{
   m_crustThicknessHistory.reallocate(&m_activityGrid);
   const GridMap* thicknessMap = m_crust.getInputThicknessMap();

   const bool retrieved = thicknessMap->retrieved();
   if (not retrieved)
   {
      thicknessMap->retrieveData(true);
   }

   for (unsigned int i = thicknessMap->firstI(); i <= thicknessMap->lastI(); ++i)
   {
      for (unsigned int j = thicknessMap->firstJ(); j <= thicknessMap->lastJ(); ++j)
      {
         if (m_validator.isValid(i, j))
         {
            m_crustThicknessHistory(i, j).AddPoint(0.0, thicknessMap->getValue(i, j));
         }
      }
   }

   if (not retrieved)
   {
      thicknessMap->restoreData(false, true);
   }
}
