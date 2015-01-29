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
/// @brief This file keeps API declaration for variable parameters sensitivity calculator

#ifndef CASA_API_SENSITIVITY_CALCULATOR_H
#define CASA_API_SENSITIVITY_CALCULATOR_H

// CMB
#include "ErrorHandler.h"

// CASA
#include "CasaSerializer.h"

/// @page CASA_SensitivityCalculatorPage Sensitivity calculator for variable parameters
/// <b>Sensitivity calculator</b> uses the results of DoE Cauldron runs and 
///  -# Allows to perform parameters sensitivity calculation
///  -# Creates Tornado plot per observable for all variable parameters
///  -# Creates Pareto diagram. Using the Pareto diagram and user specified cut off value it 
///     could help to define the most sensitive parameters for all observables.
///
/// <b>Parameter sensitivity calculation</b> 
/// For parameter sensitivity calculation casa::SensitivityCalculator uses derivative of 
/// the parameter with respect to observables:
/// @f[
/// \begin{array}{ccc}
/// -\frac{\partial O}{\partial P} & \approx & \frac{O(P_{middle})-O(P_{min})}{P_{midle}-P_{min}}\\
/// +\frac{\partial O}{\partial P} & \approx & \frac{O(P_{max})-O(P_{midle})}{P_{max}-P_{middle}}
/// \end{array}
/// @f]
/// Here @f$ O @f$ – is an observable and @f$ P @f$  is a parameter. 
/// For derivatives calculation we need an observable values for @f$ P_{min}, P_{middle}, P_{max} @f$
/// parameters values when other parameters are fixed for their middle values of range.
/// For observables value calculation SensitivityCalculator uses the 1st order response surface 
/// with global kriging is created “under the hood”. If this response surface is created using Tornado DoE,
/// the sensitivities are "exact" because response surface evaluation points are in the same places as 
/// experiments made by DoE (Tornado DoE makes 3 runs for each parameter: min, middle, and max values 
/// while the other parameters are fixed for the their middle values).
///
/// <b> Tornado diagram </b> also called tornado plot or tornado chart, is a special type of 
/// Bar chart, where the data categories are listed vertically instead of the standard horizontal 
/// presentation, and the categories are ordered so that the largest bar appears at the top of the
/// chart, the second largest appears second from the top, and so on. They are so named because the
/// final chart visually resembles either one half of or a complete tornado (Wikipedia)
///
/// Figure 
/// @image html TornadoDiagram.jpg "Tornado plot example"
/// 
/// <b>Pareto diagram</b> or <b>Pareto chart</b, named after Vilfredo Pareto, is a type of chart 
/// that contains both bars and a line graph, where individual values are represented in descending 
/// order by bars, and the cumulative total is represented by the line (Wikipedia)
///
/// Figure
/// @image html ParetoDiagram.png "Pareto chart example"
///
/// A Pareto chart, here accumulates each parameter sensitivity over all observables and shows sorted 
/// normalized values. We can read this diagram like: 
/// More than 80% of observables variation gives the HeatProductionRate parameter and less than 20% - TOC.
/// This diagram allows to easily cut of parameters which are having the minimal influence on observables.
///


namespace casa
{
   class VarSpace;
   class ObsSpace;
   class RSProxySet;
   class RSProxy;
   class RunCaseSet;
   
   /// @brief Data structure for keeping Pareto sensitivity calculation results
   struct ParetoSensitivityInfo
   {
      /// @brief Get a list of VarParameters that together have a cumulative sensitivity of the specified value, or more.
      /// @param fraction cumulative sensitivity: fractional number in range [0.0:1.0]
      /// @returns        a vector parameters numbers as they are numbered in VarSpace
      const std::vector< std::pair<const VarParameter *, int > > getVarParametersWithCumulativeImpact( double fraction ) const;

      /// @brief Get the sensitivity of specified VarParameter
      /// @param varPrm variable parameter object pointer
      /// @param subPrmID  subparameter ID
      /// @returns      the sensitivity value
      double getSensitivity( const VarParameter * varPrm, int subPrmNum ) const;

      /// @brief Get the cumulative sensitivity of specified VarParameter
      /// @param varPrm variable parameter  object pointer
      /// @param subPrmID  sub-parameter ID
      /// @returns the cumulative sensitivity value
      double getCumulativeSensitivity( const VarParameter * varPrm, int subPrmID ) const;

      /// @brief Add new parameter sensitivity to the list
      /// @param varPrm parameter number in VarSpace
      /// @param subPrmID subparameter ID
      /// @param val variable parameter sensitivity
      void add( const VarParameter * varPrm, int subPrmID, double val );

