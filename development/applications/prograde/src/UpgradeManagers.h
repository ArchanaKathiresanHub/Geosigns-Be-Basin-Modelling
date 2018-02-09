//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_UPGRADE_MANAGERS_H
#define PROGRADE_UPGRADE_MANAGERS_H

namespace mbapi {
   class Model;
}

namespace Prograde {
   /// @class UpgradeManagers Utility class which stores all the upgrade managers and runs them on the given model
   class UpgradeManagers {
      public:

         UpgradeManagers()                                   = delete;
         UpgradeManagers(const UpgradeManagers &)            = delete;
         UpgradeManagers& operator=(const UpgradeManagers &) = delete;

         /// @param[in] model The model to upgrade
         explicit UpgradeManagers(mbapi::Model& model):
            m_model(model) {}

         /// @brief Runs all the upgrade managers on the model
         /// @throw std::invalid_argument if the model is invalid
         void runAll() const;

      private:
         mbapi::Model& m_model; ///< The model to upgrade
   };
}

#endif