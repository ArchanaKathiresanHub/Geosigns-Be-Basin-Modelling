// Helper methods for the generation of a quartiles
#pragma once

#include <QVector>

namespace casaWizard
{

namespace percentileGenerator
{

QVector<double> getPercentileValues(const QVector<double>& values, const QVector<double>& pTargets); // pTargets between 0 and 1

} // namespace percentileGenerator

} // namespace casaWizard
