//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FINITE_ELEMENT_METHOD__FEM_ARRAY_OPERATIONS_H
#define FINITE_ELEMENT_METHOD__FEM_ARRAY_OPERATIONS_H

#include "FiniteElementArrayTypes.h"
#include "AlignedDenseMatrix.h"
#include "JacobianStorage.h"

namespace FiniteElementMethod {

   /// \brief Contains a set of operations on arrays of matrices and vectors.
   struct ArrayOperations {

      /// \brief Multiply each of the grad bases by the Jacobian inverse (the 3x3 matrix).
      static void scaleGradBases ( const Numerics::AlignedDenseMatrix& gradBases,
                                   const ArrayOfMatrix3x3&             matrices,
                                   Numerics::AlignedDenseMatrix&       scaledGradBases );

      static void scaleGradBases ( const Numerics::AlignedDenseMatrix& gradBases,
                                   const ArrayOfVector3&               vectors,
                                   Numerics::AlignedDenseMatrix&       result );

      /// \brief Scale each basis function by a scalar value.
      static void scaleBases ( const Numerics::AlignedDenseMatrix& bases,
                               const ArrayDefs::Real_ptr&          scalarValues,
                               Numerics::AlignedDenseMatrix&       scaledBases );

      /// \brief Compute the gradient of a single property.
      static void computeGradProperty ( const Numerics::AlignedDenseMatrix& gradBases,
                                        const ElementVector&                propertyCoefficients,
                                        ArrayOfVector3&                     gradProperty );

      /// \brief Compute the gradient of a two properties.
      static void computeGradProperty ( const Numerics::AlignedDenseMatrix& gradBases,
                                        const ElementVector&                propertyCoefficients1,
                                        const ElementVector&                propertyCoefficients2,
                                        ArrayOfVector3&                     gradProperty1,
                                        ArrayOfVector3&                     gradProperty2 );

      /// \brief Inplace multiply a vector by the transpose of the matrix.
      static void productWithInverse ( const JacobianStorage& jacobians,
                                             ArrayOfVector3&  results );

   };

}


#endif // FINITE_ELEMENT_METHOD__FEM_ARRAY_OPERATIONS_H
