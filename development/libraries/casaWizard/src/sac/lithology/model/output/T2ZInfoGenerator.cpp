//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "T2ZInfoGenerator.h"

#include "model/LithologyScenario.h"

namespace casaWizard
{

namespace sac
{

namespace lithology
{

T2ZInfoGenerator::T2ZInfoGenerator(const LithologyScenario& scenario, ProjectReader& t2zProjectReader) :
  LithologyInfoGenerator(scenario, t2zProjectReader)
{}

void T2ZInfoGenerator::generateInfoTextFile()
{
  addGeneralSettingsSection();

  if (!scenario().t2zRunOnOriginalProject())
  {
    addInputTabSection();
    addMapsGenerationSection();
    addWellsSection();
  }

  addT2ZSection();
  addSectionSeparator();

  writeTextToFile();
}

void T2ZInfoGenerator::addT2ZSection()
{
  addSectionSeparator();
  addT2ZRunSettings();
  addSurfaceInfo();
}

void T2ZInfoGenerator::addT2ZRunSettings()
{
    addHeader("T2Z Settings");
    if (scenario().t2zRunOnOriginalProject())
    {
      addOption("Project Selection", "Original");
    }
    else
    {
      addOption("Project Selection", "Optimized");
    }
    addOption("Reference Surface", projectReader().surfaceNames()[scenario().t2zReferenceSurface()].toStdString());
    addOption("Subsampling", std::to_string(scenario().t2zSubSampling()));
}

void T2ZInfoGenerator::addSurfaceInfo()
{
  addHeader("Surfaces");
  int id = 0;
  QStringList mapNames = projectReader().mapNamesT2Z();
  for (const QString& surface : projectReader().surfaceNames())
  {
    std::string optionValue;
    if (!projectReader().hasTWTData(id))
    {
      optionValue += "NO TWTT; ";
    }

    QString surfaceNameMap = projectReader().getDepthGridName(id);
    if (surfaceNameMap.startsWith("T2Z"))
    {
      optionValue += "RECALCULATED DEPTH";
      if (surfaceNameMap.startsWith("T2ZIsoSurf"))
      {
        optionValue += " using isopack";
      }
      else if (!projectReader().hasTWTData(id))
      {
        optionValue += " using calculated TWTT map";
      }
    }

    if (optionValue.empty() || optionValue == "NO TWTT; ")
    {
      optionValue += "NO CHANGES";
    }

    addOption("Surface " + surface.toStdString(), optionValue);
    id++;
  }
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
