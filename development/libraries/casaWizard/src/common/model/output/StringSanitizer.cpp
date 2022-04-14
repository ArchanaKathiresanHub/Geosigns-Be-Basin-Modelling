//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "StringSanitizer.h"

#include <QRegularExpression>

namespace casaWizard
{
namespace stringSanitizer
{

QString escapeSpecialCharacters(QString str)
{
   str = str.replace("\a","\\a")
         .replace("\b","\\b")
         .replace("\f","\\f")
         .replace("\n","\\n")
         .replace("\r","\\r")
         .replace("\t","\\t")
         .replace("\v","\\v");

   return str;
}

} // casaWizard
} // stringSanitizer
