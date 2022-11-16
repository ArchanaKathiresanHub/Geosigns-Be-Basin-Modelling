//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

# pragma once

#include <QString>

namespace casaWizard
{
namespace ua
{
namespace optimalCaseExporter
{
void exportOptimalCase(const QString& optimal, const QString& workingDirectory, QString optimalProjectName = "Project.project3d", QString timeStamp = "");
} //optimalCaseExporter
} //ua
} //casaWizard
