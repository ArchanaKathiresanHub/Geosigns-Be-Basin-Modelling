//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file  ObsGridPropertyWell.h
/// @brief This file keeps declaration of the class of grid property well value as observable

#ifndef CASA_API_OBS_GRID_PROPERTY_WELL_H
#define CASA_API_OBS_GRID_PROPERTY_WELL_H

#include "Observable.h"
#include "ObsValue.h"

// STL
#include <memory>

/// @page CASA_ObservableGridPropWellPage Cauldron grid property along a well trajectory
/// This observable could retrieve values for any grid property along the given well trajectory.
/// The trajectory must be defined as a set of XYZ points in basin model coordinate space.

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
                                                    , const std::string         & myName = ""   ///< user specified name for observable
                                                    )
      {
        return new ObsGridPropertyWell( x, y, z, propName, simTime, myName );
      }

      /// @brief Create observable for the given grid property for specified grid position
      ObsGridPropertyWell( const std::vector<double> & x         ///< well trajectory X-th coordinates [m]
                         , const std::vector<double> & y         ///< well trajectory Y-th coordinates [m]
                         , const std::vector<double> & z         ///< well trajectory Z-th coordinates [m]
                         , const char                * propName  ///< name of the property
                         , double                      simTime   ///< simulation time [Ma]
                         , const std::string         & myName = "" ///< custom name for observable
                         );

      /// @brief Destructor
      virtual ~ObsGridPropertyWell();

      /// @brief Get observable dimension
      /// @return dimension of observable
      virtual size_t dimension() const;

      /// @brief Set reference value
      /// @param refVal reference value itself
      /// @param stdDevVal standard deviation value for the reference value
      virtual void setReferenceValue( ObsValue * refVal, ObsValue * stdDevVal );

      /// @brief Get Z coordinates list
      /// @return array with Z coordinate of each observable point along well
      virtual std::vector<double> depth() const;

      /// @brief Get X coordinates list
      /// @return array with X coordinate of each observable point along well
      virtual std::vector<double> xCoords() const;

      /// @brief Get Y coordinates list
      /// @return array with Y coordinate of each observable point along well
      virtual std::vector<double> yCoords() const;

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
      /// @return              true if check didn't find any problems with this observable, otherwise return false
      virtual bool checkObservableForProject( mbapi::Model & caldModel ) const;

      /// @brief Check the observable position for the given model based on the model boundaries as well as observable origin.
      ///        If observable falls within the model boundaries, check whether or not the observable origin matches the observable origin attached to window.
      /// @param[in] caldModel reference to Cauldron model
      /// @return              true if check didn't find any problems with this observable, otherwise return false
      virtual bool checkObservableOriginForProject( mbapi::Model & caldModel ) const;

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
      /// @return true if it succeeds, false if it fails.
      virtual bool save(CasaSerializer & sz) const;

      /// @brief Create a new observable instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      /// @return new observable instance on susccess, or throw and exception in case of any error
      ObsGridPropertyWell( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      virtual void saveCommon( const Observable * observableClass
                             , CasaSerializer & sz
                             , bool & ok
                             , const std::string& variableTypeName = ""
                             , const std::string& variableName = "" ) const;

   private:
      ObsGridPropertyWell( const ObsGridPropertyWell & );
      ObsGridPropertyWell & operator = ( const ObsGridPropertyWell & );

      std::vector<double>  m_x;                ///< X-th coordinates
      std::vector<double>  m_y;                ///< Y-th coordinates
      std::vector<double>  m_z;                ///< Z-th coordinates
   };
}

#endif // CASA_API_OBS_GRID_PROPERTY_WELL_H
