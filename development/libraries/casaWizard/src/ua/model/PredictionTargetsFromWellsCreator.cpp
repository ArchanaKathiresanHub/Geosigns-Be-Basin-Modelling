//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PredictionTargetsFromWellsCreator.h"
#include "predictionTargetManager.h"

#include "model/calibrationTargetManager.h"
#include "model/logger.h"
#include "model/TargetInputFromWellsInfo.h"

#include <assert.h>

namespace casaWizard
{

namespace ua
{

PredictionTargetsFromWellsCreator::PredictionTargetsFromWellsCreator(const TargetInputFromWellsInfo& inputInfo,
                                                                     const CalibrationTargetManager& calibrationTargetManager,
                                                                     PredictionTargetManager& predictionTargetmanager):
   m_inputInfo(inputInfo),
   m_calibrationTargetManager(calibrationTargetManager),
   m_predictionTargetmanager(predictionTargetmanager)
{}

void PredictionTargetsFromWellsCreator::createTargetsFromWellData(const TargetInputFromWellsInfo& inputInfo,
                                                                  const CalibrationTargetManager& calibrationTargetManager,
                                                                  PredictionTargetManager& predictionTargetmanager)
{
   PredictionTargetsFromWellsCreator creator(inputInfo, calibrationTargetManager, predictionTargetmanager);
   creator.createTargetsFromWellDataPrivate();
}

void PredictionTargetsFromWellsCreator::createTargetsFromWellDataPrivate()
{
   const QVector<XYLocation> targetLocations = getTargetLocations();
   const QVector<QString> properties = getProperties();
   createDepthTargets(targetLocations,properties);
   createSurfaceTargets(targetLocations,properties);
}

const QVector<PredictionTargetsFromWellsCreator::XYLocation> PredictionTargetsFromWellsCreator::getTargetLocations() const
{
   const QVector<const Well*> wells = m_calibrationTargetManager.wells();
   const QVector<bool>& wellSelectionStates = m_inputInfo.wellSelectionStates;

   assert(wells.size()==wellSelectionStates.size());

   QVector<XYLocation> targetLocations;

   for(int i = 0; i < wells.size(); i++)
   {
      if (wellSelectionStates[i])
      {
         const Well* well = wells[i];
         targetLocations.push_back({well->x(),well->y(),well->name()});
      }
   }

   return targetLocations;
}

const QVector<QString> PredictionTargetsFromWellsCreator::getProperties() const
{
   const QVector<QString>& predictionTargetOptions = m_predictionTargetmanager.predictionTargetOptions();

   QVector<QString> properties;
   if( predictionTargetOptions.contains("Temperature")
       && m_inputInfo.temperatureTargetsSelected)
   {
      properties.push_back("Temperature");
   }

   if (predictionTargetOptions.contains("VRe")
       && m_inputInfo.vreTargetsSelected)
   {
      properties.push_back("VRe");
   }

   return properties;
}

void PredictionTargetsFromWellsCreator::createDepthTargets(const QVector<PredictionTargetsFromWellsCreator::XYLocation>& targetLocations, const QVector<QString>& properties)
{
   const QVector<double> depths = getDepths();
   for (const XYLocation& loc : targetLocations)
   {
      for (double depth : depths)
      {
         m_predictionTargetmanager.addDepthTarget(loc.x,loc.y,depth,properties,0.0,loc.name);
      }
   }
}

const QVector<double> PredictionTargetsFromWellsCreator::getDepths() const
{
   const QString& depthInput = m_inputInfo.depthInput;
   if (depthInput.isEmpty()) return QVector<double>{};

   const QStringList zList = depthInput.split(QLatin1Char(','));

   QVector<double> depths;
   for (const QString& str: zList)
   {
      bool success;
      const double depthVal = str.toDouble(&success);
      if (success)
      {
         depths.push_back(depthVal);
      }
      else
      {
         Logger::log() << "Target input from well locations: Warning, depth input " << str << " cannot be converted to a scalar value." << Logger::endl();
      }
   }
   return depths;
}

void PredictionTargetsFromWellsCreator::createSurfaceTargets(const QVector<PredictionTargetsFromWellsCreator::XYLocation>& targetLocations, const QVector<QString>& properties)
{
   const QStringList surfaces = getSurfaces();
   for (const XYLocation& loc : targetLocations)
   {
      for (const QString& surface : surfaces)
      {
         m_predictionTargetmanager.addSurfaceTarget(loc.x,loc.y,surface,properties,0.0,loc.name);
      }
   }
}

const QStringList PredictionTargetsFromWellsCreator::getSurfaces() const
{
   const QVector<bool>& surfaceSelectionStates = m_inputInfo.surfaceSelectionStates;
   const QStringList surfaceNames = m_predictionTargetmanager.validSurfaceNames();

   assert(surfaceSelectionStates.size() == surfaceNames.size());
   QStringList selectedSurfaces;
   for(int i = 0; i < surfaceSelectionStates.size(); i++)
   {
      if (surfaceSelectionStates[i])
      {
         selectedSurfaces.append(surfaceNames[i]);
      }
   }
   return selectedSurfaces;
}

} // namespace ua

} // namespace casaWizard
