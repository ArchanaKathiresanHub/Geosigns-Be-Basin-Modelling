//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Given an input model with input hdf maps at a specified areal location (X, Y), this application generates a pseudo1d model.
// The new pseudo1d model does not need hdf maps (replaced by scalar values in project file).

#include "ModelPseudo1d.h"
#include "ModelPseudo1dCommonDefinitions.h"
#include "ModelPseudo1dInputManager.h"
#include "ModelPseudo1dProjectManager.h"

#include "LogHandler.h"

#include <cstring>
#include <iostream>
#include <string>

void printHelp();

int main(int argc, char** argv)
{
  try
  {
    LogHandler("modelPseudo1d", LogHandler::DETAILED_LEVEL);
    modelPseudo1d::ModelPseudo1dInputManager inputDataMgr(argc, argv);

    std::string ioErrorMessage = "";

    typedef modelPseudo1d::ModelPseudo1dInputManager::ExitStatus ExitState;

    const ExitState ioErrorFlag = inputDataMgr.initialCheck(ioErrorMessage);
    if (ioErrorFlag == ExitState::NO_ERROR_EXIT)
    {
      return 0;
    }
    else if (ioErrorFlag == ExitState::WITH_ERROR_EXIT)
    {
      throw modelPseudo1d::ModelPseudo1dException() << ioErrorMessage;
    }

    if (inputDataMgr.storeInput(ioErrorMessage) == ExitState::WITH_ERROR_EXIT)
    {
      throw modelPseudo1d::ModelPseudo1dException() << ioErrorMessage;
    }

    const modelPseudo1d::ModelPseudo1dInputData& inputData = inputDataMgr.inputData();

    modelPseudo1d::ModelPseudo1dProjectManager mdlPseudo1dPrjctManager(inputData.projectFilename, inputData.outProjectFilename);

    modelPseudo1d::ModelPseudo1d mdlPseudo1d(mdlPseudo1dPrjctManager.mdl(), inputData.xCoord, inputData.yCoord);

    mdlPseudo1d.initialize();
    mdlPseudo1d.extractScalarsFromInputMaps();
    mdlPseudo1d.setScalarsInModel();
    mdlPseudo1d.setSingleCellWindowXY();
    mdlPseudo1dPrjctManager.save();

  }
  catch (const ErrorHandler::Exception & ex)
  {
    LogHandler(LogHandler::FATAL_SEVERITY) << "CMB API fatal error: " << ex.errorCode() << ", " << ex.what();
    return 1;
  }
  catch (const modelPseudo1d::ModelPseudo1dException & ex)
  {
    LogHandler(LogHandler::FATAL_SEVERITY) << "modelPseudo1d fatal error: " << ex.what();
    return 1;
  }

  return 0;
}
