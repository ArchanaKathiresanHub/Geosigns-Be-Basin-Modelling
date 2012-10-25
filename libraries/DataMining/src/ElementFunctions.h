#ifndef _DATA_MINING__ELEMENT_FUNCTIONS__H_
#define _DATA_MINING__ELEMENT_FUNCTIONS__H_

#include "ElementPosition.h"
#include "FiniteElementTypes.h"
#include "Interface/GridMap.h"
#include "CauldronGridDescription.h"

namespace DataAccess {

   namespace Mining {

      /// \brief Assembles the element geometry matrix.
      void getGeometryMatrix ( const ElementPosition&                      element,
                               const Interface::Grid*                      grid,
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

   }

}

#endif // _DATA_MINING__ELEMENT_FUNCTIONS__H_
