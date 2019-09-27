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
/// @brief This file keeps declaration of the class for all possible types of observables

#ifndef CASA_API_OBSERVABLE_H
#define CASA_API_OBSERVABLE_H

#include "CasaSerializer.h"
#include "CasaDeserializer.h"
#include "ErrorHandler.h"

// STL
#include <memory>
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
/// fall much further than two standard deviations away from what would have been expected are considered
/// statistically significant - normal random error or variation in the measurements is in this way distinguished
/// from causal variation.
/// @image html Standard_deviation_diagram.png "A plot of a normal distribution (or bell-shaped curve) where each band has
/// a width of 1 standard deviation (68-95-99.7 rule)"
///
///The following list of Observable types is implemented in CASA API:
///
/// - @subpage CASA_ObservableGridPropXYZPage
/// - @subpage CASA_ObservableGridPropWellPage
/// - @subpage CASA_ObservableSourceRockMapPropPage
/// - @subpage CASA_ObservableTrapPropPage
/// - @subpage CASA_ObservableTrapDerivedPropPage
///
/// A new casa::Observable object could be created by one of the static functions from @link casa::DataDigger Data Digger @endlink
///

namespace mbapi
{
   class Model;
}

namespace casa
{
   class ObsValue; // class which will keep the real value from the results which corresponds this observable

   /// @brief Base class for keeping description of some value (not the value itself) from Cauldron simulation results
   /// Also this class keeps observable reference value and observable weights for Sensitivity and Uncertainty analysis
   class Observable : public CasaSerializable
   {
   public:
      static const std::string s_dataMinerTable; ///< name of the table which keeps observable values after simulations

      /// @brief Destructor
      virtual ~Observable();

      /// @brief Get observable dimension
      /// @return dimension of observable
      virtual size_t dimension() const = 0;

      /// @brief Set reference value
      /// @param refVal reference value itself
      /// @param stdDevVal standard deviation value for the reference value
      virtual void setReferenceValue( ObsValue * refVal, ObsValue * stdDevVal ) = 0;

      /// @brief Update Model to be sure that requested property will be saved at requested time
      /// @param[in] caldModel Cauldron model
      /// @return NoError in case of success, or error code otherwise, error message will be set in caldModel.
      virtual ErrorHandler::ReturnCode requestObservableInModel( mbapi::Model & caldModel ) = 0;

      /// @brief Get this observable value from Cauldron model
      /// @param[in] caldModel Cauldron model
      /// @return a new ObsValue object with observable value on success or NULL otherwise.
      /// Error code could be obtained from the Model object. Caller is responsible for deleting the object
      /// when it can be discarded
      virtual ObsValue * getFromModel( mbapi::Model & caldModel ) = 0;

      /// @brief Check the observable position for the given model, is the observable outside of the model
      ///        boundaries and can the observable value be extracted from the model
      /// @param[in] caldModel reference to Cauldron model
      /// @return              true if check didn't find any problems with this observable, otherwise return false
      virtual bool checkObservableForProject( mbapi::Model & caldModel ) const = 0;

      /// @brief Check the observable position for the given model based on the model boundaries as well as observable origin.
      ///        If observable falls within the model boundaries, check whether or not the observable origin matches the observable origin attached to window.
      /// @param[in] caldModel reference to Cauldron model
      /// @return              true if check didn't find any problems with this observable, otherwise return false
      virtual bool checkObservableOriginForProject( mbapi::Model & caldModel ) const = 0;

      /// @brief Create new observable value from set of doubles. This method is used for data conversion between SUMlib and CASA
      /// @param[in,out] val iterator for double array
      /// @return new observable value on success, or NULL pointer otherwise
      virtual ObsValue * createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const = 0;

      /// @brief Get Z coordinates list
      /// @return array with Z coordinate of each observable point along well
      virtual std::vector<double> depth() const = 0;

      /// @brief Get X coordinates list
      /// @return array with X coordinate of each observable point along well
      virtual std::vector<double> xCoords() const = 0;

      /// @brief Get Y coordinates list
      /// @return array with Y coordinate of each observable point along well
      virtual std::vector<double> yCoords() const = 0;

      /// @{
      /// @brief Make observable transformation to present observable value to the user. Some observables
      ///        (like trap properties) should be differently treated when they are aproximated by response surface
      ///        and when they are presented to the user. This function performs such conversion. If specific observable type
      ///        does not need such convertion, this function will return zero pointer.
      // @param val Original observable value from the run case or from MonteCarlo
      /// @return The new Observable value object which will keep the transformed observable value. This object must be destroyed
      ///         by calling function.
      virtual ObsValue * transform( const ObsValue * /* val */ ) const;

