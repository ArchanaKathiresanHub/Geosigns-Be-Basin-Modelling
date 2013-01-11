// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <math.h>

#include "EosUtils.h"

/* Use intel math library */
// #define EOS_INTEL
#ifdef EOS_INTEL
#include "C:/Program Files/Intel/MKL61/include/mkl.h"
#endif

/// 
/// \brief Back substitution routine  
///
/// \param iM  Number of objects
/// \param iNc Number of components
/// \param pMatrix Pointer to matrix.  The array order is 
///         Objects (first)
///         Columns
///         Rows (last)
///    Only the diagonal and bottom subdiagonal elements are used
///
/// \param pRhs Pointer to the right hand side. The array order is
///             Objects (first)
///             Columns (last) 
//
// 1) This routine assumes that Cholesky has first been called
// 2) This routine solves an equation of the form
//    
//          T
//       L L  S = R
// 
//    by solving, in order
//
//       ^    -1               -T ^
//       R = L   R        R = L   R
//
//    The right hand side is destroyed in the process.  See
//    J. E. Dennis and R. Schnabel, Numerical Methods for
//    Unconstrained Optimization and Nonlinear Equations, 
//    Prentice Hall, Englewood Cliffs, N.J., 1983.
void EosLinAlg::BackSolve( int iM, int iNc, double *pMatrix, double *pRhs )
{
   int     i;
   int     iNc1;
   int     iNcm;
   int     iNi;
   int     iNj;
   int     iNk;
   int     iNn;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;

   /* Code for more than one grid block */
   if ( iM > 1 )
   {
      iNcm = iNc * iM;
      iNc1 = iNcm + iM;

      /* Forward loop */
      pTa = pRhs;
      pTb = pMatrix;
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         pTa += iM;
         pTb += iM;
         pTc = pRhs;
         pTd = pTb;
         for ( iNj = 0; iNj < iNi; iNj++ )
         {
#ifdef IPFtune
            int cnt = iM;
#pragma ivdep
            for ( i = 0; i < cnt; i++ )
            {
               pTa[i] += pTd[i] * pTc[i];
            }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               pTa[i] += pTd[i] * pTc[i];
            }
#endif

            pTc += iM;
            pTd += iNcm;
         }
      }

      /* Hit by the diagonal matrix */
      pTa = pRhs;
      pTb = pMatrix;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
#ifdef IPFtune
         int cnt = iM;
#pragma ivdep
         for ( i = 0; i < cnt; i++ )
         {
            pTa[i] *= pTb[i];
         }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            pTa[i] *= pTb[i];
         }
#endif

         pTa += iM;
         pTb += iM + iNcm;
      }

      /* Backsolve loop */
      pTa -= iM;
      pTb -= iNcm;
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         pTc = pTa;
         pTa -= iM;
         pTb -= iNc1;
         pTd = pTb;
         for ( iNj = 0; iNj < iNi; iNj++ )
         {
#ifdef IPFtune
            int cnt = iM;
#pragma ivdep
            for ( i = 0; i < cnt; i++ )
            {
               pTa[i] += pTd[i] * pTc[i];
            }
#else    
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               pTa[i] += pTd[i] * pTc[i];
            }
#endif

            pTc += iM;
            pTd += iM;
         }
      }
   }

   /* Code for one grid block */
   else
   {
      iNc1 = iNc + 1;

      /* Forward loop */
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         iNk = iNi;
#ifdef IPFtune
         double tRhs = pRhs[iNi];
#pragma ivdep
         for ( iNj = 0; iNj < iNi; iNj++ )
         {
            tRhs += pMatrix[iNk] * pRhs[iNj];
            iNk += iNc;
         }
         pRhs[iNi] = tRhs;
#else
         for ( iNj = 0; iNj < iNi; iNj++ )
         {
            pRhs[iNi] += pMatrix[iNk] * pRhs[iNj];
            iNk += iNc;
         }
#endif 
      }

      /* Hit by the diagonal matrix */
      iNk = 0;
