//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _DATA_MINING__ELEMENT_FUNCTIONS__H_
#define _DATA_MINING__ELEMENT_FUNCTIONS__H_

#include "AbstractGrid.h"

#include "FormationProperty.h"

#include "ElementPosition.h"
#include "FiniteElementTypes.h"
#include "GridMap.h"
#include "CauldronGridDescription.h"

namespace DataAccess {

   namespace Mining {

      /// \brief Assembles the element geometry matrix.
      void getGeometryMatrix ( const ElementPosition&                      element,
                               const DataModel::AbstractGrid*              grid,
                               const FiniteElementMethod::ElementVector&   depth,
                               FiniteElementMethod::ElementGeometryMatrix& geometryMatrix );

      /// \brief Assembles the element geometry matrix.
      void getGeometryMatrix ( const ElementPosition&                      element,
                               const GeoPhysics::CauldronGridDescription&  grid,
                               const FiniteElementMethod::ElementVector&   depth,
                               FiniteElementMethod::ElementGeometryMatrix& geometryMatrix );

      /// \brief Assembles the element geometry matrix.
      void getGeometryMatrix ( const ElementPosition&                      element,
                               const Interface::GridMap*                   depth,
                               FiniteElementMethod::ElementGeometryMatrix& geometryMatrix );

      /// \brief Extracts the coefficients of the property for the element.
      void getElementCoefficients ( const ElementPosition&              element,
                                    const Interface::GridMap*           property,
                                    FiniteElementMethod::ElementVector& coefficients );

      /// \brief Extracts the coefficients of the property for the element.
      void getElementCoefficients ( const unsigned int                  iStart,
                                    const unsigned int                  jStart,
                                    const unsigned int                  kStart,
                                    const Interface::GridMap*           property,
                                    FiniteElementMethod::ElementVector& coefficients );



      /// \brief Assembles the element geometry matrix.
      void getGeometryMatrix ( const ElementPosition&                          element,
                               AbstractDerivedProperties::FormationPropertyPtr depth,
                               FiniteElementMethod::ElementGeometryMatrix&     geometryMatrix );


      // /// \brief Assembles the element geometry matrix.
      // void getGeometryMatrix ( const ElementPosition&                      element,
                               // const DataModel::AbstractGrid*              grid,
                               // const FiniteElementMethod::ElementVector&   depth,
                               // FiniteElementMethod::ElementGeometryMatrix& geometryMatrix );


      /// \brief Extracts the coefficients of the property for the element.
      void getElementCoefficients ( const ElementPosition&                          element,
                                    AbstractDerivedProperties::FormationPropertyPtr grid,
                                    FiniteElementMethod::ElementVector&             coefficients );

      /// \brief Extracts the coefficients of the property for the element.
      void getElementCoefficients ( const unsigned int                              iStart,
                                    const unsigned int                              jStart,
                                    const unsigned int                              kStart,
                                    AbstractDerivedProperties::FormationPropertyPtr grid,
                                    FiniteElementMethod::ElementVector&             coefficients );


   }

}

#endif // _DATA_MINING__ELEMENT_FUNCTIONS__H_
