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

#include <QVector>

namespace casaWizard
{

typedef QVector<std::string> MetaDataSection;

class ExtractWellDataVSET : public ExtractWellData
{
  struct VSETWell
  {
    double x;
    double y;
    QVector<double> depths;
    QVector<double> values;
    VSETWell(double x_=0.0, double y_=0.0, double depth_=0.0, double value_=0.0)
    {
      x=x_;
      y=y_;
      depths = {depth_};
      values = {value_};
    }
  };

public:
  ExtractWellDataVSET(const QString& fileName);
  ~ExtractWellDataVSET() final;
  bool hasNextWell() const final;
  void extractDataNextWell() final;
  void extractMetaDataNextWell() final;
  void resetExtractor() final;

  void setImportOptions(const ImportOptionsVSET& importOptions);
  ImportOptionsVSET getImportOptions() const;

private:
  void setFilterOption();

  void createMetaDataSection();
  void handleMetaDataSection();

  void readWellData();
  void handleDataLine(std::stringstream& line);
  void handleData(double x, double y, double depth, double velocityValue);

  void handleWellsAtInterval(double x, double y, double depth, double velocityValue);
  void handleWellsAtDistance(double x, double y, double depth, double velocityValue);
  void handleWellsAtXY(double x, double y, double depth, double velocityValue);

  bool handleWellAllowedInterval(double x, double y);
  bool handleWellAllowedDistance(double x, double y);  
  QVector<double> getDepthVector(const QVector<double>& depth, const QVector<double>& velocities);
  void removeInvalidWells();

  MetaDataSection metaDataSection_;
  int currentWell_;
  int currentWellIndex_;
  bool firstWellXY_;

  double currentX_;
  double currentY_;

  double skipX_;
  double skipY_;
  int skipped_;

  bool skipCurrent_;

  QVector<VSETWell> wells_;

  QString fileName_;  
  ImportOptionsVSET importOptions_;
  int filterOption_;
};

} // namespace casaWizard


