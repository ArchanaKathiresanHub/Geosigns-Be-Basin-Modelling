// Function to interpolate a set of (x,y) to values x_in
// This method assumes that the values in x are sorted
#pragma once

#include <QVector>

namespace casaWizard
{

namespace functions
{

QVector<double> interpolateVector(const QVector<double> x,
                                  const QVector<double> y,
                                  const QVector<double> xIn);

}  // namespace functions

}  // namespace casaWizard
