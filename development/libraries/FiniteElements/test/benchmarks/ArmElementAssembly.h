#ifndef ARM_ELEMENT_ASSEMBLY_H
#define ARM_ELEMENT_ASSEMBLY_H

#include "ElementAssembly.h"

#include <armadillo>
#include <vector>

namespace arma
{
   ///////////////////////////////////////////////////////////////////////////////
   /// @brief Class which implements element matrix assembly using Aramdillo library
   ///////////////////////////////////////////////////////////////////////////////
   class ArmaElementAssembly : public ElementAssembly
   {
   public:
      ArmaElementAssembly() : Element_Jacobian( 8, 8, fill::zeros ),
                              Element_Residual( 8, fill::zeros ),
                              geometryMatrix( 3, 8, fill::zeros ),
                              previousGeometryMatrix( 3, 8, fill::zeros ),
                              Current_Element_VES( 8, fill::zeros ),
                              Current_Element_Max_VES( 8, fill::zeros ),
                              Current_Ph( 8, fill::zeros ),
                              Current_Po( 8, fill::zeros ),
                              Current_Lp( 8, fill::zeros ),
                              Previous_Element_Temperature( 8, fill::zeros ),
                              Current_Element_Temperature( 8, fill::zeros ),
                              Current_Element_Chemical_Compaction( 8, fill::zeros ),
                              Element_Heat_Production( 8, fill::zeros ),
                              K( 8, 8, fill::zeros ) {;}

      ~ArmaElementAssembly(){;}

      mat Element_Jacobian;
      vec Element_Residual;
      mat geometryMatrix;
      mat previousGeometryMatrix;

      vec Current_Element_VES;
      vec Current_Element_Max_VES;
      vec Current_Ph;
      vec Current_Po;
      vec Current_Lp;
      vec Previous_Element_Temperature;
      vec Current_Element_Temperature;
      vec Current_Element_Chemical_Compaction;
      vec Element_Heat_Production;

      void AssembleElement();

      // resize matrices/vectors for new numbers of quadrature points. 
      // Also init them random values
      void InitNewImplementationOfAssembly( int xyQuadPts, int zQuadPts, const std::vector<double> & randData );
      void AssembleElementNew();

      // result matrix 8x8
      mat K; // K1 + K2 + K3

   private:
      // Zero order term for new implementation PCP'
      mat P;
      mat Pt;
      vec C;

      // First order term
      mat    G; // 8  x 3n
      sp_mat B; // 3n x n

      // Second order term 
      mat      Gt; // 3n x 8
      sp_mat   A;  // 3n x 3n

      void basisFunction( double xi, double eta, double zeta, vec & basis, mat & gradBasis );
   };
};

#endif
