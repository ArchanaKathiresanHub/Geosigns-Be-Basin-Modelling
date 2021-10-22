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
#include "lasWellInfoSectionReader.h"

#include <fstream>

namespace casaWizard
{

ExtractWellDataLAS::ExtractWellDataLAS(const QString& fileName, ImportOptions& importOptions) :
  sections_{},
  wrapping_{false},
  hasNextWell_{true},
  fileName_{fileName},
  sectionReader_{},
  importOptions_{importOptions}
{
}

ExtractWellDataLAS::~ExtractWellDataLAS()
{
}

bool ExtractWellDataLAS::hasNextWell() const
{
  return hasNextWell_;
}

void ExtractWellDataLAS::extractDataNextWell()
{
  createSections();
  readSections();

  hasNextWell_ = false;
}

void ExtractWellDataLAS::extractMetaDataNextWell()
{
  createMetaDataSections();
  readSections();

  hasNextWell_ = false;
}

void ExtractWellDataLAS::resetExtractor()
{
  hasNextWell_ = true;
  delete wellData_;
  wellData_ = new WellData();
  sections_.clear();
}

void ExtractWellDataLAS::createSections()
{
  std::fstream fileStream(fileName_.toStdString());
  if (!fileStream.good())
  {
    throw std::runtime_error("The chosen file cannot be opened. Either the file does not exist, or the permissions are wrong") ;
  }

  std::string line;
  while (fileStream)
  {
    std::getline(fileStream, line);
    if (line.find("#") == 0)
    {
      continue;
    }

    if (line.find("~") == 0)
    {
      sections_.push_back(Section());
    }
    sections_.back().push_back(line);
  }
}

void ExtractWellDataLAS::createMetaDataSections()
{
  std::fstream fileStream(fileName_.toStdString());
  if (!fileStream.good())
  {
    throw std::runtime_error("The chosen file cannot be opened. Either the file does not exist, or the permissions are wrong") ;
  }

  std::string line;
  while (fileStream)
  {
    std::getline(fileStream, line);
    if (line.find("#") == 0)
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
    sections_.back().push_back(line);
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

  return false;
}

} // namespace casaWizard

