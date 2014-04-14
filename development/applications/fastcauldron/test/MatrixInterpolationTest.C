#include <iostream>
#include <cmath>
#include <cassert>
#include <cstdlib>

#include "MatrixInterpolator.h"

#include <gtest/gtest.h>

double constantFunction ( const double x );
double linearFunction ( const double x );
double quadraticFunction ( const double x );
double zeroFunction ( const double x );

double cubicFunction ( const double x );
// Is error for cubic poly:  1.0 + x - 3.0 * x * x + 2.0 * x * x * x;   
// over the range 0 .. 1 with x values 0, 0.5 and 1.
// Interpolated by a quadratic polynomial.
double cubicFunctionError ( const double x );

typedef double (*TestFunction)( const double x);
typedef double (*ErrorFunction)( const double x);

typedef MatrixInterpolator<2,2> Interpolator;

void computeInterp ( const Interpolator::CoefficientArray& xs,
                     Interpolator& interp );

void doTest ( const Interpolator& interp,
              const Interpolator::CoefficientArray& xs,
              const unsigned int row,
              const unsigned int col,
              TestFunction func,
              ErrorFunction errorFunc );

TEST( MatrixInterpolationTest, InterpolatorDegree )
{
   ASSERT_EQ( 2, Interpolator::Degree );

   Interpolator interp;
   Interpolator::CoefficientArray xs;

   xs [ 0 ] = 0.0;
   xs [ 1 ] = 0.5;
   xs [ 2 ] = 1.0;

   computeInterp ( xs, interp );


   // A quadratic interpolator should be able to interpolate a constant, linea and quadratic function with zero error.
   // Any error will come from the method used to solve the system of linear equations used to compute the interpolator coefficients.
   doTest ( interp, xs, 0, 0, constantFunction, zeroFunction );
   doTest ( interp, xs, 0, 1, linearFunction, zeroFunction );

   doTest ( interp, xs, 1, 0, quadraticFunction, zeroFunction );
   doTest ( interp, xs, 1, 1, cubicFunction, cubicFunctionError );
}



double zeroFunction ( const double x ) {
   return 0.0;
}

double constantFunction ( const double x ) {
   return 2.0;
}

double linearFunction ( const double x ) {
   return 1.0 + x;
}

double quadraticFunction ( const double x ) {
   return 1.0 + x - 3.0 * x * x;
}

double cubicFunction ( const double x ) {
   return 1.0 + x - 3.0 * x * x + 2.0 * x * x * x;   
}

double cubicFunctionError ( const double x ) {
   return x - 3.0 * x * x + 2.0 * x * x * x;
}


void computeInterp ( const Interpolator::CoefficientArray& xs,
                     Interpolator& interp ) {

   Interpolator::CoefficientTensor ys;
   
   ys [ 0 ][ 0 ][ 0 ] = constantFunction ( xs [ 0 ]);
   ys [ 0 ][ 0 ][ 1 ] = constantFunction ( xs [ 1 ]);
   ys [ 0 ][ 0 ][ 2 ] = constantFunction ( xs [ 2 ]);

   ys [ 0 ][ 1 ][ 0 ] = linearFunction ( xs [ 0 ]);
   ys [ 0 ][ 1 ][ 1 ] = linearFunction ( xs [ 1 ]);
   ys [ 0 ][ 1 ][ 2 ] = linearFunction ( xs [ 2 ]);

   ys [ 1 ][ 0 ][ 0 ] = quadraticFunction ( xs [ 0 ]);
   ys [ 1 ][ 0 ][ 1 ] = quadraticFunction ( xs [ 1 ]);
   ys [ 1 ][ 0 ][ 2 ] = quadraticFunction ( xs [ 2 ]);

   ys [ 1 ][ 1 ][ 0 ] = cubicFunction ( xs [ 0 ]);
   ys [ 1 ][ 1 ][ 1 ] = cubicFunction ( xs [ 1 ]);
   ys [ 1 ][ 1 ][ 2 ] = cubicFunction ( xs [ 2 ]);


   interp.compute ( xs, ys );

}

void doTest ( const Interpolator& interp,
              const Interpolator::CoefficientArray& xs,
              const unsigned int row,
              const unsigned int col,
             TestFunction func,
             ErrorFunction errorFunc ) {


   int numbeOfSteps = 129;
   double h = ( xs [ 2 ] - xs [ 0 ]) / double ( numbeOfSteps - 1 );
   double x = 0.0;

   double relativeError = 1.0e-10; 

   for ( int i = 1; i <= numbeOfSteps; ++i, x += h )
   {
      double expectance = func ( x );
      EXPECT_NEAR( interp.evaluate ( row, col, x ) + errorFunc ( x ), expectance, std::abs(expectance * relativeError ));
   }

}
