//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/writable.h"

#include <QVector>

class QString;

namespace casaWizard
{

class ProjectReader;
class MapReader;

namespace sac
{

class MapsManager : public Writable
{
public:
   MapsManager();

   void clear() override;
   void readFromFile(const ScenarioReader &reader) override;
   void writeToFile(ScenarioWriter &writer) const override;

   int interpolationMethod() const;
   void setInterpolationMethod(int interpolationMethod);

   int smoothingOption() const;
   void setSmoothingOption(int smoothingOption);

   int pIDW() const;
   void setPIDW(int pIDW);

   int radiusSmoothing() const;
   void setRadiusSmoothing(int radiusSmoothing);

   bool smartGridding() const;
   void setSmartGridding(bool smartGridding);

   void exportOptimizedLithofractionMapsToZycor(const ProjectReader& projectReader, MapReader& mapReader, const QString& targetPath);

   QVector<int> transformToActiveAndIncluded(const QVector<int>& selectedWellIndices, const QVector<int>& excludedWells);

private:
   int m_interpolationMethod;
   int m_smoothingOption;
   int m_pIDW;
   int m_radiusSmoothing; //[m]
   bool m_smartGridding;
};

} // namespace sac

} // namespace casaWizard
