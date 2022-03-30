#include "model/scenarioReader.h"

#include <QStringList>
#include <QVector>

#include <gtest/gtest.h>

struct Dummy
{
  QString name;
  int id;
  Dummy() = default;
  Dummy(const QString& n, const int i) : name{n}, id{i}{}
  static Dummy read(const int /*version*/, const QStringList& p)
  {
    return Dummy{p[0], p[1].toInt()};
  }
};

TEST( ScenarioReaderTest, testSimpleReads )
{
  casaWizard::ScenarioReader reader(QString::fromStdString("scenarioSimple.dat"));

  EXPECT_EQ(reader.readInt("testInt"), 123);
  EXPECT_EQ(reader.readBool("testBool"), true);
  EXPECT_EQ(reader.readString("testString").toStdString(), "valueString");
  EXPECT_DOUBLE_EQ(reader.readDouble("testDouble"), 3.14);

  const QVector<Dummy> vector = reader.readVector<Dummy>("testVector");
  ASSERT_EQ(vector.size(), 2);
  EXPECT_EQ(vector[0].name.toStdString(), "testName");
  EXPECT_EQ(vector[0].id, 1);
  EXPECT_EQ(vector[1].name.toStdString(), "testName2");
  EXPECT_EQ(vector[1].id, 12);
}

TEST( ScenarioReaderTest, testQVectorBoolRead )
{
  casaWizard::ScenarioReader reader{"scenarioQVectorBool.dat"};
  const QVector<bool> vectorActual{reader.readVector<bool>("testBoolVector")};
  const QVector<bool> vectorExpected{true, false, true};

  for (int i = 0; i < 3; ++i)
  {
    EXPECT_EQ(vectorExpected[i], vectorActual[i]) << "Mismatch at " << i;
  }

}

TEST( ScenarioReaderTest, testQVectorIntRead )
{
  casaWizard::ScenarioReader reader{"scenarioQVectorInt.dat"};
  const QVector<int> vectorActual{reader.readVector<int>("testIntVector")};
  const QVector<int> vectorExpected{1, 11, 111};

  for (int i = 0; i < 3; ++i)
  {
    EXPECT_EQ(vectorExpected[i], vectorActual[i]) << "Mismatch at " << i;
  }
}

TEST( ScenarioReaderTest, testQVectorDoubleRead )
{
  casaWizard::ScenarioReader reader{"scenarioQVectorDouble.dat"};
  const QVector<double> vectorActual{reader.readVector<double>("testDoubleVector")};
  const QVector<double> vectorExpected{1, 2, 3};

  for (int i = 0; i < 3; ++i)
  {
    EXPECT_DOUBLE_EQ(vectorExpected[i], vectorActual[i]) << "Mismatch at " << i;
  }
}

TEST( ScenarioReaderTest, testQVectorQVectorDoubleRead )
{
  casaWizard::ScenarioReader reader{"scenarioQVectorDouble.dat"};
  const QVector<QVector<double>> matrixActual{reader.readVector<double>("testDoubleVector")};
  const QVector<QVector<double>> matrixExpected{{1,2,3},{4,5,6},{7,8,9}};

  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      EXPECT_DOUBLE_EQ(matrixExpected[i][j], matrixExpected[i][j]) << "Mismatch at [" << i << "," << j << "]";
    }
  }
}

TEST( ScenarioReaderTest, testQMapStringBoolRead )
{
  casaWizard::ScenarioReader reader{"scenarioQMap.dat"};
  const QMap<QString, bool> matrixActual{reader.readMap<QString, bool>("testStringBoolMap")};

  EXPECT_EQ(matrixActual.size(), 3);

}