      /// @brief If observable has transformation, it could has different dimension
      /// @return untransformed observable dimension
      virtual size_t dimensionUntransformed() const;

      /// @brief Check the extracted from the model observable value (values if the dimension of the
      ///        observable more then one), is it defined or undefined.
      /// @param[in] obv observable value object pointer
      /// @return vector with size of the observable dimension with true/false values for defined/undefined observable value
      virtual std::vector<bool> isValid( const ObsValue * obv ) const;

      /// @brief Get name of the observable. If dimension of observable is more than 1
      ///        it return name for each dimension
      /// @return observable name for each dimension
      std::vector<std::string> name() const;

      /// @brief Get Cauldron property name
      /// @return name of the property as a string
      std::string propertyName() const;

      /// @brief Does observable has a reference value (measurement)
      /// @return true if reference value was set, false otherwise
      bool hasReferenceValue() const;

      /// @brief Get reference value
      /// @return reference value
      const ObsValue * referenceValue() const;

      /// @brief Get standard deviations for the reference value
      /// @return a standard deviation for reference value
      const ObsValue * stdDeviationForRefValue() const;

      /// @brief Get weighting coefficient for sensitivity analysis
      /// return weighting coefficient. This coefficient should be used in Pareto diagram calculation
      double saWeight() const;

      /// @brief Get weighting coefficient for uncertainty analysis
      /// return weighting coefficient. This coefficient should be used for RMSE calculation in Monte Carlo simulation
      double uaWeight() const;

      /// @brief Set weight coefficient for Sensitivity analysis
      /// @param w weight coefficient value
      void setSAWeight( const double w );

      /// @brief Set weight coefficient for Uncertainty analysis
      /// @param w weight coefficient value
      void setUAWeight( const double w );

      /// @brief Wrapper function to use in C# through Swig due to absence of iterators in Swig
      /// @param[in] vals vector of observables value
      /// @param[in,out] off offset in the vector, where the observable values are located
      /// @return new observable value on success, or NULL pointer otherwise
      ObsValue * newObsValueFromDoubles( const std::vector<double> & vals, int & off );

      /// @brief Create a new observable instance and deserialize it from the given stream
      /// @param[in] dz input stream
      /// @param[in] objName expected object name
      /// @return new observable instance on susccess, or throw and exception in case of any error
      static Observable * load( CasaDeserializer & dz, const char * objName );

   protected:
      Observable();
      Observable( const std::string & propName, const double simTime );

      virtual void saveCommon( const Observable * observableClass
                             , CasaSerializer & sz
                             , bool & ok
                             , const std::string& variableTypeName = ""
                             , const std::string& variableName = "" ) const = 0;

      bool checkObsMatchesModel( mbapi::Model      & caldModel
                               , const int           iTable
                               , const double        xCoord
                               , const double        yCoord
                               , const double        zCoord
                               , const double        epsilon
                               , const std::string & variableTypeName = ""
                               , const std::string & variableName = "" ) const;

      bool setCommonTableValues( mbapi::Model      & caldModel
                               , const int           posData
                               , const double        xCoord
                               , const double        yCoord
                               , const double        zCoord
                               , const std::string & variableTypeName = ""
                               , const std::string & variableName = ""  ) const;

      double simulationTime() const;
      void setSimTime( double simTime );
      void setPropertyName( const std::string & propName );
      void setRefValue( const ObsValue * refValue );
      void createRefValues( const ObsValue * refValue, const ObsValue * devValue );
      const ObsValue * devValue() const;
      void setDevValue( const ObsValue * devValue );
      std::vector<int> posDataMiningTbl() const;
      void setPosDataMiningTbl(const std::vector<int>& posDataVec );
      void setName( std::vector<std::string> & name );

    private:
      Observable(const Observable &);
      Observable & operator = ( const Observable & );

      std::vector<std::string>        m_name;             ///< name of the observable
      std::string                     m_propName;         ///< Property name
      double                          m_simTime;          ///< simulator time
      double                          m_saWeight;         ///< Observable weight for sensitivity analysis
      double                          m_uaWeight;         ///< Observable weight for uncertainty analysis
      std::unique_ptr<const ObsValue> m_refValue;         ///< reference value
      std::unique_ptr<const ObsValue> m_devValue;         ///< standard deviation for reference value
      std::vector<int>                m_posDataMiningTbl; ///< row number in DataMiningIoTbl which corresponds this observable
   };
} // namespace casa

#endif // CASA_API_OBSERVABLE_H
