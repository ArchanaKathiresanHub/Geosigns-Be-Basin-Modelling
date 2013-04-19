// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef EOSUTILS_H
#define EOSUTILS_H

///
/// \brief Linear algebra utility routines
///
class EosLinAlg
{
public:
   //--------------------------------------------------------------------// 
   // Linear solver routines 
   //--------------------------------------------------------------------// 

   // Performs the back substitution on a factored matrix
   static void BackSolve( int iM, int iNc, double *pMatrix, double *pRhs );

   // Performs a modified Cholesky decomposition
   static void Cholesky( int iM, int iNc, double dTiny, double *pMatrix,
      double *pWork1, double *pWork2 );
};


///
/// \brief Mathematical functions of general utility
///
class EosUtils
{
public:
   // Vector exponential function
   static void VectorExp( int iM, double *pInput, double *pOutput );

   // Vector log function
   static void VectorLog( int iM, double *pInput, double *pOutput );

   // Vector power function
   static void VectorPow( int iM, double *pInput1, double dInput2, double *pOutput );

   // Vector power function
   static void VectorPowX( int iM, double *pInput1, double *pInput2, double *pOutput );
};

#endif
