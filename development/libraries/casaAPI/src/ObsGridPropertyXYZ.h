//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsGridPropertyXYZ.h
/// @brief This file keeps declaration of the class of grid property value as observable

#ifndef CASA_API_OBS_GRID_PROPERTY_XYZ_H
#define CASA_API_OBS_GRID_PROPERTY_XYZ_H

#include "Observable.h"

#include <memory>

/// @page CASA_ObservableGridPropXYZPage any Cauldron grid property at specified XYZ point
/// This observable could retrieve grid property value for given position in a grid
/// 

namespace mbapi
{
   class Model;
}

namespace casa
{
   /// @brief Base class for keeping some value from Cauldron simulation results
   class ObsGridPropertyXYZ : public Observable
   {
   public:
      /// @brief Create observable for the given grid property for specified grid position
      /// @param x X-th grid coordinate
      /// @param y Y-th grid coordinate
      /// @param z Z-th grid coordinate
      /// @param propName name of the property
      /// @param simTime simulation time
      ObsGridPropertyXYZ( double x, double y, double z, const char * propName, double simTime );

      /// @brief Destructor
      virtual ~ObsGridPropertyXYZ( );

      /// @brief Get name of the observable
      /// @return observable name
      virtual const char * name() const;
      
      /// @brief Does observable has a reference value (measurement)
      /// @return true if reference value was set, false otherwise
      virtual bool hasReferenceValue() const { return m_refValue.get() == NULL ? false : true; }

      /// @brief Get reference value
      /// @return reference value
      virtual const ObsValue * referenceValue() const { return m_refValue.get(); }

      /// @brief Get standard deviations for the reference value
      /// @return a standard deviation for reference value
      virtual double stdDeviationForRefValue() const { return m_devValue; }

      /// @brief Set reference value
      /// @param refVal reference value itself
      /// @param stdDevVal standard deviation value for the reference value
      virtual void setReferenceValue( ObsValue * refVal, double stdDevVal );

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

      /// @brief Create new observable value from set of doubles. This method is used for data conversion between SUMlib and CASA
      /// @param[in,out] val iterator for double array
      /// @return new observable value
      virtual ObsValue * createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const;

   protected:
      double      m_x;  ///< X-th coordinate
      double      m_y;  ///< Y-th coordinate
      double      m_z;  ///< Z-th coordinate

      std::string m_propName;  ///< Property name
      double      m_simTime;   ///< simulator time

      std::string m_name;      ///< name of the observable

      size_t      m_posDataMiningTbl; ///< row number in DataMiningIoTbl which corresponds this observable

      std::auto_ptr<ObsValue> m_refValue;  ///< reference value
      double                  m_devValue;  ///< standard deviation for reference value

      double      m_saWeight;  ///< Observable weight for sensitivity analysis
      double      m_uaWeight;  ///< Observable weight for uncertainty analysis

   private:
      ObsGridPropertyXYZ( const ObsGridPropertyXYZ & );
      ObsGridPropertyXYZ & operator = ( const ObsGridPropertyXYZ & );
   };
}

#endif // CASA_API_OBS_GRID_PROPERTY_XYZ_H
