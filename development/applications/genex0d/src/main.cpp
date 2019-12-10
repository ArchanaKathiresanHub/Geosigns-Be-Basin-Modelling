//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// This application does Genex calculations on a given point in the numerical domain for a given source rock type
// Genex0d needs input from the user, the database (source rock properties) and p/T history data

#include "CommonDefinitions.h"

#include "Genex0d.h"
#include "Genex0dInputManager.h"

#include "ErrorHandler.h"

#include "LogHandler.h"

#include <iostream>
#include <memory>
#include <string.h>

int main (int argc, char** argv)
{
  try
  {
    LogHandler("genex0d", LogHandler::DETAILED_LEVEL);
    genex0d::Genex0dInputManager inputDataMgr(argc, argv);

    typedef genex0d::Genex0dInputManager::ExitStatus ExitState;

    std::string ioErrorMessage = "";

    ExitState ioErrorFlag = inputDataMgr.initialCheck(ioErrorMessage);
    if (ioErrorFlag == ExitState::NO_ERROR_EXIT)
    {
      return 0;
    }
    else if (ioErrorFlag == ExitState::WITH_ERROR_EXIT)
    {
      throw genex0d::Genex0dException() << ioErrorMessage;
    }

    ioErrorFlag = inputDataMgr.storeInput(ioErrorMessage);
    if (ioErrorFlag == ExitState::WITH_ERROR_EXIT)
    {
      throw genex0d::Genex0dException() << ioErrorMessage;
    }

    genex0d::Genex0d gnx0d(inputDataMgr.inputData());
    gnx0d.initialize();
    gnx0d.run();
  }
  catch (const ErrorHandler::Exception & ex)
  {
    LogHandler(LogHandler::FATAL_SEVERITY) << "CMB API fatal error: " << ex.errorCode() << ", " << ex.what();
    return 1;
  }
  catch (const genex0d::Genex0dException & ex)
  {
    LogHandler(LogHandler::FATAL_SEVERITY) << "Genex0d fatal error: " << ex.what();
    return 1;
  }

  return 0;
}
