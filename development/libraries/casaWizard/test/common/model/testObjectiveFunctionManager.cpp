#include "model/objectiveFunctionManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "expectFileEq.h"

#include <gtest/gtest.h>

TEST(ObjectiveFunctionTest, testEmptyVector)
{  
  casaWizard::ObjectiveFunctionManager objectiveFunction;

  QStringList values = objectiveFunction.variablesCauldronNames();

  int expectedSize{0};

  ASSERT_EQ(values.size(), expectedSize);
}

TEST(ObjectiveFunctionTest, testDefaultVector)
{
  QStringList variables{"Temperature", "Vre", "Pressure"};
  QMap<QString, QString> mapping;
  mapping["Temperature"] = "Temperature";
  mapping["Vre"] = "VRe";
  mapping["Pressure"] = "Pressure";

  casaWizard::ObjectiveFunctionManager objectiveFunction;  
  objectiveFunction.setVariables(variables, mapping);

  QStringList values = objectiveFunction.variablesCauldronNames();

  const int expectedSize{variables.size()};
  const std::vector<double> expectedAbsErr{10.0, 0.2, 0.0};
  const std::vector<double> expectedRelErr{0.0, 0.0, 0.01};
  const double expectedWeight{1.0};

  ASSERT_EQ(values.size(), expectedSize);
  for (int i = 0; i<expectedSize; ++i)
  {
    ASSERT_DOUBLE_EQ(objectiveFunction.absoluteError(i), expectedAbsErr[i]) << " mismatch at index " << i;
    ASSERT_DOUBLE_EQ(objectiveFunction.relativeError(i), expectedRelErr[i]) << " mismatch at index " << i;
    ASSERT_DOUBLE_EQ(objectiveFunction.weight(i), expectedWeight) << " mismatch at index " << i;
  }
}

TEST(ObjectiveFunctionTest, testSetValue)
{
  QStringList variables{"Temperature"};
  QMap<QString, QString> mapping;
  mapping["Temperature"] = "Temperature";
  casaWizard::ObjectiveFunctionManager objectiveFunction;  
  objectiveFunction.setVariables(variables, mapping);

  double expectedValue{10};
  objectiveFunction.setValue(0, 1, expectedValue);
  double value = objectiveFunction.absoluteError(0);

  ASSERT_DOUBLE_EQ(value, expectedValue);
}

TEST(ObjectiveFunctionTest, testWriteToFile)
{
  QMap<QString, QString> mapping;
  mapping["Temperature"] = "Temperature";
  mapping["VReUserName"] = "VRe";
  casaWizard::ObjectiveFunctionManager objectiveFunction;  
  QStringList variables{"Temperature", "VReUserName"};
  objectiveFunction.setVariables(variables, mapping);

  objectiveFunction.setValue(0, 0, 1);
  objectiveFunction.setValue(0, 1, 2);
  objectiveFunction.setValue(0, 2, 3);
  objectiveFunction.setValue(1, 0, 4);
  objectiveFunction.setEnabledState(false, 1);

  casaWizard::ScenarioWriter writer{"objectiveFunctionManagerActual.dat"};
  objectiveFunction.writeToFile(writer);
  writer.close();

  expectFileEq("objectiveFunctionManagerVersion2.dat","objectiveFunctionManagerActual.dat");
}

void testReading(const QString& testFile, const int version)
{
  QMap<QString, QString> mapping;
  if (version > 1)
  {
    mapping["Temperature"] = "Temperature";
    mapping["VReUserName"] = "VRe";
  }
  else
  {
    mapping["Temperature"] = "Temperature";
    mapping["VRe"] = "VRe";
  }

  casaWizard::ScenarioReader reader{testFile};
  casaWizard::ObjectiveFunctionManager objectiveFunction;
  objectiveFunction.setUserNameToCauldronNameMapping(mapping);
  objectiveFunction.readFromFile(reader);

  QStringList variables{objectiveFunction.variablesCauldronNames()};
  ASSERT_EQ(variables.size(), 2);
  EXPECT_EQ(variables[0].toStdString(), "Temperature");
  EXPECT_EQ(variables[1].toStdString(), "VRe");

  QStringList variablesUserName{objectiveFunction.variablesUserNames()};
  ASSERT_EQ(variablesUserName.size(), 2);
  EXPECT_EQ(variablesUserName[0].toStdString(), "Temperature");
  EXPECT_EQ(variablesUserName[1].toStdString(), version > 1 ? "VReUserName" : "VRe");

  EXPECT_DOUBLE_EQ(objectiveFunction.absoluteError(0), 1);
  EXPECT_DOUBLE_EQ(objectiveFunction.relativeError(0), 2);
  EXPECT_DOUBLE_EQ(objectiveFunction.weight(0), 3);
  EXPECT_TRUE(objectiveFunction.enabled(0));
  EXPECT_DOUBLE_EQ(objectiveFunction.absoluteError(1), 4);
  EXPECT_EQ(objectiveFunction.enabled(1), version == 0);
}

TEST(ObjectiveFunctionTest, testReadVersion0FromFile)
{
  const QString testFile = "objectiveFunctionManagerVersion0.dat";
  testReading(testFile, 0);
}

TEST(ObjectiveFunctionTest, testReadVersion1FromFile)
{
  const QString testFile = "objectiveFunctionManagerVersion1.dat";
  testReading(testFile, 1);
}

TEST(ObjectiveFunctionTest, testReadVersion2FromFile)
{
  const QString testFile = "objectiveFunctionManagerVersion2.dat";
  testReading(testFile, 2);
}
