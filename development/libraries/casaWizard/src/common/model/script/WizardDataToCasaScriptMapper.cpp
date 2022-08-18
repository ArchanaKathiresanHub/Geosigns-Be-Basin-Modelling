//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "WizardDataToCasaScriptMapper.h"

namespace casaWizard
{
namespace wizardDataToCasaScriptMapper
{

QString mapName(const QString& wizardName)
{
   if (wizardName == "VRe")
   {
     return QString("Vr");
   }
   return wizardName;
}

}; //casaWizard
}; //wizardDataToCasaScriptMapper

