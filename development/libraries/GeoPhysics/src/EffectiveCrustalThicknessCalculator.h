//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef GEOPHYSICS_EFFECTIVECRUSTALTHICKNESSCALCULATOR_H
#define GEOPHYSICS_EFFECTIVECRUSTALTHICKNESSCALCULATOR_H

// DataModel library
#include "AbstractValidator.h"

// DataAccess library
#include "Interface/Interface.h"
#include "Interface/GridMap.h"
#include "Interface/PaleoFormationProperty.h"
#include "Interface/TableOceanicCrustThicknessHistory.h"
using namespace DataAccess::Interface;

// GeoPhysics library
#include "Local2DArray.h"

// CBMGenerics library
#include "Polyfunction.h"

namespace GeoPhysics
{
   /// @class EffectiveCrustalThicknessCalculator Computes the Effective Crustal Thickness (ECT) and its associated properties (Basalt Thicknesses and End Of Rift)
   /// @details The class owns two version of the same algorithm:
   ///    * Legacy version computes the Basalt Thicknesses from the crust thickness at melt onset, and then ses them to compute the ECT.
   ///    * v2017.05 version gets the Basalt Thicknesses as input and computes directly the ECT
   class EffectiveCrustalThicknessCalculator{
      public:
         typedef Local2DArray <CBMGenerics::Polyfunction> PolyFunction2DArray;

         /// @brief The verion of the algorithm used
         /// @details Defined according to inputs
         enum VersionALC {
            LEGACY,  ///< Computes the Basalt Thicknesses from the crust thickness at melt onset, and then ses them to compute the ECT
            V2017_05 ///< Gets the Basalt Thicknesses as input and computes directly the ECT
         };

         /// @details If the present day basaltThickness and crust thickness at melt onset are not null pointers, then the version is set to legacy
         ///    Else, the version is set to v2017.05 and the oceanic crust thickness history must have the same size as the continental crust thickness history and not empty
         /// @param[in] oceanicCrustThicknessHistory Used only in v2017.05 version
         /// @param[in] presentDayBasaltThickness Used only in legacy version
         /// @param[in] crustThicknessMeltOnset Used only in legacy version
         /// @throw std::invalid_argument When one of the input is missing or invalid
         EffectiveCrustalThicknessCalculator( const PaleoFormationPropertyList*        continentalCrustThicknessHistory,
                                              const TableOceanicCrustThicknessHistory& oceanicCrustThicknessHistory,
                                              const GridMap*                           presentDayBasaltThickness,
                                              const GridMap*                           crustThicknessMeltOnset,
                                              const double                             initialLithosphericMantleThickness,
                                              const double                             initialCrustThickness,
                                              const AbstractValidator&                 validator
         );

         /// @brief Computes the Effective Crustal Thickness History, Oceanic Crust Thickness History and End Of Rifting Event History
         /// @details If the version used is v2017.05 then the Oceanic Crust Thickness History output is a copy from the Oceanic Crust Thickness History input
         /// @param[out] effectiveCrustThicknessHistory The Effective Crustal Thickness (ECT) history which represents the thickness of the crust formation
         /// @param[out] oceanicCrustThicknessHistory The Oceanic Crustal Thickness history (also called basalt crustal thicknes history) which represents the thickness of the basalt lithology
         ///    which will be lithoswitched in the mantle/crust formations
         /// @param[out] endOfRiftEvent The end age of the rifting event, used to determine when the boundary conditions are changing in the temperature solver
         /// @throw std::invalid_argument if the input thicknesses are negative
         /// @throw std::runtime_error if the algorithm version is unknown
         /// @throw std::runtime_error if the algorithm version is v2017.05 and the oceanic crustal thickness history doesn't correspond (in ages) with the continental crustal thickness history
         void compute( PolyFunction2DArray&   effectiveCrustThicknessHistory,
                       PolyFunction2DArray&   oceanicCrustThicknessHistory,
                       Local2DArray <double>& endOfRiftEvent );

      private:

         /// @brief Stores outputs when computing the basalt thickness from the crustal thickness at melt onset
         struct Output {
            double basaltThickness; ///< The oceanic crust thickness [m]
            bool   basaltStatus;    ///< False if the computed basalt thickness was negative
            bool   onsetStatus;     ///< False if the crustal thickness at melt onset equals the present day contiental crust thickness
         };

         /// @brief Calculates the basalt thickness from the crust thickness at melt onset
         /// @details To be used in legacy version only
         Output calculateBasaltThicknessFromMeltOnset( const double crustThicknessAtMeltOnset,
                                                       const double continentalCrustThickness,
                                                       const double presentDayContinentalCrustThickness,
                                                       const double previousContinentalCrustThickness,
                                                       const double presentDayBasaltThickness,
                                                       const double previousBasaltThickness,
                                                       const double age) const noexcept;

         /// @brief Calculates the effective crustal thickness
         /// @details To be used in both legacy and v2017.05 version
         /// @param[in] coeff The multiplicative coefficient which is the ration of the inital crust thickness by the total initial crust and lithospheric mantle thickness
         double calculateEffectiveCrustalThickness( const double continentalCrustThickness,
                                                    const double basaltThickness,
                                                    const double coeff) const noexcept;

         /// @brief Calculates the end of the rift (last cust thinning age)
         /// @details This property is used as a double check in the temperature solver
         double calculateEndOfRift( const double continentalCrustThickness,
                                    const double previousContinentalCrustThickness,
                                    const double age ) const noexcept;

         /// @brief Retrieve all input maps accoring to the algorithm version
         /// @throw std::runtime_error if the algorithm version is unknown
         void retrieveData();
         /// @breif Retrieve all output maps accoring to the algorithm version
         /// @throw std::runtime_error if the algorithm version is unknown
         void restoreData();

         /// @brief Check that the given value is positive and throw a detailed exception if not
         /// @throw std::invalid_argument if the given value is negative
         void checkThicknessValue( const char*        thicknessMapName,
                                   const unsigned int i,
                                   const unsigned int j,
                                   const double       age,
                                   const double       value) const;

         const PaleoFormationPropertyList*       m_continentalCrustThicknessHistory;
         const TableOceanicCrustThicknessHistory m_oceanicCrustThicknessHistory;       ///< Only used in v2017.05 version
         const GridMap*                          m_presentDayContCrustThickness;       ///< The present day continental crust thickness
         const GridMap*                          m_presentDayBasaltThickness;          ///< Only used in legacy version
         const GridMap*                          m_crustThicknessMeltOnset;            ///< Only used in legacy version
         const double                            m_initialLithosphericMantleThickness;
         const double                            m_initialCrustThickness;
         const AbstractValidator&                m_validator;                          ///< The validator to check if a node (i,j) is valid or not

         static const double s_minimumEffectiveCrustalThickness; ///< The minimum allowed Effective Crustal Thickness [m]
         static const bool   s_gosthNodes;                       ///< Defines if the input maps should output the gosth nodes

         VersionALC m_version; /// The algorithm version, which corresponds to the ALC version
   };
}

#endif