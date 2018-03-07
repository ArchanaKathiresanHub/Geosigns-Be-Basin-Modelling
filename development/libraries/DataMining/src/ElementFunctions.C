//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ElementFunctions.h"

#include "Interface/Grid.h"

using namespace AbstractDerivedProperties;

void DataAccess::Mining::getGeometryMatrix ( const ElementPosition&                      element,
                                             const DataModel::AbstractGrid*              grid,
                                             const FiniteElementMethod::ElementVector&   depth,
                                             FiniteElementMethod::ElementGeometryMatrix& geometryMatrix ) {

   if ( depth ( 1 ) != Interface::DefaultUndefinedMapValue ) {
      int i;

      double deltaX  = grid->deltaI ();
      double deltaY  = grid->deltaJ ();
      double elementOriginX = grid->minI () + double ( element.getI ()) * deltaX;
      double elementOriginY = grid->minJ () + double ( element.getJ ()) * deltaY;

      for ( i = 1; i <= 8; ++i ) {
         geometryMatrix ( 3, i ) = depth ( i );
      }

      geometryMatrix ( 1, 1 ) = elementOriginX;
      geometryMatrix ( 1, 2 ) = elementOriginX + deltaX;
      geometryMatrix ( 1, 3 ) = elementOriginX + deltaX;
      geometryMatrix ( 1, 4 ) = elementOriginX;

      geometryMatrix ( 1, 5 ) = elementOriginX;
      geometryMatrix ( 1, 6 ) = elementOriginX + deltaX;
      geometryMatrix ( 1, 7 ) = elementOriginX + deltaX;
      geometryMatrix ( 1, 8 ) = elementOriginX;

      geometryMatrix ( 2, 1 ) = elementOriginY;
      geometryMatrix ( 2, 2 ) = elementOriginY;
      geometryMatrix ( 2, 3 ) = elementOriginY + deltaX;
      geometryMatrix ( 2, 4 ) = elementOriginY + deltaX;

      geometryMatrix ( 2, 5 ) = elementOriginY;
      geometryMatrix ( 2, 6 ) = elementOriginY;
      geometryMatrix ( 2, 7 ) = elementOriginY + deltaX;
      geometryMatrix ( 2, 8 ) = elementOriginY + deltaX;

   } else {
      geometryMatrix.zero ();
   }

}

void DataAccess::Mining::getGeometryMatrix ( const ElementPosition&                      element,
                                             const GeoPhysics::CauldronGridDescription&  grid,
                                             const FiniteElementMethod::ElementVector&   depth,
                                             FiniteElementMethod::ElementGeometryMatrix& geometryMatrix ) {

   if ( depth ( 1 ) != Interface::DefaultUndefinedMapValue ) {
      int i;

      double deltaX  = grid.deltaI;
      double deltaY  = grid.deltaJ;
      double elementOriginX = grid.originI + double ( element.getI ()) * deltaX;
      double elementOriginY = grid.originJ + double ( element.getJ ()) * deltaY;

      for ( i = 1; i <= 8; ++i ) {
         geometryMatrix ( 3, i ) = depth ( i );
      }

      geometryMatrix ( 1, 1 ) = elementOriginX;
      geometryMatrix ( 1, 2 ) = elementOriginX + deltaX;
      geometryMatrix ( 1, 3 ) = elementOriginX + deltaX;
      geometryMatrix ( 1, 4 ) = elementOriginX;

      geometryMatrix ( 1, 5 ) = elementOriginX;
      geometryMatrix ( 1, 6 ) = elementOriginX + deltaX;
      geometryMatrix ( 1, 7 ) = elementOriginX + deltaX;
      geometryMatrix ( 1, 8 ) = elementOriginX;

      geometryMatrix ( 2, 1 ) = elementOriginY;
      geometryMatrix ( 2, 2 ) = elementOriginY;
      geometryMatrix ( 2, 3 ) = elementOriginY + deltaX;
      geometryMatrix ( 2, 4 ) = elementOriginY + deltaX;

      geometryMatrix ( 2, 5 ) = elementOriginY;
      geometryMatrix ( 2, 6 ) = elementOriginY;
      geometryMatrix ( 2, 7 ) = elementOriginY + deltaX;
      geometryMatrix ( 2, 8 ) = elementOriginY + deltaX;

   } else {
      geometryMatrix.zero ();
   }


}

