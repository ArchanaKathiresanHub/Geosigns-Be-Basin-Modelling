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

class SacMapsManager : public Writable
{
public:
   SacMapsManager();
   virtual ~SacMapsManager() override = default;

   virtual void clear() override;
   virtual void readFromFile(const ScenarioReader &reader) override;
   virtual void writeToFile(ScenarioWriter &writer) const override;

   int interpolationMethod() const;
   void setInterpolationMethod(int interpolationMethod);

   int smoothingOption() const;
   void setSmoothingOption(int smoothingOption);

   int pIDW() const;
   void setPIDW(int pIDW);

   int radiusSmoothing() const;
   void setRadiusSmoothing(int radiusSmoothing);

   QVector<int> transformToActiveAndIncluded(const QVector<int>& selectedWellIndices, const QVector<int>& excludedWells);
   virtual void exportOptimizedMapsToZycor(const ProjectReader& projectReader, MapReader& mapReader, const QString& targetPath) = 0;

private:
   int m_interpolationMethod;
   int m_smoothingOption;
   int m_pIDW;
   int m_radiusSmoothing; //[m]
};

} // namespace sac

} // namespace casaWizard
