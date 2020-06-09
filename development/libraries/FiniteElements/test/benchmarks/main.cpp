#include "ArmElementAssembly.h"
#include "CldElementAssembly.h"
#include "MklElementAssembly.h"
#include "SseElementAssembly.h"
#include "EgnElementAssembly.h"
#include "BoostElementAssembly.h"

#include <time.h>
#include <iostream>
#include <string>
#include <utility> 

#ifdef NDEBUG
const int  ElementsNumber = 50 * 50 * 20;
#else
const int  ElementsNumber = 100;
#endif

const int maxQPtsXY = 4;
const int maxQPtsZ = 4;

std::string getCompilerName()
{
   std::string ver( __VERSION__ );
   return ver.substr( 0, 5 );
}

// Run benchamrks in main()
void InitObjects( FiniteElementMethod::CldElementAssembly & cld, Eigen::EgnElementAssembly & egn, arma::ArmaElementAssembly & arma );

int main( int argc, char ** argv )
{
   FiniteElementMethod::CldElementAssembly  cld;
   Eigen::EgnElementAssembly                egn;
   arma::ArmaElementAssembly                arm;
   boost_namespace::BoostElementAssembly    bbs;

   double CPS = static_cast<double>( CLOCKS_PER_SEC );

   InitObjects( cld, egn, arm );

   cld.AssembleElement();
   egn.AssembleElement();
   arm.AssembleElement();

   // do checking the results
   for ( int i = 0; i < 8; ++i )
   {
      for ( int j = 0; j < 8; ++j )
      {
         assert( std::abs((cld.Element_Jacobian(i+1,j+1) - egn.Element_Jacobian(i,j)) / (cld.Element_Jacobian(i+1,j+1) + egn.Element_Jacobian(i,j))) < 1e-3 );
         assert( std::abs((cld.Element_Jacobian(i+1,j+1) - arm.Element_Jacobian(i,j)) / (cld.Element_Jacobian(i+1,j+1) + arm.Element_Jacobian(i,j))) < 1e-3 );
      }
   }


   std::cout << "\n#Old implementation benchmarks: " << std::endl;
   clock_t t1 = clock();

   for ( int el = 0; el < ElementsNumber; ++el )
   {
      cld.AssembleElement();
   }
   double cldTotTime = static_cast<double>( (clock() - t1 ) ) / CPS;

   std::cout << "CauldronOld" << getCompilerName() << " = [ 12 " << cldTotTime << " ];\n";


   t1 = clock();

   for ( int el = 0; el < ElementsNumber; ++el )
   {
      egn.AssembleElement();
   }
   double egnTotTime = static_cast<double>( (clock() - t1 ) ) / CPS; 

   std::cout << "EigenOld" << getCompilerName() << " = [ 12 " << egnTotTime << " ];\n";


   t1 = clock();

   for ( int el = 0; el < ElementsNumber; ++el )
   {
      arm.AssembleElement();
   }
   double armTotTime = static_cast<double>( (clock() - t1 ) ) / CPS;

   std::cout << "ArmadilloOld" << getCompilerName() << " = [ 12 " << armTotTime << " ];\n";

   std::cout << "#Eigen     / Cauldtron time relation: " << egnTotTime/cldTotTime << std::endl;
   std::cout << "#Armadillo / Cauldtron time relation: " << armTotTime/cldTotTime << std::endl;
   std::cout << "#Eigen     / Armadillo time relation: " << egnTotTime/armTotTime << std::endl;

   std::cout << "\n#New implementation benchmarks: " << std::endl;

   std::vector< std::pair<int, double> > egnTiming;
   std::vector< std::pair<int, double> > armTiming;
   std::vector< std::pair<int, double> > mklTiming;
   std::vector< std::pair<int, double> > sseTiming;
   std::vector< std::pair<int, double> > bbsTiming;

   // generat random array for equal data initialisation for armadillo and eigen
   int N = maxQPtsXY * maxQPtsXY * maxQPtsZ; 
   std::vector<double> randData( ( 8*N /*P*/ + N*N /*C*/ + 8*3*N /*G*/ + 3*N*N /*B*/ + 3*N*3*N /*A*/ ) * 10 );
   for ( std::vector<double>::iterator it = randData.begin(); it != randData.end(); ++it ) *it = drand48();

   // do check:
   {
      Eigen::EgnNewElementAssembly<8> egn8;
      egn8.InitAssembly( randData );
      egn8.AssembleElement();

      arm.InitNewImplementationOfAssembly( 2, 2, randData );
      arm.AssembleElementNew();

      Mkl::MklNewElementAssembly mmkl;
      mmkl.InitAssembly( 2, 2, randData );
      mmkl.AssembleElement();
 
      Sse::SseNewElementAssembly ssel;
      ssel.InitAssembly( 2, 2, randData );
      ssel.AssembleElement();

      bbs.InitNewImplementationOfAssembly( 2, 2, randData );
      bbs.AssembleElementNew();

      for ( int i = 0; i < 8; ++i )
      {
         for ( int j = 0; j < 8; ++j )
         {
            assert( std::abs( egn8.K(i,j) - arm.K(i,j)     ) / std::abs( egn8.K(i,j) + arm.K(i,j)     )  < 1e-3 );
            assert( std::abs( egn8.K(i,j) - mmkl.getK(i,j) ) / std::abs( egn8.K(i,j) + mmkl.getK(i,j) )  < 1e-3 );
            assert( std::abs( egn8.K(i,j) - ssel.getK(i,j) ) / std::abs( egn8.K(i,j) + ssel.getK(i,j) )  < 1e-3 );
            assert( std::abs( egn8.K(i,j) - bbs.K(i,j) ) / std::abs( egn8.K(i,j) + bbs.K(i,j) )  < 1e-3 );
         }
      }
   }

   // Eigen run
   for ( int xyQuadPts = 2;  xyQuadPts <= maxQPtsXY ; ++xyQuadPts )
   {
      for ( int zQuadPts = xyQuadPts; zQuadPts <= maxQPtsZ; ++zQuadPts )
      {
         switch( xyQuadPts * xyQuadPts * zQuadPts )
         {
            case 8: 
               { 
                  Eigen::EgnNewElementAssembly<8> egn8;
                  egn8.InitAssembly( randData );
                  t1 = clock();
                  for ( int el = 0; el < ElementsNumber; ++el ) { egn8.AssembleElement(); }
                  egnTiming.push_back( std::pair<int,double>( xyQuadPts * xyQuadPts * zQuadPts, static_cast<double>( (clock() - t1 ) ) / CPS ) );
               }
               break;

            case 12:
               {
                  Eigen::EgnNewElementAssembly<12> egn12;
                  egn12.InitAssembly( randData );
                  t1 = clock();
                  for ( int el = 0; el < ElementsNumber; ++el ) { egn12.AssembleElement(); }
                  egnTiming.push_back( std::pair<int,double>( xyQuadPts * xyQuadPts * zQuadPts, static_cast<double>( (clock() - t1 ) ) / CPS ) );
               }
               break;

            case 16:
               {
                  Eigen::EgnNewElementAssembly<16>  egn16;
                  egn16.InitAssembly( randData );
                  t1 = clock();
                  for ( int el = 0; el < ElementsNumber; ++el ) { egn16.AssembleElement(); }
                  egnTiming.push_back( std::pair<int,double>( xyQuadPts * xyQuadPts * zQuadPts, static_cast<double>( (clock() - t1 ) ) / CPS ) );
               }
               break;

            case 27:
               {
                  Eigen::EgnNewElementAssembly<27>  egn27;
                  egn27.InitAssembly( randData );
                  t1 = clock();
                  for ( int el = 0; el < ElementsNumber; ++el ) { egn27.AssembleElement(); }
                  egnTiming.push_back( std::pair<int,double>( xyQuadPts * xyQuadPts * zQuadPts, static_cast<double>( (clock() - t1 ) ) / CPS ) );
               }
               break;

            case 36:
               {
                  Eigen::EgnNewElementAssembly<36>  egn36;
                  egn36.InitAssembly( randData );
                  t1 = clock();
                  for ( int el = 0; el < ElementsNumber; ++el ) { egn36.AssembleElement(); }
                  egnTiming.push_back( std::pair<int,double>( xyQuadPts * xyQuadPts * zQuadPts, static_cast<double>( (clock() - t1 ) ) / CPS ) );
               }
               break;

            case 64:
               {
                  Eigen::EgnNewElementAssembly<64>  egn64;
                  egn64.InitAssembly( randData );
                  t1 = clock();
                  for ( int el = 0; el < ElementsNumber; ++el ) { egn64.AssembleElement(); }
                  egnTiming.push_back( std::pair<int,double>( xyQuadPts * xyQuadPts * zQuadPts, static_cast<double>( (clock() - t1 ) ) / CPS ) );
               }
               break;

            default:
               assert( 0 );
               break;
         }
      }
   }

   std::cout << "\nEigenNew" << getCompilerName() << " = [\n";
   for ( size_t i = 0; i < egnTiming.size(); ++i ) std::cout << egnTiming[i].first << ", " << egnTiming[i].second << "; ";
   std::cout << "];\n";

   // Armadillo run
   for ( int xyQuadPts = 2;  xyQuadPts <= maxQPtsXY; ++xyQuadPts )
   {
      for ( int zQuadPts = xyQuadPts; zQuadPts <= maxQPtsZ; ++zQuadPts )
      {
         arm.InitNewImplementationOfAssembly( xyQuadPts, zQuadPts, randData );

         t1 = clock();
         for ( int el = 0; el < ElementsNumber; ++el )
         {
            arm.AssembleElementNew();
         }
         armTiming.push_back( std::pair<int,double>( xyQuadPts * xyQuadPts * zQuadPts, static_cast<double>( (clock() - t1 ) ) / CPS ) ); 
      }
   }
   std::cout << "\nArmadilloNew" << getCompilerName() << " = [\n";
   for ( size_t i = 0; i < armTiming.size(); ++i ) std::cout << armTiming[i].first << ", " << armTiming[i].second << "; ";
   std::cout << "];\n";

   // MKL run
   Mkl::MklNewElementAssembly mmkl;

   for ( int xyQuadPts = 2;  xyQuadPts <= maxQPtsXY; ++xyQuadPts )
   {
      for ( int zQuadPts = xyQuadPts; zQuadPts <= maxQPtsZ; ++zQuadPts )
      {
         mmkl.InitAssembly( xyQuadPts, zQuadPts, randData );

         t1 = clock();
         for ( int el = 0; el < ElementsNumber; ++el )
         {
            mmkl.AssembleElement();
         }
         mklTiming.push_back( std::pair<int,double>( xyQuadPts * xyQuadPts * zQuadPts, static_cast<double>( (clock() - t1 ) ) / CPS ) ); 
      }
   }

   std::cout << "\nMklNew" << getCompilerName() << " = [\n";
   for ( size_t i = 0; i < mklTiming.size(); ++i ) std::cout << mklTiming[i].first << ", " << mklTiming[i].second << "; ";
   std::cout << "];\n";

   
    // Boost run
   for ( int xyQuadPts = 2;  xyQuadPts <= maxQPtsXY; ++xyQuadPts )
   {
      for ( int zQuadPts = xyQuadPts; zQuadPts <= maxQPtsZ; ++zQuadPts )
      {
         bbs.InitNewImplementationOfAssembly( xyQuadPts, zQuadPts, randData );

         t1 = clock();
         for ( int el = 0; el < ElementsNumber; ++el )
         {
            bbs.AssembleElementNew();
         }
         bbsTiming.push_back( std::pair<int,double>( xyQuadPts * xyQuadPts * zQuadPts, static_cast<double>( (clock() - t1 ) ) / CPS ) ); 
      }
   }

   std::cout << "\nBoostNew" << getCompilerName() << " = [\n";
   for ( size_t i = 0; i < bbsTiming.size(); ++i ) std::cout << bbsTiming[i].first << ", " << bbsTiming[i].second << "; ";
   std::cout << "];\n";

   // Bill's SSE run
   Sse::SseNewElementAssembly ssel;
   for ( int xyQuadPts = 2;  xyQuadPts <= maxQPtsXY; ++xyQuadPts )
   {
      for ( int zQuadPts = xyQuadPts; zQuadPts <= maxQPtsZ; ++zQuadPts )
      {
         ssel.InitAssembly( xyQuadPts, zQuadPts, randData );

         t1 = clock();
         for ( int el = 0; el < ElementsNumber; ++el )
         {
            ssel.AssembleElement();
         }
         sseTiming.push_back( std::pair<int,double>( xyQuadPts * xyQuadPts * zQuadPts, static_cast<double>( (clock() - t1 ) ) / CPS ) ); 
      }
   }
   std::cout << "\nSSENew" << getCompilerName() << " = [\n";
   for ( size_t i = 0; i < sseTiming.size(); ++i ) std::cout << sseTiming[i].first << ", " << sseTiming[i].second << "; ";
   std::cout << "];\n";
 
}


