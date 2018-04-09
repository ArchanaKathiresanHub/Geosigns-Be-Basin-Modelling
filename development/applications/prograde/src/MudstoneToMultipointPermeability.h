//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_MUDSTONE_TO_MULTIPOINT_PERMEABILITY_H
#define PROGRADE_MUDSTONE_TO_MULTIPOINT_PERMEABILITY_H

//std
#include <vector>

namespace Prograde
{
   /// @class MudstoneToMultipointPermeability Converts a mudstone permeability model to a multipoint permeability model
   /// @brief The mudstone permeability model is described by \f$ K=K_0((\sigma+\sigma_0)/\sigma_0)^{-s} \f$
   class MudstoneToMultipointPermeability{

      /// @brief Data container for constructor parameters
      struct Data {
         const double m_permAnisotropy;          ///< The permeability anisotropy
         const double m_depositionalPerm;        ///< The depositional permeability [mD]
         const double m_mudPermSensitivityCoeff; ///< The mudstone permeability sensitivity coefficient
         const double m_depositionalPoro;        ///< The depositional porosity [%]
         const double m_compactionCoeff;         ///< The compaction coefficient
         const double m_minPoro;                 ///< The minimum porosity [%]
         const bool m_chemicalCompaction;        ///< Flag for applying chemical compaction
      };

      public:

         MudstoneToMultipointPermeability()                                                    = delete;
         MudstoneToMultipointPermeability(const MudstoneToMultipointPermeability &)            = delete;
         MudstoneToMultipointPermeability& operator=(const MudstoneToMultipointPermeability &) = delete;

         explicit MudstoneToMultipointPermeability( const Data& data );

         ~MudstoneToMultipointPermeability() = default;

         /// @brief Computes the corresponding multipoint permeability model
         /// @details The mudstone permeability model is converted by sampling both the porosity and the permeability along a define set of VES values
         void compute(std::vector<double>& modelPrms,
                      std::vector<double>& mpPor,
                      std::vector<double>& mpPerm) const;

      private:

         /// @defgroup MudstonePermeability
         /// @{
         const double m_permAnisotropy;          ///< The permeability anisotropy
         const double m_depositionalPerm;        ///< The depositional permeability [mD]
         const double m_mudPermSensitivityCoeff; ///< The mudstone permeability sensitivity coefficient
         /// @}

         /// @defgroup ExponentialPorosity
         /// @{
         const double m_compactionCoeff;  ///< The compaction coefficient [1/Mpa]
         const double m_depositionalPoro; ///< The depositional porosity [%]
         const double m_minPoro;          ///< The minimum porosity [%]
         const bool m_chemicalCompaction; ///< Flag for applying chemical compaction
         /// @}

         /// @brief The list of VES value [MPa] for which we compute a perm.-poro. point in the mulitpoint permeability model
         static const std::vector<double> s_VesSampling;
         /// @brief The initial vertical effective stress [MPa]
         static constexpr double s_initialVes = 1;
   };
}

#endif