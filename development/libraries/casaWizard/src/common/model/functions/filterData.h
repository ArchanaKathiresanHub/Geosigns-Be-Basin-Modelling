//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Function to smoothen a set of calibration data values using a Gaussian filter
#pragma once

#include <QVector>

namespace casaWizard
{

namespace functions
{

QVector<double> smoothenData(const QVector<double>& depths,
                             const QVector<double>& values,
                             const double radius);

QVector<int> subsampleData(const QVector<double>& depths,
                           const double lenght);
}  // namespace functions

}  // namespace casaWizard
