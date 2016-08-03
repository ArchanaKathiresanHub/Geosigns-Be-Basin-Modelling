//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _FINITE_ELEMENT_METHOD__BASIS_FUNCTION_CACHE_H_
#define _FINITE_ELEMENT_METHOD__BASIS_FUNCTION_CACHE_H_

#include "FiniteElementTypes.h"
#include "AlignedDenseMatrix.h"

namespace FiniteElementMethod {
   
   /// Has matrices containing the basis functions and grad-basis function evaluated at all of the quadrature points.
   ///
   /// The quickest varying direction is the z, followed by y and the slowest is the x direction.
   ///
   /// The basis functions will be stored column-wise.
   /// \f[ P_{i,j} = \phi_i(\xi_j) \f]
   ///
   /// The grad basis functions will be stored column-wise.
   /// \f[ GP_{i,3j..3j+2} = \frac{\partial \phi_i(\xi_j)}{\partial x},
   ///                       \frac{\partial \phi_i(\xi_j)}{\partial y},
   ///                       \frac{\partial \phi_i(\xi_j)}{\partial z} \f]
   /// GP(i,3j..3j+2) = (phi_x,i,j, phi_y,i,j, phi_z,i,j)
   class BasisFunctionCache {

   public :

      /// Constructor.
      ///
      /// The x-, y- and z-point-count are the number of quadrature points in each direction.
      BasisFunctionCache  ( const int xPointCount,
                            const int yPointCount,
                            const int zPointCount );

      /// Get the number of quadrature points in the x-direction.
      int getNumberOfPointsX () const;

      /// Get the number of quadrature points in the y-direction.
      int getNumberOfPointsY () const;

      /// Get the number of quadrature points in the z-direction.
      int getNumberOfPointsZ () const;

      /// The total number of quadrature points.
      int getNumberOfQuadraturePoints () const;

      /// Get the matrix containing the basis functions.
      const Numerics::AlignedDenseMatrix & getBasisFunctions () const;

      /// Get the matrix containing the grad-basis functions.
      const Numerics::AlignedDenseMatrix & getGradBasisFunctions () const;
      

   private :

      /// A matrix holding all the basis functions evaluated at each Gaussian quadrature point.
      Numerics::AlignedDenseMatrix m_basisFunctions;

      /// A matrix holding all the gradient of the basis functions evaluated at each Gaussian quadrature point.
      Numerics::AlignedDenseMatrix m_gradBasisFunctions;

      /// Number of quadrature points in the x-direction.
      int m_numberOfPointsX;

      /// Number of quadrature points in the y-direction.
      int m_numberOfPointsY;

      /// Number of quadrature points in the z-direction.
      int m_numberOfPointsZ;

      /// Total number of quadrature points.
      int m_numberOfQuadraturePoints;

   };

} // end namespace

inline int FiniteElementMethod::BasisFunctionCache::getNumberOfPointsX () const {
   return m_numberOfPointsX;
}

inline int FiniteElementMethod::BasisFunctionCache::getNumberOfPointsY () const {
   return m_numberOfPointsY;
}

inline int FiniteElementMethod::BasisFunctionCache::getNumberOfPointsZ () const {
   return m_numberOfPointsZ;
}

inline int FiniteElementMethod::BasisFunctionCache::getNumberOfQuadraturePoints () const {
   return m_numberOfQuadraturePoints;
}

inline const Numerics::AlignedDenseMatrix & FiniteElementMethod::BasisFunctionCache::getBasisFunctions () const {
   return m_basisFunctions;
} 

inline const  Numerics::AlignedDenseMatrix& FiniteElementMethod::BasisFunctionCache::getGradBasisFunctions () const {
   return m_gradBasisFunctions;
} 

#endif // _FINITE_ELEMENT_METHOD__BASIS_FUNCTION_CACHE_H_
