//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/TargetInputInfoFromWells.h"
#include "model/well.h"
#include <assert.h>

namespace casaWizard{
namespace ua {

TargetInputInfoFromWells::TargetInputInfoFromWells(const CalibrationTargetManager& calibrationTargetManager,
                                                   QVector<bool> wellSelectionStates,
                                                   QVector<bool> surfaceSelectionStates,
                                                   bool temperatureTargetsSelected,
                                                   bool vreTargetsSelected,
                                                   QString depthInput):
   TargetInputInfo(surfaceSelectionStates,
                   temperatureTargetsSelected,
                   vreTargetsSelected,
                   depthInput),
   m_wellSelectionStates(wellSelectionStates),
   m_calibrationTargetManager(calibrationTargetManager)
{}


QVector<TargetInputInfo::XYName> TargetInputInfoFromWells::getTargetLocations() const{
   const QVector<const Well*> wells = m_calibrationTargetManager.wells();
   assert(wells.size()==m_wellSelectionStates.size());

   QVector<TargetInputInfo::XYName> targetLocations;

   for(int i = 0; i < wells.size(); i++)
   {
      if ( m_wellSelectionStates[i])
      {
         const Well* well = wells[i];
         targetLocations.push_back({well->x(),well->y(),well->name()});
      }
   }

   return targetLocations;
}

QVector<bool> TargetInputInfoFromWells::getWellSelectionStates(){
   return m_wellSelectionStates;
}

void TargetInputInfoFromWells::setWellSelectionStates(QVector<bool> states){
   m_wellSelectionStates = states;
}

} //namespace ua
} //namespace casaWizard
