//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_SANDSTONE_TO_MULTIPOINT_PERMEABILITY_H
#define PROGRADE_SANDSTONE_TO_MULTIPOINT_PERMEABILITY_H

//std
#include <vector>

namespace Prograde
{
   /// @class SandstoneToMultipointPermeability Converts a sandstone permeability model to a multipoint permeability model
   /// @brief The sandstone permeability model is described by \f$ K=K_0(10^{100(0.12+0.02c)(\phi_0-\phi}) \f$
   class SandstoneToMultipointPermeability{

      /// @brief Data container for constructor parameters
      struct Data {
         const double m_permAnisotropy;         ///< The permeability anisotropy
         const double m_depositionalPerm;       ///< The depositional permeability \f$ K_0 \f$       [mD]
         const double m_permSandClayPercentage; ///< The permeability sand over clay ratio \f$ c \f$ [%]
         const double m_depositionalPoro;       ///< The depositional porosity \f$ \phi_0 \f$        [%]
         const double m_minPoro;                ///< The minimum porosity                            [%]
      };

      public:

         SandstoneToMultipointPermeability()                                                     = delete;
         SandstoneToMultipointPermeability(const SandstoneToMultipointPermeability &)            = delete;
         SandstoneToMultipointPermeability& operator=(const SandstoneToMultipointPermeability &) = delete;

         explicit SandstoneToMultipointPermeability(const Data& data);

         ~SandstoneToMultipointPermeability() = default;

         /// @brief Computes the corresponding multipoint permeability model
         /// @details The sandstone permeability model is converted using the linear relationship between porosity and permeability
         void compute(std::vector<double>& modelPrms,
                      std::vector<double>& mpPor,
                      std::vector<double>& mpPerm) const;

      private:

         /// @defgroup SandstonePermeability The sandstone permeability  model parameters
         /// @{
         const double m_permAnisotropy;         ///< The permeability anisotropy
         const double m_depositionalPerm;       ///< The depositional permeability \f$ K_0 \f$       [mD]
         const double m_permSandClayPercentage; ///< The permeability sand over clay ratio \f$ c \f$ [%]
         /// @}

         /// @defgroup ExponentialPorosity The exponential porosity model parameters
         /// @{
         const double m_depositionalPoro; ///< The depositional porosity \f$ \phi_0 \f$ [%]
         const double m_minPoro;          ///< The minimum porosity                     [%]
         /// @}
   };
}

#endif