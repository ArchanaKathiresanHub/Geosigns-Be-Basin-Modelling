//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "LithologyInfoGenerator.h"

#include <QString>

namespace casaWizard
{

namespace sac
{

namespace lithology
{

class LithologyScenario;

class T2ZInfoGenerator : public LithologyInfoGenerator
{
public:
   T2ZInfoGenerator(const LithologyScenario& scenario, ProjectReader& t2zProjectReader);
   void generateInfoTextFile() override;

private:
   void addT2ZSection();
   void addT2ZRunSettings();
   void addSurfaceInfo();

   void getOptionValue(const QString& surface, const int id);
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
