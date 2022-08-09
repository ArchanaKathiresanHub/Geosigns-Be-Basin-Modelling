//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QVector>

#include "model/TargetInputInfo.h"
#include "model/predictionTargetManager.h"
#include "model/calibrationTargetManager.h"

namespace casaWizard
{

namespace ua
{
class PredictionTargetCreator
{
public:
   PredictionTargetCreator(const TargetInputInfo& inputInfo,
                           PredictionTargetManager& predictionTargetmanager);

   void createTargets();
private:
   void createDepthTargets(const QVector<TargetInputInfo::XYName> targetLocations, const QVector<QString> properties);
   void createSurfaceTargets(const QVector<TargetInputInfo::XYName> targetLocations, const QVector<QString> properties);
   QVector<QString> getProperties() const;


   const QStringList getSurfaces() const;
   const QVector<double> getDepths() const;

   const TargetInputInfo& m_inputInfo;
   PredictionTargetManager& m_predictionTargetManager;

};

}
}
