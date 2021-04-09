//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "stubInfoGenerator.h"

#include "expectFileEq.h"

#include <QDir>

#include <gtest/gtest.h>

using namespace casaWizard;

TEST( InfoGenerator, testInfoGenerator )
{
  std::remove("infoActual.txt");
  StubInfoGenerator generator;
  generator.setFileName("infoActual.txt");
  generator.generateInfoTextFile();

  expectFileEq("infoExpected.txt", "infoActual.txt");
}
