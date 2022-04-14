//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QString>

namespace casaWizard
{
namespace stringSanitizer
{

//Escapes the special characters, such that they are written out as text
QString escapeSpecialCharacters(QString str);

} // casaWizard
} // stringSanitizer

