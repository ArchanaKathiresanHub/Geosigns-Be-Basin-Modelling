//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "oneDModelDataExtractor.h"

#include "calibrationTargetManager.h"
#include "casaScenario.h"
#include "functions/sortedByXWellIndices.h"
#include "track1d.h"
#include "well.h"

namespace casaWizard
{

OneDModelDataExtractor::OneDModelDataExtractor(const CalibrationTargetManager& ctManager, const std::string& iterationFolder, const std::string& project3dFilename) :
  ctManager_{ctManager},
  iterationFolder_{iterationFolder},
  project3dFilename_{project3dFilename}
{
}

ModelDataPropertyMap OneDModelDataExtractor::extract(const std::string& property) const
{
  ModelDataPropertyMap propertyModelDataAtWellLocation;

  for (const Well* well : ctManager_.wells())
  {
    if ( well->isIncludedInOptimization() )
    {
      DataExtraction::Track1d track1d(iterationFolder_ + well->name().toStdString() + "/" + project3dFilename_);
      if (!track1d.isCorrect())
      {
        continue;
      }

      track1d.acquireCoordinatePairs({{well->x(), well->y()}}, {});
      track1d.acquireSnapshots( {0, -1, -1} ); // the -1 are separators.
      track1d.acquireProperties({property}, false, false);
      track1d.acquireFormationSurfacePairs({}, {}, {}, {}, false);

      const bool success = track1d.run( true );
      if (success)
      {
        DepthVector depthData = track1d.getData("Depth");
        PropertyVector propertyData = track1d.getData(property);

        propertyModelDataAtWellLocation.insert({well->name().toStdString(), {depthData, propertyData}});
      }
    }
  }

  return propertyModelDataAtWellLocation;
}


} //namespace casaWizard
