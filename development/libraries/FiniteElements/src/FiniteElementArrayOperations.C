//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "FiniteElementArrayOperations.h"

void FiniteElementMethod::ArrayOperations::computeGradProperty ( const Numerics::AlignedDenseMatrix& gradBases,
                                                                 const ElementVector&                propertyCoefficients,
                                                                 ArrayOfVector3&                     gradProperty ) {

   const double* coef = propertyCoefficients.data ();
   const double* gp   = gradBases.data ();
   unsigned int stride = gradBases.rows () * 3; // = 8 * 3

   for ( unsigned int i = 0; i < gradProperty.size (); ++i, gp += stride ) {
      gradProperty [ i ]( 1 ) = gp [  0 ] * coef [ 0 ] + gp [  1 ] * coef [  1 ] + gp [  2 ] * coef [ 2 ] + gp [  3 ] * coef [ 3 ] +
                                gp [  4 ] * coef [ 4 ] + gp [  5 ] * coef [  5 ] + gp [  6 ] * coef [ 6 ] + gp [  7 ] * coef [ 7 ];
      gradProperty [ i ]( 2 ) = gp [  8 ] * coef [ 0 ] + gp [  9 ] * coef [  1 ] + gp [ 10 ] * coef [ 2 ] + gp [ 11 ] * coef [ 3 ] +
                                gp [ 12 ] * coef [ 4 ] + gp [ 13 ] * coef [  5 ] + gp [ 14 ] * coef [ 6 ] + gp [ 15 ] * coef [ 7 ];
      gradProperty [ i ]( 3 ) = gp [ 16 ] * coef [ 0 ] + gp [ 17 ] * coef [  1 ] + gp [ 18 ] * coef [ 2 ] + gp [ 19 ] * coef [ 3 ] +
                                gp [ 20 ] * coef [ 4 ] + gp [ 21 ] * coef [  5 ] + gp [ 22 ] * coef [ 6 ] + gp [ 23 ] * coef [ 7 ];
   }

}

void FiniteElementMethod::ArrayOperations::computeGradProperty ( const Numerics::AlignedDenseMatrix& gradBases,
                                                                 const ElementVector&                propertyCoefficients1,
                                                                 const ElementVector&                propertyCoefficients2,
                                                                 ArrayOfVector3&                     gradProperty1,
                                                                 ArrayOfVector3&                     gradProperty2 ) {

   const double* coef1 = propertyCoefficients1.data ();
   const double* coef2 = propertyCoefficients2.data ();
   const double* gp    = gradBases.data ();
   unsigned int stride = gradBases.rows () * 3;

   for ( unsigned int i = 0; i < gradProperty1.size (); ++i, gp += stride ) {
      gradProperty1 [ i ]( 1 ) = gp [  0 ] * coef1 [ 0 ] + gp [  1 ] * coef1 [  1 ] + gp [  2 ] * coef1 [ 2 ] + gp [  3 ] * coef1 [ 3 ] +
                                 gp [  4 ] * coef1 [ 4 ] + gp [  5 ] * coef1 [  5 ] + gp [  6 ] * coef1 [ 6 ] + gp [  7 ] * coef1 [ 7 ];
      gradProperty1 [ i ]( 2 ) = gp [  8 ] * coef1 [ 0 ] + gp [  9 ] * coef1 [  1 ] + gp [ 10 ] * coef1 [ 2 ] + gp [ 11 ] * coef1 [ 3 ] +
                                 gp [ 12 ] * coef1 [ 4 ] + gp [ 13 ] * coef1 [  5 ] + gp [ 14 ] * coef1 [ 6 ] + gp [ 15 ] * coef1 [ 7 ];
      gradProperty1 [ i ]( 3 ) = gp [ 16 ] * coef1 [ 0 ] + gp [ 17 ] * coef1 [  1 ] + gp [ 18 ] * coef1 [ 2 ] + gp [ 19 ] * coef1 [ 3 ] +
                                 gp [ 20 ] * coef1 [ 4 ] + gp [ 21 ] * coef1 [  5 ] + gp [ 22 ] * coef1 [ 6 ] + gp [ 23 ] * coef1 [ 7 ];

      gradProperty2 [ i ]( 1 ) = gp [  0 ] * coef2 [ 0 ] + gp [  1 ] * coef2 [  1 ] + gp [  2 ] * coef2 [ 2 ] + gp [  3 ] * coef2 [ 3 ] +
                                 gp [  4 ] * coef2 [ 4 ] + gp [  5 ] * coef2 [  5 ] + gp [  6 ] * coef2 [ 6 ] + gp [  7 ] * coef2 [ 7 ];
      gradProperty2 [ i ]( 2 ) = gp [  8 ] * coef2 [ 0 ] + gp [  9 ] * coef2 [  1 ] + gp [ 10 ] * coef2 [ 2 ] + gp [ 11 ] * coef2 [ 3 ] +
                                 gp [ 12 ] * coef2 [ 4 ] + gp [ 13 ] * coef2 [  5 ] + gp [ 14 ] * coef2 [ 6 ] + gp [ 15 ] * coef2 [ 7 ];
      gradProperty2 [ i ]( 3 ) = gp [ 16 ] * coef2 [ 0 ] + gp [ 17 ] * coef2 [  1 ] + gp [ 18 ] * coef2 [ 2 ] + gp [ 19 ] * coef2 [ 3 ] +
                                 gp [ 20 ] * coef2 [ 4 ] + gp [ 21 ] * coef2 [  5 ] + gp [ 22 ] * coef2 [ 6 ] + gp [ 23 ] * coef2 [ 7 ];

   }

}