#ifdef IPFtun
#pragma ivdep
#endif      
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         pRhs[iNi] *= pMatrix[iNk];
         iNk += iNc1;
      }

      /* Backsolve loop */
      iNk -= iNc1;
      for ( iNi = iNc - 2; iNi >= 0; iNi-- )
      {
         iNk -= iNc1;
         iNn = iNk;
#ifdef IPFtune
         double tRhs = pRhs[iNi];
#pragma ivdep 
         for ( iNj = iNi + 1; iNj < iNc; iNj++ )
         {
            tRhs += pMatrix[++iNn] * pRhs[iNj];
         }
         pRhs[iNi] = tRhs;
#else 
         for ( iNj = iNi + 1; iNj < iNc; iNj++ )
         {
            pRhs[iNi] += pMatrix[++iNn] * pRhs[iNj];
         }
#endif
      }
   }
}


///
/// \brief Cholesky factorization routine  
///
/// \param iM   Number of objects
/// \param iNc  Number of components
/// \param pMatrix Pointer to matrix.  The array order is
///       Objects (first)
///       Columns
///       Rows (last)
//
//    Only the diagonal and bottom subdiagonal elements are used.
//    The matrix is destroyed in the process, and only the
//    factored matrix remains
//
// 1) Generates a modified cholesky decomposition
//    of pMatrix, which is destroyed in the process.
//    this modified cholesky method is needed due
//    to the fact that negative curvature is often
//    encountered during the flash calculations
//    The first step is to compute the square root
//    of the maximum diagonal element of the matrix
//
//    AMAX = SQRT ( MAX ( ABS ( A    ) , ABS ( A    ) / NC ) )
//                               I,I            I,J
//
// 2) Now for each column, i = 1 to n, do
//
//                  I-1  2
//    A    = A    - SUM A
//     I,I    I,I   J=1  J,I
//
// 3) For each row j = i+1 to n in column i, do
//
//                  I-1
//    A    = A    + SUM A    * A
//     J,I    J,I   K=1  J,K    I,K
//
// 4) Compute perturbation parameter
//
//             N
//    MAXA =  MAX  ( A    ) / AMAX
//           J=I+1    J,I
//
// 5) Compute the diagonal term
//
//                                   2
//    A    = SQRT ( MAX ( A    , MAXA  , EPSMAC ) )
//     I,I                 I,I
//
// 6) For each row j = i+1 to n in column i, do
//
//    A    = A    / A
//     J,I    J,I    I,I
//
// 7) If the matrix is safely positive definite, we
//    have then formed a lower traingular matrix l
//    stored in "a" such that
//
//         T
//    A = L L - D
//
//    where D is a zero matrix.  If the original
//    matrix was not positive definite, the matrix
//    D is a positive diagonal matrix which is can
//    be computed from the maxa perturbation.  See
//    J. E. Dennis and R. Schnabel, Numerical Methods for
//    Unconstrained Optimization and Nonlinear Equations, 
//    Prentice Hall, Englewood Cliffs, N.J., 1983.
void EosLinAlg::Cholesky( int iM, int iNc, double dTiny, double *pMatrix, double *pWork1, double *pWork2 )
{
   int     iNc1;
   int     iNcm;
   int     iNi;
   int     iNj;
   int     iNk;
   int     iNm;
   int     iNn;
   int     iNo;
   int     iNp;
   int     iNq;
   int     i;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dTerm;
   double  dNc;
   double  dTerm1;
   double  dTerm2;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;

   /* Temporary term */
   dTerm = sqrt( dTiny );
   dNc = (double)iNc;

   /* Code for more than one grid block */
   if ( iM > 1 )
   {
      iNcm = iNc * iM;
      iNc1 = iNcm + iM;

      /* Set scaling factor */
      pTa = pMatrix;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
      for ( i = 0; i < iM; i++ )
      {
         dA = pTa[i];
         pWork1[i] = dA > 0.0 ? dA : -dA;
      }

      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         pTa += iNcm + iM;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            dB = pTa[i];
            dA = dB > 0.0 ? dB : -dB;
            dB = pWork1[i];
            pWork1[i] = ( dA > dB ) ? dA : dB;
         }
      }

      dA = 1.0 / dNc;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
      for ( i = 0; i < iM; i++ )
      {
         pWork1[i] = pWork1[i] * dA;
      }

      iNk = 0;
      pTa = pMatrix;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         pTa += iNk;
         iNk += iM;
         for ( iNj = iNi + 1; iNj < iNc; iNj++ )
         {
#ifdef IPFtune
            int     cnt = iM;
            double *pT3 = pWork1;
#pragma ivdep
            for ( i = 0; i < cnt; i++ )
            {
               dB = pTa[i];
               dA = fabs(dB);
               dB = pT3[i];
               pT3[i] = ( dA > dB ) ? dA : dB;
            }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               dB = pTa[i];
               dA = dB > 0.0 ? dB : -dB;
               dB = pWork1[i];
               pWork1[i] = ( dA > dB ) ? dA : dB;
            }
