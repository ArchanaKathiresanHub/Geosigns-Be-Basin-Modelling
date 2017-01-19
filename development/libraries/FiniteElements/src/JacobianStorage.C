//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "JacobianStorage.h"
#include "FiniteElementArrayTypes.h"
#include "ConstantsNumerical.h"

FiniteElementMethod::JacobianStorage::JacobianStorage ( const int numberOfQuadraturePoints ) :
   m_jacobian3rdRow ( numberOfQuadraturePoints ),
   m_inverseJacobian3rdRow ( numberOfQuadraturePoints ),
   m_determinants ( numberOfQuadraturePoints ),
   m_jacobian11 ( Utilities::Numerical::CauldronNoDataValue ),
   m_jacobian22 ( Utilities::Numerical::CauldronNoDataValue ),
   m_inverseJacobian11 ( Utilities::Numerical::CauldronNoDataValue ),
   m_inverseJacobian22 ( Utilities::Numerical::CauldronNoDataValue )
{
   // Nothing else to do.
}


void FiniteElementMethod::JacobianStorage::set ( const double                        deltaX,
                                                 const double                        deltaY,
                                                 const ElementVector&                zValues,
                                                 const Numerics::AlignedDenseMatrix& gradBases ) {
   m_jacobian11 = 0.5 * deltaX;
   m_jacobian22 = 0.5 * deltaY;
   m_inverseJacobian11 = 1.0 / m_jacobian11;
   m_inverseJacobian22 = 1.0 / m_jacobian22;

   // ArrayOperations::computeGradProperty ( gradBases, zValues, m_jacobian3rdRow );

   const double* coef = zValues.data ();
   const double* gp   = gradBases.data ();
   const unsigned int stride = gradBases.rows () * 3; // = 8 * 3

   for ( unsigned int i = 0; i < m_jacobian3rdRow.size (); ++i, gp += stride ) {
      m_jacobian3rdRow [ i ]( 1 ) = gp [  0 ] * coef [ 0 ] + gp [  1 ] * coef [  1 ] + gp [  2 ] * coef [ 2 ] + gp [  3 ] * coef [ 3 ] +
                                    gp [  4 ] * coef [ 4 ] + gp [  5 ] * coef [  5 ] + gp [  6 ] * coef [ 6 ] + gp [  7 ] * coef [ 7 ];
      m_jacobian3rdRow [ i ]( 2 ) = gp [  8 ] * coef [ 0 ] + gp [  9 ] * coef [  1 ] + gp [ 10 ] * coef [ 2 ] + gp [ 11 ] * coef [ 3 ] +
                                    gp [ 12 ] * coef [ 4 ] + gp [ 13 ] * coef [  5 ] + gp [ 14 ] * coef [ 6 ] + gp [ 15 ] * coef [ 7 ];
      m_jacobian3rdRow [ i ]( 3 ) = gp [ 16 ] * coef [ 0 ] + gp [ 17 ] * coef [  1 ] + gp [ 18 ] * coef [ 2 ] + gp [ 19 ] * coef [ 3 ] +
                                    gp [ 20 ] * coef [ 4 ] + gp [ 21 ] * coef [  5 ] + gp [ 22 ] * coef [ 6 ] + gp [ 23 ] * coef [ 7 ];
   }


   // Now compute the inverse of the Jacobian matrices.
   double prodj11j22 = m_jacobian11 * m_jacobian22;

   for ( size_t i = 0; i < m_jacobian3rdRow.size (); ++i ) {
      double j33 = m_jacobian3rdRow [ i ]( 3 );
      double invJ33 = 1.0 / j33;

      m_determinants [ i ] = prodj11j22 * j33;
      m_inverseJacobian3rdRow [ i ]( 1 ) = -m_jacobian3rdRow [ i ]( 1 ) * m_inverseJacobian11 * invJ33;
      m_inverseJacobian3rdRow [ i ]( 2 ) = -m_jacobian3rdRow [ i ]( 2 ) * m_inverseJacobian22 * invJ33;
      m_inverseJacobian3rdRow [ i ]( 3 ) = invJ33;
   }

}
