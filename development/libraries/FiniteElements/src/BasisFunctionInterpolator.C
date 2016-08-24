//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "BasisFunctionInterpolator.h"

void FiniteElementMethod::BasisFunctionInterpolator::interpolatePropertiesMain ( const int NA, const int MA, const int MB,
                                                                                 const int colBlocks,
                                                                                 const int rowBlocks,
                                                                                 const int rowBlocksRemaining,
                                                                                 const double* a, const int LDA,
                                                                                 const double*& b, const int LDB,
                                                                                 double*& c, const int LDC ) {

   const double* bufA;
   const double* bufB;
   double* bufR;

   FourByFour a1;
   FourByFour a2;
   FourByFour b1;
   FourByFour b2;
   FourByFour r1;
   FourByFour r2;

   for ( int i = 0; i < colBlocks; ++i ) {
      bufA = a;
      bufB = b;
      bufR = c;

      // Load 2 blocks from the B matrix.
      loadContiguousTrans4x4 ( bufB, LDB, b1 );
      loadContiguousTrans4x4 ( bufB + 4, LDB, b2 );

      for ( int j = 1; j <= rowBlocks; ++j ) {
         zero ( r1 );
         zero ( r2 );

         loadContiguous ( bufA, LDA, a1 );
         loadContiguous ( bufA + 4, LDA, a2 );
         product ( a1, a2, b1, r1, r2 );

         loadContiguous ( bufA + 4 * LDA, LDA, a1 );
         loadContiguous ( bufA + 4 * LDA + 4, LDA, a2 );
         product ( a1, a2, b2, r1, r2 );

         store ( bufR, LDC, r1 );
         store ( bufR + 4, LDC, r2 );

         // Move both basis and result buffers by 8 rows.
         bufA += 8;
         bufR += 8;
      }

      if ( rowBlocksRemaining == 1 ) {
         zero ( r1 );
         loadContiguous ( bufA, LDA, a1 );
         product ( a1, b1, r1 );

         loadContiguous ( bufA + 4 * LDA, LDA, a1 );
         product ( a1, b2, r1 );

         store ( bufR, LDC, r1 );

         // Move both basis and result buffers by 4 rows.
         bufA += 4;
         bufR += 4;
      }

      // Move both property and result buffers by 4 columns
      b += 4 * LDB;
      c += 4 * LDC;
   }

}

double FiniteElementMethod::BasisFunctionInterpolator::innerProduct ( const double* a, const int posA, const int LDA, const double* b, const int posB ) {
   return a [ posA ]           * b [ posB ]     + a [ posA +     LDA ] * b [ posB + 1 ] +
          a [ posA + 2 * LDA ] * b [ posB + 2 ] + a [ posA + 3 * LDA ] * b [ posB + 3 ] +
          a [ posA + 4 * LDA ] * b [ posB + 4 ] + a [ posA + 5 * LDA ] * b [ posB + 5 ] +
          a [ posA + 6 * LDA ] * b [ posB + 6 ] + a [ posA + 7 * LDA ] * b [ posB + 7 ];
}

void FiniteElementMethod::BasisFunctionInterpolator::interpolatePropertiesRemainingRows ( const int NA, const int MA, const int MB,
                                                                                          int rowsRemaining,
                                                                                          const double* a, const int LDA,
                                                                                          const double* b, const int LDB,
                                                                                          double* c, const int LDC ) {

   if ( rowsRemaining >= 1 ) {
      int posC = NA - rowsRemaining;
      int posA = NA - rowsRemaining;
      int posB = 0;

      if ( rowsRemaining % 2 == 1 ) {

         for ( int i = 0; i < MB; ++i, posC += LDC, posB += LDB ) {
            c [ posC ] = innerProduct ( a, posA, LDA, b, posB );
         }

         --rowsRemaining;
      }

      if ( rowsRemaining == 2 ) {
         posC = NA - rowsRemaining;
         posA = NA - rowsRemaining;
         posB = 0;

         for ( int i = 0; i < MB; ++i, posC += LDC, posB += LDB ) {
            c [ posC     ] = innerProduct ( a, posA,     LDA, b, posB );
            c [ posC + 1 ] = innerProduct ( a, posA + 1, LDA, b, posB );
         }

      }

   }

}

