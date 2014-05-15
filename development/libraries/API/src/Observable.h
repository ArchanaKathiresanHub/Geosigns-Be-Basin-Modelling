//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file Observable.h
/// @brief This file keeps declaration of the class for handling range for 2D map

#ifndef CASA_API_OBSERVABLE_H
#define CASA_API_OBSERVABLE_H

#include "ErrorHandler.h"

/// @page CASA_ObservablePage Observables
///
/// The following list of of Observable types is implemented in CASA API:
///
/// - Any Cauldron property value at the grid IJK position
/// - Any Cauldron property value at XYZ model position
///
/// A new observable object could be created by one of the static functions from @link casa::DataDigger Data Digger @endlink

namespace casa
{
   /// @brief Base class for keeping some value from Cauldron simulation results
   class Observable
   {
   public:
      /// @brief Destructor
      virtual ~Observable();

      /// @brief Copy operator. Creates deep copy of given Observable. Observables type\n
      ///        should be the same
      /// @param otherObs observable to be copied
      /// @return refernce to the current observable
      virtual Observable & operator = ( const Observable & otherObs ) = 0;

      /// @brief Get name of the observable
      /// @return observable name
      virtual const char * name() = 0;
      
      /// @brief Get observable value
      /// @return value for observable
      virtual double value() = 0;

      /// @brief Does observable has a reference value (mesurement)
      /// @return true if reference value was set, false otherwise
      virtual bool hasReferenceValue() = 0;
      /// @brief Get reference value
      /// @return reference value
      virtual double referenceValue() = 0;
      
      /// @brief Get standard deviations for the reference value
      /// @return a standart deviation for reference value
      virtual double stdDeviationForRefValue() = 0;

      /// @brief Get weighting coefficient for sensitivity analysis
      /// return weighting coefficient. This coefficient should be used in Pareto diagram calculation
      virtual double saWeight() = 0;

      /// @brief Get weighting coefficient for uncertainty analysis
      /// return weighting coefficient. This coefficient should be used for RMSE calculation in Monte Carlo simulation
      virtual double uaWeight() = 0;
   
   protected:
      Observable();
   }; 
}

#endif // CASA_API_OBSERVABLE_H
