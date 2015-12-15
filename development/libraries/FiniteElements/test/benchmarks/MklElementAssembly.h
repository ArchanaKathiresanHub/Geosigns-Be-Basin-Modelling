#ifndef MKL_ELEMENT_ASSEMBLY_H
#define MKL_ELEMENT_ASSEMBLY_H


#include <vector>

namespace Mkl 
{
   ///////////////////////////////////////////////////////////////////////////////
   /// @brief Class which implements element matrix assembly using Aramdillo library
   ///////////////////////////////////////////////////////////////////////////////
   class MklNewElementAssembly
   {
   public:
      MklNewElementAssembly();
      ~MklNewElementAssembly() { freeMatrices(); }

      // resize matrices/vectors for new numbers of quadrature points. 
      // Also init them random values
      void InitAssembly( int xyQuadPts, int zQuadPts, const std::vector<double> & randData );
      void AssembleElement();
      
      double getK( int i, int j ) { return K ? K[i + j * 8]: 0.0; }

   private:
      int      N;
      // Zero order term for new implementation PCP'
      double * P;   // 8 x n P matrix
      double * C;   // n x n  C diagonal matrix
      double * PC;  // 8 x n P * C intermediate matrix

      // result matrix 8x8
      double * K;  // K1 + K2 + K3

      // First order term
      double * G;  // 8 x 3n
      double * B;  // 3n x 3n diagonal
      double * GB; // 8 x 3n  G * B intermediate matrix

      // Second order term 
      double * A;  // 3nx3 diagonal
      double * GA; // 3n x 8  G * A intermediate matrix

      void freeMatrices();
   };
};

#endif