void FiniteElementMethod::BasisFunctionInterpolator::interpolatePropertiesOneRemainingColumn ( const int rowBlocks,
                                                                                               const int rowBlocksRemaining,
                                                                                               const double* a, const int LDA,
                                                                                               const double* b, const int LDB,
                                                                                                     double* c, const int LDC ) {

   FourByFour a1;
   FourByFour a2;
   FourByFour b1b;
   FourByFour b2b;
   __m256d    r1b;
   __m256d    r2b;

   const double* bufA = a;
   const double* bufB = b;
   double* bufR = c;

   loadBroadcast4x1 ( bufB, b1b );
   loadBroadcast4x1 ( bufB + 4, b2b );

   for ( int j = 1; j <= rowBlocks; ++j ) {
      zero ( r1b );
      zero ( r2b );

      loadContiguous ( bufA, LDA, a1 );
      loadContiguous ( bufA + 4, LDA, a2 );
      product ( a1, b1b, r1b );
      product ( a2, b1b, r2b );

      loadContiguous ( bufA + 4 * LDA, LDA, a1 );
      loadContiguous ( bufA + 4 * LDA + 4, LDA, a2 );
      product ( a1, b2b, r1b );
      product ( a2, b2b, r2b );

      store ( bufR, r1b );
      store ( bufR + 4, r2b );

      bufA += 8;
      bufR += 8;
   }

   if ( rowBlocksRemaining == 1 ) {
      zero ( r1b );
      loadContiguous ( bufA, LDA, a1 );
      product ( a1, b1b, r1b );
      loadContiguous ( bufA + 4 * LDA, LDA, a1 );
      product ( a1, b2b, r1b );
      store ( bufR, r1b );
   }

}

void FiniteElementMethod::BasisFunctionInterpolator::interpolatePropertiesTwoRemainingColumns ( const int rowBlocks,
                                                                                                const int rowBlocksRemaining,
                                                                                                const double*  a, const int LDA,
                                                                                                const double*& b, const int LDB,
                                                                                                      double*& c, const int LDC ) {

   const double* bufA;
   const double* bufB;
   double* bufR;

   FourByFour a1;
   FourByFour a2;
   FourByFour b1;
   FourByFour b2;
   FourByTwo  r1a;
   FourByTwo  r2a;

   bufA = a;
   bufB = b;
   bufR = c;

   // Load 2 4x2 blocks from the B matrix.
   loadContiguousTrans4x2 ( bufB, LDB, b1 );
   loadContiguousTrans4x2 ( bufB + 4, LDB, b2 );

   for ( int j = 1; j <= rowBlocks; ++j ) {
      zero ( r1a );
      zero ( r2a );

      loadContiguous ( bufA, LDA, a1 );
      loadContiguous ( bufA + 4, LDA, a2 );
      product ( a1, b1, r1a );
      product ( a2, b1, r2a );

      loadContiguous ( bufA + 4 * LDA, LDA, a1 );
      loadContiguous ( bufA + 4 * LDA + 4, LDA, a2 );
      product ( a1, b2, r1a );
      product ( a2, b2, r2a );

      store ( bufR, LDC, r1a );
      store ( bufR + 4, LDC, r2a );

      bufA += 8;
      bufR += 8;
   }

   if ( rowBlocksRemaining == 1 ) {
      zero ( r1a );
      loadContiguous ( bufA, LDA, a1 );
      product ( a1, b1, r1a );
      loadContiguous ( bufA + 4 * LDA, LDA, a1 );
      product ( a1, b2, r1a );
      store ( bufR, LDC, r1a );
   }

}

