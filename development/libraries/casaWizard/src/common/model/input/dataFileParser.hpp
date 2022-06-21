//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <model/input/dataFileParser.h>

#include <utility>

namespace casaWizard
{

namespace
{

template<class T>
T convertStringToType(const QString& entry);

template<> double convertStringToType<double>(const QString& entry)
{
   return entry.toDouble();
}

template<> int convertStringToType<int>(const QString& entry)
{
   return entry.toInt();
}

template<> QString convertStringToType<QString>(const QString& entry)
{
   return entry;
}

template<class Type>
QVector<Type> readRowToVector(const QString& line)
{
   QVector<Type> rowData;
   const QStringList entries{line.split(" ", QString::SplitBehavior::SkipEmptyParts)};

   for (const QString& entry : entries)
   {
      rowData.push_back(convertStringToType<Type>(entry));
   }

   return rowData;
}

template<class Type>
QVector<QVector<Type>> convertToColumDominant(const QVector<QVector<Type>>& rowDominantData)
{
   //Assumes square matrix, which is checked by the readMatrix function.
   if (rowDominantData.size() == 0)
   {
      return {};
   }

   QVector<QVector<Type>> columnDominantData;
   const int rows{rowDominantData.size()};
   const int cols{rowDominantData[0].size()};

   for (int j = 0; j < cols; j++)
   {
      QVector<Type> columnData;
      for (int i = 0; i < rows; i++)
      {
         columnData.push_back(rowDominantData[i][j]);
      }
      columnDominantData.push_back(columnData);
   }
   return columnDominantData;
}

} // namespace

template<class Type>
DataFileParser<Type>::DataFileParser(const QString& fileName) :
   m_filename(fileName),
   m_file(fileName)
{
   if (!m_file.open(QIODevice::ReadOnly | QIODevice::Text))
   {
      throw std::runtime_error("Opening datafile " + fileName.toStdString() + " failed");
   }
   m_inputStream.setDevice(&m_file);
}

template<class Type>
QVector<QVector<Type>> DataFileParser<Type>::readFile(const QString& fileName)
{
   DataFileParser<Type> parser(fileName);
   return parser.readFile();
}

template<class Type>
QVector<QVector<Type>> DataFileParser<Type>::colDominantMatrix(const QString& fileName)
{
   return convertToColumDominant(rowDominantMatrix(fileName));
}

template<class Type>
QVector<QVector<Type>> DataFileParser<Type>::rowDominantMatrix(const QString& fileName)
{
   DataFileParser<Type> parser(fileName);
   return parser.readMatrix();
}

template<class Type>
QVector<QVector<Type>> DataFileParser<Type>::parseMatrixFileWithHeaderColDominant(const QString& fileName)
{
   return convertToColumDominant(parseMatrixFileWithHeaderRowDominant(fileName));
}

template<class Type>
QVector<QVector<Type>> DataFileParser<Type>::parseMatrixFileWithHeaderColDominant(const QString& fileName, QVector<QString>& headers)
{
   return convertToColumDominant(parseMatrixFileWithHeaderRowDominant(fileName,headers));
}

template<class Type>
QVector<QVector<Type>> DataFileParser<Type>::parseMatrixFileWithHeaderRowDominant(const QString& fileName)
{
   DataFileParser<Type> parser(fileName);
   parser.readHeader();
   return parser.readMatrix();
}

template<class Type>
QVector<QVector<Type>> DataFileParser<Type>::parseMatrixFileWithHeaderRowDominant(const QString& fileName, QVector<QString>& headers)
{
   DataFileParser<Type> parser(fileName);
   headers = parser.readHeader();
   return parser.readMatrix();
}

template<class Type>
QVector<QVector<Type>> DataFileParser<Type>::readFile()
{
   QVector<QVector<Type>> data;
   while (!m_inputStream.atEnd())
   {
      const QString textLine{m_inputStream.readLine()};
      const QVector<Type> rowData = readRowToVector<Type>(textLine);
      data.push_back(rowData);
   }
   return data;
}

template<class Type>
QVector<QVector<Type>> DataFileParser<Type>::readMatrix()
{
   QVector<QVector<Type>> rowDominantData = readFile();
   if (rowDominantData.size() > 0)
   {
      const int firstRowSize = rowDominantData.at(0).size();
      for (const auto& row : rowDominantData)
      {
         if (row.size() != firstRowSize)
         {
            throw std::runtime_error("Datafile " + m_filename.toStdString() + " does not have the expected file format and is not read");
         }
      }
   }
   return rowDominantData;
}

template<class Type>
QVector<QString> DataFileParser<Type>::readHeader()
{
   if (!m_inputStream.atEnd())
   {
      const QString textLine{m_inputStream.readLine()};
      return readRowToVector<QString>(textLine);
   }
   return {};
}

} //namespace casaWizard
