//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file TornadoSensitivityInfo.h
/// @brief This file keeps class declaration for keeping the Tornado diagram data

#ifndef CASA_TORNADO_SENSITIVITY_INFO_H
#define CASA_TORNADO_SENSITIVITY_INFO_H

// CMB API
#include "UndefinedValues.h"

// STL
#include <vector>
#include <utility>

namespace casa
{
   class VarParameter;
   class Observable;
   
   /// @brief Data structure for keeping Tornado sensitivity calculation results
   class TornadoSensitivityInfo
   {
   public:
      typedef std::vector< std::vector< double > > SensitivityData;

      /// @brief Constructor from raw data
      TornadoSensitivityInfo( const  Observable * obs
                            , int    obsSubID
                            , double obsRefVal
                            , const  std::vector< std::pair<const VarParameter *, int> > & varPrms
                            , const  SensitivityData & sensData
                            , const  SensitivityData & relSensData
                            ); 

      /// @brief Create empty tornado sensitivity object for the given observable and sensitivity data will be added later
      ///        using TornadoSensitivityInfo::addSensitivity() method
      /// @param obs observable
      /// @param obsSubID observable subindex if observable has dimension more than 1
      /// @param obsRefVal observable value for the base case
      TornadoSensitivityInfo( const Observable * obs, int obsSubID, double obsRefVal );

      /// @brief Append sensitivity values for the given parameter. 
      ///        Relative sensitivities must be calculated after all sensitivities were added by calling calcRelSensitivities
      /// @param minV observable value for the parameter minimal range value
      /// @param maxV observable value for the parameter maximal range value
      /// @param prm variable parameter
      /// @param prmSubID parameter subindex if parameter dimension more than 1
      void addSensitivity( double minV, double maxV, const VarParameter * prm, size_t prmSubID );
      
      /// @brief Add found by proxy evalutaion min max observable value in [min:max] variable parameter range interval
      ///        Must be called just after TornadoSensitivityInfo::addSensitivity() call.
      /// @param minVal observable minimal value over [min:max] parameter range
      /// @param maxVal observable maximal value over [min:max] parameter range
      /// @param minPrmVal parameter value correcponded to minmal observable value
      /// @param maxPrmVal parameter value correcponded to maximal observable value
      void addMinMaxSensitivityInRange( double minVal, double maxVal, double minPrmVal, double maxPrmVal );

      /// @brief This function is searching for valid observable values intervals in the given 1D function ( observableVal( prmVal ) )
      ///        and stores found intervals for the last added parameter. It must be called after TornadoSensitivityInfo::addSensitivity call.
      /// @param allPrmVals 1D array of parameter values in [min:max] variable parameter range interval
      /// @param allObsVals 1D array of observable values which are calculated by tornado proxy evalutation for each value of allPrmVals array
      void calculateAndAddValidRanges( const std::vector<double> & allPrmVals, const std::vector<double> & allObsVals );

      /// @brief After all parameters sensitivities were added, this function will calculate relative sensitivies
      void calculateRelativeSensitivities();

      /// @brief Copy constructor
      TornadoSensitivityInfo( const TornadoSensitivityInfo & tsi );
      
      /// @brief Copy operator
      TornadoSensitivityInfo & operator = ( const TornadoSensitivityInfo & tsi );

      const Observable                  * observable()                 const { return m_obs; }
      int                                 observableSubID()            const { return m_obsSubID; }
      double                              refObsValue()                const { return m_refObsValue; }
      const SensitivityData             & sensitivities()              const { return m_sensitivities; }
      const SensitivityData             & relSensitivities()           const { return m_relSensitivities; }
      const SensitivityData             & maxSensitivities()           const { return m_maxSensitivities; }
      const SensitivityData             & maxRelSensitivities()        const { return m_maxRelSensitivities; }
      const SensitivityData             & prmValsForMaxSensitivities() const { return m_prmValForMaxSensitivities; }
      const SensitivityData             & validRangesObsVals()         const { return m_validSensitivitiesObsVals; }
      const SensitivityData             & validRangesPrmVals()         const { return m_validSensitivitiesPrmVals; }

      /// @brief Get value which is used for indicating the absense of observable value
      /// @return undefined value
      double                              undefinedValue()   const { return UndefinedDoubleValue; }

      /// @brief Get minimum of variable parameter range value. If parameter is Categorical - it will be integer->double conversion
      /// @param vPrmNum parameter number
      /// @return minimum of variable parameter value range
      double minVarParameterRangeValue( size_t vPrmNum ) const;

      /// @brief Get maximum of variable parameter range value. If parameter is Categorical - it will be integer->double conversion
      /// @param vPrmNum parameter number
      /// @return maximum of variable parameter value range
      double maxVarParameterRangeValue( size_t vPrmNum ) const;


      /// @brief If observable takes undefined value for some parameter variation, for such parameter SensitivityCalculator
      ///        calculating ranges where observable value is valid. This function returns array Kx2 which keeps range values for
      ///        parameter: ( min1, max1, min2, max2, ... )
      /// @parm prmID variable parameter sequence number (the same order as sensitivities array has)
      /// @return array of min/max ranges values for parameter
      std::vector<double> validRngsPrmVals( size_t prmID ) const 
      {
         return m_validSensitivitiesPrmVals.empty() ? std::vector<double>() : m_validSensitivitiesPrmVals[prmID];
      }

      /// @brief If observable takes undefined value for some parameter variation, for such parameter SensitivityCalculator
      ///        calculating ranges where observable value is valid. This function returns array Kx2 which keeps corresponded
      ///        observable values for parameter ranges values: ( min1obs, max1obs, min2obs, max2obs, ... )
      /// @parm prmID variable parameter sequence number (the same order as sensitivities array has)
      /// @return array of observable values which correspond to min/max ranges values for parameter
      std::vector<double> validRngsObsVals( size_t prmID ) const 
      {
         return m_validSensitivitiesObsVals.empty() ? std::vector<double>() : m_validSensitivitiesObsVals[prmID];
      }

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
      const Observable  * m_obs;              ///< corresponded observable for which this sensitivities were calculated
      int                 m_obsSubID;         ///< observable could has dimension more than one, in this case here it is ID of sub-observable
      double              m_refObsValue;      ///< reference observable value

      std::vector< std::pair<const VarParameter *, int> > m_vprmPtr;          /*! Variable parameters set (size N + M) in the same order as sensitivities
                                                                                   N is number of continuios parameters and M - categorical */
      SensitivityData     m_sensitivities;    ///< Array Nx2+Mx1 which keep for each var parameter min/max values for observable
      SensitivityData     m_relSensitivities;          ///< Array Nx2+Mx1 with relative sensitivities for min/max from IP boundaries
      SensitivityData     m_maxSensitivities;          ///< Array Nx2+Mx1 with maximumu/minimum targets values over IP range
      SensitivityData     m_maxRelSensitivities;       ///< Array Nx2+Mx1 with relative sensitivities for min/max targets values over IP range
      SensitivityData     m_prmValForMaxSensitivities; ///< Array Nx2+Mx1 with parameter range value for maximumu/minimum targets values

      // Valid subintervals data for targets which could be undefined on min/max variable parameter range interval
      SensitivityData     m_validSensitivitiesObsVals; /*! Array with size N+M where each element keeps array of mim/max 
                                                           values observable values for valid intervals */
      SensitivityData     m_validSensitivitiesPrmVals; /*! Array with size N+M where each element keeps array of min/max
                                                           parameter values  for valid intervals. */
   };
}

#endif // CASA_TORNADO_SENSITIVITY_INFO_H
