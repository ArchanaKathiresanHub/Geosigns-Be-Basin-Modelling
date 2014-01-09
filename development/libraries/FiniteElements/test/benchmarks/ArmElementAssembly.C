#include "ArmElementAssembly.h"

#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////
// ArmaElementAssembly methods
///////////////////////////////////////////////////////////////////////////////

void arma::ArmaElementAssembly::AssembleElement()
{
   Element_Jacobian.zeros();
   Element_Residual.zeros();

   vec Basis( 8 );
   mat Grad_Basis( 8, 3 );
   mat Scaled_Grad_Basis( 8, 3 );
   mat Scaled_Grad_Basis2( 8, 3 );

   mat Jacobian( 3, 3 );
   mat Jacobian_Inverse( 3, 3 );
   mat Previous_Jacobian( 3, 3 );
   mat Conductivity_Tensor( 3, 3 );

   double Current_Bulk_Density_X_Capacity;

   double timeStepInv = 1.0;

   vec Heat_Flow( 3 );
   vec Term_3( 8 );

   for ( int I = 0; I < Number_Of_X_Points; I++ )
   {
      for ( int J = 0; J < Number_Of_Y_Points; J++ )
      {

         for ( int K = 0; K < Number_Of_Z_Points; K++ )
         {
            basisFunction( X_Quadrature_Points[I], Y_Quadrature_Points[J], Z_Quadrature_Points[K], Basis, Grad_Basis );

            Jacobian = geometryMatrix * Grad_Basis;
            Previous_Jacobian = previousGeometryMatrix * Grad_Basis;
         
            Scaled_Grad_Basis = Grad_Basis * Jacobian.i();
         
            double integrationWeight = X_Quadrature_Weights[I] * Y_Quadrature_Weights [J] * Z_Quadrature_Weights [K] * det( Jacobian );

            double Current_VES                   = dot( Basis, Current_Element_VES );
            double Current_Max_VES               = dot( Basis, Current_Element_Max_VES );
            double Current_Hydrostatic_Pressure  = dot( Basis, Current_Ph );
            double Current_Overpressure          = dot( Basis, Current_Po );
            double Current_LithostaticPressure   = dot( Basis, Current_Lp );
            double Current_Pore_Pressure         = Current_Hydrostatic_Pressure  + Current_Overpressure;
            double Current_Temperature           = dot( Basis, Current_Element_Temperature );
            double Previous_Temperature          = dot( Basis, Previous_Element_Temperature );
         
            double Current_Chemical_Compaction_Term = dot( Basis, Current_Element_Chemical_Compaction );
         
            double Current_Porosity = 0.0; //Current_Compound_Porosity.mixedProperty();

            //
            // Term 1
            //
            Current_Bulk_Density_X_Capacity = 4.4458451220825845e+06;

            double Scaling = Current_Bulk_Density_X_Capacity * Current_Temperature * integrationWeight * timeStepInv;
            Element_Residual -= Basis * Scaling;

            //
            // Term 2
            //
            Scaling = Current_Bulk_Density_X_Capacity * Previous_Temperature * integrationWeight * timeStepInv;
            Element_Residual += Basis * Scaling;
            //
            // Term 3
            //
            //Compute_Heat_Flow( isBasementFormation, lithology, Fluid, Current_Element_Temperature, Current_Temperature, Current_Porosity, Current_Pore_Pressure,
            //                   Current_LithostaticPressure, Jacobian, Scaled_Grad_Basis, Heat_Flow, Conductivity_Tensor );
            Heat_Flow(0) = 0;
            Heat_Flow(1) = 0;
            Heat_Flow(2) = 3.5413333e-2;

            Conductivity_Tensor.zeros();
            Conductivity_Tensor(0,0) = 3.2;
            Conductivity_Tensor(1,1) = 3.2;
            Conductivity_Tensor(2,2) = 3.2;
                        
            Element_Residual -= Scaled_Grad_Basis * Heat_Flow * integrationWeight;

            //
            // Term 4
            //
            double Heat_Source_Term = dot( Element_Heat_Production, Basis );
            Element_Residual += Basis * (integrationWeight * Heat_Source_Term);
 
            //----------------------------//
            //
            // Jacobian
            //
            //

            //
            // Term 1
            //
            Element_Jacobian += (Basis * Basis.t()) * (integrationWeight * Current_Bulk_Density_X_Capacity * timeStepInv);

            //
            // Term 3
            //
            Scaled_Grad_Basis2 = Scaled_Grad_Basis * Conductivity_Tensor;
            Element_Jacobian += (Scaled_Grad_Basis * Scaled_Grad_Basis2.t())* integrationWeight;
        }
      }
   }
}

void arma::ArmaElementAssembly::InitNewImplementationOfAssembly( int xyQuadPts, int zQuadPts, const std::vector<double> & randData )
{
   int N = xyQuadPts * xyQuadPts * zQuadPts;
   
   size_t ir = 0;

   // resize matrices according to given dimensions
   P.set_size( 8, N );
   C.set_size( N );
   Pt.set_size( N, 8 );

   // init zero order term P and C matrices with random values
   for ( int i = 0; i < C.n_rows; ++i ) C(i) = randData[ir++];

   for (    int i = 0; i < P.n_rows; ++i )
      for ( int j = 0; j < P.n_cols; ++j )
         Pt( j, i ) = P( i, j ) = randData[ir++];
 
   // init first order term G and B
   G.reset();
   G.set_size(  8,   3*N );
   
   Gt.reset();
   Gt.set_size( 3*N, 8   );

   B.reset();
   B.set_size( 3*N, N );

   for (    int i = 0; i < G.n_rows; ++i )
      for ( int j = 0; j < G.n_cols; ++j ) Gt( j, i ) = G( i, j ) = randData[ir++];

   for ( int i = 0; i < B.n_cols; ++i )
   {
      B( i*3,   i ) = randData[ir++];
      B( i*3+1, i ) = randData[ir++];
      B( i*3+2, i ) = randData[ir++];
   }

   // init second order term
   A.reset();
   A.set_size( 3 * N, 3 * N );

   for ( int i = 0; i < A.n_rows; i += 3 )
   {
      for ( int li = 0; li < 3; ++li )
      {
         for ( int lj = 0; lj < 3; ++lj )
         {
            A( i+li, i+lj ) = randData[ir++];
         }
      }
   }
}

