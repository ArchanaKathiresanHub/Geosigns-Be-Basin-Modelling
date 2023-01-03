//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "expectFileEq.h"
#include "model/input/cmbProjectReader.h"
#include "model/output/T2ZInfoGenerator.h"
#include "model/LithologyScenario.h"

#include <gtest/gtest.h>

using namespace casaWizard::sac;

TEST( T2ZInfoGenerator, testGenerateOnOriginalProject )
{
  // Given
  std::remove("infoT2ZOnOriginalActual.txt");
  casaWizard::ProjectReader* t2zProjectReader = new casaWizard::CMBProjectReader();
  t2zProjectReader->load("T2ZProject.project3d");
  LithologyScenario scenario(t2zProjectReader);
  scenario.setT2zSubSampling(4);
  scenario.setT2zReferenceSurface(0);
  scenario.setT2zRunOnOriginalProject(true);

  // When
  T2ZInfoGenerator generator(scenario, *t2zProjectReader);
  generator.setFileName("infoT2ZOnOriginalActual.txt");
  generator.generateInfoTextFile();

  // Then
  expectFileEq("infoT2ZOnOriginalExpected.txt", "infoT2ZOnOriginalActual.txt");
}

TEST( T2ZInfoGenerator, testOnOptimizedProject )
{
  // Given
  std::remove("infoT2ZOnOptimizedActual.txt");
  casaWizard::ProjectReader* t2zProjectReader = new casaWizard::CMBProjectReader();
  t2zProjectReader->load("T2ZProject.project3d");
  LithologyScenario scenario(t2zProjectReader);
  scenario.setT2zSubSampling(4);
  scenario.setT2zReferenceSurface(0);
  scenario.setT2zRunOnOriginalProject(false);

  // When
  T2ZInfoGenerator generator(scenario, *t2zProjectReader);
  generator.setFileName("infoT2ZOnOptimizedActual.txt");
  generator.generateInfoTextFile();

  // Then
  expectFileEq("infoT2ZOnOptimizedExpected.txt", "infoT2ZOnOptimizedActual.txt");
}


