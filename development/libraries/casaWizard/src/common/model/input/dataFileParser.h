//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Reading a plain text files, with its data separated by spaces.  Creates a matrix with the data
#pragma once

#include <QString>
#include <QVector>
#include <QTextStream>
#include <QFile>

namespace casaWizard
{

template<class Type>
class DataFileParser
{
public:
   static QVector<QVector<Type>> colDominantMatrix(const QString& fileName);
   static QVector<QVector<Type>> rowDominantMatrix(const QString& fileName);

   static QVector<QVector<Type>> parseFileWithHeaderColDominant(const QString& fileName);
   static QVector<QVector<Type>> parseFileWithHeaderColDominant(const QString& fileName, QVector<QString>& headers);

   static QVector<QVector<Type>> parseFileWithHeaderRowDominant(const QString& fileName);
   static QVector<QVector<Type>> parseFileWithHeaderRowDominant(const QString& fileName, QVector<QString>& headers);

private:
   explicit DataFileParser(const QString& fileName);

   QVector<QVector<Type>> readMatrix();
   QVector<QString> readHeader();

   QFile m_file;
   const QString m_filename;
   QTextStream m_inputStream;
};

} // namespace casaWizard

#include "dataFileParser.hpp"
