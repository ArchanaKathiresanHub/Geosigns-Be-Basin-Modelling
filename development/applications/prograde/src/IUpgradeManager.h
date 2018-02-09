//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_IUPGRADE_MANAGERS_H
#define PROGRADE_IUPGRADE_MANAGERS_H

#include <string>
#include <utility>

namespace Prograde{
   /// @class IUpgradeManager Abstract class (interface) for upgrade managers
   /// @brief Upgrade managers should inherit from this interface, and be added to the manager list in the UpgradeManagers class
   class IUpgradeManager {

      public:

        explicit IUpgradeManager(std::string name):
            m_name(std::move(name)) {}

         virtual ~IUpgradeManager() = default;

         /// @brief The upgrade function to be called by the UpgradeManagers class
         virtual void upgrade() = 0;

         /// @return The name of the upgrade manager
         /// @details Used in the application logging
         const std::string& getName() const noexcept { return m_name; }

      private:

         const std::string m_name; ///< The name of the upgrade manager
   };
}

#endif