void DataAccess::Mining::getGeometryMatrix ( const ElementPosition&                      element,
                                             const Interface::GridMap*                   depth,
                                             FiniteElementMethod::ElementGeometryMatrix& geometryMatrix ) {

   FiniteElementMethod::ElementVector depthValues;
   getElementCoefficients ( element, depth, depthValues );
   getGeometryMatrix ( element, depth->getGrid (), depthValues, geometryMatrix );

}

void DataAccess::Mining::getElementCoefficients ( const unsigned int                  iStart,
                                                  const unsigned int                  jStart,
                                                  const unsigned int                  kStart,
                                                  const Interface::GridMap*           property,
                                                  FiniteElementMethod::ElementVector& coefficients ) {

   int l;
   bool valueIsNull = false;

   coefficients ( 1 ) = property->getValue ( iStart,     jStart,     kStart );
   coefficients ( 2 ) = property->getValue ( iStart + 1, jStart,     kStart );
   coefficients ( 3 ) = property->getValue ( iStart + 1, jStart + 1, kStart );
   coefficients ( 4 ) = property->getValue ( iStart,     jStart + 1, kStart );

   coefficients ( 5 ) = property->getValue ( iStart,     jStart,     kStart - 1 );
   coefficients ( 6 ) = property->getValue ( iStart + 1, jStart,     kStart - 1 );
   coefficients ( 7 ) = property->getValue ( iStart + 1, jStart + 1, kStart - 1 );
   coefficients ( 8 ) = property->getValue ( iStart,     jStart + 1, kStart - 1 );
 
   for ( l = 1; l <= 8; ++l ) {

      if ( coefficients ( l ) == DataAccess::Interface::DefaultUndefinedMapValue ) {
         valueIsNull = true;
         break;
      }

   }

   // If one value is null then set all values to be null.
   if ( valueIsNull ) {

      for ( l = 1; l <= 8; ++l ) {
         coefficients ( l ) = DataAccess::Interface::DefaultUndefinedMapValue;
      }

   }

}

void DataAccess::Mining::getElementCoefficients ( const ElementPosition&              element,
                                                  const Interface::GridMap*           property,
                                                  FiniteElementMethod::ElementVector& coefficients ) {

   getElementCoefficients ( element.getI (), element.getJ (), element.getLocalK (), property, coefficients );
}


void DataAccess::Mining::getGeometryMatrix ( const ElementPosition&                      element,
                                             FormationPropertyPtr                        depth,
                                             FiniteElementMethod::ElementGeometryMatrix& geometryMatrix ) {

   FiniteElementMethod::ElementVector depthValues;
   getElementCoefficients ( element, depth, depthValues );
   getGeometryMatrix ( element, depth->getGrid (), depthValues, geometryMatrix );
}


void DataAccess::Mining::getElementCoefficients ( const unsigned int                      iStart,
                                                  const unsigned int                      jStart,
                                                  const unsigned int                      kStart,
                                                  FormationPropertyPtr                    property,
                                                  FiniteElementMethod::ElementVector&     coefficients ) {

   int l;
   bool valueIsNull = false;

   coefficients ( 1 ) = property->get ( iStart,     jStart,     kStart );
   coefficients ( 2 ) = property->get ( iStart + 1, jStart,     kStart );
   coefficients ( 3 ) = property->get ( iStart + 1, jStart + 1, kStart );
   coefficients ( 4 ) = property->get ( iStart,     jStart + 1, kStart );

   coefficients ( 5 ) = property->get ( iStart,     jStart,     kStart - 1 );
   coefficients ( 6 ) = property->get ( iStart + 1, jStart,     kStart - 1 );
   coefficients ( 7 ) = property->get ( iStart + 1, jStart + 1, kStart - 1 );
   coefficients ( 8 ) = property->get ( iStart,     jStart + 1, kStart - 1 );

   for ( l = 1; l <= 8; ++l ) {

      if ( coefficients ( l ) == DataAccess::Interface::DefaultUndefinedMapValue ) {
         valueIsNull = true;
         break;
      }

   }

   // If one value is null then set all values to be null.
   if ( valueIsNull ) {

      for ( l = 1; l <= 8; ++l ) {
         coefficients ( l ) = DataAccess::Interface::DefaultUndefinedMapValue;
      }

   }

}

void DataAccess::Mining::getElementCoefficients ( const ElementPosition&                  element,
                                                  FormationPropertyPtr                    property,
                                                  FiniteElementMethod::ElementVector&     coefficients ) {

   getElementCoefficients ( element.getI (), element.getJ (), element.getLocalK (), property, coefficients );

}
