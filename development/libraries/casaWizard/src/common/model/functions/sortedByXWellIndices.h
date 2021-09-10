// Function to sort the wells by x-coordinate, and return the list of sorted indices
// This function is useful, because this sorting is used for case generation
#pragma once

template<class T> class QVector;

namespace casaWizard
{

class Well;

namespace functions
{

QVector<int> sortedByXYWellIndices(const QVector<const Well *> &wells);

} // namespace functions

} // namespace casaWizard
