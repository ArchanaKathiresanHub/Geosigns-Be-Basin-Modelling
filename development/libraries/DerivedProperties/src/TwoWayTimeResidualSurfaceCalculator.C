//                                                                      
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Derived property library
#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedSurfaceProperty.h"
#include "DerivedPropertyManager.h"
#include "PropertyRetriever.h"

// DataAccess library
#include "Interface/Surface.h"

// Utility library
#include "FormattingException.h"
#include "LogHandler.h"

#include "TwoWayTimeResidualSurfaceCalculator.h"

typedef formattingexception::GeneralException TwoWayTimeResidualException;

using namespace AbstractDerivedProperties;

DerivedProperties::TwoWayTimeResidualSurfaceCalculator::TwoWayTimeResidualSurfaceCalculator( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle( projectHandle ) {
   addPropertyName  ( "TwoWayTimeResidual" );
   addDependentPropertyName ( "TwoWayTime" );
}

void DerivedProperties::TwoWayTimeResidualSurfaceCalculator::calculate( AbstractPropertyManager&     propertyManager,
                                                                  const DataModel::AbstractSnapshot* snapshot,
                                                                  const DataModel::AbstractSurface*  surface,
                                                                        SurfacePropertyList&         derivedProperties ) const {

   ///1. Get the input two-way-(travel)-time
   const DataAccess::Interface::Surface* dataAccessSurface = dynamic_cast<const DataAccess::Interface::Surface*>(surface);
   const DataAccess::Interface::GridMap* twoWayTimeInitialMap = dataAccessSurface->getInputTwoWayTimeMap();
   // if there is no input map we need to check for a scalar
   double twoWayTimeInitialScalar;
   if (!twoWayTimeInitialMap) {
      twoWayTimeInitialScalar = dataAccessSurface->getInputTwoWayTimeScalar();
   }
   else{
      twoWayTimeInitialMap->retrieveData();
   }


   ///2. Get two-way-(travel)-time properties
   const DataModel::AbstractProperty* twoWayTimeProperty         = propertyManager.getProperty( "TwoWayTime" );
   const DataModel::AbstractProperty* twoWayTimeResidualProperty = propertyManager.getProperty( "TwoWayTimeResidual" );

   const SurfacePropertyPtr twoWayTime = propertyManager.getSurfaceProperty( twoWayTimeProperty, snapshot, surface );

   derivedProperties.clear();

   ///3. Compute two-way-(travel)-time residual
   if (twoWayTime != 0) {

      PropertyRetriever twoWayTimeRetriever( twoWayTime );

      DerivedSurfacePropertyPtr twoWayTimeResidual = DerivedSurfacePropertyPtr( new DerivedProperties::DerivedSurfaceProperty( twoWayTimeResidualProperty,
                                                                                                                               snapshot,
                                                                                                                               surface,
                                                                                                                               propertyManager.getMapGrid() ));
      double undefinedValue = twoWayTimeResidual->getUndefinedValue();
      double twoWayTimeResidualValue;

      for (unsigned int i = twoWayTimeResidual->firstI( true ); i <= twoWayTimeResidual->lastI( true ); ++i) {

         for (unsigned int j = twoWayTimeResidual->firstJ( true ); j <= twoWayTimeResidual->lastJ( true ); ++j) {

            if (m_projectHandle->getNodeIsValid( i, j )){

               // if we don't have cauldron two way time value at this node
               // info: m_twoWayTimeCauldron and TwoWayTimeResidualMap have the same undefined value 99999
               if (twoWayTime->get( i, j ) == undefinedValue) {
                  twoWayTimeResidualValue = undefinedValue;
               }

               else {
                  // if the surface is linked to an intial two way time map
                  if (twoWayTimeInitialMap != 0) {
                     // if we don't have intial two way time value at this node
                     if (twoWayTimeInitialMap->getValue( i, j ) == twoWayTimeInitialMap->getUndefinedValue()) {
                        twoWayTimeResidualValue = undefinedValue;
                     }
                     else {
                        twoWayTimeResidualValue = twoWayTime->get( i, j ) - twoWayTimeInitialMap->getValue( i, j );
                     }
                  }
                  // else the surface is linked to an intial two way time scalar
                  else {
                     assert( ("There must be a two way time map input or a two way time scalar input", twoWayTimeInitialScalar != -9999) );
                     twoWayTimeResidualValue = twoWayTime->get( i, j ) - twoWayTimeInitialScalar;
                  }
               }

               twoWayTimeResidual->set( i, j, twoWayTimeResidualValue );
            }

            else {
               twoWayTimeResidual->set( i, j, undefinedValue );
            }

         }
      }

      derivedProperties.push_back( twoWayTimeResidual );
   }

   else{
      throw TwoWayTimeResidualException() << "Cannot get two-way-time residual dependent property 'two-way-time'.";
   }

   if (twoWayTimeInitialMap) {
      twoWayTimeInitialMap->restoreData();
   }

}

bool DerivedProperties::TwoWayTimeResidualSurfaceCalculator::isComputable( const AbstractPropertyManager&     propManager,
                                                                           const DataModel::AbstractSnapshot* snapshot,
                                                                           const DataModel::AbstractSurface*  surface ) const {

   const DataModel::AbstractProperty* twoWayTime = propManager.getProperty( "TwoWayTime" );
   if (!surface){
      ///0. Initial check used when adding calculators
      return propManager.surfacePropertyIsComputable( twoWayTime, snapshot, surface );
   }
   else{
      ///1. Check that snapshot age is 0
      assert( snapshot );
      if (snapshot->getTime() != 0.0) {
         LogHandler( LogHandler::WARNING_SEVERITY )
            << "Cannot compute Two-Way-(Travel)-Time Residual property at specified snpashot"
            << snapshot->getTime() << "Ma for the surface ' " << surface->getName()
            << "'. This property can only be computed at present day 0.0Ma.";
         return false;
      }

      ///2. Check that we have an input two-way-(travel)-time
      const DataAccess::Interface::Surface* dataAccessSurface = dynamic_cast<const DataAccess::Interface::Surface*>(surface);
      const DataAccess::Interface::GridMap* twoWayTimeInitialMap = dataAccessSurface->getInputTwoWayTimeMap();
      // if there is no input map we need to check for a scalar
      double twoWayTimeInitialScalar;
      if (!twoWayTimeInitialMap) {
         twoWayTimeInitialScalar = dataAccessSurface->getInputTwoWayTimeScalar();
         if (twoWayTimeInitialScalar == -9999) {
            // then there is also no scalar input
            LogHandler( LogHandler::WARNING_SEVERITY )
               << "Cannot compute Two-Way-(Travel)-Time Residual property because no initial Two-Way-(Travel)-Time is specified for the surface '"
               << surface->getName() << "' in the stratigraphy table.";
            return false;
         }
      }

      ///3. Check that we can compute the TwoWayTime
      return propManager.surfacePropertyIsComputable( twoWayTime, snapshot, surface );
   }

}
