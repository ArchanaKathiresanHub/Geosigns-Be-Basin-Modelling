//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FINITE_ELEMENT_METHOD__FEM_ARRAY_TYPES_H
#define FINITE_ELEMENT_METHOD__FEM_ARRAY_TYPES_H

#include <vector>
#include "FiniteElementTypes.h"
#include "AlignedDenseMatrix.h"

namespace FiniteElementMethod {

   /// \brief Array of $\f 3 \times 3 \$f matrices.
   class ArrayOfMatrix3x3 {
   public :

      /// \brief Construct with the number of matrices required.
      ArrayOfMatrix3x3 ( const unsigned int numberOfMats );

      /// \brief Return the number of matrices stored.
      unsigned int size () const;

      /// \brief Get a reference to the matrix at position i.
      Matrix3x3& operator []( const unsigned int i );

      /// \brief Get a constant reference to the matrix at position i.
      const Matrix3x3& operator []( const unsigned int i ) const;

   private :

      std::vector<Matrix3x3> m_allMatrices;

   };

   /// \brief Array of vectors of size 3.
   class ArrayOfVector3 {

   public :

      /// \brief Construct with the number of vectors required.
      ArrayOfVector3 ( const unsigned int numberOfVecs );

      /// \brief Return the number of vectors stored.
      unsigned int size () const;

      /// \brief Get a reference to the vector at position i.
      ThreeVector& operator []( const unsigned int i );

      /// \brief Get a constant reference to the vector at position i.
      const ThreeVector& operator []( const unsigned int i ) const;

   private :

      std::vector<ThreeVector> m_allVectors;

   };

} // namespace FiniteElementMethod

//--------------------------------
// Inline functions

inline unsigned int FiniteElementMethod::ArrayOfMatrix3x3::size () const {
   return m_allMatrices.size ();
}

inline FiniteElementMethod::Matrix3x3& FiniteElementMethod::ArrayOfMatrix3x3::operator []( const unsigned int i ) {
   return m_allMatrices [ i ];
}

inline const FiniteElementMethod::Matrix3x3& FiniteElementMethod::ArrayOfMatrix3x3::operator []( const unsigned int i ) const {
   return m_allMatrices [ i ];
}

inline unsigned int FiniteElementMethod::ArrayOfVector3::size () const {
   return m_allVectors.size ();
}

inline FiniteElementMethod::ThreeVector& FiniteElementMethod::ArrayOfVector3::operator []( const unsigned int i ) {
   return m_allVectors [ i ];
}

inline const FiniteElementMethod::ThreeVector& FiniteElementMethod::ArrayOfVector3::operator []( const unsigned int i ) const {
   return m_allVectors [ i ];
}


#endif // FINITE_ELEMENT_METHOD__FEM_ARRAY_TYPES_H
