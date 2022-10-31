//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/SacMapsManager.h"

#include <QVector>

class QString;

namespace casaWizard
{

class ProjectReader;
class MapReader;

namespace sac
{
class LithologyMapsManager : public SacMapsManager
{
public:
   LithologyMapsManager();

   void clear() override;
   void readFromFile(const ScenarioReader &reader) override;
   void writeToFile(ScenarioWriter &writer) const override;

   bool smartGridding() const;
   void setSmartGridding(bool smartGridding);

   void exportOptimizedMapsToZycor(const ProjectReader& projectReader, MapReader& mapReader, const QString& targetPath) final;

private:
   bool m_smartGridding;
};

} // namespace sac

} // namespace casaWizard

