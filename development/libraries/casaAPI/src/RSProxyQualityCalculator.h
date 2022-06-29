//
// Copyright (C) Shell. All rights reserved.
//

/// @file RSProxyQualityCalculator.h
/// @brief This class calculates response surface proxy quality measures R2, R2adj, and Q2

#ifndef CASA_API_RSPROXY_QUALITY_CALCULATOR_H
#define CASA_API_RSPROXY_QUALITY_CALCULATOR_H

#include "ErrorHandler.h"

/*! @page CASA RSProxyQualityCalculatorPage Response surface proxy calculator for
    <b>RSProxyQualityCalculator</b> Calculates the proxy quality measures  R2, R2adj, and Q2
*/

namespace casa
{

class ScenarioAnalysis;
class RSProxy;

/// @brief Calculates quality measures R2, R2adj, and Q2 for Response proxy surface
class RSProxyQualityCalculator : public ErrorHandler
{
public:
  /// @brief Constructor
  RSProxyQualityCalculator(ScenarioAnalysis& scenario);

  /// @brief Destructor
  virtual ~RSProxyQualityCalculator() = default;

  /// @brief Calculate response surface proxy quality data for R2, R2adj
  /// @param proxyName                      Name of the proxy surface
  /// @param experimentList                 List of the DoE designs to be included
  /// @return                               Matrix containing R2 (first row), R2adj (second row)
  std::vector<std::vector<double>> calculateR2AndR2adj(const std::string& proxyName, const std::vector<std::string>& experimentList) const;

  /// @brief Calculate response surface proxy quality data for Q2
  /// @param proxyName                      Name of the proxy surface
  /// @param experimentList                 List of the DoE designs to be included
  /// @return                               Matrix containing Q2
  std::vector<double> calculateQ2(const std::string& proxyName, const std::vector<std::string>& experimentList) const;

  /// @brief  Calculate response surface proxy quality data (R2, R2adj) from observable
  /// @param  runCasesObservables[in]        Matrix containing target run case values for observable
  /// @param  proxyEvaluationObservables[in] Matrix containing target proxy evaluation values for observables
  /// @param  nCoefficients[in]              Number of coefficients in the proxy polynomial
  /// @return                                Matrix containing R2 (first row), R2adj (second row)
  static const std::vector<std::vector<double>> calculateR2AndR2adjFromObservables(const std::vector<std::vector<double>>& runCasesObservables,
                                                                                   const std::vector<std::vector<double>>& proxyEvaluationObservables,
                                                                                   const int nCoefficients);

private:
  RSProxyQualityCalculator(const RSProxyQualityCalculator&) = delete;
  RSProxyQualityCalculator& operator = (const RSProxyQualityCalculator&) = delete;

  /// @brief Calculate response surface proxy quality data for Q2 from observables
  /// @param runCasesObservables[in]        Matrix containing target run case values for observable
  /// @param proxyEvaluationObservables[in] Matrix containing target proxy evaluation values for observables
  /// @return                               Matrix containing Q2
  const std::vector<double> calculateQ2FromObservables(const std::vector<std::vector<double>>& runCasesObservables, const std::vector<std::vector<double>>& proxyEvaluationObservables) const;

  /// @brief Calculates response surface observables for Q2 response surface quality measure computations of existing proxy surface (leaving one out)
  /// Calculates response surface using design points of given doe list. Calculations are performed by removing one design point
  /// from the run cases (based on provided index)
  /// @param proxy[in]                  Response surface
  /// @param doeList[in]                List of included does
  /// @param iGlobalDoEPointRemove[in]  Index of the doe point to remove
  void calculateRSProxyQ2(RSProxy& proxy, const std::vector<std::string> & doeList, const int iGlobalDoEPointRemove ) const;

  ScenarioAnalysis& m_scenario;
};

} // namespace casa

#endif // CASA_API_RSPROXY_QUALITY_CALCULATOR_H

