//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "ToDepthConverter.h"

namespace casaWizard
{

class CMBMapReader;
class ProjectReader;

class SurfaceToDepthConverter : public ToDepthConverter
{
public:
   SurfaceToDepthConverter(const ProjectReader& projectReader, const CMBMapReader& mapReader);
   double getDepth(double x, double y, QString surfaceName) const override;

private:
   double getDepthFromMap(double x, double y, QString mapName) const;
   static bool isValidValue(double val);

   const ProjectReader& m_projectReader;
   const CMBMapReader& m_cmbMapReader;
};

} // namespace casaWizard


