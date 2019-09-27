#include "model/objectiveFunctionManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "expectFileEq.h"

#include <gtest/gtest.h>

TEST(ObjectiveFunctionTest, testEmptyVector)
{
  casaWizard::ObjectiveFunctionManager objectiveFunction;

  QStringList values = objectiveFunction.variables();

  int expectedSize{0};

  ASSERT_EQ(values.size(), expectedSize);
}

TEST(ObjectiveFunctionTest, testDefaultVector)
{
  QStringList variables{"Temperature","VRe","Pressure"};
  casaWizard::ObjectiveFunctionManager objectiveFunction;
  objectiveFunction.setVariables(variables);

  QStringList values = objectiveFunction.variables();

  const int expectedSize{variables.size()};
  const double expectedAbsErr{0.0};
  const double expectedRelErr{0.1};
  const double expectedWeight{1.0/3.0};

  ASSERT_EQ(values.size(), expectedSize);
  for (int i = 0; i<expectedSize; ++i)
  {
    ASSERT_DOUBLE_EQ(objectiveFunction.absoluteError(i), expectedAbsErr) << " mismatch at index " << i;
    ASSERT_DOUBLE_EQ(objectiveFunction.relativeError(i), expectedRelErr) << " mismatch at index " << i;
    ASSERT_DOUBLE_EQ(objectiveFunction.weight(i), expectedWeight) << " mismatch at index " << i;
  }
}

TEST(ObjectiveFunctionTest, testSetValue)
{
  QStringList variables{"Temperature"};
  casaWizard::ObjectiveFunctionManager objectiveFunction;
  objectiveFunction.setVariables(variables);

  double expectedValue{10};
  objectiveFunction.setValue(0.0, 0.0, expectedValue);
  double value = objectiveFunction.absoluteError(0.0);

  ASSERT_DOUBLE_EQ(value, expectedValue);
}

TEST(ObjectiveFunctionTest, testWriteToFile)
{
  casaWizard::ObjectiveFunctionManager objectiveFunction{};
  objectiveFunction.setVariables({"Temperature", "VRe"});

  objectiveFunction.setValue(0, 0, 1);
  objectiveFunction.setValue(0, 1, 2);
  objectiveFunction.setValue(0, 2, 3);
  objectiveFunction.setValue(1, 0, 4);

  casaWizard::ScenarioWriter writer{"objectiveFunctionManagerActual.dat"};
  objectiveFunction.writeToFile(writer);
  writer.close();

  expectFileEq("objectiveFunctionManager.dat","objectiveFunctionManagerActual.dat");
}

TEST(ObjectiveFunctionTest, testReadFromFile)
{
  casaWizard::ScenarioReader reader{"objectiveFunctionManager.dat"};
  casaWizard::ObjectiveFunctionManager objectiveFunction;
  objectiveFunction.readFromFile(reader);

  QStringList variables{objectiveFunction.variables()};
  ASSERT_EQ(variables.size(), 2);
  EXPECT_EQ(variables[0].toStdString(), "Temperature");
  EXPECT_EQ(variables[1].toStdString(), "VRe");

  EXPECT_DOUBLE_EQ(objectiveFunction.absoluteError(0), 1);
  EXPECT_DOUBLE_EQ(objectiveFunction.relativeError(0), 2);
  EXPECT_DOUBLE_EQ(objectiveFunction.weight(0), 3);
  EXPECT_DOUBLE_EQ(objectiveFunction.absoluteError(1), 4);
}
