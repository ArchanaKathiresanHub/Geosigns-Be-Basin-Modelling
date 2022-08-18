#include "model/scenarioWriter.h"

#include "expectFileEq.h"

#include <QVector>

#include <gtest/gtest.h>

struct Dummy
{
   QString name;
   int id;
   int version() const { return 0; }
   QStringList write() const
   {
      return {name, QString::number(id)};
   }
};

TEST( ScenarioWriterTest, testSimpleReads )
{
   const std::string actualFile{"scenarioWriterActual.dat"};
   const std::string expectedFile{"scenarioSimple.dat"};

   casaWizard::ScenarioWriter writer{QString::fromStdString(actualFile)};

   writer.writeValue("testString", "valueString");
   writer.writeValue("testBool", true);
   writer.writeValue("testInt", 123);
   writer.writeValue("testDouble", 3.14);

   QVector<Dummy> vector;

   vector.push_back({"testName", 1});
   vector.push_back({"testName2", 12});

   writer.writeValue("testVector", vector);

   writer.close();

   expectFileEq(expectedFile, actualFile);
}

TEST( ScenarioWriterTest, testQVectorBoolWrite )
{
   casaWizard::ScenarioWriter writer{"scenarioQVectorBoolActual.dat"};
   const QVector<bool> vec{true, false, true};

   writer.writeValue("testBoolVector", vec);
   writer.close();

   expectFileEq("scenarioQVectorBool.dat", "scenarioQVectorBoolActual.dat");
}

TEST( ScenarioWriterTest, testQVectorIntWrite )
{
   casaWizard::ScenarioWriter writer{"scenarioQVectorIntActual.dat"};
   const QVector<int> vec{1, 11, 111};

   writer.writeValue("testIntVector", vec);
   writer.close();

   expectFileEq("scenarioQVectorInt.dat", "scenarioQVectorIntActual.dat");
}

TEST( ScenarioWriterTest,  testQVectorDoubleWrite )
{
   casaWizard::ScenarioWriter writer{"scenarioQVectorDoubleActual.dat"};
   const QVector<double> vec{1,2,3};

   writer.writeValue("testDoubleVector", vec);
   writer.close();

   expectFileEq("scenarioQVectorDouble.dat", "scenarioQVectorDoubleActual.dat");
}

TEST( ScenarioWriterTest,  testQVectorQVectorDoubleWrite )
{
   casaWizard::ScenarioWriter writer{"scenarioMatrixActual.dat"};
   const QVector<QVector<double>> vec{{1,2,3},{4,5,6},{7,8,9}};

   writer.writeValue("testDoubleMatrix", vec);
   writer.close();

   expectFileEq("scenarioMatrix.dat", "scenarioMatrixActual.dat");
}

TEST( ScenarioWriterTest,  testQMapStringBoolWrite )
{
   casaWizard::ScenarioWriter writer{"scenarioQMapActual.dat"};
   const QMap<QString, bool> map{{"test1", false}, {"test2", true}, {"test3", false}};

   writer.writeValue("testStringBoolMap", map);
   writer.close();

   expectFileEq("scenarioQMap.dat", "scenarioQMapActual.dat");
}

