//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FINITE_ELEMENT_METHOD__JACOBIAN_STORAGE__H
#define FINITE_ELEMENT_METHOD__JACOBIAN_STORAGE__H

#include "FiniteElementTypes.h"
#include "FiniteElementArrayTypes.h"
#include "AlignedDenseMatrix.h"

namespace FiniteElementMethod {

   /// \brief Storage for an array of Jacobian matrices, their inverses and determinants.
   ///
   /// The Jacobian is a 3x3 matrix but some of the values are zero ( j(1,2), j(1,3), j(2,1) and j(2,3))
   /// some other values are the same for all quadrature points ( j(1,1) and j(2,2)).
   /// So the values that vary are in the third row of the Jacobian.
   /// The same is also true for its inverse.
   /// Computation of the Jacobian, its inverse and the determinant can be optimised.
   /// Also operations using the Jacobian and its inverse can be optimised.
   ///
   ///     [a 0 0]
   /// J = [0 b 0]
   ///     [c d e]
   ///
   ///        [1/a   0      0  ]
   /// J^-1 = [0     1/b    0  ]
   ///        [-c/ae -d/be  1/e]
   ///
   /// \$f \det ( J ) = a \cdot b \cdot e \$f
   class JacobianStorage {
   public :

      JacobianStorage ( const int numberOfQuadraturePoints );

      /// \brief Set the values for a particular element.
      ///
      /// The Jacobian is compute are \$f GM \cdot \nabla \phi \$f.
      /// Only the last row need be computed, so only the last row of the geometry matrix
      /// is passed, which is the depths of the nodes of the element.
      void set ( const double                        deltaX,
                 const double                        deltaY,
                 const ElementVector&                zValues,
                 const Numerics::AlignedDenseMatrix& gradBases );

      /// \brief Get the number of quadrature points.
      unsigned int size () const;

      /// \brief Get the (constant) value for the (1,1) position in the Jacobian.
      double getJacobian11 () const;

      /// \brief Get the (constant) value for the (2,2) position in the Jacobian.
      double getJacobian22 () const;

      /// \brief Get the values for the third row of the Jacobian for the quadrature point i.
      const ThreeVector& getJacobian3 ( const int i ) const;

      /// \brief Get the (constant) value for the (1,1) position in the Jacobian.
      double getInverseJacobian11 () const;

      /// \brief Get the (constant) value for the (2,2) position in the Jacobian.
      double getInverseJacobian22 () const;

      /// \brief Get the values for the third row of the Jacobian for the quadrature point i.
      const ThreeVector& getInverseJacobian3 ( const int i ) const;

      /// \brief Get the determinant of the determinant of the Jacobian for the quadrature point i.
      double getDeterminant ( const int i ) const;

   private :

      /// \briief Remove the possibility of copy construction.
      JacobianStorage ( const JacobianStorage& copy ) = delete;

      /// \briief Remove the possibility of copy construction.
      JacobianStorage& operator=( const JacobianStorage& copy ) = delete;

      ArrayOfVector3 m_jacobian3rdRow;
      ArrayOfVector3 m_inverseJacobian3rdRow;
      std::vector<double> m_determinants;
      double m_jacobian11;
      double m_jacobian22;
      double m_inverseJacobian11;
      double m_inverseJacobian22;

   };

}

inline unsigned int FiniteElementMethod::JacobianStorage::size () const {
   return m_determinants.size ();
}

inline double FiniteElementMethod::JacobianStorage::getJacobian11 () const {
   return m_jacobian11;
}

inline double FiniteElementMethod::JacobianStorage::getJacobian22 () const {
   return m_jacobian22;
}

inline const FiniteElementMethod::ThreeVector& FiniteElementMethod::JacobianStorage::getJacobian3 ( const int i ) const {
   return m_jacobian3rdRow [ i ];
}

inline double FiniteElementMethod::JacobianStorage::getInverseJacobian11 () const {
   return m_inverseJacobian11;
}

inline double FiniteElementMethod::JacobianStorage::getInverseJacobian22 () const {
   return m_inverseJacobian22;
}

inline const FiniteElementMethod::ThreeVector& FiniteElementMethod::JacobianStorage::getInverseJacobian3 ( const int i ) const{
   return m_inverseJacobian3rdRow [ i ];
}

inline double FiniteElementMethod::JacobianStorage::getDeterminant ( const int i ) const {
   return m_determinants [ i ];
}

#endif // FINITE_ELEMENT_METHOD__JACOBIAN_STORAGE__H
