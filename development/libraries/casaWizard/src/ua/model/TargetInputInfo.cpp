//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


#include "model/TargetInputInfo.h"

namespace casaWizard{
namespace ua {
TargetInputInfo::TargetInputInfo(QVector<bool> surfaceSelectionStates,
                                 bool temperatureTargetsSelected,
                                 bool vreTargetsSelected,
                                 QString depthInput):
   m_surfaceSelectionStates(surfaceSelectionStates),
   m_temperatureTargetsSelected(temperatureTargetsSelected),
   m_vreTargetsSelected(vreTargetsSelected),
   m_depthInput(depthInput)
{}


QVector<bool> TargetInputInfo::getSurfaceSelectionStates() const{
   return m_surfaceSelectionStates;
}

bool TargetInputInfo::getTemperatureTargetsSelected() const{
   return m_temperatureTargetsSelected;
}

bool TargetInputInfo::getVreTargetsSelected() const{
   return m_vreTargetsSelected;
}
QString TargetInputInfo::getDepthInput() const{
   return m_depthInput;
}

void TargetInputInfo::setSurfaceSelectionStates(QVector<bool> states){
   m_surfaceSelectionStates = states;
}

void TargetInputInfo::setTemperatureTargetsSelected(bool state){
   m_temperatureTargetsSelected = state;
}

void TargetInputInfo::setVreTargetsSelected(bool state){
   m_vreTargetsSelected = state;
}
void TargetInputInfo::setDepthInput(QString depthInput){
   m_depthInput = depthInput;
}

} //namespace ua
} //namespace casaWizard


