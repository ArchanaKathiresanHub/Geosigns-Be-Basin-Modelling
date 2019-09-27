//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file ObservableSinglePoint.h
/// @brief This file keeps declaration of the class for handling observables with single point

#ifndef CASA_API_OBSERVABLE_SINGLE_POINT_H
#define CASA_API_OBSERVABLE_SINGLE_POINT_H

#include "Observable.h"

#include "ConstantsNumerical.h"
#include "ErrorHandler.h"

// STL
#include <vector>

namespace casa
{
   /// @brief Base class for keeping description and common functionality of observable classes with single point.
   ///        Also this class keeps x,y,z coordinates of observables with single point.
   class ObservableSinglePoint : public Observable
   {
   public:
      /// @brief Destructor
      virtual ~ObservableSinglePoint();

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

      /// @brief Create new observable value from set of doubles. This method is used for data conversion between SUMlib and CASA
      /// @param[in,out] val iterator for double array
      /// @return new observable value on success, or NULL pointer otherwise
      virtual ObsValue * createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const = 0;

      /// @brief Get Z coordinates list
      /// @return array with Z coordinate of each observable point along well
      virtual std::vector<double> depth() const;

      /// @brief Get X coordinates list
      /// @return array with X coordinate of each observable point along well
      virtual std::vector<double> xCoords() const;

      /// @brief Get Y coordinates list
      /// @return array with Y coordinate of each observable point along well
      virtual std::vector<double> yCoords() const;

      /// @brief Get observable dimension
      /// @return dimension of observable
      virtual size_t dimension() const;

      /// @brief Set reference value
      /// @param refVal reference value itself
      /// @param stdDevVal standard deviation value for the reference value
      virtual void setReferenceValue( ObsValue * refVal, ObsValue * stdDevVal );

      /// @brief Check the observable position for the given model, is the observable outside of the model
      ///        boundaries and can the observable value be extracted from the model
      /// @param[in] caldModel reference to Cauldron model
      /// @return              true if check didn't find any problems with this observable, otherwise return false
      virtual bool checkObservableForProject( mbapi::Model & caldModel ) const;

      /// @brief Check the observable position for the given model based on the model boundaries as well as observable origin.
      ///        If observable falls within the model boundaries, check whether or not the observable origin matches the observable origin attached to window.
      /// @param[in] caldModel reference to Cauldron model
      /// @return              true if check didn't find any problems with this observable, otherwise return false
      virtual bool checkObservableOriginForProject( mbapi::Model & caldModel ) const;

   protected:
     ObservableSinglePoint();
     ObservableSinglePoint( const std::string & myName
                          , const std::string & propName
                          , const double        simTime
                          , const double        x
                          , const double        y
                          , const double        z = Utilities::Numerical::IbsNoDataValue
                          );

      double xCoord() const;
      double yCoord() const;
      double zCoord() const;

      void setCoords( const double x, const double y, const double z = Utilities::Numerical::IbsNoDataValue );

      virtual void saveCommon( const Observable * observableClass
                             , CasaSerializer & sz
                             , bool & ok
                             , const std::string & variableTypeName = ""
                             , const std::string & variableName = "" ) const;

   private:
      ObservableSinglePoint( const ObservableSinglePoint & );
      ObservableSinglePoint & operator = ( const ObservableSinglePoint & );

      double m_x; ///< X coordinate
      double m_y; ///< Y coordinate
      double m_z; ///< Z coordinate
   };
} // namespace casa

#endif // CASA_API_OBSERVABLE_SINGLE_POINT_H
