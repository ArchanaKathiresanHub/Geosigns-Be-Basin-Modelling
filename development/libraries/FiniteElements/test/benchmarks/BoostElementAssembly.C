#include "BoostElementAssembly.h"


//-------------------------------------------------------------------------------------
using namespace boost_namespace;

BoostElementAssembly::BoostElementAssembly() {

   K.resize( 8, 8 );
  
}

//-------------------------------------------------------------------------------------

void BoostElementAssembly::InitNewImplementationOfAssembly( int xyQuadPts, int zQuadPts, const std::vector<double> & randData ) {

   int N = xyQuadPts * xyQuadPts * zQuadPts;
   
   size_t ir = 0;

   P.resize( 8, N, false );
   Pt.resize( N, 8, false );
   C.resize( N, N , 0, 0, false ); // diagonal 1x1
   PC.resize( 8, N, false  );
   G.resize( 8, 3 * N, false  );
   GB.resize(8, N, false  );
   B.resize( 3 * N, N, false  ); // diagonal 3x1
   A.resize( 3 * N,  3 * N, false  ); // diagonal 3x3
   GA.resize( 8, 3 * N, false  );

   for ( int i = 0; i < N; ++i ) 
      C(i, i) = randData[ir++];

   for ( int i = 0; i < 8; ++i ) {
      for ( int j = 0; j < N; ++j ) {
         P( i, j ) = randData[ir++];
      }
   }

   for ( int i = 0; i < 8; ++i ) {
      for ( int j = 0; j < 3 * N; ++j ) {
         G( i, j ) = randData[ir++];
      }
   }
   for ( int i = 0; i < B.size1(); i += 3 ) {
      for ( int li = 0; li < 3; ++li ) {
         B( i + li, i ) = randData[ir++];
      }
   }
#if 0
   for ( int i = 0; i < A.size1(); ++ i ) {
      matrix3d Ai;
      for ( int li = 0; li < 3; ++li ) {
         for ( int lj = 0; lj < 3; ++lj ) {
            Ai( li, lj ) = randData[ir++];
         }
      }
      A( i, i ) = Ai;
   }
#endif

   for ( int i = 0; i < A.size1(); i += 3 ) {
      for ( int li = 0; li < 3; ++li ) {
         for ( int lj = 0; lj < 3; ++lj ) {
            A( i+li, i+lj ) = randData[ir++];
         }
      }
   }
}


//-------------------------------------------------------------------------------------
void BoostElementAssembly::AssembleElementNew()  {

   //8, 2.1; 12, 1.08; 16, 1.54; 27, 3.83; 36, 5.82; 64, 16.84; 
   noalias(K)  = block_prod<matrix<double>, 64> ( block_prod<matrix<double>, 64>( P, C ), trans( P ) ) ; 
   // first order term
   noalias(K) += block_prod<matrix<double>, 64> ( block_prod<matrix<double>, 64> ( G, B ), trans( P ) ); 
   // second order term
   noalias(K) += block_prod<matrix<double>, 64> ( block_prod<matrix<double>, 64> ( G, A ), trans( G ) );


 #if 0   
   //8, 2.64; 12, 4.81; 16, 7.96; 27, 20.25; 36, 33.48; 64, 106.53; 

   Pt = trans( P );
   opb_prod( P, C, PC, true );
   opb_prod ( PC, Pt, K, true );

   // first order term
   opb_prod( G, B, GB, true );
   opb_prod( GB, Pt, K, false );
   
   // second order term
   opb_prod( G, A, GA, true );
   opb_prod( GA, trans ( G ), K,  false );
#endif

#if 0
   //8, 1.08; 12, 1.62; 16, 2.11; 27, 4.36; 36, 6.56; 64, 30.95;
   Pt = trans( P );
   PC = prod( P, C );
   K  = prod ( PC, Pt );

   // first order term
   GB = prod (G, B);
   K += prod ( GB, Pt );
   
   // second order term
   GA = prod (  G, A ); 
   K += prod ( GA, trans( G ) );
#endif   

#if 0
   //8, 0.91; 12, 1.42; 16, 2.11; 27, 5.62; 36, 8.1; 64, 23.03; 
   Pt = trans( P );
   PC = block_prod<matrix<double>, 64>( P, C );

   K  = block_prod<matrix<double>, 64> ( PC, Pt );
   // first order term

   GB = block_prod<matrix<double>, 64> (G, B);
   K += block_prod<matrix<double>, 64> ( GB, Pt );
   
   // second order term
   GA = block_prod<matrix<double>, 64> (  G, A );
   K += block_prod<matrix<double>, 64> ( GA, trans( G ) );
#endif  

#if 0
  //8, 1.35; 12, 2.26; 16, 3.43; 27, 7.85; 36, 12.68; 64, 35.3; 
   Pt = trans( P );

   axpy_prod( P, C, PC, true );
   axpy_prod ( PC, Pt, K, true );

   // first order term
   axpy_prod( G, B, GB, true );
   axpy_prod( GB, Pt, K, false );
   
   // second order term
   axpy_prod( G, A, GA, true );
   axpy_prod( GA, trans ( G ), K,  false );
#endif

}
//-------------------------------------------------------------------------------------
void BoostElementAssembly::OtherTests() {

   // tests of other methods
   c_matrix<double, 8, 64 > P1;
   c_matrix<double, 8, 64 > PC1;
   c_matrix<double, 64, 64 > C1;
   
   C1.clear();
   P1.clear();

   //  noalias(PC1) = prod( P1, C1 );
   noalias(PC1) = block_prod< c_matrix<double, 8, 64 >, 64 >( P1, C1 );
   
   diagonal_matrix< c_matrix<double, 1, 1> > C2;
   diagonal_matrix< c_matrix<double, 1, 1> > P2;
   diagonal_matrix< c_matrix<double, 1, 1> > PC2;
   
   P2.resize( 8, 64, 0, 0, false ); 
   C2.resize( 64, 8, 0, 0, false ); 
   PC2.resize( 8, 8, 0, 0, false ); 
   PC2 = P2 + C2;
   //   noalias(PC2) = block_prod< diagonal_matrix< c_matrix<double, 1, 1> >, 64 >( P2, C2 );
   //axpy_prod( P2, C2, PC2, false );
}

