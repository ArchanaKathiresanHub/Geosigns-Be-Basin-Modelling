//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "collectObservablesScript.h"

#include "model/uaScenario.h"

#include <QFile>
#include <QTextStream>

namespace casaWizard
{

namespace ua
{

CollectObservablesScript::CollectObservablesScript(UAScenario& scenario) :
    UAScript(scenario)
{}

QString CollectObservablesScript::scriptFilename() const
{
   return QString("collectObservablesScript.casa");
}

void CollectObservablesScript::writeScriptContents(QFile& file) const
{
   QString stateFileNameDoE = uaScenario().runLocation() + "/" + uaScenario().iterationDirName() + "/" + uaScenario().stateFileNameDoE();
   QTextStream out(&file);
   out << writeBaseProject(uaScenario().project3dFilename());
   out << writeLoadState(stateFileNameDoE);
   writeTargets(out);
   out << writeExportDataTxt("RunCasesObservables", uaScenario().runCasesObservablesTextFileName());
}

} // namespace ua

} // namespace casaWizard
