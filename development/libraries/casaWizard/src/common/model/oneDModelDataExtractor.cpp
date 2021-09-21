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

#include <QVector>

namespace casaWizard
{

OneDModelDataExtractor::OneDModelDataExtractor(const CasaScenario& scenario) :
  scenario_{scenario}
{
}

ModelDataPropertyMap OneDModelDataExtractor::extract(const std::string& property) const
{
  ModelDataPropertyMap propertyModelDataAtWellLocation;

  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManagerWellPrep();

  for (const Well* well : ctManager.wells())
  {
    if (well->isActive())
    {
      DataExtraction::Track1d track1d(scenario_.original1dDirectory().toStdString() + "/"
                                      + scenario_.runLocation().toStdString()
                                      + "/Iteration_1/"
                                      + well->name().toStdString() + "/"
                                      + scenario_.project3dFilename().toStdString());
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

        propertyModelDataAtWellLocation.insert({well->name(), {depthData, propertyData}});
      }
    }
  }

  return propertyModelDataAtWellLocation;
}


} //namespace casaWizard