#endif
         }

         pTa += iM;
      }

#ifdef IPFtune
      double *pT3 = pWork1;
      double tiny = dTiny;
#pragma ivdep
      for ( i = 0; i < iM; i++ )
      {
         dA = dNc * pT3[i];
         pT3[i] = 1.0 / sqrt( ( dA > tiny ) ? dA : tiny );
      }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
      for ( i = 0; i < iM; i++ )
      {
         dA = dNc * pWork1[i];
         pWork1[i] = 1.0 / sqrt( ( dA > dTiny ) ? dA : dTiny );
      }
#endif

      /* For every row */
      pTa = pMatrix;
      pTb = pTa;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         /* Modify the diagonal element */
         pTd = pTb;
         pTe = pMatrix;
         for ( iNj = 0; iNj < iNi; iNj++ )
         {
#ifdef IPFtune
            int cnt = iM;
#pragma ivdep
            for ( i = 0; i < cnt; i++ )
            {
               dB = pTd[i];
               dA = 0-pTe[i] * dB;
               pTa[i] += dA * dB;
               pTd[i] = dA;
            }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               dB = pTd[i];
               dA = -pTe[i] * dB;
               pTa[i] += dA * dB;
               pTd[i] = dA;
            }
#endif

            pTd += iNcm;
            pTe += iNc1;
         }

         /* Prepare to patch column */
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            pWork2[i] = 0.0;
         }

         /* Go down column */
         pTd = pTb;
         pTc = pTa + iM;
         for ( iNj = iNi + 1; iNj < iNc; iNj++ )
         {
            pTd += iM;
            pTe = pTb;
            pTf = pTd;
            for ( iNk = 0; iNk < iNi; iNk++ )
            {
#ifdef IPFtune
               int cnt = iM;
#pragma ivdep
               for ( i = 0; i < cnt; i++ )
               {
                  pTc[i] += pTe[i] * pTf[i];
               }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
               for ( i = 0; i < iM; i++ )
               {
                  pTc[i] += pTe[i] * pTf[i];
               }
#endif

               pTe += iNcm;
               pTf += iNcm;
            }

#ifdef IPFtune
            double *pT2 = pWork2;
            int     cnt = iM;
#pragma ivdep
            for ( i = 0; i < cnt; i++ )
            {
               dB = pTc[i];
               dA = fabs(dB);
               dB = pT2[i];
               pT2[i] = ( dA > dB ) ? dA : dB;
            }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               dB = pTc[i];
               dA = dB > 0.0 ? dB : -dB;
               dB = pWork2[i];
               pWork2[i] = ( dA > dB ) ? dA : dB;
            }
#endif

            pTc += iM;
         }

         /* Patch term for column and take inverse */
#ifdef IPFtune
                 pT3 = pWork1;
         double *pT2 = pWork2;
#pragma ivdep
         for ( i = 0; i < iM; i++ )
         {
            dB = pTa[i];
            dA = fabs(dB);
            dB = pT3[i] * pT2[i];
            dC = dB * dB;
            dD = dTerm > dC ? dTerm : dC;
            pTa[i] = 1.0 / ( dD > dA ? dD : dA );
         }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            dB = pTa[i];
            dA = dB > 0.0 ? dB : -dB;
            dB = pWork1[i] * pWork2[i];
            dC = dB * dB;
            dD = dTerm > dC ? dTerm : dC;
            pTa[i] = 1.0 / ( dD > dA ? dD : dA );
         }
