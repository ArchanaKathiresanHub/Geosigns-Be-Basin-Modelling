//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "extractWellDataLAS.h"
#include "model/wellData.h"
#include "lasCurveInfoSectionReader.h"
#include "lasDataSectionReader.h"
#include "lasSectionReader.h"
#include "lasVersionInfoSectionReader.h"
#include "lasWellInfoSectionReader.h"

#include <fstream>

namespace casaWizard
{

ExtractWellDataLAS::ExtractWellDataLAS(const QStringList& fileNames, ImportOptionsLAS& importOptions) :
  sections_{},
  wrapping_{false},
  fileNames_{fileNames},
  currentWell_{0},
  sectionReader_{},
  importOptions_{importOptions}
{
  importOptions_.singleFile = fileNames_.size() == 1;
}

ExtractWellDataLAS::~ExtractWellDataLAS()
{
}

bool ExtractWellDataLAS::hasNextWell() const
{
  return currentWell_ < fileNames_.size();
}

void ExtractWellDataLAS::extractDataNextWell()
{
  delete wellData_;
  wellData_ = new WellData();
  resetImportOptionsWhichDifferPerFile();

  createSections();
  readSections();

  currentWell_++;
}

void ExtractWellDataLAS::extractMetaDataNextWell()
{
  delete wellData_;
  wellData_ = new WellData();
  resetImportOptionsWhichDifferPerFile();
  importOptions_.depthUserPropertyName = "";

  createMetaDataSections();
  readSections();

  currentWell_++;
}

void ExtractWellDataLAS::resetImportOptionsWhichDifferPerFile()
{
  importOptions_.correctForElevation = false;
  importOptions_.elevationCorrection = 0.0;
  importOptions_.elevationCorrectionUnit = "";
  importOptions_.referenceCorrection = 0.0;
  importOptions_.referenceCorrectionUnit = "";
  importOptions_.depthColumn = 0;
  importOptions_.undefinedValue = -99999;
  importOptions_.wrapped = false;
}

void ExtractWellDataLAS::resetExtractor()
{
  currentWell_ = 0;
  importOptions_.wellNamesWithoutXYCoordinates.clear();
  delete wellData_;
  wellData_ = new WellData();
  sections_.clear();
}

void ExtractWellDataLAS::createSections()
{
  sections_.clear();
  std::fstream fileStream(fileNames_[currentWell_].toStdString());
  if (!fileStream.good())
  {
    throw std::runtime_error("The chosen file cannot be opened. Either the file does not exist, or the permissions are wrong") ;
  }

  std::string line;
  while (fileStream)
  {
    std::getline(fileStream, line);
    if (line.find("#") == 0 || line.empty() || line == "\r")
    {
      continue;
    }

    if (line.find("~") == 0)
    {
      sections_.push_back(Section());
    }

    if (sections_.size() > 0) // ignore lines before all sections
    {
      sections_.back().push_back(line);
    }
  }
}

void ExtractWellDataLAS::createMetaDataSections()
{
  sections_.clear();
  std::fstream fileStream(fileNames_[currentWell_].toStdString());
  if (!fileStream.good())
  {
    throw std::runtime_error("The chosen file cannot be opened. Either the file does not exist, or the permissions are wrong") ;
  }

  std::string line;
  while (fileStream)
  {
    std::getline(fileStream, line);
    if (line.find("#") == 0 || line.empty() || line == "\r")
    {
      continue;
    }

    if (line.find("~A") == 0)
    {
      break;
    }

    if (line.find("~") == 0)
    {
      sections_.push_back(Section());
    }

    if (sections_.size() > 0)
    {
      sections_.back().push_back(line);
    }
  }
}


void ExtractWellDataLAS::readSections()
{
  for (const Section& section : sections_)
  {
    if (createSectionReader(section))
    {
      sectionReader_->readSection();
    }
  }

  mapTargetVarNames();
}

bool ExtractWellDataLAS::createSectionReader(const Section& section)
{
  if (section[0].find("~W") == 0)
  {
    sectionReader_.reset(new LASWellInfoSectionReader(section, *wellData_, importOptions_));
    return true;
  }
  if (section[0].find("~A") == 0)
  {
    sectionReader_.reset(new LASDataSectionReader(section, *wellData_, importOptions_));
    return true;
  }
  if (section[0].find("~C") == 0)
  {
    sectionReader_.reset(new LASCurveInfoSectionReader(section, *wellData_, importOptions_));
    return true;
  }
  if (section[0].find("~V") == 0)
  {
    sectionReader_.reset(new LASVersionInfoSectionReader(section, *wellData_, importOptions_));
    return true;
  }

  return false;
}

} // namespace casaWizard

