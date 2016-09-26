#include "../src/PiecewiseInterpolator.h"
#include <gtest/gtest.h>

// Test the scalar evaluateDerivative function of the interpolator.
// Range of input is in range of interpolator.
// Interpolator will consis of a single panel.
TEST ( PiecewiseInterpolatorDerivative, TestSize2Scalar ) {

   const unsigned int Size = 2;
   const unsigned int NumberOfEvaluations = 10;

   ibs::PiecewiseInterpolator interp;
   double xs [ Size ] = { 0.0, 1.0 };
   double ys [ Size ] = { 1.0, 2.0 };

   interp.setInterpolation ( Size, xs, ys );

   double h = (xs [1] - xs [0]) / double ( NumberOfEvaluations - 1 );
   double x = xs [0];
   double expectedValue = (ys [1] - ys [0]) / ( xs [1] - xs [0]);

   for ( unsigned int i = 0; i < NumberOfEvaluations; ++i ) {
      EXPECT_NEAR ( expectedValue, interp.evaluateDerivative ( x ), 1.0e-10 );
      x += h;
   }

}

// Test the vector evaluateDerivative function of the interpolator.
// Range of input is in range of interpolator.
// Interpolator will consis of a single panel.
TEST ( PiecewiseInterpolatorDerivative, TestSize2Vector ) {

   const unsigned int Size = 2;
   const unsigned int NumberOfEvaluations = 10;

   ibs::PiecewiseInterpolator interp;
   double xs [ Size ] = { 0.0, 1.0 };
   double ys [ Size ] = { 1.0, 2.0 };

   interp.setInterpolation ( Size, xs, ys );

   double evaluationPoints [ NumberOfEvaluations ];
   double interpolationValues [ NumberOfEvaluations ];

   double h = (xs [1] - xs [0]) / double ( NumberOfEvaluations - 1 );
   double x = xs [0];
   double expectedValue = (ys [1] - ys [0]) / ( xs [1] - xs [0]);

   // Generate expected values
   for ( unsigned int i = 0; i < NumberOfEvaluations; ++i ) {
      evaluationPoints [ i ] = x;
      x += h;
   }

   interp.evaluateDerivative ( NumberOfEvaluations, evaluationPoints, interpolationValues );

   for ( unsigned int i = 0; i < NumberOfEvaluations; ++i ) {
      EXPECT_NEAR ( expectedValue, interpolationValues [ i ], 1.0e-11 );
   }

}


// Test the scalar evaluateDerivative function of the interpolator.
// Range of input is in range of interpolator.
// Interpolator will consis of two equally spaced panels panel.
TEST ( PiecewiseInterpolatorDerivative, TestSize3Scalar ) {

   const unsigned int Size = 3;
   const unsigned int NumberOfEvaluations = 20;

   ibs::PiecewiseInterpolator interp;
   double xs [ Size ] = { 0.0, 1.0, 2.0 };
   double ys [ Size ] = { 1.0, 2.0, 3.0 };

   interp.setInterpolation ( Size, xs, ys );

   double h = (xs [2] - xs [0]) / double ( NumberOfEvaluations - 1 );
   double x = xs [0];
   double expectedValue = ys [0];

   for ( unsigned int i = 0; i < NumberOfEvaluations; ++i ) {

      if ( x < xs [1] ) {
         expectedValue = (ys [1] - ys [0]) / ( xs [1] - xs [0]);
      } else {
         expectedValue = (ys [2] - ys [1]) / ( xs [2] - xs [1]);
      }

      EXPECT_NEAR ( expectedValue, interp.evaluateDerivative ( x ), 1.0e-10 );
      x += h;
   }

}

// Test the vector evaluateDerivative function of the interpolator.
// Range of input is in range of interpolator.
// Interpolator will consis of two equally spaced panels panel.
TEST ( PiecewiseInterpolatorDerivative, TestSize3Vector ) {

   const unsigned int Size = 3;
   const unsigned int NumberOfEvaluations = 20;

   ibs::PiecewiseInterpolator interp;
   double xs [ Size ] = { 0.0, 1.0, 2.0 };
   double ys [ Size ] = { 1.0, 2.0, 3.0 };

   interp.setInterpolation ( Size, xs, ys );

   double evaluationPoints [ NumberOfEvaluations ];
   double interpolationValues [ NumberOfEvaluations ];
   double expectedInterpolations [ NumberOfEvaluations ];

   double h = (xs [2] - xs [0]) / double ( NumberOfEvaluations - 1 );
   double x = xs [0];
   double expectedValue = ys [0];

   // Generate expected values
   for ( unsigned int i = 0; i < NumberOfEvaluations; ++i ) {
      evaluationPoints [ i ] = x;

      if ( x < xs [1] ) {
         expectedValue = (ys [1] - ys [0]) / ( xs [1] - xs [0]);
      } else {
         expectedValue = (ys [2] - ys [1]) / ( xs [2] - xs [1]);
      }

      expectedInterpolations [ i ] = expectedValue;
      x += h;
   }

   interp.evaluateDerivative ( NumberOfEvaluations, evaluationPoints, interpolationValues );

   // Compare with expected value
   for ( unsigned int i = 0; i < NumberOfEvaluations; ++i ) {
      EXPECT_NEAR (expectedInterpolations [ i ], interpolationValues [ i ], 1.0e-11 );
   }

   // Compare with scalar value
   for ( unsigned int i = 0; i < NumberOfEvaluations; ++i ) {
      EXPECT_NEAR ( interp.evaluateDerivative ( evaluationPoints [ i ]), interpolationValues [ i ], 1.0e-11 );
   }

}
