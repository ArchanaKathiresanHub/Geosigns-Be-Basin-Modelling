//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Manager for the Heat crush production in the SAC Thermal
#pragma once

#include "model/writable.h"
#include "OptimizedTCHP.h"

#include <QVector>

class QString;

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class TCHPManager : public Writable
{
public:
  TCHPManager();

  double maxTCHP() const;
  double minTCHP() const;

  void setMinTCHP(const double minHeatProduction);
  void setMaxTCHP(const double maxHeatProduction);

  void addOptimizedTCHP(const OptimizedTCHP& optimal);
  const QVector<OptimizedTCHP>& optimizedTCHPs() const;
  OptimizedTCHP optimizedInWell(const int wellIndex) const;

  void clearOptimizedTCHPs();

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;

private:
  TCHPManager(const TCHPManager&) = delete;
  TCHPManager& operator=(TCHPManager) = delete;

  double m_minHeatProduction;
  double m_maxHeatProduction;
  QVector<OptimizedTCHP> m_optimizedTCHPs;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
