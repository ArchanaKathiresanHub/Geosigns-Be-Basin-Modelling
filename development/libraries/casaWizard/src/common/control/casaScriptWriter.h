//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Namespace with method to write casa scripts to file

class QString;

namespace casaWizard
{

class CasaScript;

namespace casaScriptWriter
{
  bool writeCasaScript(const CasaScript& script);
}

} // namespace casaWizard
