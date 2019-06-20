//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
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
#include "Interface.h"
#include "Local2DArray.h"
#include "TableOceanicCrustThicknessHistory.h"

// CBMGenerics library
#include "Polyfunction.h"

using namespace DataAccess::Interface;

namespace GeoPhysics
{
   /// @class EffectiveCrustalThicknessCalculator Computes the Effective Crustal Thickness (ECT) and its associated properties (Basalt Thicknesses and End Of Rift)
   class EffectiveCrustalThicknessCalculator{
      public:
         typedef Local2DArray <CBMGenerics::Polyfunction> PolyFunction2DArray;

         /// @param[in] oceanicCrustThicknessHistory
         /// @param[in] continentalCrustThicknessPolyfunction Used to retrieve the present day crustal thickness
         /// @throw std::invalid_argument When one of the input is missing or invalid
         EffectiveCrustalThicknessCalculator( const PaleoFormationPropertyList*        continentalCrustThicknessHistory,
                                              const TableOceanicCrustThicknessHistory& oceanicCrustThicknessHistory,
                                              const double                             initialLithosphericMantleThickness,
                                              const double                             initialCrustThickness,
                                              const DataModel::AbstractValidator&      validator
         );

         /// @brief Computes the Effective Crustal Thickness History, Oceanic Crust Thickness History and End Of Rifting Event History
         /// @param[out] effectiveCrustThicknessHistory The Effective Crustal Thickness (ECT) history which represents the thickness of the crust formation
         /// @param[out] oceanicCrustThicknessHistory The Oceanic Crustal Thickness history (also called basalt crustal thicknes history) which represents the thickness of the basalt lithology
         ///    which will be lithoswitched in the mantle/crust formations
         /// @param[out] endOfRiftEvent The end age of the rifting event, used to determine when the boundary conditions are changing in the temperature solver
         /// @throw std::invalid_argument if the input thicknesses are negative
         /// @throw std::runtime_error if the oceanic crustal thickness history doesn't correspond (in ages) with the continental crustal thickness history
         void compute( PolyFunction2DArray&   effectiveCrustThicknessHistory,
                       PolyFunction2DArray&   oceanicCrustThicknessHistory,
                       Local2DArray <double>& endOfRiftEvent ) const;

      private:

         /// @brief Stores outputs when computing the basalt thickness from the crustal thickness at melt onset
         struct Output {
            double basaltThickness; ///< The oceanic crust thickness [m]
            bool   basaltStatus;    ///< False if the computed basalt thickness was negative
            bool   onsetStatus;     ///< False if the crustal thickness at melt onset equals the present day continental crust thickness
         };

         /// @brief Stores the (i,j) indexes of a map node
         struct Node
         {
            const unsigned int i;
            const unsigned int j;
            Node(const unsigned int i, const unsigned int j) : i(i), j(j) {};
         };

         /// @brief Calculates the effective crustal thickness
         /// @param[in] coeff The multiplicative coefficient which is the ration of the inital crust thickness by the total initial crust and lithospheric mantle thickness
      static double calculateEffectiveCrustalThickness( const double continentalCrustThickness,
                                                        const double basaltThickness,
                                                        const double coeff) noexcept;

         /// @brief Update the end of the rift (last crustal thinning age) of the given node to the given age
         ///    if and only if the current continetal crust is thinner than the previous continental crust
         /// @details This end of rift property is used as in the temperature solver boundary conditions
         /// @param[out] endOfRiftEvent The array of the end of rift for each (i,j) node
         void updateEndOfRift( const double continentalCrustThickness,
                               const double previousContinentalCrustThickness,
                               const double age,
                               const Node& node,
                               Local2DArray <double>& endOfRiftEvent) const noexcept;

         /// @brief Retrieve all input maps accoring to the algorithm version
         void retrieveData() const;
         /// @breif Retrieve all output maps accoring to the algorithm version
         void restoreData() const;

         /// @brief Check that the given value is positive and throw a detailed exception if not
         /// @throw std::invalid_argument if the given value is negative
      static void checkThicknessValue( const char*        thicknessMapName,
                                       const unsigned int i,
                                       const unsigned int j,
                                       const double       age,
                                       const double       value);

         const PaleoFormationPropertyList*       m_continentalCrustThicknessHistory;
         const TableOceanicCrustThicknessHistory m_oceanicCrustThicknessHistory;
         const double                            m_initialLithosphericMantleThickness;
         const double                            m_initialCrustThickness;
         const DataModel::AbstractValidator&     m_validator;                          ///< The validator to check if a node (i,j) is valid or not

         static const double s_minimumEffectiveCrustalThickness; ///< The minimum allowed Effective Crustal Thickness [m]
         static const bool   s_gosthNodes;                       ///< Defines if the input maps should output the gosth nodes
   };
}

#endif