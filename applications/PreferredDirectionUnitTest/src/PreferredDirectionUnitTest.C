#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <string>

#include "NumericFunctions.h"
#include "FiniteElementTypes.h"
#include "BasisFunction.h"

#include "FiniteElement.h"

//
// Unit test for secondary migration direction vector.
//
// Tests the code for generating the preferred flow direction from a tilted
// strata, possibly, with a non unit anisotropy.
//
//
// There are 2 ways of performing the test, 
//
//      i) a single test direction vector, based on a single tilt angle in 
//         one direction and a user defined anisotropy. NOTE the z-component of
//         the predicted direction will the opposite sign to that of the computed 
//         direction this is due to the sign of the depth direction; and
//
//      ii) multiple test direction vectors:
//
//           a) Fixed tilt angle (18 deg, pi/10 rads) in one direction, varying 
//              anisotropy, in increasing steps in the range of [ 1 .. 1000 ];
//
//           b) Fixed anisotropy (4) and varying the tilt angle (again in one direction)
//              the angle is in that range [0 .. 45 deg (pi/4 rads)]
//
//           c) Fixed anisotropy (10) and varying tilt angles in two directions,
//              both angles are in that range [0 .. 45 deg (pi/4 rads)].
//
//
//
// To run the test:
//
//    To get a single direction vector, run programme with the following parameters:
//
//          -aniso <anisotropy>
//
//    To get tables with i) with fixed tile angle (18 deg, pi/10 rads), ii) tilting in 
//    a single direction with a fixed anisotropy (4) and iii) tilting in 2 directions
//    again with a fixed anisotropy (4), run with the following parameter:
//
//          -table
//
//


using namespace std;
using namespace FiniteElementMethod;

void fillGeometryMatrix  ( const double depth1,
                           const double depth2,
                           ElementGeometryMatrix& gm );

void computeSingleDirection ( const double anisotropy );

void computeMultipleDirections ();

void stringToDouble ( const char* str, double& value, bool& error );

void printHelp ( const char* commandName );

int main ( int argc, char* argv []) {

   if ( argc == 3 and string ( argv [ 1 ]) == "-aniso" ) {
      double anisotropy;
      bool error;

      stringToDouble ( argv [ 2 ], anisotropy, error );

      if ( not error ) {
         computeSingleDirection ( anisotropy );
      } else {
         printHelp ( argv [ 0 ]);
      }

   } else if ( argc == 2 and string ( argv [ 1 ] ) == "-table" ) {
      computeMultipleDirections ();
   } else {
      printHelp ( argv [ 0 ]);
   }

   return 0;
}


void fillGeometryMatrix ( const double depth1,
                          const double depth2,
                          ElementGeometryMatrix& gm ) {

   double cornerDepth1 = 1000.0;
   double cornerDepth2 = cornerDepth1 + depth1;
   double cornerDepth3 = cornerDepth1 + depth2;

   gm ( 1, 1 ) = 0.0;
   gm ( 2, 1 ) = 0.0;
   gm ( 3, 1 ) = 0.0;

   gm ( 1, 2 ) = 1000.0;
   gm ( 2, 2 ) = 0.0;
   gm ( 3, 2 ) = 0.0;

   gm ( 1, 3 ) = 1000.0;
   gm ( 2, 3 ) = 1000.0;
   gm ( 3, 3 ) = 0.0;

   gm ( 1, 4 ) = 0.0;
   gm ( 2, 4 ) = 1000.0;
   gm ( 3, 4 ) = 0.0;



   gm ( 1, 5 ) = 0.0;
   gm ( 2, 5 ) = 0.0;
   gm ( 3, 5 ) = cornerDepth2;

   gm ( 1, 6 ) = 1000.0;
   gm ( 2, 6 ) = 0.0;
   gm ( 3, 6 ) = cornerDepth1;

   gm ( 1, 7 ) = 1000.0;
   gm ( 2, 7 ) = 1000.0;
   gm ( 3, 7 ) = cornerDepth1;

   gm ( 1, 8 ) = 0.0;
   gm ( 2, 8 ) = 1000.0;
   gm ( 3, 8 ) = cornerDepth3;

//    gm ( 1, 5 ) = 0.0;
//    gm ( 2, 5 ) = 0.0;
//    gm ( 3, 5 ) = cornerDepth2;

//    gm ( 1, 6 ) = 1000.0;
//    gm ( 2, 6 ) = 0.0;
//    gm ( 3, 6 ) = cornerDepth3;

//    gm ( 1, 7 ) = 1000.0;
//    gm ( 2, 7 ) = 1000.0;
//    gm ( 3, 7 ) = cornerDepth1;

//    gm ( 1, 8 ) = 0.0;
//    gm ( 2, 8 ) = 1000.0;
//    gm ( 3, 8 ) = cornerDepth1;

}



void computeSingleDirection ( const double anisotropy ) {


   // This depth corresponds to a angle of 18 degrees (pi / 10 rads).
   const double depth      = 324.9197; //std::atof ( argv [ 2 ]);

   // The z value here is the top of the element, used rather than the 
   // element-centre because its easier to compute what the angle is.
   // For the element-centre change from 1.0 to 0.0.
   const double zed        = 1.0;

   const double permeability = 1.0;

   ElementGeometryMatrix gm;

   ThreeVector direction;
   ThreeVector direction2;
   ThreeVector vertical;

   double      theta;

   fillGeometryMatrix ( depth, depth, gm );

   vertical.zero ();
   vertical ( 3 ) = -1.0;

   // Predicted preferred direction vector.
   theta = atan ( depth / 1000.0 );
   direction2 ( 1 ) = 0.5 * ( anisotropy - 1.0 ) * sin ( 2.0 * theta );
   direction2 ( 2 ) = 0.0;
   direction2 ( 3 ) = pow ( cos ( theta ), 2 ) + anisotropy * pow ( sin ( theta ), 2 );
   FiniteElementMethod::normalise ( direction2 );

   cout << endl;
   cout << " theta " << theta << "  " << theta * 180 / M_PI << endl;
   cout << endl;
   cout << " predicted direction :";
   cout << "  " 
        << setw ( 14 ) << direction2 ( 1 ) << "  " 
        << setw ( 14 ) << direction2 ( 2 ) << "  " 
        << setw ( 14 ) << direction2 ( 3 ) << "  " 
        << endl;

   FiniteElement element;
   Matrix3x3 permTensor2;

   // Computed preferred direction vector.
   element.setGeometry ( gm );
   element.setQuadraturePoint ( 0.0, 0.0, zed );
   element.setTensor ( permeability, anisotropy * permeability, permTensor2 );

   matrixVectorProduct ( permTensor2, vertical, direction );
   FiniteElementMethod::normalise ( direction );
   
   cout << " computed direction  :";
   cout << "  " 
        << setw ( 14 ) << direction  ( 1 ) << "  " 
        << setw ( 14 ) << direction  ( 2 ) << "  " 
        << setw ( 14 ) << direction  ( 3 ) << "  " 
        << endl;

   cout << endl;

   cout << " ratio of directions |  angle (rad) | angle (deg)" << endl;
   cout << " --------------------+--------------+------------" << endl;
   cout << setw ( 14 ) << fabs ( direction ( 1 ) / direction ( 3 )) << "       |"
        << setw ( 10 ) << acos ( FiniteElementMethod::innerProduct ( direction, vertical )) << "    |"
        << setw ( 11 ) << acos ( FiniteElementMethod::innerProduct ( direction, vertical )) * 180.0 / M_PI
        << endl;
   cout << endl;


}

void computeMultipleDirections () {

   // This depth corresponds to a angle of 18 degrees (pi / 10 rads).
   const double depth      = 324.9197; //std::atof ( argv [ 2 ]);

   // The zed value here is the top of the element, used rather than the element-centre
   // because its easier to compute what the angle is.
   const double zed        = 1.0;

   double permeability = 1.0;

   ElementGeometryMatrix gm;

   ThreeVector direction;
   ThreeVector direction2;
   ThreeVector vertical;

   double      theta;
   FiniteElement element;
   Matrix3x3 permTensor2;

   fillGeometryMatrix ( depth, depth, gm );

   vertical.zero ();
   vertical ( 3 ) = -1.0;

   element.setGeometry ( gm );
   element.setQuadraturePoint ( 0.0, 0.0, zed );

   const int size = 19;
   int i;

   double anisotropy;
   double anisotropies [ size ] = {    1.0,   1.5,   2.2,   3.3,   4.7,   6.8, 
                                      10.0,  15.0,  22.0,  33.0,  47.0,  68.0, 
                                     100.0, 150.0, 220.0, 330.0, 470.0, 680.0, 
                                    1000.0 };

   theta = atan ( depth / 1000.0 );

   cout << endl;
   cout << " Single direction tilt with varying anisotropy." << endl;
   cout << endl;
   cout << " Tilt angle: " << theta << " rads, " << theta * 180 / M_PI << " degs " << endl;
   cout << endl;

                         

   cout << "  anisotropy  | ratio of directions |  angle (rad) | angle (deg)" << endl;
   cout << " -------------+---------------------+--------------+------------" << endl;
   
   for ( i = 0; i < size; ++i ) {
      anisotropy = anisotropies [ i ];

      direction2 ( 1 ) = 0.5 * ( anisotropy - 1.0 ) * sin ( 2.0 * theta );
      direction2 ( 2 ) = 0.0;
      direction2 ( 3 ) = pow ( cos ( theta ), 2 ) + anisotropy * pow ( sin ( theta ), 2 );
      FiniteElementMethod::normalise ( direction2 );

      element.setTensor ( permeability, anisotropy * permeability, permTensor2 );

      matrixVectorProduct ( permTensor2, vertical, direction );
      FiniteElementMethod::normalise ( direction );

      cout << setw ( 10 ) << anisotropy  << "    |"
           << setw ( 14 ) << fabs ( direction ( 1 ) / direction ( 3 )) << "       |"
           << setw ( 10 ) << acos ( FiniteElementMethod::innerProduct ( direction, vertical )) << "    |"
           << setw ( 11 ) << acos ( FiniteElementMethod::innerProduct ( direction, vertical )) * 180.0 / M_PI
           << endl;
   }

   cout << endl;
   cout << endl;

   anisotropy = 4.0;

   cout << " Multi-direction tilt (same in both directions) with fixed anisotropy of " << anisotropy << endl;
   cout << endl;
   cout << "  tilt angle  | ratio of directions |  angle (rad) | angle (deg)" << endl;
   cout << " -------------+---------------------+--------------+------------" << endl;
   

   const int numberOfDepths = 20;
   double extraDepthX;
   double extraDepthY;
   double extraDepths [ numberOfDepths ] = {    0.0,
                                                1.0,   1.5,   2.2,   3.3,   4.7,   6.8, 
                                               10.0,  15.0,  22.0,  33.0,  47.0,  68.0, 
                                              100.0, 150.0, 220.0, 330.0, 470.0, 680.0, 
                                             1000.0 };

   for ( i = 0; i < numberOfDepths; ++i ) {
      extraDepthX = extraDepths [ i ];
      theta = atan ( extraDepthX / 1000.0 );

      direction2 ( 1 ) = 0.5 * ( anisotropy - 1.0 ) * sin ( 2.0 * theta );
      direction2 ( 2 ) = 0.0;
      direction2 ( 3 ) = pow ( cos ( theta ), 2 ) + anisotropy * pow ( sin ( theta ), 2 );
      FiniteElementMethod::normalise ( direction2 );
      fillGeometryMatrix ( extraDepthX, extraDepthX, gm );
      element.setGeometry ( gm );
      element.setQuadraturePoint ( 0.0, 0.0, zed );

      element.setTensor ( permeability, anisotropy * permeability, permTensor2 );

      matrixVectorProduct ( permTensor2, vertical, direction );
      FiniteElementMethod::normalise ( direction );

      cout << setw ( 10 ) << 180.0 * theta / M_PI  << "    |"
           << setw ( 16 ) << fabs ( direction ( 1 ) / direction ( 3 )) << "     |"
           << setw ( 12 ) << acos ( FiniteElementMethod::innerProduct ( direction, vertical )) << "  |"
           << setw ( 11 ) << acos ( FiniteElementMethod::innerProduct ( direction, vertical )) * 180.0 / M_PI
           << endl;
   }

   cout << endl;
   cout << endl;

   anisotropy = 10.0;

   cout << " Multi-direction tilt with fixed anisotropy of " << anisotropy << endl;
   cout << endl;
   cout << "  x tilt angle  | y tilt angle  | ratio of directions |  angle (rad) | angle (deg)" << endl;
   cout << " ---------------+---------------+---------------------+--------------+------------" << endl;
   

   double thetaX;
   double thetaY;

   for ( i = 0; i < numberOfDepths; ++i ) {
      extraDepthX = extraDepths [ i ];
      thetaX = atan ( extraDepthX / 1000.0 );

      extraDepthY = extraDepths [ numberOfDepths - i - 1 ];
      thetaY = atan ( extraDepthY / 1000.0 );

      direction2 ( 1 ) = 0.5 * ( anisotropy - 1.0 ) * sin ( 2.0 * theta );
      direction2 ( 2 ) = 0.0;
      direction2 ( 3 ) = pow ( cos ( theta ), 2 ) + anisotropy * pow ( sin ( theta ), 2 );
      FiniteElementMethod::normalise ( direction2 );
      fillGeometryMatrix ( extraDepthX, extraDepthY, gm );
      element.setGeometry ( gm );
      element.setQuadraturePoint ( 0.0, 0.0, zed );

      element.setTensor ( permeability, anisotropy * permeability, permTensor2 );

      matrixVectorProduct ( permTensor2, vertical, direction );
      FiniteElementMethod::normalise ( direction );

      cout << "  " 
           << setw ( 10 ) << 180.0 * thetaX / M_PI  << "    |"
           << setw ( 11 ) << 180.0 * thetaY / M_PI  << "    |"
           << setw ( 14 ) << fabs ( direction ( 1 ) / direction ( 3 )) << "       |"
           << setw ( 10 ) << acos ( FiniteElementMethod::innerProduct ( direction, vertical )) << "    |"
           << setw ( 11 ) << acos ( FiniteElementMethod::innerProduct ( direction, vertical )) * 180.0 / M_PI
           << endl;
   }

   cout << endl;
   cout << endl;


}

void printHelp ( const char* commandName ) {

   cout << endl;
   cout << "usage: " << commandName << "  [-aniso permeability-anisotropy] [-table]" << endl;
   cout << endl 
        << "     where permeability-anisotropy is a number such that 0 <= permeability-anisotropy < inf " << endl;
   cout << "     -aniso number       prints values for a singe anisotropy" << endl;
   cout << "     -table              prints values for a range of anisotropies" << endl;
   cout << endl;

}

void stringToDouble ( const char* str, double& value, bool& error ) {

  char* strEnd;

  value = int ( std::strtod ( str, &strEnd ));
  error = ( strEnd == str );

}
