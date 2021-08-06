//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


#include "zycorWriter.h"

#include "ConstantsNumerical.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

namespace casaWizard
{

ZycorWriter::ZycorWriter() :
  outputFile_{},
  fieldSize_{20},
  numberOfColumns_{4}
{
}

void ZycorWriter::writeToFile(const std::string& fileName,
                              const std::vector<std::vector<double>>& data,
                              const MapMetaData& metaData)
{
  if (data.empty())
  {
    return;
  }

  outputFile_ = std::ofstream(fileName);
  outputFile_ << std::setprecision(10);

  writeHeader(metaData);
  writeData(data);

  outputFile_.close();
}

void ZycorWriter::writeHeader(const MapMetaData& metaData)
{
  outputFile_ << "@Grid HEADER, GRID, "<< numberOfColumns_ << std::endl;
  outputFile_ << fieldSize_ << ", " << Utilities::Numerical::CauldronNoDataValue << ", , 7, 1" << std::endl;
  outputFile_ << metaData.numJ << ", " << metaData.numI << ", " << metaData.xMin <<", " << metaData.xMax <<", " << metaData.yMin <<", " << metaData.yMax << std::endl;
  outputFile_ << "0.0, 0.0, 0.0\n";
  outputFile_ << "@" << std::endl;
}

void ZycorWriter::writeData(const std::vector<std::vector<double>>& data)
{
  if (data.empty())
  {
    return;
  }
  for (int i = 0; i < data[0].size(); i++)
  {
    int numberOfDataFields = 0;
    // Invert loop in y-direction to account for 'matrix-layout of the data'
    for (int j = data.size() - 1; j >= 0; j--)
    {
      writeDataField(data[j][i]);
      numberOfDataFields++;

      if (numberOfDataFields % numberOfColumns_ == 0)
      {
        outputFile_ << std::endl;
      }
    }
    outputFile_ << std::endl;
  }
}

void ZycorWriter::writeDataField(const double dataPoint)
{
  std::ostringstream dataPointStringStream;
  dataPointStringStream << std::setw(fieldSize_) << std::setprecision(10) << std::noshowpoint << dataPoint;
  outputFile_ << dataPointStringStream.str();
}

} // namespace casaWizard

