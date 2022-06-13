#include "model/input/dataFileParser.h"

#include <gtest/gtest.h>

void compareMatricesDouble(const QVector<QVector<double>>& m1, const QVector<QVector<double>>& m2)
{
   const int rows{m1.size()};
   const int cols{m1[0].size()};

   for (int i = 0; i < rows; i++)
   {
      for (int j = 0; j < cols; j++)
      {
         EXPECT_DOUBLE_EQ( m1[i][j], m2[i][j] ) << "Mismatch at (" << i << "," << j << ")";
      }
   }
}

void compareHeaders(const QVector<QString>& h1, const QVector<QString>& h2)
{
   for (int i = 0; i < h1.size(); i++)
   {
      EXPECT_EQ(h1[i],h2[i]);
   }
}

TEST( DataFileParserTest, testFileParseDoubleColDominant )
{
   const QString inputFilename{"testDataFileParserDouble.dat"};
   const QVector<QVector<double>> dataActual = casaWizard::DataFileParser<double>::colDominantMatrix(inputFilename);
   const QVector<QVector<double>> dataExpected = {{1.188, 4.0, 7.4}, {2.76, 5.1, 8.24}};
   compareMatricesDouble(dataActual,dataExpected);
}

TEST( DataFileParserTest, testFileParseDoubleRowDominant )
{
   const QString inputFilename{"testDataFileParserDouble.dat"};
   const QVector<QVector<double>> dataActual = casaWizard::DataFileParser<double>::rowDominantMatrix(inputFilename);
   const QVector<QVector<double>> dataExpected = {{1.188, 2.76}, {4.0, 5.1}, {7.4, 8.24}};
   compareMatricesDouble(dataActual,dataExpected);
}

TEST( DataFileParserTest, testFileParseDoubleHeaders )
{
   const QString inputFilename{"testDataFileParserDoubleHeaders.dat"};
   QVector<QString> headersActualColDom;
   QVector<QVector<double>> dataActual = casaWizard::DataFileParser<double>::parseFileWithHeaderColDominant(inputFilename,headersActualColDom);
   QVector<QVector<double>> dataExpected = {{1.188, 4.0, 7.4}, {2.76, 5.1, 8.24}};

   compareMatricesDouble(dataActual,dataExpected);
   const QVector<QString> headersExpected = {"h1","h2"};
   compareHeaders(headersActualColDom,headersExpected);

   QVector<QString> headersActualRowDom;
   dataActual = casaWizard::DataFileParser<double>::parseFileWithHeaderRowDominant(inputFilename,headersActualRowDom);
   dataExpected = {{1.188, 2.76}, {4.0, 5.1}, {7.4, 8.24}};
   compareMatricesDouble(dataActual,dataExpected);
   compareHeaders(headersActualRowDom,headersExpected);
}

TEST( DataFileParserTest, testFileParseInt )
{
   const QString inputFilename{"testDataFileParserInt.dat"};
   const QVector<QVector<int>> dataActual = casaWizard::DataFileParser<int>::colDominantMatrix(inputFilename);
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

TEST( DataFileParserTest, testFileParseIntWrongFormat )
{
   const QString inputFilename{"testDataFileParserWrongFormat.dat"};
   EXPECT_THROW(casaWizard::DataFileParser<int>::colDominantMatrix(inputFilename),std::runtime_error);
}

TEST( DataFileParserTest, testFileWrongHeaderFormat )
{
   const QString inputFilename{"testDataFileParserDoubleHeadersWrongFormat.dat"};
   EXPECT_THROW(casaWizard::DataFileParser<int>::colDominantMatrix(inputFilename),std::runtime_error);
}
