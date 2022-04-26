//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QVector>

namespace casaWizard
{

class CalibrationTargetManager;

namespace ua
{

struct TargetInputFromWellsInfo;
class PredictionTargetManager;

class PredictionTargetsFromWellsCreator
{
public:
   static void createTargetsFromWellData(const TargetInputFromWellsInfo& inputInfo,
                                    const CalibrationTargetManager& calibretionTargetManager,
                                    PredictionTargetManager& predictionTargetmanager);

private:

   PredictionTargetsFromWellsCreator(const TargetInputFromWellsInfo& inputInfo,
                                     const CalibrationTargetManager& calibrationTargetManager,
                                     PredictionTargetManager& predictionTargetmanager);

   struct XYLocation
   {
      XYLocation():
         x(0.0),
         y(0.0)
      {}

      XYLocation(double x, double y,const QString name = ""):
         x(x),
         y(y),
         name(name)
      {}

      double x;
      double y;
      QString name;
   };

   void createTargetsFromWellDataPrivate();
   void createDepthTargets(const QVector<XYLocation>& targetLocations, const QVector<QString>& properties);
   void createSurfaceTargets(const QVector<XYLocation>& targetLocations, const QVector<QString>& properties);

   const QVector<XYLocation> getTargetLocations() const;
   const QVector<double> getDepths() const;
   const QStringList getSurfaces() const;
   const QVector<QString> getProperties() const;

   const TargetInputFromWellsInfo& m_inputInfo;
   const CalibrationTargetManager& m_calibrationTargetManager;
   PredictionTargetManager& m_predictionTargetmanager;
};

} // namespace ua

} // namespace casaWizard

