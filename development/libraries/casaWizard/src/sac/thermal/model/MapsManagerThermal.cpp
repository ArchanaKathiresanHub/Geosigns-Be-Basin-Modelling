//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MapsManagerThermal.h"
#include "model/input/mapReader.h"
#include "model/input/projectReader.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "model/output/zycorWriter.h"

namespace casaWizard
{

namespace sac
{

void MapsManagerThermal::exportOptimizedMapsToZycor(const ProjectReader& projectReader, MapReader& mapReader, const QString& targetPath)
{
   ZycorWriter writer;
   double xMin = 0.0;
   double xMax = 0.0;
   double yMin = 0.0;
   double yMax = 0.0;
   long numI = 0;
   long numJ = 0;
   mapReader.getMapDimensions(xMin, xMax, yMin, yMax, numI, numJ);
   const casaWizard::MapMetaData metaData(xMin, xMax, yMin, yMax, numI, numJ);

   //Write maps
}

} // namespace sac

} // namespace casaWizard
