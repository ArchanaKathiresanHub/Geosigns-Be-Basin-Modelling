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

namespace thermal
{

class ThermalMapManager: public SacMapsManager
{
public:
   void exportOptimizedMapsToZycor(const ProjectReader& projectReader, MapReader& mapReader, const QString& targetPath) final;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard

