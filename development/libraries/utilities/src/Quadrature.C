//------------------------------------------------------------//

#include "Quadrature.h"

//------------------------------------------------------------//

#include <math.h>
#include <float.h>
#include <limits>

//------------------------------------------------------------//

#include "NumericFunctions.h"

//------------------------------------------------------------//

NumericFunctions::Quadrature* NumericFunctions::Quadrature::s_quadratureInstance = 0;

//------------------------------------------------------------//

NumericFunctions::Quadrature& NumericFunctions::Quadrature::getInstance () {

   if ( s_quadratureInstance == 0 ) {
      s_quadratureInstance = new Quadrature;
      s_quadratureInstance->initialiseQuadrature ();
   } 

   return *s_quadratureInstance;
}

//------------------------------------------------------------//

void NumericFunctions::Quadrature::getGaussLegendreQuadrature ( const int              degree,
                                                                      QuadratureArray& Points,
                                                                      QuadratureArray& Weights ) const {

   // Can only return a set of abscissae and weights if degree is on range,
   // otherwise return null.
   if ( 0 < degree && degree <= MaximumQuadratureDegree ) {
      Points  = m_gaussLegendreQuadraturePoints  [ degree - 1 ];
      Weights = m_gaussLegendreQuadratureWeights [ degree - 1 ];
   } else {

      // Set Points and Weights to Null
      Points  = 0;
      Weights = 0;
   }

}

//------------------------------------------------------------//

void NumericFunctions::Quadrature::getGaussLobattoQuadrature ( const int              degree,
                                                                     QuadratureArray& Points,
                                                                     QuadratureArray& Weights ) const {

   // Can only return a set of abscissae and weights if degree is on range.
   // otherwise return null.
   if ( 0 < degree && degree <= MaximumQuadratureDegree ) {
      Points  = m_gaussLobattoQuadraturePoints  [ degree - 1 ];
      Weights = m_gaussLobattoQuadratureWeights [ degree - 1 ];
   } else {

      // Set Points and Weights to Null
      Points  = 0;
      Weights = 0;
   }

}

//------------------------------------------------------------//

void NumericFunctions::Quadrature::setLegendrePolynomials ( const int     degree,
                                                            const double  Evaluation_Point,
                                                                  double& Phi,
                                                                  double& Phi_Dash,
                                                                  double& Phi_Dash_Dash ) {

   const double& X = Evaluation_Point;
   int I;

   if ( degree == 0 ) {
      Phi           = 1.0;
      Phi_Dash      = 0.0;
      Phi_Dash_Dash = 0.0;
   } else if ( degree == 1 ) {

      Phi           = Evaluation_Point;
      Phi_Dash      = 1.0;
      Phi_Dash_Dash = 0.0;

   } else {

      double Phis   [ 3 ] = { 1.0, X, 1.5 * X * X - 0.5 };
      double PhisD  [ 3 ] = { 0.0, 1.0, 3.0 * X };
      double PhisDD [ 3 ] = { 0.0, 0.0, 3.0 };

      double Float_I;
      double Float_Ip1;
      double Float_2Ip1;

      // Use the 3-term recurrence relation to compute the higher order polynomials + derivatives
      for ( I = 2; I <= degree - 1; I++ ) {
         Float_I = double ( I );
         Float_Ip1 = double ( I + 1 );
         Float_2Ip1 = double ( 2 * I + 1 );

         Phis [ 0 ] = Phis [ 1 ];
         Phis [ 1 ] = Phis [ 2 ];

         PhisD [ 0 ] = PhisD [ 1 ];
         PhisD [ 1 ] = PhisD [ 2 ];

         PhisDD [ 0 ] = PhisDD [ 1 ];
         PhisDD [ 1 ] = PhisDD [ 2 ];

         Phis [ 2 ] = ( X * Float_2Ip1 * Phis [ 1 ] - Float_I * Phis [ 0 ]) / Float_Ip1;
         PhisD [ 2 ] = ( X * Float_2Ip1 * PhisD [ 1 ] - Float_Ip1 * PhisD [ 0 ]) / Float_I;
         PhisDD [ 2 ] =  ( Float_2Ip1 * ( PhisD [ 1 ] + X * PhisDD [ 1 ]) - Float_Ip1 * PhisDD [ 0 ] ) / Float_I;
      }

      Phi           = Phis   [ 2 ];
      Phi_Dash      = PhisD  [ 2 ];
      Phi_Dash_Dash = PhisDD [ 2 ];

   }

}

