//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file ObsGridPropertyWell.h
/// @brief This file keeps declaration of the class of grid property value as observable

#ifndef CASA_API_OBS_GRID_PROPERTY_WELL_H
#define CASA_API_OBS_GRID_PROPERTY_WELL_H

#include "Observable.h"
#include "ObsValue.h"

// STL
#include <memory>

/// @page CASA_ObservableGridPropWellPage Cauldron grid property along a well trajectory
/// This observable could retrieve values for any grid property along the given well trajectory.
/// The trajectory must be defined as a set of XYZ points in basin model coordinate space.

namespace mbapi
{
   class Model;
}

namespace casa
{
   /// @brief Base class for keeping some value from Cauldron simulation results
   class ObsGridPropertyWell : public Observable
   {
   public:
      /// @brief Create observable for the given grid property for specified grid position
      static ObsGridPropertyWell * createNewInstance( const std::vector<double> & x             ///< well trajectory X-th coordinates [m]
                                                    , const std::vector<double> & y             ///< well trajectory Y-th coordinates [m]
                                                    , const std::vector<double> & z             ///< well trajectory Z-th coordinates [m]
                                                    , const char                * propName      ///< name of the property
                                                    , double                      simTime = 0.0 ///< simulation time [Ma]
                                                    , const std::string         & name = ""     ///< user specified name for observable
                                                    ) { return new ObsGridPropertyWell( x, y, z, propName, simTime, name ); };

      /// @brief Create observable for the given grid property for specified grid position
      ObsGridPropertyWell( const std::vector<double> & x         ///< well trajectory X-th coordinates [m]
                         , const std::vector<double> & y         ///< well trajectory Y-th coordinates [m]
                         , const std::vector<double> & z         ///< well trajectory Z-th coordinates [m]
                         , const char                * propName  ///< name of the property
                         , double                      simTime   ///< simulation time [Ma]
                         , const std::string         & name = "" ///< custom name for observable
                         );

      /// @brief Destructor
      virtual ~ObsGridPropertyWell( );

      /// @brief Get name of the observable. If dimension of observable is more than 1
      ///        it return name for each dimension
      /// @return observable name for each dimension
      virtual std::vector<std::string> name() const;

      /// @brief Get observable dimension
      /// @return dimension of observable
      virtual size_t dimension() const { return m_x.size(); }

      /// @brief Does observable has a reference value (measurement)
      /// @return true if reference value was set, false otherwise
      virtual bool hasReferenceValue() const { return m_refValue.get() == NULL ? false : true; }

      /// @brief Get reference value
      /// @return reference value
      virtual const ObsValue * referenceValue() const { return m_refValue.get(); }

      /// @brief Get standard deviations for the reference value
      /// @return standard deviation for reference value
      virtual const ObsValue * stdDeviationForRefValue( ) const { return m_devValue.get(); }

      /// @brief Set reference value
      /// @param refVal reference value itself
      /// @param stdDevVal standard deviation value for the reference value
      virtual void setReferenceValue( ObsValue * refVal, ObsValue * stdDevVal );

      /// @brief Get weighting coefficient for sensitivity analysis
      /// return weighting coefficient. This coefficient should be used in Pareto diagram calculation
      virtual double saWeight() const { return m_saWeight; }

      /// @brief Set weight coefficient for Sensitivity analysis
      /// @param w weight coefficient value
      virtual void setSAWeight( double w ) { m_saWeight = w;  }

      /// @brief Get Z coordinates list
      /// @return array with Z coordinate of each observable point along well
      std::vector<double> depth() const { return m_z; }

      /// @brief Get X coordinates list
      /// @return array with X coordinate of each observable point along well
      std::vector<double> xCoords() const { return m_x; }

      /// @brief Get Y coordinates list
      /// @return array with Y coordinate of each observable point along well
      std::vector<double> yCoords() const { return m_y; }

      /// @brief Set weight coefficient for Uncertainty analysis
      /// @param w weight coefficient value
      virtual void setUAWeight( double w ) { m_uaWeight = w; }

      /// @brief Get weighting coefficient for uncertainty analysis
      /// return weighting coefficient. This coefficient should be used for RMSE calculation in Monte Carlo simulation
      virtual double uaWeight( ) const { return m_uaWeight; }

      /// @brief Update Model to be sure that requested property will be saved at requested time
      /// @param caldModel Cauldron model
      /// @return NoError in case of success, or error code otherwise, error message will be set in caldModel.
      virtual ErrorHandler::ReturnCode requestObservableInModel( mbapi::Model & caldModel );

      /// @brief Get this observable value from Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @return observable value on success or NULL otherwise. Error code could be obtained from the Model object
      virtual ObsValue * getFromModel( mbapi::Model & caldModel );

      /// @brief Do observable validation for the given model
      /// @param caldModel reference to Cauldron model
      /// @return empty string if there is no any problems with this observable, or error message if well is outside of the project 
      virtual std::string checkObservableForProject( mbapi::Model & caldModel ) const;

      /// @brief Create new observable value from set of doubles. This method is used for data conversion between SUMlib and CASA
      /// @param[in,out] val iterator for double array
      /// @return new observable value
      virtual ObsValue * createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const;

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 1; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "ObsGridPropertyWell"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Create a new observable instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      /// @return new observable instance on susccess, or throw and exception in case of any error
      ObsGridPropertyWell( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      std::vector<double>       m_x;                ///< X-th coordinates
      std::vector<double>       m_y;                ///< Y-th coordinates
      std::vector<double>       m_z;                ///< Z-th coordinates

      std::string               m_propName;         ///< Property name
      double                    m_simTime;          ///< simulator time

      std::vector< std::string> m_name;             ///< name of the observable

      std::vector<int>          m_posDataMiningTbl; ///< row number in DataMiningIoTbl which corresponds this observable

      std::unique_ptr<ObsValue>   m_refValue;         ///< reference value
      std::unique_ptr<ObsValue>   m_devValue;         ///< standard deviation for reference value

      double                    m_saWeight;         ///< Observable weight for sensitivity analysis
      double                    m_uaWeight;         ///< Observable weight for uncertainty analysis

   private:
      ObsGridPropertyWell( const ObsGridPropertyWell & );
      ObsGridPropertyWell & operator = ( const ObsGridPropertyWell & );
   };
}

#endif // CASA_API_OBS_GRID_PROPERTY_WELL_H
