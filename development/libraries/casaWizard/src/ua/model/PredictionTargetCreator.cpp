//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/PredictionTargetCreator.h"

#include <assert.h>
#include <model/logger.h>

namespace casaWizard
{
namespace ua
{

PredictionTargetCreator::PredictionTargetCreator(const TargetInputInfo& inputInfo,
                                                 PredictionTargetManager& predictionTargetmanager):
   m_inputInfo(inputInfo),
   m_predictionTargetManager(predictionTargetmanager)
{}

void PredictionTargetCreator::createTargets()
{
   const QVector<TargetInputInfo::XYName> targetLocations = m_inputInfo.getTargetLocations();
   const QVector<QString> properties = getProperties();
   createDepthTargets(targetLocations, properties);
   createSurfaceTargets(targetLocations, properties);
}

void PredictionTargetCreator::createDepthTargets(const QVector<TargetInputInfo::XYName> targetLocations, const QVector<QString> properties)
{
   const QVector<double> depths = getDepths();
   for (const TargetInputInfo::XYName& loc : targetLocations)
   {
      for (double depth : depths)
      {
         m_predictionTargetManager.addDepthTarget(loc.x,loc.y,depth,properties,0.0,loc.name);
      }
   }
}

void PredictionTargetCreator::createSurfaceTargets(const QVector<TargetInputInfo::XYName> targetLocations, const QVector<QString> properties)
{
   const QStringList surfaces = getSurfaces();
   for (const TargetInputInfo::XYName& loc : targetLocations)
   {
      for (const QString& surface : surfaces)
      {
         m_predictionTargetManager.addSurfaceTarget(loc.x,loc.y,surface,properties,0.0,loc.name);
      }
   }
}

QVector<QString> PredictionTargetCreator::getProperties() const
{
   const QVector<QString>& predictionTargetOptions = m_predictionTargetManager.predictionTargetOptions();

   QVector<QString> properties;
   if( predictionTargetOptions.contains("Temperature")
       && m_inputInfo.getTemperatureTargetsSelected())
   {
      properties.push_back("Temperature");
   }

   if (predictionTargetOptions.contains("VRe")
       && m_inputInfo.getVreTargetsSelected())
   {
      properties.push_back("VRe");
   }

   return properties;
}

const QStringList PredictionTargetCreator::getSurfaces() const
{
   const QVector<bool>& surfaceSelectionStates = m_inputInfo.getSurfaceSelectionStates();
   const QStringList surfaceNames = m_predictionTargetManager.validSurfaceNames();

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

const QVector<double> PredictionTargetCreator::getDepths() const
{
   const QString& depthInput = m_inputInfo.getDepthInput();
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

} //namespace ua

} //namespace casaWizard

