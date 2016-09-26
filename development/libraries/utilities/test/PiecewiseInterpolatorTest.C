#include "../src/PiecewiseInterpolator.h"
#include <gtest/gtest.h>

// Test the scalar evaluate function of the interpolator.
// Range of input is in range of interpolator.
// Interpolator will consis of a single panel.
TEST ( PiecewiseInterpolator, TestSize2Scalar ) {

   const unsigned int Size = 2;
   const unsigned int NumberOfEvaluations = 10;

   ibs::PiecewiseInterpolator interp;
   double xs [ Size ] = { 0.0, 1.0 };
   double ys [ Size ] = { 1.0, 2.0 };

   interp.setInterpolation ( Size, xs, ys );

   double h = (xs [1] - xs [0]) / double ( NumberOfEvaluations - 1 );
   double x = xs [0];
   double expectedValue = ys [0];
   double expectedValueDelta = (ys [1] - ys [0]) / double ( NumberOfEvaluations - 1 );

   for ( unsigned int i = 0; i < NumberOfEvaluations; ++i ) {
      EXPECT_NEAR ( expectedValue, interp.evaluate ( x ), 1.0e-10 );
      x += h;
      expectedValue += expectedValueDelta;
   }

}

// Test the vector evaluate function of the interpolator.
// Range of input is in range of interpolator.
// Interpolator will consis of a single panel.
TEST ( PiecewiseInterpolator, TestSize2Vector ) {

   const unsigned int Size = 2;
   const unsigned int NumberOfEvaluations = 10;

   ibs::PiecewiseInterpolator interp;
   double xs [ Size ] = { 0.0, 1.0 };
   double ys [ Size ] = { 1.0, 2.0 };

   interp.setInterpolation ( Size, xs, ys );

   double evaluationPoints [ NumberOfEvaluations ];
   double interpolationValues [ NumberOfEvaluations ];
   double expectedInterpolations [ NumberOfEvaluations ];

   double h = (xs [1] - xs [0]) / double ( NumberOfEvaluations - 1 );
   double x = xs [0];
   double expectedValue = ys [0];
   double expectedValueDelta = (ys [1] - ys [0]) / double ( NumberOfEvaluations - 1 );

   // Generate expected values
   for ( unsigned int i = 0; i < NumberOfEvaluations; ++i ) {
      evaluationPoints [ i ] = x;
      expectedInterpolations [ i ] = expectedValue;
      x += h;
      expectedValue += expectedValueDelta;
   }

   interp.evaluate ( NumberOfEvaluations, evaluationPoints, interpolationValues );

   for ( unsigned int i = 0; i < NumberOfEvaluations; ++i ) {
      EXPECT_NEAR (expectedInterpolations [ i ], interpolationValues [ i ], 1.0e-11 );
   }

}


// Test the scalar evaluate function of the interpolator.
// Range of input is in range of interpolator.
// Interpolator will consis of two equally spaced panels panel.
TEST ( PiecewiseInterpolator, TestSize3Scalar ) {

   const unsigned int Size = 3;
   const unsigned int NumberOfEvaluations = 20;

   ibs::PiecewiseInterpolator interp;
   double xs [ Size ] = { 0.0, 1.0, 2.0 };
   double ys [ Size ] = { 1.0, 2.0, 3.0 };

   interp.setInterpolation ( Size, xs, ys );

   double h = (xs [2] - xs [0]) / double ( NumberOfEvaluations - 1 );
   double x = xs [0];
   double expectedValue = ys [0];
   double expectedValueDelta = (ys [2] - ys [0]) / double ( NumberOfEvaluations - 1 );

   for ( unsigned int i = 0; i < NumberOfEvaluations; ++i ) {
      EXPECT_NEAR ( expectedValue, interp.evaluate ( x ), 1.0e-10 );
      x += h;
      expectedValue += expectedValueDelta;
   }

}

// Test the vector evaluate function of the interpolator.
// Range of input is in range of interpolator.
// Interpolator will consis of two equally spaced panels panel.
TEST ( PiecewiseInterpolator, TestSize3Vector ) {

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
   double expectedValueDelta = (ys [2] - ys [0]) / double ( NumberOfEvaluations - 1 );

   // Generate expected values
   for ( unsigned int i = 0; i < NumberOfEvaluations; ++i ) {
      evaluationPoints [ i ] = x;
      expectedInterpolations [ i ] = expectedValue;
      x += h;
      expectedValue += expectedValueDelta;
   }

   interp.evaluate ( NumberOfEvaluations, evaluationPoints, interpolationValues );

   for ( unsigned int i = 0; i < NumberOfEvaluations; ++i ) {
      EXPECT_NEAR (expectedInterpolations [ i ], interpolationValues [ i ], 1.0e-11 );
   }

}
