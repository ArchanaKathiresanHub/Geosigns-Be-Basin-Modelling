#include "expectFileEq.h"

#include <QDir>

#include <gtest/gtest.h>

using namespace casaWizard;

TEST( InfoGenerator, testInfoGenerator )
{
  std::remove("infoActual.txt");
  StubInfoGenerator generator("infoActual.txt");
  generator.generateInfoTextFile();

  expectFileEq("infoExpected.txt", "infoActual.txt");
}