void InitObjects( FiniteElementMethod::CldElementAssembly & cld, Eigen::EgnElementAssembly & egn, arma::ArmaElementAssembly & arm )
{
   // Geometry matrix initialisation
   cld.geometryMatrix(1,1) = 500;
   cld.geometryMatrix(1,2) = 4500;
   cld.geometryMatrix(1,3) = 4500;
   cld.geometryMatrix(1,4) = 500;
   cld.geometryMatrix(1,5) = 500;
   cld.geometryMatrix(1,6) = 4500;
   cld.geometryMatrix(1,7) = 4500;
   cld.geometryMatrix(1,8) = 500;

   cld.geometryMatrix(2,1) = 500;
   cld.geometryMatrix(2,2) = 500;
   cld.geometryMatrix(2,3) = 4500;
   cld.geometryMatrix(2,4) = 4500;
   cld.geometryMatrix(2,5) = 500;
   cld.geometryMatrix(2,6) = 500;
   cld.geometryMatrix(2,7) = 4500;
   cld.geometryMatrix(2,8) = 4500;

   cld.geometryMatrix(3,1) = 1.1e5;
   cld.geometryMatrix(3,2) = 1.1e5;
   cld.geometryMatrix(3,3) = 1.1e5;
   cld.geometryMatrix(3,4) = 1.1e5;
   cld.geometryMatrix(3,5) = 1.2e5;
   cld.geometryMatrix(3,6) = 1.2e5;
   cld.geometryMatrix(3,7) = 1.2e5;
   cld.geometryMatrix(3,8) = 1.2e5;

   egn.geometryMatrix << 500,   4500,  4500,  500,   500,   4500,  4500,  500,
                         500,   500,   4500,  4500,  500,   500,   4500,  4500,
                         1.1e5, 1.1e5, 1.1e5, 1.1e5, 1.2e5, 1.2e5, 1.2e5, 1.2e5;

   arm.geometryMatrix << 500   << 4500  << 4500  << 500   << 500   << 4500  << 4500  << 500   << arma::endr
                      << 500   << 500   << 4500  << 4500  << 500   << 500   << 4500  << 4500  << arma::endr
                      << 1.1e5 << 1.1e5 << 1.1e5 << 1.1e5 << 1.2e5 << 1.2e5 << 1.2e5 << 1.2e5 << arma::endr;

   cld.previousGeometryMatrix(1,1) = 5.0e+02;
   cld.previousGeometryMatrix(1,2) = 4.5e+03;
   cld.previousGeometryMatrix(1,3) = 4.5e+03;
   cld.previousGeometryMatrix(1,4) = 5.0e+02;
   cld.previousGeometryMatrix(1,5) = 5.0e+02;
   cld.previousGeometryMatrix(1,6) = 4.5e+03;
   cld.previousGeometryMatrix(1,7) = 4.5e+03;
   cld.previousGeometryMatrix(1,8) = 5.0e+02;

   cld.previousGeometryMatrix(2,1) = 5.0e+02;
   cld.previousGeometryMatrix(2,2) = 5.0e+02;
   cld.previousGeometryMatrix(2,3) = 4.5e+03;
   cld.previousGeometryMatrix(2,4) = 4.5e+03;
   cld.previousGeometryMatrix(2,5) = 5.0e+02;
   cld.previousGeometryMatrix(2,6) = 5.0e+02;
   cld.previousGeometryMatrix(2,7) = 4.5e+03;
   cld.previousGeometryMatrix(2,8) = 4.5e+03;

   cld.previousGeometryMatrix(3,1) = 0.0e+00;
   cld.previousGeometryMatrix(3,2) = 0.0e+00;
   cld.previousGeometryMatrix(3,3) = 0.0e+00;
   cld.previousGeometryMatrix(3,4) = 0.0e+00;
   cld.previousGeometryMatrix(3,5) = 0.0e+00;
   cld.previousGeometryMatrix(3,6) = 0.0e+00;
   cld.previousGeometryMatrix(3,7) = 0.0e+00;
   cld.previousGeometryMatrix(3,8) = 0.0e+00;

   egn.previousGeometryMatrix << 5.0e+02, 4.5e+03, 4.5e+03, 5.0e+02, 5.0e+02, 4.5e+03, 4.5e+03, 5.0e+02,
                                 5.0e+02, 5.0e+02, 4.5e+03, 4.5e+03, 5.0e+02, 5.0e+02, 4.5e+03, 4.5e+03,
                                 0.0e+00, 0.0e+00, 0.0e+00, 0.0e+00, 0.0e+00, 0.0e+00, 0.0e+00, 0.0e+00;

   arm.previousGeometryMatrix << 5.0e+02 << 4.5e+03 << 4.5e+03 << 5.0e+02 << 5.0e+02 << 4.5e+03 << 4.5e+03 << 5.0e+02 << arma::endr
                              << 5.0e+02 << 5.0e+02 << 4.5e+03 << 4.5e+03 << 5.0e+02 << 5.0e+02 << 4.5e+03 << 4.5e+03 << arma::endr
                              << 0.0e+00 << 0.0e+00 << 0.0e+00 << 0.0e+00 << 0.0e+00 << 0.0e+00 << 0.0e+00 << 0.0e+00 << arma::endr;

   // make element property random
   cld.Current_Element_VES.Randomise();
   cld.Current_Element_Max_VES.Randomise();

   cld.Current_Ph.Randomise();
   cld.Current_Po.Randomise();
   cld.Current_Lp.Randomise();

   cld.Current_Element_Temperature.Randomise();
   cld.Element_Heat_Production.Randomise();
   cld.Current_Element_Chemical_Compaction.Randomise();

   for ( int i = 0; i < 8; ++i )
   {  // copy to egn
      egn.Current_Element_VES(i)                 = cld.Current_Element_VES(i+1);
      egn.Current_Element_Max_VES(i)             = cld.Current_Element_Max_VES(i+1);

      egn.Current_Ph(i)                          = cld.Current_Ph(i+1);
      egn.Current_Po(i)                          = cld.Current_Po(i+1);
      egn.Current_Lp(i)                          = cld.Current_Lp(i+1);

      egn.Current_Element_Temperature(i)         = cld.Current_Element_Temperature(i+1);
      egn.Element_Heat_Production(i)             = cld.Element_Heat_Production(i+1);
      egn.Current_Element_Chemical_Compaction(i) = cld.Current_Element_Chemical_Compaction(i+1);

      // copy to arma
      arm.Current_Element_VES(i)                 = cld.Current_Element_VES(i+1);
      arm.Current_Element_Max_VES(i)             = cld.Current_Element_Max_VES(i+1);

      arm.Current_Ph(i)                          = cld.Current_Ph(i+1);
      arm.Current_Po(i)                          = cld.Current_Po(i+1);
      arm.Current_Lp(i)                          = cld.Current_Lp(i+1);

      arm.Current_Element_Temperature(i)         = cld.Current_Element_Temperature(i+1);
      arm.Element_Heat_Production(i)             = cld.Element_Heat_Production(i+1);
      arm.Current_Element_Chemical_Compaction(i) = cld.Current_Element_Chemical_Compaction(i+1);
   }

   cld.Previous_Element_Temperature(1) = 1.22233333e3;
   cld.Previous_Element_Temperature(2) = 1.22233333e3;
   cld.Previous_Element_Temperature(3) = 1.22233333e3;
   cld.Previous_Element_Temperature(4) = 1.22233333e3;
   cld.Previous_Element_Temperature(5) = 1.3330e3;
   cld.Previous_Element_Temperature(6) = 1.3330e3;
   cld.Previous_Element_Temperature(7) = 1.3330e3;
   cld.Previous_Element_Temperature(8) = 1.3330e3;

   egn.Previous_Element_Temperature << 1.22233333e3, 1.22233333e3, 1.22233333e3, 1.22233333e3, 1.3330e3, 1.3330e3, 1.3330e3, 1.3330e3;

   arm.Previous_Element_Temperature << 1.22233333e3 << 1.22233333e3 << 1.22233333e3 << 1.22233333e3 << 1.3330e3 << 1.3330e3 << 1.3330e3 << 1.3330e3;
}

