#ifndef BOOST_ELEMENT_ASSEMBLY_H
#define BOOST_ELEMENT_ASSEMBLY_H

#include "ElementAssembly.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/banded.hpp>
#include <boost/numeric/ublas/operation.hpp>
#include <boost/numeric/ublas/operation_blocked.hpp>

using namespace boost::numeric::ublas;
namespace boost_namespace
{
   class BoostElementAssembly : public ElementAssembly
   {
   public:
      BoostElementAssembly();
      ~BoostElementAssembly() {;}

      matrix <double, row_major, bounded_array<double, 64> > K;

   private:
      matrix <double> P;
      matrix <double> Pt;
      diagonal_matrix <double> C;
      matrix <double> PC;

      matrix <double> G;
      matrix <double> B; 
      matrix <double> GB; 
 
      matrix <double> GA;
      // boost::numeric::ublas::diagonal_matrix < c_matrix<double, 3, 3> > A; //sparse
      matrix <double> A; //sparse
      
    public:
      void InitNewImplementationOfAssembly( int xyQuadPts, int zQuadPts, const std::vector<double> & randData );
      void AssembleElementNew();
      void OtherTests();
   };
   
   typedef c_matrix<double, 3, 3> matrix3d;
   typedef c_matrix<double, 3, 1> matrix3x1d;
   typedef c_matrix<double, 1, 1> matrix1d;

   class BoostElementAssembly1 : public ElementAssembly
   {
   public:
      BoostElementAssembly1();
      ~BoostElementAssembly1() {;}

      matrix <double> K;

   private:
      matrix <double> P;
      matrix <double> Pt;
      diagonal_matrix <matrix1d> C;
      matrix <double> PC;

      matrix <double> G;
      diagonal_matrix <matrix3x1d> B; 
      matrix <double> GB; 
 
      matrix <double> GA;
      diagonal_matrix < matrix3d > A;
      
    public:
      void InitNewImplementationOfAssembly( int xyQuadPts, int zQuadPts, const std::vector<double> & randData );
      void AssembleElementNew();
   };

   template<int _N> class BoostElementNewElementAssembly 
   {
   public:
      c_matrix<double, 8, 8> K;

   private:
      c_matrix<double, 8, _N> P;
      c_matrix<double, _N, 8> Pt;
      c_matrix<double, _N, _N> C;
      c_matrix<double, 8, _N> PC;
      c_matrix<double, 8, 3 * _N> G;
      c_matrix<double, 8, _N> GB;
      c_matrix<double, 3 * _N, 3 * _N > A; 
      c_matrix<double, 3 * _N,  _N > B; 
      c_matrix<double, 8, 3 * _N > GA;

    public:
      void InitNewImplementationOfAssembly( const std::vector<double> & randData );
      void AssembleElement();

   };

   template<int _N> void BoostElementNewElementAssembly<_N>::InitNewImplementationOfAssembly(const std::vector<double> & randData ) {
      size_t ir = 0;
      C.clear();
      P.clear();
      for ( int i = 0; i < _N; ++i ) {
         C(i, i) = randData[ir++];
         
         for ( int j = 0; j < 8; ++j ) {
            P( j, i ) = randData[ir++];
         }
      }
      G.clear();
      for ( int i = 0; i < 8; ++i ) {
         for ( int j = 0; j < 3 * _N; ++j ) {
            G( i, j ) = randData[ir++];
         }
      }
      B.clear();
      for ( int i = 0; i < B.size1();  i += 3 ) {
         for ( int li = 0; li < 3; ++li ) {
            B( i+li, i ) = randData[ir++];
         }
      }
      A.clear();
      for ( int i = 0; i < A.size1(); i += 3 ) {
         for ( int li = 0; li < 3; ++li ) {
            for ( int lj = 0; lj < 3; ++lj ) {
               A( i+li, i+lj ) = randData[ir++];
            }
         }
      }
   }

   template<int _N> void BoostElementNewElementAssembly<_N>::AssembleElement() {
      noalias(Pt) = trans( P );
      noalias(PC) = block_prod<c_matrix<double, 8, _N>, 64>( P, C );
      
      noalias(K)  = block_prod<c_matrix<double, 8, 8>, 64> ( PC, Pt );

      // first order term    
      noalias(GB) = block_prod<c_matrix<double, 8, 3 * _N>, 64> ( G, B );
      noalias(K) += block_prod<c_matrix<double, 8, 8>, 64> ( GB, Pt );
      
      // second order term
      noalias(GA) = block_prod<c_matrix<double, 8, 3 * _N >, 64> (  G, A );    
      noalias(K) += block_prod<c_matrix<double, 8, 8>, 64> ( GA, trans( G ) );
   }
}
#endif