void FiniteElementMethod::BasisFunctionInterpolator::interpolatePropertiesThreeRemainingColumns ( const int rowBlocks,
                                                                                                  const int rowBlocksRemaining,
                                                                                                  const double*  a, const int LDA,
                                                                                                  const double*& b, const int LDB,
                                                                                                        double*& c, const int LDC ) {

   const double* bufA;
   const double* bufB;
   double* bufR;

   FourByFour a1;
   FourByFour a2;
   FourByFour b1;
   FourByFour b2;
   FourByFour b1b;
   FourByFour b2b;
   FourByTwo  r1a;
   FourByTwo  r2a;
   __m256d    r1b;
   __m256d    r2b;

   bufA = a;
   bufB = b;
   bufR = c;

   // Load 2 4x2 blocks from the B matrix.
   loadContiguousTrans4x2 ( bufB, LDB, b1 );
   loadContiguousTrans4x2 ( bufB + 4, LDB, b2 );

   // Load 2 4x1 blocks from the B matrix.
   loadBroadcast4x1 ( bufB + 2 * LDB, b1b );
   loadBroadcast4x1 ( bufB + 2 * LDB + 4, b2b );

   for ( int j = 1; j <= rowBlocks; ++j ) {
      zero ( r1a );
      zero ( r2a );
      zero ( r1b );
      zero ( r2b );

      loadContiguous ( bufA, LDA, a1 );
      loadContiguous ( bufA + 4, LDA, a2 );
      product ( a1, b1, r1a );
      product ( a2, b1, r2a );
      product ( a1, b1b, r1b );
      product ( a2, b1b, r2b );

      loadContiguous ( bufA + 4 * LDA, LDA, a1 );
      loadContiguous ( bufA + 4 * LDA + 4, LDA, a2 );
      product ( a1, b2, r1a );
      product ( a2, b2, r2a );
      product ( a1, b2b, r1b );
      product ( a2, b2b, r2b );

      store ( bufR, LDC, r1a );
      store ( bufR + 4, LDC, r2a );

      store ( bufR + 2 * LDC, r1b );
      store ( bufR + 2 * LDC + 4, r2b );

      bufA += 8;
      bufR += 8;
   }

   if ( rowBlocksRemaining == 1 ) {
      zero ( r1a );
      zero ( r1b );
      loadContiguous ( bufA, LDA, a1 );
      product ( a1, b1, r1a );
      product ( a1, b1b, r1b );
      loadContiguous ( bufA + 4 * LDA, LDA, a1 );
      product ( a1, b2, r1a );
      product ( a1, b2b, r1b );
      store ( bufR, LDC, r1a );
      store ( bufR + 2 * LDC, r1b );
   }

}

void FiniteElementMethod::BasisFunctionInterpolator::interpolatePropertiesRemainingColumns ( const int NA, const int MA, const int MB,
                                                                                             const int rowBlocks,
                                                                                             const int rowBlocksRemaining,
                                                                                             const int colsRemaining,
                                                                                             const double*  a, const int LDA,
                                                                                             const double*& b, const int LDB,
                                                                                                   double*& c, const int LDC ) {

   if ( colsRemaining == 1 ) {
      interpolatePropertiesOneRemainingColumn ( rowBlocks, rowBlocksRemaining, a, LDA, b, LDB, c, LDC );
   } else if ( colsRemaining == 2 ) {
      interpolatePropertiesTwoRemainingColumns ( rowBlocks, rowBlocksRemaining, a, LDA, b, LDB, c, LDC );
   } else if ( colsRemaining == 3 ) {
      interpolatePropertiesThreeRemainingColumns ( rowBlocks, rowBlocksRemaining, a, LDA, b, LDB, c, LDC );
   }

}


