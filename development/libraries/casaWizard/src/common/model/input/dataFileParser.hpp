#include <model/input/dataFileParser.h>

#include <QFile>
#include <QTextStream>

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

} // namespace

template<class Type>
DataFileParser<Type>::DataFileParser(const QString& filename) :
  filename_{filename}
{
}

template<class Type>
QVector<QVector<Type>> DataFileParser<Type>::matrixData() const
{
  QVector<QVector<Type>> columnDominantData;
  const QVector<QVector<Type>> rowDominantData = rowDominantMatrix();

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

template<class Type>
QVector<QVector<Type>> DataFileParser<Type>::rowDominantMatrix() const
{
  QVector<QVector<Type>> rowDominantData;
  QFile file{filename_};

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    throw std::runtime_error("Opening datafile " + filename_.toStdString() + " failed");
  }

  QTextStream inputStream{&file};

  while (!inputStream.atEnd())
  {
    const QString textLine{inputStream.readLine()};
    const QVector<Type> rowData = readRowToVector(textLine);
    rowDominantData.push_back(rowData);
  }

  return rowDominantData;
}

template<class Type>
QVector<Type> DataFileParser<Type>::readRowToVector(const QString& line) const
{
  QVector<Type> rowData;
  const QStringList entries{line.split(" ", QString::SplitBehavior::SkipEmptyParts)};

  for (const QString& entry : entries)
  {
    rowData.push_back(convertStringToType<Type>(entry));
  }

  return rowData;
}

} //namespace casaWizard