#if 0
// enable to build and run the test separatly from other benchmarks
//-------------------------------------------------------------------------------------

std::string getCompilerName() {
   std::string ver( __VERSION__ );
   return ver.substr( 0, 5 );
}

//-------------------------------------------------------------------------------------
int main ( int argc, char ** argv ) {

#ifdef NDEBUG
   const int  ElementsNumber = 50 * 50 * 20;
#else
   const int  ElementsNumber = 100;
#endif

   const int maxQPtsXY = 4;
   const int maxQPtsZ = 4;
   int N = maxQPtsXY * maxQPtsXY * maxQPtsZ; 

   double CPS = static_cast<double>( CLOCKS_PER_SEC );


   BoostElementAssembly bbs;
   std::vector< std::pair<int, double> > bbsTiming;

   std::vector<double> randData( ( 8*N /*P*/ + N*N /*C*/ + 8*3*N /*G*/ + 3*N*N /*B*/ + 3*N*3*N /*A*/ ) * 10 );
   for ( std::vector<double>::iterator it = randData.begin(); it != randData.end(); ++it ) *it = drand48();
   clock_t t1, t2 ;

   std::cout << "\nBoost " << getCompilerName() << " = [\n ";

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
         t2 = clock();
         std::cout << xyQuadPts * xyQuadPts * zQuadPts << ", " <<  static_cast<double>(( t2  - t1 )) / CPS << ";\n ";
      }
   }
   std::cout << "];\n";
 

   std::vector< std::pair<int, double> > bbsTiming1;

   std::cout << "\nBoostNew " << getCompilerName() << " = [\n ";

   for ( int xyQuadPts = 2;  xyQuadPts <= maxQPtsXY; ++xyQuadPts ) {
      for ( int zQuadPts = xyQuadPts; zQuadPts <= maxQPtsZ; ++zQuadPts ) {
         switch( xyQuadPts * xyQuadPts * zQuadPts ) {
         case 8: {
            BoostElementNewElementAssembly<8> bbs8;
            bbs8.InitNewImplementationOfAssembly( randData );
            
            t1 = clock();
            for ( int el = 0; el < ElementsNumber; ++el ) {  bbs8.AssembleElement(); }
            t2 = clock();
         }
            break;
         case 12: {
            BoostElementNewElementAssembly<12> bbs12;
            bbs12.InitNewImplementationOfAssembly( randData );
            
            t1 = clock();
            for ( int el = 0; el < ElementsNumber; ++el ) {  bbs12.AssembleElement(); }
            t2 = clock();
         }
            break;
         case 16: {
            BoostElementNewElementAssembly<16> bbs16;
            
            t1 = clock();
            for ( int el = 0; el < ElementsNumber; ++el ) {  bbs16.AssembleElement(); }
            t2 = clock();
         }
            break;
         case 27: {
            BoostElementNewElementAssembly<27> bbs27;
            bbs27.InitNewImplementationOfAssembly( randData );
            
            t1 = clock();
            for ( int el = 0; el < ElementsNumber; ++el ) {  bbs27.AssembleElement(); }
            t2 = clock();
         }
            break;
         case 36: {
            BoostElementNewElementAssembly<36> bbs36;
            bbs36.InitNewImplementationOfAssembly( randData );
            
            t1 = clock();
            for ( int el = 0; el < ElementsNumber; ++el ) {  bbs36.AssembleElement(); }
            t2 = clock();
         }
            break;
         case 64: {
            BoostElementNewElementAssembly<64> bbs64;
            bbs64.InitNewImplementationOfAssembly( randData );
            
            t1 = clock();
            for ( int el = 0; el < ElementsNumber; ++el ) {  bbs64.AssembleElement(); }
            t2 = clock();
         }
            break;
         }
         std::cout << xyQuadPts * xyQuadPts * zQuadPts << ", " <<  static_cast<double>(( t2  - t1 )) / CPS << ";\n ";
      }
   }
   std::cout << "];\n";

   return 0;
}
#endif
