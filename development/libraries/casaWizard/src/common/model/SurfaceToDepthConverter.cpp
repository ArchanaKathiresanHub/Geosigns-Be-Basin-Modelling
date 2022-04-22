//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SurfaceToDepthConverter.h"
#include "model/input/cmbMapReader.h"
#include "model/input/projectReader.h"
#include "model/logger.h"

#include "ConstantsNumerical.h"
#include "Interface.h"

#include <cmath>

namespace casaWizard
{

SurfaceToDepthConverter::SurfaceToDepthConverter(const ProjectReader& projectReader, const CMBMapReader& mapReader):
   m_projectReader(projectReader),
   m_cmbMapReader(mapReader)
{}

double SurfaceToDepthConverter::getDepth(double x, double y, QString surfaceName) const
{
   const QStringList surfaceNames = m_projectReader.surfaceNames();
   int idxSurface = surfaceNames.indexOf(surfaceName);

   double depth = m_projectReader.getDepth(idxSurface);
   if (isValidValue(depth)) return depth;

   QString mapName = m_projectReader.getDepthGridName(idxSurface);
   if (!mapName.isEmpty())
   {
      return getDepthFromMap(x,y,mapName);
   }

   double thickness = m_projectReader.getThickness(idxSurface);
   if (!isValidValue(thickness)) thickness = 0.0;

   //Increase index until finding a layer with depth or depth map:
   int maxIdx = surfaceNames.size();
   while (idxSurface < maxIdx && !isValidValue(depth) && mapName.isEmpty())
   {
      idxSurface++;
      depth = m_projectReader.getDepth(idxSurface);
      mapName = m_projectReader.getDepthGridName(idxSurface);

      double layerThickness = m_projectReader.getThickness(idxSurface);
      if (isValidValue(layerThickness))
      {
         thickness += layerThickness;
      }
   }

   if (isValidValue(depth))
   {
      return depth - thickness;
   }
   else if (!mapName.isEmpty())
   {
      return getDepthFromMap(x,y,mapName)-thickness;
   }

   Logger::log() << "Warning: Could not find depth value. StratIOtbl may be invalid" << Logger::endl();
   return DataAccess::Interface::DefaultUndefinedScalarValue;
}

bool SurfaceToDepthConverter::isValidValue(double val)
{
   return std::fabs(val - DataAccess::Interface::DefaultUndefinedScalarValue) > 1e-5;
}

double SurfaceToDepthConverter::getDepthFromMap(double x, double y, QString mapName) const
{
   try
   {
     return m_cmbMapReader.getValue(x, y, mapName.toStdString());
   }
   catch (const std::exception& /*error*/)
   {
     Logger::log() << "Warning: Could not find depth value for map: "
                   << mapName << "at location" << "x: " << x << "y: " << y << Logger::endl();
     return Utilities::Numerical::CauldronNoDataValue;
   }
}

} // namespace casaWizard
