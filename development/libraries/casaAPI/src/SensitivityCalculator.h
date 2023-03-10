//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SensitivityCalculator.h
/// @brief This file keeps API declaration for influential parameters sensitivity calculator

#ifndef CASA_API_SENSITIVITY_CALCULATOR_H
#define CASA_API_SENSITIVITY_CALCULATOR_H

// CMB
#include "ErrorHandler.h"

// CASA
#include "CasaSerializer.h"
#include "TornadoSensitivityInfo.h"
#include "ParetoSensitivityInfo.h"

/*! @page CASA_SensitivityCalculatorPage Sensitivity calculator for influential parameters
    <b>Sensitivity calculator</b> uses the results of DoE Cauldron runs and 
     -# Allows to perform parameters sensitivity calculation
     -# Creates Tornado plot per observable for all influential parameters
     -# Creates Pareto diagram. Using the Pareto diagram and user specified cut off value it 
        could help to define the most sensitive parameters for all observables.
   
    <b>Parameter sensitivity calculation</b> 
    For parameter sensitivity calculation casa::SensitivityCalculator uses derivative of 
    the parameter with respect to observables:

    @f{eqnarray*}{
    -\frac{\partial O}{\partial P} & \approx & \frac{O(P_{middle})-O(P_{min})}{P_{midle}-P_{min}}\\
    +\frac{\partial O}{\partial P} & \approx & \frac{O(P_{max})-O(P_{midle})}{P_{max}-P_{middle}}
    @f}

    Here @f$ O @f$ – is an observable and @f$ P @f$  is a parameter. 
    For derivatives calculation we need an observable values for @f$ P_{min}, P_{middle}, P_{max} @f$
    parameters values when other parameters are fixed for their middle values of range.
    For observables value calculation SensitivityCalculator uses the 1st order response surface 
    with global kriging is created “under the hood”. If this response surface is created using Tornado DoE,
    the sensitivities are "exact" because response surface evaluation points are in the same places as 
    experiments made by DoE (Tornado DoE makes 3 runs for each parameter: min, middle, and max values 
    while the other parameters are fixed for the their middle values).
   
    <b> Tornado diagram </b> also called tornado plot or tornado chart, is a special type of 
    Bar chart, where the data categories are listed vertically instead of the standard horizontal 
    presentation, and the categories are ordered so that the largest bar appears at the top of the
    chart, the second largest appears second from the top, and so on. They are so named because the
    final chart visually resembles either one half of or a complete tornado (Wikipedia)
   
    Figure 
    @image html TornadoDiagram.jpg "Tornado plot example"
    
    <b>Pareto diagram</b> or <b>Pareto chart</b>, named after Vilfredo Pareto, is a type of chart 
    that contains both bars and a line graph, where individual values are represented in descending 
    order by bars, and the cumulative total is represented by the line (Wikipedia)
   
    Figure
    @image html ParetoDiagram.png "Pareto chart example"
   
    A Pareto chart, here accumulates each parameter sensitivity over all observables and shows sorted 
    normalized values. 

    To calculate paremeters sensitivit, algorithm varying one influential parameter at a time over 100 points inside IP interval, 
    and evalutates obsrevable values with provided by the user response srface proxy. 
    Pareto sensitivities are calculated according this formula:

    @f[ S_{p}=\sum\limits_{o=1}^{o=N}\frac{V_{p,max}^{o}-V_{p,min}^{0}}{V_{max}^{o}-V_{min}^{0}}  @f]

    Here:

    @f$ S_p @f$ is sensitivity for parameter @f$ p @f$, 

    @f$ V_{p,min}^{o} @f$ minimal observable value for the @f$ p-th @f$ parameter interval, calculated from proxy evaluation.

    @f$ V_{p,max}^{o} @f$ maximal observable value for the @f$ p-th @f$ parameter interval, calculated from proxy evaluation.

    @f$ V_{min}^{o} @f$ minimal observable value for most sensitive for this observable parameter.

    @f$ V_{max}^{o} @f$ maximal observable value for most sensitive for this observable parameter.
   
    We can read this diagram like: 
    More than 80% of observables variation gives the HeatProductionRate parameter and less than 20% - TOC.
    This diagram allows to easily cut of parameters which are having the minimal influence on observables.
*/   


namespace casa
{
   class RSProxy;
   class RunCaseSet;

   /// @brief Allows to find all influential parameters sensitivity with respect to each observable and
   /// build Tornado and Pareto diagrams
   class SensitivityCalculator : public ErrorHandler, public CasaSerializable
   {
   public:     
      /// @brief Destructor
      virtual ~SensitivityCalculator() {;}

      /// @brief Calculate data for Pareto diagram construction
      /// @param proxy [in]     pointer to the proxy object which is used to calculate parameters sensitivities
      /// @param sensInfo [out] data set which contains all parameters cumulative sensitivities which can be used to create Pareto diagram
      /// @return ErrorHandler::NoError in case of success, or error code otherwise
      virtual ErrorHandler::ReturnCode calculatePareto( RSProxy * proxy, ParetoSensitivityInfo  & sensInfo ) = 0;

      /// @brief Construct 1st order proxy for given set of cases and calculate Tornado influential parameters sensitivities
      /// @param cs [in] case set manager which keeps run cases for DoE experiments
      /// @param expNames [in] list of DoE names which will be used to create proxy for parameters sensitivity calculation
      /// @return array which contains for each observable, a set of influential parameters sensitivities which could be used
      ///         for creation Tornado diagram. In case of error method will return empty array and error code and error message
      ///         could be obtained from SensitivitCalculator object
      virtual std::vector<TornadoSensitivityInfo> calculateTornado( RunCaseSet & cs, const std::vector<std::string> & expNames ) = 0;

   protected:
      SensitivityCalculator() {;}

   private:
      SensitivityCalculator( const SensitivityCalculator & sc );
      SensitivityCalculator & operator = ( const SensitivityCalculator & sc );
   };
}

#endif // CASA_API_SENSITIVITY_CALCULATOR_H

