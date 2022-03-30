//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "WellDataManipulator.h"

#include "model/calibrationTarget.h"
#include "model/logger.h"
#include "model/well.h"

#include "CutOffTargetCreator.h"
#include "ScaledTargetCreator.h"
#include "SmoothedTargetCreator.h"
#include "SubsampledTargetCreator.h"

namespace casaWizard
{

void WellDataManipulator::applyCutOff(const QMap<QString, QPair<double, double>>& propertiesWithCutOffRanges, QVector<Well>& wells)
{
   CutOffTargetCreator cutOffManipulator(propertiesWithCutOffRanges);
   applyManipulatedWellData(propertiesWithCutOffRanges.keys(), cutOffManipulator, wells);
}

void WellDataManipulator::scaleData(const QStringList& selectedProperties, const double scalingFactor, QVector<Well>& wells)
{
   ScaledTargetCreator scaler(scalingFactor);
   applyManipulatedWellData(selectedProperties, scaler, wells);
}

void WellDataManipulator::smoothenData(const QStringList& selectedProperties, const double radius, QVector<Well>& wells)
{
   SmoothedTargetCreator smoother(radius);
   applyManipulatedWellData(selectedProperties, smoother, wells);
}

void WellDataManipulator::subsampleData(const QStringList& selectedProperties, const double length, QVector<Well>& wells)
{
   SubsampledTargetCreator subsampler(length);
   applyManipulatedWellData(selectedProperties, subsampler, wells);
}

void WellDataManipulator::applyManipulatedWellData(const QStringList& selectedProperties, ManipulatedTargetCreator& manipulator, QVector<Well>& wells)
{
   for (Well& well : wells)
   {
      if (!well.isActive()) continue;

      for ( const QString& property : selectedProperties )
      {
         // Get calibration targets for the selected property
         const QVector<const CalibrationTarget*> targets = well.calibrationTargetsWithPropertyUserName(property);
         const QVector<CalibrationTarget> newTargets = manipulator.createManipulatedTargets(targets, property);

         // Delete all old calibration targets
         well.removeCalibrationTargetsWithPropertyUserName(property);

         // Add the new targets
         for( const CalibrationTarget& newTarget : newTargets)
         {
            well.addCalibrationTarget(newTarget);
         }

         // Set metadata
         const QString message(manipulator.metaDataMessage(property));
         if (message != "")
         {
            well.appendMetaData(message);

            // Log info
            Logger::log() << well.name() << ": " << message << Logger::endl();
         }
      }
   }
}

} // namespace casaWizard
