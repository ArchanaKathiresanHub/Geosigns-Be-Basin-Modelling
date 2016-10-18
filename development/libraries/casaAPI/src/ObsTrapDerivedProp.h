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

#include "Observable.h"
#include "ObsValue.h"

// STL
#include <memory>

/// @page CASA_ObservableTrapDerivedPropPage Any trap property which could be extracted by datadriller after migration calculation.
/// Example of properties: GOR,.... Trap is defined by specifying XY point coordinate and reservoir name

namespace mbapi
{
   class Model;
}

namespace casa
{
   /// @brief Class for calculating trap property value from composition
   class ObsTrapDerivedProp : public Observable
   {
   public:
      /// @brief Create new observable object for the given grid property for specified grid position
      static ObsTrapDerivedProp * createNewInstance( double              x             ///< X-th grid coordinate [m]
                                                   , double              y             ///< Y-th grid coordinate [m]
                                                   , const char        * resName       ///< reservoir name
                                                   , const char        * propName      ///< name of the trap property
                                                   , double              simTime       ///< simulation time [Ma]
                                                   , bool                logTrans      ///< should we transform mass observables to log scale?
                                                   , const std::string & name = ""     ///< user specified name for observable
                                                   ) { return new ObsTrapDerivedProp( x, y, resName, propName, simTime, logTrans, name ); }

      /// @brief Create observable for the given grid property for specified grid position
      ObsTrapDerivedProp( double              x         ///< X-th grid coordinate [m]
                        , double              y         ///< Y-th grid coordinate [m]
                        , const char        * resName   ///< reservoir name
                        , const char        * propName  ///< name of the property
                        , double              simTime   ///< simulation time [Ma]
                        , bool                logTrans  ///< should we transform mass observables to log scale?
                        , const std::string & name = "" ///< user specified name for observable
                        );

      /// @brief Destructor
      virtual ~ObsTrapDerivedProp( );

      /// @brief Get name of the observable. If dimension of observable is more than 1
      ///        it return name for each dimension
      /// @return observable name for each dimension
      virtual std::vector<std::string> name() const;

      /// @brief Get observable dimension
      /// @return dimension of observable
      virtual size_t dimension() const { return 1; }

      /// @brief If observable has transformation, it could has different dimension
      /// @return untransformed observable dimension
      virtual size_t dimensionUntransformed() const; 

      /// @brief Does observable has a reference value (measurement)
      /// @return true if reference value was set, false otherwise
      virtual bool hasReferenceValue() const { return m_refValue.get() == NULL ? false : true; }

      /// @brief Get reference value
      /// @return reference value
      virtual const ObsValue * referenceValue() const { return m_refValue.get(); }

      /// @brief Make observable transformation to present trap property value to the user. This observable should be treated differently 
      ///        when it is aproximated by a response surface and when it is presented to the user.
      /// @param val Original observable value from the run case or from MonteCarlo
      /// @return The new Observable value object which will keep the transformed observable value. This object must be destroyed
      ///         by calling function.
      virtual ObsValue * transform( const ObsValue * val ) const;

      /// @brief Get standard deviations for the reference value
      /// @return a standard deviation for reference value
      virtual const ObsValue * stdDeviationForRefValue() const { return m_devValue.get(); }

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

      /// @brief Set weight coefficient for Uncertainty analysis
      /// @param w weight coefficient value
      virtual void setUAWeight( double w ) { m_uaWeight = w; }

      /// @brief Get weighting coefficient for uncertainty analysis
      /// return weighting coefficient. This coefficient should be used for RMSE calculation in Monte Carlo simulation
      virtual double uaWeight() const { return m_uaWeight; }
   
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
      /// @return empty string if there is no any problems with this observable, or error message if trap is outside of the project 
      virtual std::string checkObservableForProject( mbapi::Model & caldModel ) const;

      /// @brief Create new observable value from set of doubles. This method is used for data conversion between SUMlib and CASA
      /// @param[in,out] val iterator for double array
      /// @return new observable value
      virtual ObsValue * createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const;

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 2; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  fileVer stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int fileVer ) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "ObsTrapDerivedProp"; }

      /// @brief Create a new observable instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      /// @return new observable instance on susccess, or throw and exception in case of any error
      ObsTrapDerivedProp( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      double                     m_x;                ///< X-th coordinate
      double                     m_y;                ///< Y-th coordinate

      std::string                m_resName;          ///< reservoir name
      std::string                m_propName;         ///< Property name
      double                     m_simTime;          ///< simulator time

      std::vector<std::string>   m_name;             ///< name of the observable

      std::vector<size_t>        m_posDataMiningTbl; ///< rows number in DataMiningIoTbl which corresponds composition of this observable

      std::unique_ptr<ObsValue>  m_refValue;         ///< reference value
      std::unique_ptr<ObsValue>  m_devValue;         ///< standard deviation for reference value

      double                     m_saWeight;         ///< Observable weight for sensitivity analysis
      double                     m_uaWeight;         ///< Observable weight for uncertainty analysis

      bool                       m_logTransf;        ///< do logarithmic transformation (needed for for Volumes/Mass)
   private:

      double calculateDerivedTrapProp( const std::vector<double> & vals ) const; ///< Calculate property value from the composition

      ObsTrapDerivedProp( const ObsTrapDerivedProp & );
      ObsTrapDerivedProp & operator = ( const ObsTrapDerivedProp & );
   };
}

#endif // CASA_API_OBS_TRAP_DERIVED_PROPERTY_XY_H
