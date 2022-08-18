//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Set of helpers for reading and writing the scenario
#pragma once

#include <QString>

namespace casaWizard
{

namespace scenarioIO
{

const QString newline{"\n"};
const QString separator{"|"};
const QString listSeparator{"><"};
const QString subListSeparator{")("};
const QString subSubListSeparator{"]["};

QString doubleToQString(const double d);
QString vectorToWrite(const QVector<double>& vector);
QVector<double> vectorFromRead(const QString& string);

} // namespace scenarioIO

} // namespace casaWizard
