#include "model/input/extractWellDataXlsx.h"
#include "wellTestData.h"
#include <gtest/gtest.h>

namespace testCasaWizard
{

TEST( ExtractWellDataXlsxTest, testWellDataConsistency )
{
  QVector<QString> extractedWellNames_;
  QVector<Well> extractedWell_;
  QVector<WellTargetVariables> extractedWellTargetVariables_;
  QVector<WellData> extractedWellData_;

  const QString inputFileName{"./testWellData.xlsx"};
  casaWizard::ExtractWellDataXlsx wellDataExtract{inputFileName};
  extractedWellNames_ = wellDataExtract.wellNames();

  extractedWell_ = {};
  extractedWellTargetVariables_ = {};
  extractedWellData_ = {};

  for (const QString& wellName : wellDataExtract.wellNames())
  {
    wellDataExtract.extractWellData(wellName);

    Well well;
    well.wellName_ = wellName;
    well.xCoord_ = wellDataExtract.xCoord();
    well.yCoord_ = wellDataExtract.yCoord();
    well.nTargetVariables = wellDataExtract.nCalibrationTargetVars();
    extractedWell_.push_back(well);

    std::size_t iDataTargetPrevious = 0;
    for (std::size_t i = 0; i < wellDataExtract.nCalibrationTargetVars(); ++i)
    {
      WellTargetVariables wellVars;
      wellVars.nData = wellDataExtract.nDataPerTargetVar()[i];
      wellVars.variable_ = wellDataExtract.calibrationTargetVars()[i];
      extractedWellTargetVariables_.push_back(wellVars);

      for (std::size_t j = iDataTargetPrevious; j < wellDataExtract.nDataPerTargetVar()[i] + iDataTargetPrevious; ++j)
      {
        WellData wellData;
        wellData.depth_ = wellDataExtract.depth()[j];
        wellData.value_ = wellDataExtract.calibrationTargetValues()[j];
        wellData.standardDeviation_ = wellDataExtract.calibrationTargetStdDeviation()[j];
        extractedWellData_.push_back(wellData);
      }
      iDataTargetPrevious += wellDataExtract.nDataPerTargetVar()[i];
    }
  }

  const int nWells = 2;
  const QString expectedWellNames[nWells] = {"NW01S", "well02"};
  const double expectedXCoords[nWells] = {1234567.9876, 123.566};
  const double expectedYCoords[nWells] = {987654.12345, 987.123};
  const std::size_t expectedNTargetVariables[nWells] = {2, 2};

  EXPECT_EQ(nWells, extractedWell_.size())
      << "Mismatching number of wells: " << "expected " << nWells
      << " wells, extracted data from " << extractedWell_.size() << " wells" << "\n";

  for (int i = 0; i < nWells; ++i)
  {
    EXPECT_EQ(expectedWellNames[i], extractedWell_[i].wellName_)
        << "Mismatching well names at well index " << i << "\n";

    EXPECT_EQ(expectedXCoords[i], extractedWell_[i].xCoord_)
        << "Mismatching X coordinates at well index" << i << "\n";
    EXPECT_EQ(expectedYCoords[i], extractedWell_[i].yCoord_)
        << "Mismatching Y coordinates at well index" << i << "\n";

    EXPECT_EQ(expectedNTargetVariables[i], extractedWell_[i].nTargetVariables)
        << "Mismatching number of calibration target variables at well index" << i << "\n";
  }

  const std::size_t nTargetVariables = 4;
  const QString expectedTargetVariables[nTargetVariables] = {"TwoWayTime", "BulkDensity", "Temperature", "VRe"};
  const std::size_t expectedNDataPerTargetVariable[nTargetVariables] = {3, 2, 4, 0};

  for (std::size_t iVar = 0; iVar < nTargetVariables; ++iVar)
  {
    EXPECT_EQ(expectedTargetVariables[iVar], extractedWellTargetVariables_[iVar].variable_)
        << "Mismatching calibration target variable names at index" << iVar << "\n";

    EXPECT_EQ(expectedNDataPerTargetVariable[iVar], extractedWellTargetVariables_[iVar].nData)
        << "Mismatching number of data per calibration target variable at index" << iVar << "\n";
  }

  const std::size_t nData = 9;
  const double expectedDepth[nData] = {0, 100, 200, 123, 456, 50.6, 223.4, 700.1, 1100.2};
  const double expectedTargetValues[nData] = {0.0, 1000.3234, 5000.652, 2200, 2553.1278, 0, 49, 61.8, 75.2};
  const double expectedTargetStandardDeviation[nData] = {0.0, 0.0, 0.0, 1.55, 56.12345, 1, 1, 1.55, 2};

  for (std::size_t iData = 0; iData < nData; ++iData)
  {
    EXPECT_EQ(expectedDepth[iData], extractedWellData_[iData].depth_)
        << "Mismatching calibration target depth data at index" << iData << "\n";

    EXPECT_EQ(expectedTargetValues[iData], extractedWellData_[iData].value_)
        << "Mismatching calibration target property data at index" << iData << "\n";

    EXPECT_EQ(expectedTargetStandardDeviation[iData], extractedWellData_[iData].standardDeviation_)
        << "Mismatching calibration target standard deviation data at index" << iData << "\n";
  }
}

} // namespace testCasaWizard