//------------------------------------------------------------//


void NumericFunctions::Quadrature::computeGaussLegendreQuadrature ( const int      degree,
                                                                          double*& Points,
                                                                          double*& Weights ) {


   const double Epsilon = DBL_EPSILON;

   double X1 = -1.0;
   double X2 =  1.0;

   double XM;
   double XL;
   double P1;
   double P2;
   double P3;
   double PP;
   double Z;
   double Z1;


   int N = degree;
   int I;
   int J;
   int M = ( N + 1 ) / 2;

   double Float_J;
   double Float_N = double ( N );
   double Den = 1.0 / ( Float_N + 0.5 );

   // Now compute the quadrature points and weights
   XM = 0.5 * ( X2 + X1 );
   XL = 0.5 * ( X2 - X1 );

   for ( I = 1; I <= M; I++ ) {
      Z = cos ( M_PI * ( double ( I ) - 0.25 ) * Den );

      // Find root of Legendre polynomial (using a Newton-Raphson method)
      do {
         P1 = 1.0;
         P2 = 0.0;

         // Compute N-th Legendre polynomial using the 3-term recurrence relation at point 'Z'.
         for ( J = 1; J <= N; J ++ ) {
            Float_J = double ( J );
            P3 = P2;
            P2 = P1;
            P1 = (( 2.0 * Float_J - 1.0 ) * Z * P2 - ( Float_J - 1.0 ) * P3 ) / Float_J;
         }

         // Derivative of Legendre polynomial.
         PP = double ( N ) * ( Z * P1 - P2 ) / ( Z * Z - 1.0 );
         Z1 = Z;

         // Newton Raphson method!
         Z  = Z1 - P1 / PP;
      } while ( fabs ( Z - Z1 ) > Epsilon );

      Points  [ I - 1 ]         = XM - XL * Z;
      Points  [ N + 1 - I - 1 ] = XM + XL * Z;
      Weights [ I - 1 ]         = 2.0 * XL / (( 1.0 - Z * Z ) * PP * PP );
      Weights [ N + 1 - I - 1 ] = Weights [ I - 1 ];

   }

}

//------------------------------------------------------------//

void NumericFunctions::Quadrature::computeGaussLobattoQuadrature ( const int      degree,
                                                                         double*& Points,
                                                                         double*& Weights ) {


   const double Epsilon = std::numeric_limits<double>::epsilon ();

   int    I;
   bool   Converged;
   double Old_Root;
   double New_Root;
   double Start_Point;
   double End_Point;

   // Since we know the maximum degree, then it is possible to use stack variables here.
   double* Legendre_Points  = new double [ degree ];
   double* Legendre_Weights = new double [ degree ];

   double L;   // Legendre polynomial
   double LD;  // Legendre polynomial first derivative
   double LDD; // Legendre polynomial second derivative

   double Weight_Scale = 2.0 / double ( degree * ( degree - 1 ));

   computeGaussLegendreQuadrature ( degree - 1, Legendre_Points, Legendre_Weights );

   Points [ 0 ]          = -1.0;
   Points [ degree - 1 ] =  1.0;

   Start_Point = -1.0;

   for ( I = 1; I < degree - 1; I++ ) {
      Converged = false;

      End_Point = Legendre_Points [ I ];
      Old_Root = 0.5 * ( End_Point + Start_Point );

      // Here we use the Newton-Raphson method to find the 
      // root of the derivative of the Legendre polynomial
      do {
         setLegendrePolynomials ( degree - 1, Old_Root, L, LD, LDD );
         New_Root = Old_Root - LD / LDD;
         Converged = fabs ( New_Root - Old_Root ) < Epsilon;
         Old_Root = New_Root;
      } while ( ! Converged );

      Points  [ I ] = New_Root;

      Weights [ I ] = Weight_Scale / ( L * L );
      Start_Point = End_Point;
   }

   Weights [ 0 ]           = Weight_Scale;
   Weights [ degree - 1 ]  = Weight_Scale;

   delete [] Legendre_Points;
   delete [] Legendre_Weights;
}

