//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QVector>

namespace casaWizard
{

class Well;
class ManipulatedTargetCreator;

class WellDataManipulator
{
public:
   static void applyCutOff(const QMap<QString,QPair<double,double>>& propertiesWithCutOffRanges, QVector<Well>& wells);
   static void scaleData(const QStringList& selectedProperties, const double scalingFactor, QVector<Well>& wells);
   static void smoothenData(const QStringList& selectedProperties, const double radius, QVector<Well>& wells);
   static void subsampleData(const QStringList& selectedProperties, const double length, QVector<Well>& wells);

private:
   static void applyManipulatedWellData(const QStringList& selectedProperties, ManipulatedTargetCreator& manipulator, QVector<Well>& wells);
};

} // namespace casaWizard
