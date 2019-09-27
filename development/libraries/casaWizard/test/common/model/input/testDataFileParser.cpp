#include "model/input/dataFileParser.h"

#include <gtest/gtest.h>

TEST( DataFileParserTest, testFileParseDouble )
{
  const QString inputFilename{"testDataFileParserDouble.dat"};
  const casaWizard::DataFileParser<double> parser{inputFilename};

  const QVector<QVector<double>> dataActual = parser.matrixData();
  const QVector<QVector<double>> dataExpected = {{1.188, 4.0, 7.4}, {2.76, 5.1, 8.24}};

  const int rows{dataActual.size()};
  const int cols{dataActual[0].size()};

  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      EXPECT_DOUBLE_EQ( dataActual[i][j], dataExpected[i][j] ) << "Mismatch at (" << i << "," << j << ")";
    }
  }
}

TEST( DataFileParserTest, testFileParseInt )
{
  const QString inputFilename{"testDataFileParserInt.dat"};
  const casaWizard::DataFileParser<int> parser{inputFilename};

  const QVector<QVector<int>> dataActual = parser.matrixData();
  const QVector<QVector<int>> dataExpected = {{1, 4, 7}, {2, 5, 8}};

  const int rows{dataActual.size()};
  const int cols{dataActual[0].size()};

  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      EXPECT_EQ( dataActual[i][j], dataExpected[i][j] ) << "Mismatch at (" << i << "," << j << ")";
    }
  }
}
