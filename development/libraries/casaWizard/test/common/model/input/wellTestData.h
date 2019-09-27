#pragma once

namespace testCasaWizard
{

struct Well
{
  QString wellName_;
  double xCoord_;
  double yCoord_;
  std::size_t nTargetVariables;
};

struct WellTargetVariables
{
  QString variable_;
  std::size_t nData;
};

struct WellData
{
  double depth_;
  double value_;
  double standardDeviation_;
};

} // namespace testCasaWizard