void FiniteElementMethod::BasisFunctionInterpolator::simpleInterpolate ( const Numerics::AlignedDenseMatrix& basisFunctionsTranspose,
                                                                         const Numerics::AlignedDenseMatrix& propertyVectors,
                                                                               Numerics::AlignedDenseMatrix& interpolatedProperties ) {

   const int rowsA = basisFunctionsTranspose.rows ();
   const int colsB = propertyVectors.cols ();

   const int LDA = basisFunctionsTranspose.leadingDimension ();
   const int LDB = propertyVectors.leadingDimension ();
   const int LDC = interpolatedProperties.leadingDimension ();

   const double* a = basisFunctionsTranspose.data ();
   const double* b = propertyVectors.data ();
   double* c = interpolatedProperties.data ();

   for ( int j = 0; j < rowsA; ++j ) {
      int posC = j;
      int posB = 0;

      for ( int i = 0; i < colsB; ++i, posC += LDC, posB += LDB ) {
         c [ posC ] = a [ j           ] * b [ posB     ] + a [ j +     LDA ] * b [ posB + 1 ] +
                      a [ j + 2 * LDA ] * b [ posB + 2 ] + a [ j + 3 * LDA ] * b [ posB + 3 ] +
                      a [ j + 4 * LDA ] * b [ posB + 4 ] + a [ j + 5 * LDA ] * b [ posB + 5 ] +
                      a [ j + 6 * LDA ] * b [ posB + 6 ] + a [ j + 7 * LDA ] * b [ posB + 7 ];

      }

   }

}


void FiniteElementMethod::BasisFunctionInterpolator::compute ( const Numerics::AlignedDenseMatrix& basisFunctionsTranspose,
                                                               const Numerics::AlignedDenseMatrix& propertyVectors,
                                                                     Numerics::AlignedDenseMatrix& interpolatedProperties ) {


   if ( basisFunctionsTranspose.cols () != 8 and propertyVectors.rows () != 8 ) {
      // Incorrect number of dofs.
   }

   if ( basisFunctionsTranspose.rows () != interpolatedProperties.rows () or
        propertyVectors.cols ()         != interpolatedProperties.cols ()) {
      // Dimension mismatch.
   }

   const double* bufAPos = basisFunctionsTranspose.data ();
   const double* bufBPos = propertyVectors.data ();
   double* bufCPos = interpolatedProperties.data ();

   const int LDA = basisFunctionsTranspose.leadingDimension ();
   const int LDB = propertyVectors.leadingDimension ();
   const int LDC = interpolatedProperties.leadingDimension ();

   const int rowsA = basisFunctionsTranspose.rows ();
   const int colsA = basisFunctionsTranspose.cols ();
   const int colsB = propertyVectors.cols ();

   int rowBlocks = ( rowsA - rowsA % 8 ) / 8;
   int rowBlocksRemaining = (( rowsA - rowBlocks * 8 ) - ( rowsA - rowBlocks * 8 ) % 4 ) / 4;
   int rowsRemaining = rowsA % 4;
   int colBlocks = ( colsB - colsB % 4 ) / 4;
   int colsRemaining = colsB % 4;

   // First compute the bulk of the values all values where: #rows mod 4 = 0 and #cols mod 4 = 0
   interpolatePropertiesMain ( rowsA, colsA, colsB,
                               colBlocks, rowBlocks, rowBlocksRemaining,
                               bufAPos, LDA, bufBPos, LDB, bufCPos, LDC );

   // Next compute values for the last columns. Ones that from #cols - #cols mod 4 + 1.
   interpolatePropertiesRemainingColumns ( rowsA, colsA, colsB,
                                           rowBlocks, rowBlocksRemaining, colsRemaining,
                                           bufAPos, LDA,
                                           bufBPos, LDB,
                                           bufCPos, LDC );

   // Next compute values for the last rows. Ones that from #rows - #rows mod 4 + 1.
   interpolatePropertiesRemainingRows ( rowsA, colsA, colsB,
                                        rowsRemaining,
                                        basisFunctionsTranspose.data (), LDA,
                                        propertyVectors.data (), LDB,
                                        interpolatedProperties.data (), LDC );
}
