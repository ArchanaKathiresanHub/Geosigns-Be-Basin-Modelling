//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "McmcTargetExportData.h"
#include <QVector>

namespace casaWizard
{

namespace ua
{

class UAScenario;

class McmcOutputDataCollector
{
public:
   static McmcTargetExportData collectMcmcOutputData(const UAScenario& scenario);

private:
   explicit McmcOutputDataCollector(const UAScenario& scenario);

   void collectMcmcOutputDataPrivate();
   void collectPredictionTargetData();
   void collectCalibrationTargetData();

   void readBaseCase();
   void readValuesFromTargetQcs();

   QVector<double> getPredictionData(const QVector<double>& values) const;

   const UAScenario& m_scenario;

   QVector<double> m_optimalCaseSimValues;
   QVector<double> m_baseCaseSimValues;
   QVector<double> m_baseCaseProxyValues;
   McmcTargetExportData m_mcmcTargetExportData;
};

} // namespace ua

} // namespace casaWizard

