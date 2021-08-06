//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <fstream>
#include <string>
#include <vector>

namespace casaWizard
{

struct MapMetaData
{
  double xMin;
  double xMax;
  double yMin;
  double yMax;
  int numI;
  int numJ;

  MapMetaData(double xMin = 0,
              double xMax = 0,
              double yMin = 0,
              double yMax = 0,
              int numI = 0,
              int numJ = 0) :
    xMin{xMin},
    xMax{xMax},
    yMin{yMin},
    yMax{yMax},
    numI{numI},
    numJ{numJ}
  {
  }
};


class ZycorWriter
{
public:
  ZycorWriter();
  void writeToFile(const std::string& fileName,
                   const std::vector<std::vector<double>>& data,
                   const MapMetaData& metaData);

private:
  void writeHeader(const MapMetaData& metaData);
  void writeData(const std::vector<std::vector<double> >& data);
  void writeDataField(const double dataPoint);

  std::ofstream outputFile_;
  const int fieldSize_;
  const int numberOfColumns_;
};

} // namespace casaWizard
