//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file ObsGridXYLayerTopSurfaceProp.h
/// @brief This file keeps declaration of the class for
///
#ifndef CASA_API_OBS_GRID_XY_LAYER_TOPSURFACE_PROP_H
#define CASA_API_OBS_GRID_XY_LAYER_TOPSURFACE_PROP_H

#include "ObservableSinglePoint.h"
#include "ObsValue.h"

// STL
#include <memory>

/// @page CASA_ObsGridXYLayerTopSurfacePropPage Given the specified XY coordinates,
/// and the geological layer (or formation) name, the class is initiated to contains
/// the observable properties at the top surface of the layer.

namespace casa
{
   /// @brief Base class for keeping some value from Cauldron simulation results
   class ObsGridXYLayerTopSurfaceProp : public ObservableSinglePoint
   {
   public:

      /// @brief Create new observable object for the given grid property for specified grid position
      static ObsGridXYLayerTopSurfaceProp * createNewInstance( const double        x             ///< X-th grid coordinate [m]
                                                             , const double        y             ///< Y-th grid coordinate [m]
                                                             , const std::string & layerName     ///< Layer name
                                                             , const std::string & propName      ///< name of the property
                                                             , const double        simTime = 0.0 ///< simulation time [Ma]
                                                             , const std::string & myName = ""   ///< user specified name for observable
                                                             ) { return new ObsGridXYLayerTopSurfaceProp( x, y, layerName, propName, simTime, myName ); }

      /// @brief Create observable for the given grid property for specified grid position
      ObsGridXYLayerTopSurfaceProp( const double        x           ///< X-th grid coordinate [m]
                                  , const double        y           ///< Y-th grid coordinate [m]
                                  , const std::string & layerName   ///< layer name
                                  , const std::string & propName    ///< name of the property
                                  , const double        simTime     ///< simulation time [Ma]
                                  , const std::string & myName = "" ///< user specified name for observable
                                  );

      /// @brief Destructor
      virtual ~ObsGridXYLayerTopSurfaceProp( );

      /// @brief Update Model to be sure that requested property will be saved at requested time
      /// @param caldModel Cauldron model
      /// @return NoError in case of success, or error code otherwise, error message will be set in caldModel.
      virtual ErrorHandler::ReturnCode requestObservableInModel( mbapi::Model & caldModel );

      /// @brief Get this observable value from Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @return observable value on success or NULL otherwise. Error code could be obtained from the Model object
      virtual ObsValue * getFromModel( mbapi::Model & caldModel );

      /// @brief Create new observable value from set of doubles. This method is used for data conversion between SUMlib and CASA
      /// @param[in,out] val iterator for double array
      /// @return new observable value
      virtual ObsValue * createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const;

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  fileVer stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz ) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual std::string typeName() const { return "ObsGridXYLayerTopSurfaceProp"; }

      /// @brief Create a new observable instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      /// @return new observable instance on susccess, or throw and exception in case of any error
      ObsGridXYLayerTopSurfaceProp( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   private:
      ObsGridXYLayerTopSurfaceProp( const ObsGridXYLayerTopSurfaceProp & );
      ObsGridXYLayerTopSurfaceProp & operator = ( const ObsGridXYLayerTopSurfaceProp & );

      std::string m_layerName; ///< Layer name
   };
}

#endif // CASA_API_OBS_GRID_XY_LAYER_TOPSURFACE_PROP_H
