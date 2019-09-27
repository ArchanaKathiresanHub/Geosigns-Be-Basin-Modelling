// Helper methods for the generation of a histogram
#pragma once

#include <QVector>

namespace casaWizard
{

namespace histogramGenerator
{

QVector<int> generateBins(const QVector<double>& valuesSorted, int bins); // valuesSorted sorted from smallest to biggest value

} // namespace histogramGenerator

} // namespace casaWizard
