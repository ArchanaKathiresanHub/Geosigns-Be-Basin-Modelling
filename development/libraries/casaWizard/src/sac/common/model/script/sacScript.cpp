//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "sacScript.h"

#include "model/SacScenario.h"
#include "model/logger.h"
#include "model/script/WizardDataToCasaScriptMapper.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

SACScript::SACScript(const QString& baseDirectory, const bool doOptimization) :
   CasaScript{baseDirectory},
   doOptimization_{doOptimization}
{
}

bool SACScript::doOptimization() const
{
   return doOptimization_;
}

QString SACScript::scriptFilename() const
{
   return QString("sacScript.casa");
}

QString SACScript::workingDirectory() const
{
   return QString(scenario().workingDirectory());
}

bool SACScript::validateScenario() const
{
   return true;
}

bool SACScript::createStopExecFile() const
{
   bool success = CasaScript::createStopExecFile();
   if (success)
   {
      QDir dir(scenario().calibrationDirectory() + QDir::separator() + scenario().runLocation() + QDir::separator() + scenario().iterationDirName());

      for ( const QString& caseDir : dir.entryList(QDir::Filter::Dirs))
      {
         if (caseDir.left(4) != QString("Case"))
         {
            continue;
         }

         QFile stopExecFile{dir.path() + QDir::separator() + caseDir + QDir::separator() + stopExecFilename_};
         if (!stopExecFile.open(QFile::OpenModeFlag::WriteOnly))
         {
            success = false;
            Logger::log() << "Failed to create " << stopExecFilename_ << " in folder " << caseDir
                          << "\n RunManager will not clean up running jobs" << Logger::endl();
         }
         stopExecFile.close();
      }
   }
   return success;
}

QString SACScript::writeWellTrajectory(const QString& wellName, const int wellIndex, const QString& propertyUserName) const
{
   const QString& propertyCauldronName = scenario().calibrationTargetManager().getCauldronPropertyName(propertyUserName);
   const QString folder{scenario().workingDirectory() + "/wells"};
   const QString filename{folder + "/" + wellName + "_" + propertyUserName + "_" + propertyCauldronName + ".in"};
   return QString("target \"" + QString::number(wellIndex) + "_" + propertyCauldronName + "\" WellTraj \"" + filename + "\" \""
                  + wizardDataToCasaScriptMapper::mapName(propertyCauldronName) + "\" 0 0.0 1.0 1.0\n");
}


void SACScript::writeScriptContents(QFile &file) const
{
   QTextStream out(&file);

   out << writeApp(1, scenario().applicationName() + " \"-allproperties\" \"-onlyat 0\"");

   // Project filename should not contain path information
   out << writeBaseProject(scenario().project3dFilename());

   writeParameters(out);

   const CalibrationTargetManager& ctManager = scenario().calibrationTargetManager();
   const QVector<const Well*>& wells = ctManager.wells();
   for (const Well* well : wells)
   {
      if ( well->isIncludedInOptimization() )
      {
         for (const QString& propertyUserName : ctManager.getPropertyUserNamesForWell(well->id()))
         {
            if (scenario().propertyIsActive(propertyUserName))
            {
               out << writeWellTrajectory(well->name(), well->id(), propertyUserName);
            }
         }
      }
   }

   out << QString("generateMulti1D \"Default\" \"none\" 0.01\n");
   out << writeLocation(scenario().runLocation(), false, !doOptimization(), true);
   out << writeRun(scenario().clusterName());
   out << writeSaveState(scenario().stateFileNameSAC());
}

} // sac

} // namespace casaWizard