void FiniteElementMethod::ArrayOperations::scaleBases ( const Numerics::AlignedDenseMatrix& bases,
                                                        const ArrayDefs::Real_ptr&          scalarValues,
                                                        Numerics::AlignedDenseMatrix&       scaledBases ) {
   const double* phi = bases.data ();
   double* res = scaledBases.data ();

   for ( unsigned int i = 0; i < bases.cols (); ++i, phi += 8, res += 8 ) {
      res [ 0 ] = scalarValues [ i ] * phi [ 0 ];
      res [ 1 ] = scalarValues [ i ] * phi [ 1 ];
      res [ 2 ] = scalarValues [ i ] * phi [ 2 ];
      res [ 3 ] = scalarValues [ i ] * phi [ 3 ];
      res [ 4 ] = scalarValues [ i ] * phi [ 4 ];
      res [ 5 ] = scalarValues [ i ] * phi [ 5 ];
      res [ 6 ] = scalarValues [ i ] * phi [ 6 ];
      res [ 7 ] = scalarValues [ i ] * phi [ 7 ];
   }

}

void FiniteElementMethod::ArrayOperations::scaleGradBases ( const Numerics::AlignedDenseMatrix& gradBases,
                                                            const ArrayOfMatrix3x3&             matrices,
                                                            Numerics::AlignedDenseMatrix&       scaledGradBases ) {

   const double* gp = gradBases.data ();
   double* res = scaledGradBases.data ();

   for ( unsigned int i = 0; i < matrices.size (); ++i, gp += 24, res += 24  ) {
      const Matrix3x3& m = matrices [ i ];

      for ( unsigned int j = 0; j < 8; j += 2 ) {
         res [ j      ] = gp [ j     ] * m ( 1, 1 ) + gp [ j + 8 ] * m ( 2, 1 ) + gp [ j + 16 ] * m ( 3, 1 );
         res [ j +  1 ] = gp [ j + 1 ] * m ( 1, 1 ) + gp [ j + 9 ] * m ( 2, 1 ) + gp [ j + 17 ] * m ( 3, 1 );

         res [ j +  8 ] = gp [ j     ] * m ( 1, 2 ) + gp [ j + 8 ] * m ( 2, 2 ) + gp [ j + 16 ] * m ( 3, 2 );
         res [ j +  9 ] = gp [ j + 1 ] * m ( 1, 2 ) + gp [ j + 9 ] * m ( 2, 2 ) + gp [ j + 17 ] * m ( 3, 2 );


         res [ j + 16 ] = gp [ j     ] * m ( 1, 3 ) + gp [ j + 8 ] * m ( 2, 3 ) + gp [ j + 16 ] * m ( 3, 3 );
         res [ j + 17 ] = gp [ j + 1 ] * m ( 1, 3 ) + gp [ j + 9 ] * m ( 2, 3 ) + gp [ j + 17 ] * m ( 3, 3 );
      }

   }

}

void FiniteElementMethod::ArrayOperations::scaleGradBases ( const Numerics::AlignedDenseMatrix& gradBases,
                                                            const ArrayOfVector3&               vectors,
                                                            Numerics::AlignedDenseMatrix&       result ) {

   const double* gp = gradBases.data ();
   double* res = result.data ();

   for ( unsigned int i = 0; i < vectors.size (); ++i, gp += 24, res += 8 ) {
      const ThreeVector& v = vectors [ i ];

#ifdef OPTIMISE_GRAD_BASIS_VECTOR
      for ( unsigned int j = 0; j < 8; j += 4 ) {
         res [ j     ] = gp [ j     ] * v ( 1 ) + gp [ j +  8 ] * v ( 2 ) + gp [ j + 16 ] * v ( 3 );
         res [ j + 1 ] = gp [ j + 1 ] * v ( 1 ) + gp [ j +  9 ] * v ( 2 ) + gp [ j + 17 ] * v ( 3 );
         res [ j + 2 ] = gp [ j + 2 ] * v ( 1 ) + gp [ j + 10 ] * v ( 2 ) + gp [ j + 18 ] * v ( 3 );
         res [ j + 3 ] = gp [ j + 3 ] * v ( 1 ) + gp [ j + 11 ] * v ( 2 ) + gp [ j + 19 ] * v ( 3 );
      }
#else
      for ( unsigned int j = 0; j < 8; ++j ) {
         res [ j ] = gp [ j ] * v ( 1 ) + gp [ j + 8 ] * v ( 2 ) + gp [ j + 16 ] * v ( 3 );
      }
#endif

   }

}

void FiniteElementMethod::ArrayOperations::productWithInverse ( const JacobianStorage& jacobians,
                                                                      ArrayOfVector3&  results ) {

   double jac11 = jacobians.getInverseJacobian11 ();
   double jac22 = jacobians.getInverseJacobian22 ();

   for ( unsigned int i = 0; i < jacobians.size (); ++i ) {
      const ThreeVector& jacRow3 = jacobians.getInverseJacobian3 ( i );
      ThreeVector v = results [ i ];
      results [ i ]( 1 ) = jac11 * v ( 1 ) + jacRow3 ( 1 ) * v ( 3 );
      results [ i ]( 2 ) = jac22 * v ( 2 ) + jacRow3 ( 2 ) * v ( 3 );
      results [ i ]( 3 ) = jacRow3 ( 3 ) * v ( 3 );
   }

}
