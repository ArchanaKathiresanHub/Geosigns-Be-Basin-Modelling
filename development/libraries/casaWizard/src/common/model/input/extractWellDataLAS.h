//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "extractWellData.h"
#include "importOptions.h"

namespace casaWizard
{

class LASSectionReader;
typedef std::vector<std::string> Section;

class ExtractWellDataLAS : public ExtractWellData
{
public:
  ExtractWellDataLAS(const QString& fileName, ImportOptions& importOptions);
  ~ExtractWellDataLAS() final;
  bool hasNextWell() const final;
  void extractDataNextWell() final;
  void extractMetaDataNextWell() final;
  void resetExtractor() final;

private:
  void createSections();
  void createMetaDataSections();
  void readSections();
  bool createSectionReader(const Section& section);

  std::vector<Section> sections_;
  bool wrapping_;
  bool hasNextWell_;
  QString fileName_;
  std::unique_ptr<LASSectionReader> sectionReader_;
  ImportOptions& importOptions_;
};

} // namespace casaWizard


