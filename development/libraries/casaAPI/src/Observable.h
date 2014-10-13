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

#include <vector>

/// @page CASA_ObservablePage Observable description
///
/// <b><em> Observable (or Target) </em></b> - any simulator output value. It could be any data value from the
/// simulation results. For example temperature or VRe at some position and depth for current time.
/// Some observables could be used for risk assessment - for example the total amount of HC in a trap.
/// The set of observables also could be denote as an output vector:  @f$ \vec{O} = \vec{(o_1,o_2,...,o_m)} @f$ .
///
/// <b><em> Observable reference value </em></b> - usually it is a measurement of corresponded observable value
/// from the real well.Observables with reference value could be used for calibration workflow.
///
/// <b><em> Standard deviation value of observable reference value </em></b> - contains the standard deviations
/// of the measurement noise. Standard deviation (SD) (represented by the Greek letter sigma, @f$ \sigma @f$)
/// measures the amount of variation or dispersion from the average. A low standard deviation indicates that
/// the data points tend to be very close to the mean (also called expected value); a high standard deviation
/// indicates that the data points are spread out over a large range of values. 
/// In science, researchers commonly report the standard deviation of experimental data, and only effects that
/// fall much farther than two standard deviations away from what would have been expected are considered
/// statistically significant - normal random error or variation in the measurements is in this way distinguished
/// from causal variation.
/// @image html Standard_deviation_diagram.png "A plot of a normal distribution (or bell-shaped curve) where each band has
/// a width of 1 standard deviation (68-95-99.7 rule)"
///
///The following list of Observable types is implemented in CASA API:
///
/// - @subpage  CASA_ObservableGridPropXYZPage
/// - @subpage CASA_ObservableGridPropWellPage
///
/// A new casa::Observable object could be created by one of the static functions from @link casa::DataDigger Data Digger @endlink

namespace mbapi
{
   class Model;
}

namespace casa
{
   class ObsValue; // class which will keep the real value from the results which corresponds this observable

   /// @brief Base class for keeping description of some value (not the value itself) from Cauldron simulation results
   /// Also this class keeps observable reference value and observable weights for Sensitivity and Uncertainty analysis
   class Observable
   {
   public:
      static const std::string s_dataMinerTable; ///< name of the table which keeps observable values after simulations

      /// @brief Destructor
      virtual ~Observable() {;}

      /// @brief Get name of the observable
      /// @return observable name
      virtual const char * name() const = 0;

      /// @brief Does observable has a reference value (measurement)
      /// @return true if reference value was set, false otherwise
      virtual bool hasReferenceValue() const = 0;

      /// @brief Get reference value
      /// @return reference value
      virtual const ObsValue * referenceValue() const = 0;

      /// @brief Get standard deviations for the reference value
      /// @return a standard deviation for reference value
      virtual double stdDeviationForRefValue() const = 0;

      /// @brief Set reference value
      /// @param refVal reference value itself
      /// @param stdDevVal standard deviation value for the reference value
      virtual void setReferenceValue( ObsValue * refVal, double stdDevVal ) = 0;

      /// @brief Get weighting coefficient for sensitivity analysis
      /// return weighting coefficient. This coefficient should be used in Pareto diagram calculation
      virtual double saWeight() const = 0;

      /// @brief Get weighting coefficient for uncertainty analysis
      /// return weighting coefficient. This coefficient should be used for RMSE calculation in Monte Carlo simulation
      virtual double uaWeight() const = 0;

      /// @brief Set weight coefficient for Sensitivity analysis
      /// @param w weight coefficient value
      virtual void setSAWeight( double w ) = 0;

      /// @brief Set weight coefficient for Uncertainty analysis
      /// @param w weight coefficient value
      virtual void setUAWeight( double w ) = 0;

      /// @brief Update Model to be sure that requested property will be saved at requested time
      /// @param caldModel Cauldron model
      /// @return NoError in case of success, or error code otherwise, error message will be set in caldModel.
      virtual ErrorHandler::ReturnCode requestObservableInModel( mbapi::Model & caldModel ) = 0;

      /// @brief Get this observable value from Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @return a new ObsValue object with observable value on success or NULL otherwise. 
      /// Error code could be obtained from the Model object. Caller is responsible for deleting the object
      /// when it can be discarded
      virtual ObsValue * getFromModel( mbapi::Model & caldModel ) = 0;

      /// @brief Create new observable value from set of doubles. This method is used for data conversion between SUMlib and CASA
      /// @param[in,out] val iterator for double array
      /// @return new observable value on success, or NULL pointer otherwise
      virtual ObsValue * createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const = 0;

      /// @brief Wrapper function to use in C# through Swig due to absence of iterators in Swig
      /// @param[in] vals vector of observables value
      /// @param[in,out] off offset in the vector, where the observable values are located
      /// @return new observable value on success, or NULL pointer otherwise
      ObsValue * newObsValueFromDoubles( const std::vector<double> & vals, int & off )
      {
         std::vector<double>::const_iterator it = vals.begin() + off;
         std::vector<double>::const_iterator sit = it;

         ObsValue * ret = createNewObsValueFromDouble( it );
         off += static_cast<unsigned int>( it - sit );
         return ret;
      }

   protected:
      Observable() { ; }
      
   private:
      Observable(const Observable &);
      Observable & operator = ( const Observable & );
   };
}

#endif // CASA_API_OBSERVABLE_H
