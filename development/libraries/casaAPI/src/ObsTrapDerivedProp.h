//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file ObsTrapDerivedProp.h
/// @brief This file keeps declaration of the class for trap property

#ifndef CASA_API_OBS_TRAP_DERIVED_PROPERTY_XY_H
#define CASA_API_OBS_TRAP_DERIVED_PROPERTY_XY_H

#include "ObservableSinglePoint.h"
#include "ObsValue.h"

// STL
#include <memory>

/// @page CASA_ObservableTrapDerivedPropPage Any trap property which could be extracted by datadriller after migration calculation.
/// Example of properties: GOR,.... Trap is defined by specifying XY point coordinate and reservoir name

namespace casa
{
   /// @brief Class for calculating trap property value from composition
   class ObsTrapDerivedProp : public ObservableSinglePoint
   {
   public:
      /// @brief Create new observable object for the given grid property for specified grid position
      static ObsTrapDerivedProp * createNewInstance( double              x             ///< X-th grid coordinate [m]
                                                   , double              y             ///< Y-th grid coordinate [m]
                                                   , const char        * resName       ///< reservoir name
                                                   , const char        * propName      ///< name of the trap property
                                                   , double              simTime       ///< simulation time [Ma]
                                                   , bool                logTrans      ///< should we transform mass observables to log scale?
                                                   , const std::string & myName = ""     ///< user specified name for observable
                                                   ) { return new ObsTrapDerivedProp( x, y, resName, propName, simTime, logTrans, myName ); }

      /// @brief Create observable for the given grid property for specified grid position
      ObsTrapDerivedProp( double              x         ///< X-th grid coordinate [m]
                        , double              y         ///< Y-th grid coordinate [m]
                        , const char        * resName   ///< reservoir name
                        , const char        * propName  ///< name of the property
                        , double              simTime   ///< simulation time [Ma]
                        , bool                logTrans  ///< should we transform mass observables to log scale?
                        , const std::string & myName = "" ///< user specified name for observable
                        );

      /// @brief Destructor
      virtual ~ObsTrapDerivedProp();

      /// @brief If observable has transformation, it could has different dimension
      /// @return untransformed observable dimension
      virtual size_t dimensionUntransformed() const;

      /// @brief Make observable transformation to present trap property value to the user. This observable should be treated differently
      ///        when it is aproximated by a response surface and when it is presented to the user.
      /// @param val Original observable value from the run case or from MonteCarlo
      /// @return The new Observable value object which will keep the transformed observable value. This object must be destroyed
      ///         by calling function.
      virtual ObsValue * transform( const ObsValue * val ) const;

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

      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 2; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz ) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual std::string typeName() const { return "ObsTrapDerivedProp"; }

      /// @brief Create a new observable instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      /// @return new observable instance on susccess, or throw and exception in case of any error
      ObsTrapDerivedProp( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   private:
      ObsTrapDerivedProp( const ObsTrapDerivedProp & );
      ObsTrapDerivedProp & operator = ( const ObsTrapDerivedProp & );

      double calculateDerivedTrapProp( const std::vector<double> & vals ) const; ///< Calculate property value from the composition

      std::string m_resName;   ///< reservoir name
      bool        m_logTransf; ///< do logarithmic transformation (needed for for Volumes/Mass)
   };
}

#endif // CASA_API_OBS_TRAP_DERIVED_PROPERTY_XY_H