      std::vector< const VarParameter * > m_vprmPtr;    ///< Variable parameter pointer
      std::vector< int >                  m_vprmSubID;  ///< Variable parameter sub-parameter ID
      std::vector< double >               m_vprmSens;   ///< Variable parameter sub-parameter sensitivity
   };

   /// @brief Data structure for keeping Tornado sensitivity calculation results
   class TornadoSensitivityInfo
   {
   public:
      typedef std::vector< std::vector< double > > SensitivityData;

      /// @brief Default constructor
      TornadoSensitivityInfo( const  Observable * obs
                            , int    obsSubID
                            , double obsRefVal
                            , const  std::vector< std::pair<const VarParameter *, int> > & varPrms
                            , const  SensitivityData & sensData
                            , const  SensitivityData & relSensData
                            ); 

      /// @brief Copy constructor
      TornadoSensitivityInfo( const TornadoSensitivityInfo & tsi );

      const Observable                  * observable()      const { return m_obs; }
      int                                 observableSubID() const { return m_obsSubID; }
      double                              refObsValue()     const { return m_refObsValue; }
      const SensitivityData             & sensitivities()   const { return m_sensitivities; }
      const SensitivityData             & relSensitivities() const { return m_relSensitivities; }

      const std::vector<std::pair<const VarParameter *, int> >  & varPrmList() const { return m_vprmPtr; }

      // interfaces for C#
      /// @{

      /// @brief Get minimal absolute value of tornado sensitivity for the given parameter number
      /// @param prmNum parameter number
      /// @return minimal absolute value of tornado sensitivity
      double minAbsSensitivityValue( size_t prmNum ) const;

      /// @brief Get maximal absolute value of tornado sensitivity for the given parameter number
      /// @param prmNum parameter number
      /// @return maximal absolute value of tornado sensitivity
      double maxAbsSensitivityValue( size_t prmNum ) const;

      /// @brief Get minimal relative value of tornado sensitivity for the given parameter number
      /// @param prmNum parameter number
      /// @return minimal absolute value of tornado sensitivity
      double minRelSensitivityValue( size_t prmNum ) const;

      /// @brief Get maximal relative value of tornado sensitivity for the given parameter number
      /// @param prmNum parameter number
      /// @return maximal absolute value of tornado sensitivity
      double maxRelSensitivityValue( size_t prmNum ) const;
      
      const VarParameter     * varParameter(      size_t vPrmNum ) { return m_vprmPtr[vPrmNum].first; }
      int                      varParameterSubID( size_t vPrmNum ) { return m_vprmPtr[vPrmNum].second; }
      std::vector<std::string> varParametersNameList();

      ///@}

   private:
      const Observable                                  * m_obs;              ///< corresponded observable for which this sensitivities were calculated
      int                                                 m_obsSubID;         ///< observable could has dimension more than one, in this case here it is ID of sub-observable
      double                                              m_refObsValue;      ///< reference observable value
      std::vector< std::pair<const VarParameter *, int> > m_vprmPtr;          ///< Variable parameters set (size N) in the same order as sensitivities
      SensitivityData                                     m_sensitivities;    ///< Array Nx2 which keep for each var parameter min/max values for observable
      SensitivityData                                     m_relSensitivities; ///< Array Nx2 with relative sensitivities
   };


   /// @brief Allows to find all variable parameters sensitivity with respect to each observable and
   /// build Tornado and Pareto diagrams
   class SensitivityCalculator : public ErrorHandler, public CasaSerializable
   {
   public:     
      /// @brief Destructor
      virtual ~SensitivityCalculator() {;}

      /// @brief Calculate data for Pareto diagram construction
      /// @param proxy[in]     pointer to the proxy object which is used to calculate parameters sensitivities
      /// @param sensInfo[out] data set which contains all parameters cumulative sensitivities which can be used to create Pareto diagram
      /// @return ErrorHandler::NoError in case of success, or error code otherwise
      virtual ErrorHandler::ReturnCode calculatePareto(const RSProxy * proxy, ParetoSensitivityInfo  & sensInfo) = 0;

      /// @brief Construct 1st order proxy for given set of cases and calculate Tornado variable parameters sensitivities
      /// @param cs[in] case set manager which keeps run cases for DoE experiments
      /// @param expName[in] list of DoE names which will be used to create proxy for parameters sensitivity calculation
      /// @return array which contains for each observable, a set of variable parameters sensitivities which could be used
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

