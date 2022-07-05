//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "extractWellDataVSET.h"
#include "model/logger.h"
#include "model/wellData.h"

#include "ConstantsMathematics.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

namespace casaWizard
{

ExtractWellDataVSET::ExtractWellDataVSET(const QString& fileName) :
  ExtractWellData(),
  metaDataSection_(),
  currentWell_(0),
  currentWellIndex_(0),
  firstWellXY_(true),
  currentX_(0.0),
  currentY_(0.0),
  skipX_(0.0),
  skipY_(0.0),
  skipped_(0),
  skipCurrent_(false),
  wells_(),
  fileName_(fileName),
  importOptions_(),
  filterOption_(0)
{
  wellData_->units_ = {"m/s"};
  wellData_->calibrationTargetVarsUserName_ = {"Interval Velocity"};
  wellData_->calibrationTargetVarsCauldronName_ = {"Velocity"};
  wellData_->nCalibrationTargetVars_ = 1;
}

ExtractWellDataVSET::~ExtractWellDataVSET()
{
}

bool ExtractWellDataVSET::hasNextWell() const
{
  return currentWell_ == 0 || currentWell_ < wells_.size();
}

void ExtractWellDataVSET::extractDataNextWell()
{
  if (currentWell_ == 0) readWellData();

  if (!wells_.empty())
  {
    const VSETWell& well = wells_[currentWell_];
    wellData_->wellName_ = "PSW_" + importOptions_.wellIdentifierName +"_" + QString::number(currentWell_+1);
    wellData_->xCoord_ = well.x;
    wellData_->yCoord_ = well.y;
    wellData_->depth_ = getDepthVector(well.depths, well.values);
    wellData_->calibrationTargetValues_ = well.values;
    wellData_->nDataPerTargetVar_ = {static_cast<unsigned int>(well.values.size())};
    wellData_->calibrationTargetStdDeviation_ = QVector<double>(well.values.size(), 0.0);
  }

  currentWell_++;
}

void ExtractWellDataVSET::extractMetaDataNextWell()
{  
  if (currentWell_ == 0)
  {
    createMetaDataSection();
    handleMetaDataSection();
  }

  currentWell_++;
}

void ExtractWellDataVSET::resetExtractor()
{
  metaDataSection_.clear();
  wells_.clear();
  currentWell_ = 0;
  firstWellXY_ = true;
}

void ExtractWellDataVSET::setImportOptions(const ImportOptionsVSET& importOptions)
{
  importOptions_ = importOptions;
  setFilterOption();
}

ImportOptionsVSET ExtractWellDataVSET::getImportOptions() const
{
  return importOptions_;
}

void ExtractWellDataVSET::setFilterOption()
{
  filterOption_ = 0;
  if (importOptions_.interval>1)
  {
    filterOption_ = 1;
  }
  else if (importOptions_.distance > 0.0)
  {
    filterOption_ = 2;
  }
  else if (!importOptions_.xyPairs.empty())
  {
    filterOption_ = 3;
  }
}

void ExtractWellDataVSET::handleWellsAtInterval(double x, double y, double depth, double velocityValue)
{
  if (currentX_ == x && currentY_ == y)
  {
    if (skipCurrent_) return;
    wells_.back().depths.push_back(depth);
    wells_.back().values.push_back(velocityValue);
    return;
  }
  else if (handleWellAllowedInterval(x, y))
  {
    wells_.push_back(VSETWell(x, y, depth, velocityValue));
    skipCurrent_ = false;
  }
  else
  {
    skipCurrent_ = true;
  }

  currentX_ = x;
  currentY_ = y;
}

void ExtractWellDataVSET::handleWellsAtDistance(double x, double y, double depth, double velocityValue)
{
  if (currentX_ == x && currentY_ == y)
  {
    if (skipCurrent_) return;
    wells_.back().depths.push_back(depth);
    wells_.back().values.push_back(velocityValue);
    return;
  }
  else if (handleWellAllowedDistance(x, y))
  {
    wells_.push_back(VSETWell(x, y, depth, velocityValue));
    skipCurrent_ = false;
  }
  else
  {
    skipCurrent_ = true;
  }

  currentX_ = x;
  currentY_ = y;
}

void ExtractWellDataVSET::handleWellsAtXY(double x, double y, double depth, double velocityValue)
{
  if (firstWellXY_)
  {
    wells_.clear();

    for (int i = 0; i< importOptions_.xyPairs.size(); ++i)
    {
      wells_.push_back(VSETWell(1.e10, 1.e10, 0.0, 0.0));
    }
    firstWellXY_ = false;
  }

  if (currentX_ == x && currentY_ == y)
  {
    if (skipCurrent_) return;
    wells_[currentWellIndex_].depths.push_back(depth);
    wells_[currentWellIndex_].values.push_back(velocityValue);
    return;
  }
  else
  {
    int iClosest = -1;
    double minVal = 1.e10;

    int i = 0;
    for (const QPair<double,double>& xyPair : importOptions_.xyPairs)
    {
      const double dx = xyPair.first - x;
      const double dy = xyPair.second - y;
      const double distanceCurrent = dx*dx + dy*dy;

      const double dx1 = xyPair.first - wells_[i].x;
      const double dy1 = xyPair.second - wells_[i].y;
      const double distanceSaved = dx1*dx1 + dy1*dy1;
      if (distanceCurrent <= minVal && distanceCurrent < distanceSaved)
      {
        iClosest = i;
        minVal = dx*dx + dy*dy;
      }
      ++i;
    }
    if (iClosest > -1)
    {
      wells_[iClosest] = VSETWell(x, y, depth, velocityValue);
      currentWellIndex_ = iClosest;
      skipCurrent_ = false;
    }
    else
    {
      skipCurrent_ = true;
    }
  }

  currentX_ = x;
  currentY_ = y;
}

void ExtractWellDataVSET::createMetaDataSection()
{
  std::fstream fileStream(fileName_.toStdString());
  if (!fileStream.good())
  {
    throw std::runtime_error("The chosen file cannot be opened. Either the file does not exist, or the permissions are wrong") ;
  }

  std::string line;
  std::string firstWord;
  std::stringstream ss;
  while (fileStream)
  {
    std::getline(fileStream, line);
    if (line.empty())
    {
      continue;
    }

    ss.clear();
    ss << line;
    ss >> firstWord;

    if (firstWord[0] == '#')
    {
      continue;
    }

    if (firstWord == "PVRTX")
    {      
      break;
    }
    else
    {
      metaDataSection_.push_back(line);
    }

    if (firstWord == "END")
    {
      break;
    }
  }  
}

void ExtractWellDataVSET::handleMetaDataSection()
{
  bool lookForZunit =false;
  bool foundZunit = false;
  std::stringstream ss;
  std::string word;
  for (const std::string& line : metaDataSection_)
  {
    ss.clear();
    ss << line;
    std::stringstream ss(line);
    std::string word;
    ss >> word;
    if (word == "NO_DATA_VALUES")
    {
      ss >> importOptions_.undefinedValue;
    }
    else if (word == "PROPERTY_CLASS_HEADER")
    {
      ss >> word;
      if (word == "Z")
      {
        lookForZunit = true;
      }
    }
    if (lookForZunit)
    {      
      if (word == "unit:")
      {
        ss >> word;
        importOptions_.depthNotTWT = (word == "m");
        lookForZunit = false;
        foundZunit = true;
      }
    }
  }

  if (!foundZunit)
  {
    Logger::log() << "No unit found for the 'z' property in the header of this file. Assuming it is Depth[m], which can be corrected to TwoWayTime[ms] in the dialog." << Logger::endl();
  }
}

void ExtractWellDataVSET::removeInvalidWells()
{
  QVector<VSETWell>::iterator it = wells_.begin();
  while (it != wells_.end())
  {
    if (it->x == 1.e10)
    {
      it = wells_.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

void ExtractWellDataVSET::readWellData()
{
  std::fstream fileStream(fileName_.toStdString());
  if (!fileStream.good())
  {
    throw std::runtime_error("The chosen file cannot be opened. Either the file does not exist, or the permissions are wrong") ;
  }

  std::string line;
  std::string firstWord;
  std::stringstream ss;
  while (fileStream)
  {
    std::getline(fileStream, line);
    if (line.empty())
    {
      continue;
    }

    ss.clear();
    ss << line;
    ss >> firstWord;

    if (firstWord[0] == '#')
    {
      continue;
    }

    if (firstWord == "PVRTX")
    {
      handleDataLine(ss);
    }
    else
    {
      continue;
    }

    if (firstWord == "END")
    {
      break;
    }
  }

  removeInvalidWells();
}

void ExtractWellDataVSET::handleDataLine(std::stringstream& ss)
{
  unsigned long index;  ss >> index;
  double x;             ss >> x;
  double y;             ss >> y;
  double depth;         ss >> depth;
  double velocityValue; ss >> velocityValue;

  if (velocityValue < 1.0 || velocityValue == importOptions_.undefinedValue)
  {
    return;
  }

  handleData(x, y, depth, velocityValue);
}

void ExtractWellDataVSET::handleData(double x, double y, double depth, double velocityValue)
{
  switch (filterOption_)
  {
    case 0:
    case 1:
      handleWellsAtInterval(x, y, depth, velocityValue);
      break;
    case 2:
      handleWellsAtDistance(x, y, depth, velocityValue);
      break;
    case 3:
      handleWellsAtXY(x, y, depth, velocityValue);
      break;
  }
}

bool ExtractWellDataVSET::handleWellAllowedInterval(double x, double y)
{
  if (importOptions_.interval == skipped_ + 1)
  {
    skipped_=0;
    skipX_ = x;
    skipY_ = y;
    return true;
  }
  else
  {
    if (x != skipX_ || y != skipY_)
    {
      skipped_++;
      skipX_ = x;
      skipY_ = y;
    }
    return false;
  }
}

bool ExtractWellDataVSET::handleWellAllowedDistance(double x, double y)
{
  for (const VSETWell& well : wells_)
  {
    const double dx = x - well.x;
    const double dy = y - well.y;
    if (std::sqrt(dx*dx + dy*dy) < importOptions_.distance)
    {
      return false;
    }
  }
  return true;
}

QVector<double> ExtractWellDataVSET::getDepthVector(const QVector<double>& depth, const QVector<double>& velocities)
{
  if (importOptions_.depthNotTWT)
  {
    return depth;
  }
  else // depth values are in fact two way travel times in [ms]
  {
    if (depth.empty()) return {};
    QVector<double> depthFromTWT;
    depthFromTWT.push_back(depth[0]*velocities[0]*0.5*Utilities::Maths::MilliSecondToSecond);
    double prevTWT = depth[0];
    for (int i = 1; i<depth.size(); ++i)
    {
      depthFromTWT.push_back(depthFromTWT.last() + ((depth[i] - prevTWT)*(velocities[i] + velocities[i-1])*0.5)*0.5*Utilities::Maths::MilliSecondToSecond);
      prevTWT = depth[i];
    }
    return depthFromTWT;
  }
}

} // namespace casaWizard
