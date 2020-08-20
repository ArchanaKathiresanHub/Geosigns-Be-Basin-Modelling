//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

//Prograde
#include "IUpgradeManager.h"

//std
#include <memory>

namespace mbapi
{
  class Model;
}

namespace DataAccess
{
  namespace Interface
  {
    class ProjectHandle;
  }
}

namespace Prograde
{
/// @class UserDefinedSnapshotUpgradeManager is used to upgrade the newly introduced UserDefinedSnapshotIoTbl with user defined entries stored in the SnapshotIoTbl.
class UserDefinedSnapshotUpgradeManager : public IUpgradeManager
{
public:
  UserDefinedSnapshotUpgradeManager() = delete;
  UserDefinedSnapshotUpgradeManager(const UserDefinedSnapshotUpgradeManager &) = delete;
  UserDefinedSnapshotUpgradeManager& operator=(const UserDefinedSnapshotUpgradeManager &) = delete;

  /// @param[in] model The model on which to perform the user defined snapshot table upgrade
  /// throw std::invalid_argument if the model's project handle is null
  explicit UserDefinedSnapshotUpgradeManager(mbapi::Model& model);

  ~UserDefinedSnapshotUpgradeManager() final = default;

  /// @brief Upgrades the model to the BPA-2 standards
  /// @details Upgrades only if necessary
  void upgrade() final;

private:
  mbapi::Model& m_model;                                      ///< The model to upgrade
  std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
};
}