//------------------------------------------------------------//

void NumericFunctions::Quadrature::getFixedQuadrature ( const int              face,
                                                              QuadratureArray& points,
                                                              QuadratureArray& weights ) const {

   if ( 0 <= face and face < NumberOfFaces ) {
      points  = m_fixedFaceQuadraturePoints  [ face ];
      weights = m_fixedFaceQuadratureWeights [ face ];
   } else {
      points  = 0;
      weights = 0;
   }

}

//------------------------------------------------------------//

void NumericFunctions::Quadrature::initialiseQuadrature () {

   int degree;
   int i;

   for ( degree = 1; degree <= MaximumQuadratureDegree; degree++ ) {
      m_gaussLegendreQuadraturePoints  [ degree - 1 ] = new double [ degree ];
      m_gaussLegendreQuadratureWeights [ degree - 1 ] = new double [ degree ];
    
      computeGaussLegendreQuadrature ( degree,
                                       m_gaussLegendreQuadraturePoints  [ degree - 1 ],
                                       m_gaussLegendreQuadratureWeights [ degree - 1 ] );

      m_gaussLobattoQuadraturePoints   [ degree - 1 ] = new double [ NumericFunctions::Maximum ( 2, degree )];
      m_gaussLobattoQuadratureWeights  [ degree - 1 ] = new double [ NumericFunctions::Maximum ( 2, degree )];

      computeGaussLobattoQuadrature  ( NumericFunctions::Maximum ( 2, degree ),
                                       m_gaussLobattoQuadraturePoints  [ degree - 1 ],
                                       m_gaussLobattoQuadratureWeights [ degree - 1 ] );

   }

   for ( i = 0; i < NumberOfFaces; ++i ) {
      m_fixedFaceQuadraturePoints  [ i ] = new double [ 1 ];
      m_fixedFaceQuadratureWeights [ i ] = new double [ 1 ];
      m_fixedFaceQuadratureWeights [ i ][ 0 ] = 1.0;
   }

   // Bottom face.
   m_fixedFaceQuadraturePoints  [ 0 ][ 0 ] = -1.0;

   // Front face.
   m_fixedFaceQuadraturePoints  [ 1 ][ 0 ] = -1.0;

   // Right face.
   m_fixedFaceQuadraturePoints  [ 2 ][ 0 ] =  1.0;

   // Back face.
   m_fixedFaceQuadraturePoints  [ 3 ][ 0 ] =  1.0;

   // Left face.
   m_fixedFaceQuadraturePoints  [ 4 ][ 0 ] = -1.0;

   // Top face.
   m_fixedFaceQuadraturePoints  [ 5 ][ 0 ] =  1.0;

}

//------------------------------------------------------------//

void NumericFunctions::Quadrature::finaliseQuadrature () {

   if ( s_quadratureInstance == 0 ) {
      return;
   }

   int degree;
   int face;

   for ( degree = 0; degree < MaximumQuadratureDegree; degree++ ) {
      delete [] s_quadratureInstance->m_gaussLegendreQuadraturePoints  [ degree ];
      delete [] s_quadratureInstance->m_gaussLegendreQuadratureWeights [ degree ];

      delete [] s_quadratureInstance->m_gaussLobattoQuadraturePoints   [ degree ];
      delete [] s_quadratureInstance->m_gaussLobattoQuadratureWeights  [ degree ];
   }

   for ( face = 0; face < NumberOfFaces; ++face ) {
      delete [] s_quadratureInstance->m_fixedFaceQuadraturePoints  [ face ];
      delete [] s_quadratureInstance->m_fixedFaceQuadratureWeights [ face ];
   }

   s_quadratureInstance = 0;
}

//------------------------------------------------------------//
