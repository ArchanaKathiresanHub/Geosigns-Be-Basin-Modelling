// Reading a plain text files, with its data separated by spaces.  Creates a matrix with the data
#pragma once

#include <QString>
#include <QVector>

namespace casaWizard
{

template<class Type>
class DataFileParser
{
public:
  explicit DataFileParser(const QString& filename);
  QVector<QVector<Type>> matrixData() const;
  QVector<QVector<Type>> rowDominantMatrix() const;

private:
  QVector<Type> readRowToVector(const QString& line) const;

  const QString filename_;
};

} // namespace casaWizard

#include "dataFileParser.hpp"
