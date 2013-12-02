#ifndef EGN_ELEMENT_ASSEMBLY_H
#define EGN_ELEMENT_ASSEMBLY_H

#include "ElementAssembly.h"

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <vector>

namespace Eigen
{
   typedef Matrix<double,3,3> Matrix3d;
   typedef Matrix<double,8,8> Matrix8d;
   typedef Matrix<double,3,1> VectorC3d;
   typedef Matrix<double,1,8> Vector8d;
   typedef Matrix<double,8,3> Matrix8x3d;
   typedef Matrix<double,3,8> Matrix3x8d;

   ///////////////////////////////////////////////////////////////////////////////
   /// @brief Class which implements element matrix assembly using Eigen library
   ///////////////////////////////////////////////////////////////////////////////
   class EgnElementAssembly : public ElementAssembly
   {
   public:
      Matrix8d   Element_Jacobian;
      Vector8d   Element_Residual;
      Matrix3x8d geometryMatrix;
      Matrix3x8d previousGeometryMatrix;

      Vector8d   Current_Element_VES;
      Vector8d   Current_Element_Max_VES;
      Vector8d   Current_Ph;
      Vector8d   Current_Po;
      Vector8d   Current_Lp;
      Vector8d   Previous_Element_Temperature;
      Vector8d   Current_Element_Temperature;
      Vector8d   Current_Element_Chemical_Compaction;
      Vector8d   Element_Heat_Production;

      void AssembleElement();

   private:
      void basisFunction( double xi, double eta, double zeta, Vector8d &  basis, Matrix8x3d & gradBasis );
   };


   template<int _N> class EgnNewElementAssembly 
   {
   public:
      // resize matrices/vectors for new numbers of quadrature points. 
      // Also init them random values
      void InitAssembly( const std::vector<double> & randData );
      void AssembleElement();

      // result matrix for the new implementation 8x8
      Matrix< double, 8, 8 >   K; // K1 + K2 + K3      

   private:
      // Zero order term for new implementation PCP'
      Matrix< double, 8, _N > P;
      Matrix< double, _N, 8 > Pt;
      Matrix< double, _N, 1 > C;

      // First order term
#ifdef EIGEN_USE_SPARSE_MATRIX      
      Matrix<       double, 8, 3*_N >  G; // 8  x 3n
      SparseMatrix< double, ColMajor > B; // 3n x n
#else
      Matrix< Matrix<double, 1, 3>,  8, _N >  G; // 8 x n
      Matrix< Matrix<double, 3, 1>, _N,  1 >  B; // n x 1
#endif

      // Second order term 
#ifdef EIGEN_USE_SPARSE_MATRIX      
      SparseMatrix<double, ColMajor>   A;  // 3n x 3n
      Matrix<double, 3*_N, 8>          Gt; // 3n x 8
#else
      Matrix< Matrix3d,             _N, 1 >  A;  // n x 1
      Matrix< Matrix<double, 3, 1>, _N, 8 >  Gt; // n x 8
#endif
   };

