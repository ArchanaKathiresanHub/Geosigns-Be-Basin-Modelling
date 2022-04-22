//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PredictionTargetReader.h"

#include "predictionTargetDepth.h"
#include "predictionTargetSurface.h"

namespace casaWizard
{

namespace ua
{

PredictionTarget* PredictionTargetReader::readTarget(const int version, const QStringList& parameters)
{
   if (parameters[0] == "surface")
   {
      return new PredictionTargetSurface(PredictionTargetSurface::read(version, parameters));
   }
   else if (parameters[0] == "depth")
   {
      return new PredictionTargetDepth(PredictionTargetDepth::read(version, parameters));
   }

   return nullptr;
}

}

}
