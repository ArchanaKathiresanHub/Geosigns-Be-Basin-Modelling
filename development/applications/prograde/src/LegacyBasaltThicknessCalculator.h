//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_LEGACY_BASALTTHICKNESS_CALCULATOR_H
#define PROGRADE_LEGACY_BASALTTHICKNESS_CALCULATOR_H

//std library
#include <map>
#include <memory>

// DataAccess library
#include "Interface.h"
#include "Local2DArray.h"

// CBMGenerics library
#include "Polyfunction.h"

//DataModel
#include "AbstractSnapshot.h"

namespace DataAccess {
   namespace Interface {
      class GridMap;
      class ObjectFactory;
      class Grid;
      class Snapshot;
   }
}

namespace DataModel {
   class AbstractSnapshot;
   class AbstractValidator;
}

namespace Prograde
{
   /// @class LegacyBasaltThicknessCalculator Computes the basalt thickness using the legacy alc algorithms (v2016.11 release)
   /// @details Computes the Basalt Thicknesses from the crust thickness at melt onset and the present day basalt thickness
   class LegacyBasaltThicknessCalculator{

      typedef DataAccess::Interface::Local2DArray <CBMGenerics::Polyfunction> PolyFunction2DArray;

      typedef std::map<const std::shared_ptr<const DataModel::AbstractSnapshot>,
                       const std::shared_ptr<const DataAccess::Interface::GridMap>,
                       DataModel::AbstractSnapshot::ComparePointers<const std::shared_ptr<const DataModel::AbstractSnapshot>>> SmartAbstractSnapshotVsSmartGridMap;

      typedef std::map<const DataModel::AbstractSnapshot* const,
                       const DataAccess::Interface::GridMap* const,
                       DataModel::AbstractSnapshot::ComparePointers<const DataModel::AbstractSnapshot* const>> AbstractSnapshotVsGridMap;

      public:

         LegacyBasaltThicknessCalculator()                                                   = delete;
         LegacyBasaltThicknessCalculator(const LegacyBasaltThicknessCalculator &)            = delete;
         LegacyBasaltThicknessCalculator& operator=(const LegacyBasaltThicknessCalculator &) = delete;

         /// @param[in] continentalCrustThicknessHistory The continental crust thickness history        [m]
         /// @param[in] continentalCrustThicknessPolyfunction The continental crust thickness polyfunction used to retrieve the present day crustal thickness (via interpolation if needed) [Ma,m]
         /// @param[in] presentDayBasaltThickness The present day oceanic crust (basalt) thickness      [m]
         /// @param[in] crustMeltOnsetMap The thickness of the continental crust at melting point       [m]
         /// @param[in] initialLithosphericMantleThickness The initial lithospheric mantle thickness    [m]
         /// @param[in] initialCrustThickness The initial continental crust thickness                   [m]
         /// @param[in] validator The node validator to check if a node (i,j) is valid or not (from project handle for application or mock for unit tests)
         /// @param[in] referenceGrid The reference grid on which computed basalt thickness maps are based
         /// @param[in] factory The object factory, used to produce grid maps
         /// @throw std::invalid_argument When one of the input is missing or invalid
         explicit LegacyBasaltThicknessCalculator( const AbstractSnapshotVsGridMap&            continentalCrustThicknessHistory,
                                                   const PolyFunction2DArray&                  continentalCrustThicknessPolyfunction,
                                                   const DataAccess::Interface::GridMap*       presentDayBasaltThickness,
                                                   const DataAccess::Interface::GridMap*       crustMeltOnsetMap,
                                                   const double                                initialCrustThickness,
                                                   const DataModel::AbstractValidator&         validator,
                                                   const DataAccess::Interface::Grid*          referenceGrid,
                                                   const DataAccess::Interface::ObjectFactory* factory );

         ~LegacyBasaltThicknessCalculator() = default;

         /// @brief Computes the Oceanic Crust Thickness History (using the legacy alc algorithms) for each continental crust thickness entry
         /// @param[out] oceanicCrustThicknessHistory The Oceanic Crustal Thickness history (also called basalt crustal thickness history) which represents the thickness of the basalt lithology
         ///    which will be lithoswitched in the mantle/crust formations
         /// @throw std::invalid_argument if the input thicknesses are negative
         void compute( SmartAbstractSnapshotVsSmartGridMap& oceanicCrustThicknessHistory ) const;

      private:

         /// @brief Stores outputs when computing the basalt thickness from the crustal thickness at melt onset
         struct Output {
            double basaltThickness; ///< The oceanic crust thickness [m]
            bool   basaltStatus;    ///< False if the computed basalt thickness was negative
            bool   onsetStatus;     ///< False if the crustal thickness at melt onset equals the present day continental crust thickness
         };

         /// @brief Calculates the basalt thickness from the crust thickness at melt onset
         Output calculateBasaltThicknessFromMeltOnset(const double crustThicknessAtMeltOnset,
                                                      const double continentalCrustThickness,
                                                      const double presentDayContinentalCrustThickness,
                                                      const double previousContinentalCrustThickness,
                                                      const double presentDayBasaltThickness,
                                                      const double previousBasaltThickness) const noexcept;

         /// @brief Retrieve all input maps
         void retrieveData() const;
         /// @brief Restore all input maps
         void restoreData() const;

         /// @brief Check that the given value is positive and throw a detailed exception if not
         /// @throw std::invalid_argument if the given value is negative
         static void checkThicknessValue( const char*        thicknessMapName,
                                          const unsigned int i,
                                          const unsigned int j,
                                          const double       age,
                                          const double       value );

         /// The continental crust thickness polyfunction used to retrieve the present day crustal thickness (via interpolation if needed) [Ma,m]
         const PolyFunction2DArray&                  m_contCrustThicknessPolyfunction;
         const AbstractSnapshotVsGridMap&            m_continentalCrustThicknessHistory;   ///< The continental crust thickness history                 [Ma,m]
         const DataAccess::Interface::GridMap*       m_presentDayBasaltThickness;          ///< The present day oceanic crust (basalt) thickness        [m]
         const DataAccess::Interface::GridMap*       m_crustThicknessMeltOnset;            ///< The thickness of the continental crust at melting point [m]
         const double                                m_initialCrustThickness;              ///< The initial continental crust thickness                 [m]
         const DataModel::AbstractValidator&         m_validator;                          ///< The validator to check if a node (i,j) is valid or not
         const DataAccess::Interface::Grid*          m_referenceGrid;                      ///< The refence grid on which computed basalt thickness maps are based
         const DataAccess::Interface::ObjectFactory* m_factory;                            ///< The object factory, used to produce grid maps

         static const bool s_gosthNodes;  ///< Defines if the input maps should output the gosth nodes (initialyzed as true)

   };
}

#endif