   template<int _N> void EgnNewElementAssembly<_N>::InitAssembly(const std::vector<double> & randData )
   {
      size_t ir = 0;

      // init zero order term P and C matrices with random values
      for ( int i = 0; i < P.rows(); ++i ) {
         C(i) = randData[ir++];

         for ( int j = 0; j < P.cols(); ++j )  Pt( j, i ) = P( i, j ) = randData[ir++];
      }

      // init first order term G and B
#ifdef EIGEN_USE_SPARSE_MATRIX      
      B.resize( G.cols(), G.cols()/3 );
      B.reserve( G.cols() );

      // init first order term G and B matrices with random values
      for (    int i = 0; i < G.rows(); ++i )
         for ( int j = 0; j < G.cols(); ++j ) Gt( j, i ) = G( i, j ) = randData[ir++];

      for (    int i = 0; i < B.cols(); ++i )
         for ( int j = 0; j < 3;        ++j ) B.insert( i * 3 + j, i ) = randData[ir++];
      
      B.makeCompressed();
#else
      for (    int i = 0; i < G.rows(); ++i ) {
         for ( int j = 0; j < G.cols(); ++j ) {
            Matrix<double, 1, 3> v;
            for ( int k = 0; k < 3; ++k ) v(k) = randData[ir++];         
            G( i, j ) = v;
         }
      }
 
      for ( int i = 0; i < B.rows(); ++i ) {
         Matrix<double, 3, 1> v;
         for ( int k = 0; k < 3; ++k ) v(k) = randData[ir++];
         B( i ) = v;
      }
#endif

      // init second order term
#ifdef EIGEN_USE_SPARSE_MATRIX      
      A.resize( G.cols(), G.cols() );
      A.reserve( 3 * G.cols() );

      for ( int i = 0; i < A.rows(); i += 3 )
         for (    int li = 0; li < 3; ++li )
            for ( int lj = 0; lj < 3; ++lj ) A.insert( i+li,   i+lj ) = randData[ir++];
   
#else
      for ( int i = 0; i < G.rows(); ++i ) 
         for ( int j = 0; j < G.cols(); ++j )
            Gt( j, i ) = G( i, j ).transpose();
 
      for ( int i = 0; i < A.rows(); ++i ) {
         Matrix3d mat;
         for (   int k = 0; k < 3; ++k )
            for( int l = 0; l < 3; ++l )
               mat(k,l) = randData[ir++];

         A( i ) = mat;
      }
#endif
   }

   template<int _N> void EgnNewElementAssembly<_N>::AssembleElement()
   {
      // zero order term
      // P * C * P'
      K = (P * C.asDiagonal()) * Pt;

#ifdef  DEBUG_PRINT
      std::cout << "\n\nEigen: P matrix:\n" << P << "\n\nEigen: C matrix:\n" << C << "\n\nEigen: P' matrix:\n" << Pt << "\n\nEigen: K0 matrix:\n" << K;
#endif

      // first order term 
#ifdef EIGEN_USE_SPARSE_MATRIX      
      K += (G * B) * Pt;
#else
      MatrixXd mat( G.rows(), G.cols() );
   
      for (    int i = 0; i < G.rows(); ++i )
         for ( int j = 0; j < G.cols(); ++j )
            mat( i, j ) = G( i, j ) * B(j);

      K += mat * Pt;

#endif

#ifdef  DEBUG_PRINT
#ifdef EIGEN_USE_SPARSE_MATRIX      
      std::cout << "\n\nEigen: G matrix:\n" << G << "\n\nEigen: B matrix:\n" << B << "\n\nEigen: K1 matrix:\n" << K;
#else
   std::cout << "\n\nEigen: G matrix:\n";
   for ( int i = 0; i < G.rows(); ++i )
   {
      for ( int j = 0; j < G.cols(); ++j ) std::cout << G(i,j) << " ";
      std::cout << "\n";
   }

   std::cout << "\n\nEigen: B matrix:\n";
   for ( int i = 0; i < B.rows(); ++i ) std::cout << B(i,0) << "\n";

   std::cout << "\n\nEigen: K1 matrix:\n" << K;
#endif
#endif

      // second order term
#ifdef EIGEN_USE_SPARSE_MATRIX      
      K += ( G * A ) * G.transpose();
#else
      Matrix< Matrix<double, 1, 3>, 8, Dynamic > GA(8, G.cols());
   
      for (    int i = 0; i < G.rows(); ++i )
         for ( int j = 0; j < G.cols(); ++j )
            GA( i, j ) = G( i, j ) * A(j);

      for (       int i = 0; i < GA.rows(); ++i )
         for (    int j = 0; j < Gt.cols(); ++j )
            for ( int k = 0; k < Gt.rows(); ++k )
               K( i, j ) += GA( i, k ) * Gt( k, j );

#endif

#ifdef  DEBUG_PRINT
#ifdef EIGEN_USE_SPARSE_MATRIX      
      std::cout << "\n\nEigen: A matrix:\n" << A << "\n\nEigen: K2 matrix:\n" << K;
#else
      std::cout << "\n\nEigen: A matrix:\n";
      for ( int i = 0; i < A.rows(); ++i ) std::cout << A(i,0) << "\n";

      std::cout << "\n\nEigen: K2 matrix:\n" << K;
#endif
#endif
   }
};

#endif