#endif

         pTa += iNcm + iM;
         pTb += iM;
      }
   }

   /* Code for one grid block */
   else
   {
      iNc1 = iNc + 1;

      /* Set scaling factor */
      iNk = 0;
      dB = pMatrix[0];
      dTerm1 = dB > 0.0 ? dB : -dB;
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         iNk += iNc1;
         dB = pMatrix[iNk];
         dA = dB > 0.0 ? dB : -dB;
         dTerm1 = ( dA > dTerm1 ) ? dA : dTerm1;
      }

      iNk = 0;
      dTerm1 = dTerm1 / dNc;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         iNk += iNi;
         for ( iNj = iNi + 1; iNj < iNc; iNj++ )
         {
            dB = pMatrix[iNk++];
            dA = dB > 0.0 ? dB : -dB;
            dTerm1 = ( dA > dTerm1 ) ? dA : dTerm1;
         }
      }

      dA = dNc * dTerm1;
      dTerm1 = 1.0 / sqrt( ( dA > dTiny ) ? dA : dTiny );

      /* For every row */
      iNm = 0;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         /* Modify the diagonal element */
         iNn = iNi;
         iNo = 0;
         dC = pMatrix[iNm];
         for ( iNj = 0; iNj < iNi; iNj++ )
         {
            dB = pMatrix[iNn];
            dA = -pMatrix[iNo] * dB;
            dC += dA * dB;
            pMatrix[iNn] = dA;
            iNn += iNc;
            iNo += iNc1;
         }

         pMatrix[iNm] = dC;

         /* Go down column */
         iNo = iNm;
         dTerm2 = 0;
         for ( iNj = iNi + 1; iNj < iNc; iNj++ )
         {
            iNo++;
            iNp = iNi;
            iNq = iNj;
            for ( iNk = 0; iNk < iNi; iNk++ )
            {
               pMatrix[iNo] += pMatrix[iNp] * pMatrix[iNq];
               iNp += iNc;
               iNq += iNc;
            }

            dA = fabs( pMatrix[iNo] );
            dTerm2 = ( dA > dTerm2 ) ? dA : dTerm2;
         }

         /* Patch term for column and take inverse */
         dA = fabs( pMatrix[iNm] );
         dB = dTerm1 * dTerm2;
         dC = dB * dB;
         dD = dTerm > dC ? dTerm : dC;
         pMatrix[iNm] = 1.0 / ( dD > dA ? dD : dA );
         iNm += iNc1;
      }
   }
}

///
/// \brief Function for vector exponentiation
///
void EosUtils::VectorExp( int iM, double *pInput, double *pOutput )
{
#ifdef EOS_INTEL
   vdExp( iM, pInput, pOutput );
#else
   for ( int i = 0; i < iM; i++ )
   {
      pOutput[i] = exp( pInput[i] );
   }
#endif
}

///
/// \brief Function for vector log
///
void EosUtils::VectorLog( int iM, double *pInput, double *pOutput )
{
#ifdef EOS_INTEL
   vdLn( iM, pInput, pOutput );
#else
   for ( int i = 0; i < iM; i++ )
   {
      pOutput[i] = log( pInput[i] );
   }
#endif
}

///
/// \brief Function for vector power
///
void EosUtils::VectorPow( int iM, double *pInput1, double dInput2, double *pOutput )
{
#ifdef EOS_INTEL
   vdPowx( iM, pInput1, dInput2, pOutput );
#else
   for ( int i = 0; i < iM; ++i )
   {
      pOutput[i] = pow( pInput1[i], dInput2 );
   }
#endif
}

///
/// \brief Function for vector power
///
/// \param[in]  iM (constant) number of objects
/// \param[in] pInput1 (constant) variable to take power of
/// \param[in] pInput2 (constant) exponent
/// \param[out] pOutput   result
///
void EosUtils::VectorPowX( int iM, double *pInput1, double *pInput2, double *pOutput )
{
#ifdef EOS_INTEL
   vdPow( iM, pInput1, pInput2, pOutput );
#else
   for ( int i = 0; i < iM; ++i )
   {
      pOutput[i] = pow( pInput1[i], pInput2[i] );
   }
#endif
}