void arma::ArmaElementAssembly::AssembleElementNew()
{
   // P * C * P'
   // C is diagornal matrix. Firs calc Q = C * P'
   K = (P * diagmat(C)) * Pt;

#ifdef  DEBUG_PRINT
      std::cout << "\n\nArmadillo: P  matrix:\n"; P.print();
      std::cout << "\n\nArmadillo: C  matrix:\n"; C.print();
      std::cout << "\n\nArmadillo: P' matrix:\n"; Pt.print();
      std::cout << "\n\nArmadillo: K0 matrix:\n"; K.print();
#endif

   // first order term
   K += (G * B) * Pt;

#ifdef  DEBUG_PRINT
      std::cout << "\n\nArmadillo: G  matrix:\n"; G.print();
      std::cout << "\n\nArmadillo: B  matrix:\n"; B.print();
      std::cout << "\n\nArmadillo: K1 matrix:\n"; K.print();
#endif

   // second order term
   K += ( G * A ) * G.t();

#ifdef  DEBUG_PRINT
   std::cout << "\n\nArmadillo: A  matrix:\n"; A.print();
   std::cout << "\n\nArmadillo: K2 matrix:\n"; K.print();
#endif
}

void arma::ArmaElementAssembly::basisFunction( double xi, double eta, double zeta, vec & basis, mat & gradBasis )
{
  const double One_Minus_xi = 1.0 - xi;
  const double One_Plus_xi  = 1.0 + xi;

  const double One_Minus_eta = 1.0 - eta;
  const double One_Plus_eta  = 1.0 + eta;

  const double One_Minus_zeta = 1.0 - zeta;
  const double One_Plus_zeta  = 1.0 + zeta;


  basis[0] = 0.125 * One_Minus_xi * One_Minus_eta * One_Minus_zeta;
  basis[1] = 0.125 * One_Plus_xi  * One_Minus_eta * One_Minus_zeta;
  basis[2] = 0.125 * One_Plus_xi  * One_Plus_eta  * One_Minus_zeta;
  basis[3] = 0.125 * One_Minus_xi * One_Plus_eta  * One_Minus_zeta;
        
  basis[4] = 0.125 * One_Minus_xi * One_Minus_eta * One_Plus_zeta;
  basis[5] = 0.125 * One_Plus_xi  * One_Minus_eta * One_Plus_zeta;
  basis[6] = 0.125 * One_Plus_xi  * One_Plus_eta  * One_Plus_zeta;
  basis[7] = 0.125 * One_Minus_xi * One_Plus_eta  * One_Plus_zeta;
        

  gradBasis( 0, 0 ) = -0.125 * One_Minus_eta * One_Minus_zeta;
  gradBasis( 0, 1 ) = -0.125 * One_Minus_xi  * One_Minus_zeta;
  gradBasis( 0, 2 ) = -0.125 * One_Minus_xi  * One_Minus_eta;

  gradBasis( 1, 0 ) =  0.125 * One_Minus_eta * One_Minus_zeta;
  gradBasis( 1, 1 ) = -0.125 * One_Plus_xi   * One_Minus_zeta;
  gradBasis( 1, 2 ) = -0.125 * One_Plus_xi   * One_Minus_eta;

  gradBasis( 2, 0 ) =  0.125 * One_Plus_eta * One_Minus_zeta;
  gradBasis( 2, 1 ) =  0.125 * One_Plus_xi  * One_Minus_zeta;
  gradBasis( 2, 2 ) = -0.125 * One_Plus_xi  * One_Plus_eta;
                   
  gradBasis( 3, 0 ) = -0.125 * One_Plus_eta * One_Minus_zeta;
  gradBasis( 3, 1 ) =  0.125 * One_Minus_xi * One_Minus_zeta;
  gradBasis( 3, 2 ) = -0.125 * One_Minus_xi * One_Plus_eta;


  gradBasis( 4, 0 ) = -0.125 * One_Minus_eta * One_Plus_zeta;
  gradBasis( 4, 1 ) = -0.125 * One_Minus_xi  * One_Plus_zeta;
  gradBasis( 4, 2 ) =  0.125 * One_Minus_xi  * One_Minus_eta;
                   
  gradBasis( 5, 0 ) =  0.125 * One_Minus_eta * One_Plus_zeta;
  gradBasis( 5, 1 ) = -0.125 * One_Plus_xi   * One_Plus_zeta;
  gradBasis( 5, 2 ) =  0.125 * One_Plus_xi   * One_Minus_eta;
                   
  gradBasis( 6, 0 ) =  0.125 * One_Plus_eta * One_Plus_zeta;
  gradBasis( 6, 1 ) =  0.125 * One_Plus_xi  * One_Plus_zeta;
  gradBasis( 6, 2 ) =  0.125 * One_Plus_xi  * One_Plus_eta;
                   
  gradBasis( 7, 0 ) = -0.125 * One_Plus_eta * One_Plus_zeta;
  gradBasis( 7, 1 ) =  0.125 * One_Minus_xi * One_Plus_zeta;
  gradBasis( 7, 2 ) =  0.125 * One_Minus_xi * One_Plus_eta;
}
