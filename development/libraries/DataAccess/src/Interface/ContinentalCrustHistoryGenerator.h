//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_CONTINENTAL_CRUST_THICKNESS_GENERATOR_H
#define INTERFACE_CONTINENTAL_CRUST_THICKNESS_GENERATOR_H

//DataAccess
#include "Interface/Interface.h"
#include "Interface/Local2DArray.h"

//CBMGenerics
#include "Polyfunction.h"

namespace DataModel {
   class AbstractValidator;
}

namespace DataAccess
{
   namespace Interface
   {

      class ApplicationGlobalOperations;

      /// @class ContinentalCrustHistoryGenerator Contains method to create the continental crustal thickness history polyfunction depending
      ///    on the bottom boundary condition mode
      class ContinentalCrustHistoryGenerator {

      public:

         /// @brief Define typedef as public as SWIG needs it for DataAccessAPI
         typedef Local2DArray <CBMGenerics::Polyfunction> PolyFunction2DArray;

         ContinentalCrustHistoryGenerator()                                                    = delete;
         ContinentalCrustHistoryGenerator(const ContinentalCrustHistoryGenerator &)            = delete;
         ContinentalCrustHistoryGenerator& operator=(const ContinentalCrustHistoryGenerator &) = delete;

         ContinentalCrustHistoryGenerator( const Interface::Grid& activityGrid,
                                           const Interface::CrustFormation& crust,
                                           const DataModel::AbstractValidator& validator,
                                           const ApplicationGlobalOperations& operations) :
             m_activityGrid(activityGrid),
             m_crust(crust),
             m_validator(validator),
             m_operations(operations),
             m_initialCrustalThickness (0.0) {}

         ~ContinentalCrustHistoryGenerator() = default;

         /// @brief Creates the continental crustal thickness history polyfunction depending
         ///    on the bottom boundary condition mode
         /// @details If the bottom boundary condition mode is ALC, also computes the initial crustal thickness
         /// @param[in] mode The bottom boundary condition mode (ALC, BCT, HF)
         void createCrustThickness(BottomBoundaryConditions mode);

         /// @defgroup Accessors
         /// @{
         double getInitialCrustalThickness()                   const {return m_initialCrustalThickness;}
         const PolyFunction2DArray& getcrustThicknessHistory() const {return m_crustThicknessHistory  ;}
         /// @}

      private:
         /// @defgroup CrustGenerators
         /// @{
         /// @brief Creates the continental crustal thickness history polyfunction for the Basic Crustal Thinning mode (BCT)
         void createFixedBasementTemperatureCrust();
         /// @brief Creates the continental crustal thickness history polyfunction for the Advanced Lithospheric Calculator mode (ALC),
         ///    also computes the initial crustal thickness
         void createAdvancedLithosphereCalculatorCrust();
         /// @brief Creates the continental crustal thickness history polyfunction for the Heat Flow mode (HF)
         void createMantleHeatFlowCrust();
         /// @}

         /// @defgroup Inputs
         /// @{
         const Grid& m_activityGrid;                      ///< The activity gird used by the project handle
         const CrustFormation& m_crust;                   ///< The crust formation
         const DataModel::AbstractValidator& m_validator; ///< Contains information about node validity
         const ApplicationGlobalOperations& m_operations; ///< Virtual class containing global math operations
         /// @}

         /// @defgroup Outputs
         /// @{
         double m_initialCrustalThickness;            ///< The continental crustal thickness at basin age [m]
         PolyFunction2DArray m_crustThicknessHistory; ///< The continental crustal thickness polyfunction [Ma,m]
         /// @}
      };

   }
}
#endif