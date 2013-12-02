#include "EgnElementAssembly.h"

#include <stdlib.h>


///////////////////////////////////////////////////////////////////////////////
// EgnElementAssembly methods
///////////////////////////////////////////////////////////////////////////////
void Eigen::EgnElementAssembly::AssembleElement()
{
   Element_Jacobian.Zero();
   Element_Residual.Zero();

   Vector8d   Basis;
   Matrix8x3d Grad_Basis;
   Matrix8x3d Scaled_Grad_Basis;
   Matrix8x3d Scaled_Grad_Basis2;

   Matrix3d   Jacobian;
   Matrix3d   Jacobian_Inverse;
   Matrix3d   Previous_Jacobian;
   Matrix3d   Conductivity_Tensor;

   double Current_Bulk_Density_X_Capacity;

   double timeStepInv = 1.0;

   VectorC3d     Heat_Flow;
   Vector8d      Term_3;

   for ( int I = 0; I < Number_Of_X_Points; I++ )
   {
      for ( int J = 0; J < Number_Of_Y_Points; J++ )
      {
         for ( int K = 0; K < Number_Of_Z_Points; K++ )
         {
            basisFunction( X_Quadrature_Points[I], Y_Quadrature_Points[J], Z_Quadrature_Points[K], Basis, Grad_Basis );

            Jacobian = geometryMatrix * Grad_Basis;
            Previous_Jacobian = previousGeometryMatrix * Grad_Basis;
         
            Scaled_Grad_Basis = Grad_Basis * Jacobian.inverse();
         
            double integrationWeight = X_Quadrature_Weights[I] * Y_Quadrature_Weights [J] * Z_Quadrature_Weights [K] * Jacobian.determinant();

            double Current_VES                   = Basis.dot( Current_Element_VES );
            double Current_Max_VES               = Basis.dot( Current_Element_Max_VES );
            double Current_Hydrostatic_Pressure  = Basis.dot( Current_Ph );
            double Current_Overpressure          = Basis.dot( Current_Po );
            double Current_LithostaticPressure   = Basis.dot( Current_Lp );
            double Current_Pore_Pressure         = Current_Hydrostatic_Pressure  + Current_Overpressure;
            double Current_Temperature           = Basis.dot( Current_Element_Temperature );
            double Previous_Temperature          = Basis.dot( Previous_Element_Temperature );
         
            double Current_Chemical_Compaction_Term = Basis.dot( Current_Element_Chemical_Compaction );
         
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

            Conductivity_Tensor.Zero();
            Conductivity_Tensor(0,0) = 3.2;
            Conductivity_Tensor(1,1) = 3.2;
            Conductivity_Tensor(2,2) = 3.2;
                        
            Element_Residual -= Scaled_Grad_Basis * Heat_Flow * integrationWeight;

            //
            // Term 4
            //
            double Heat_Source_Term = Element_Heat_Production.dot( Basis );
            Element_Residual += Basis * (integrationWeight * Heat_Source_Term);
 
            //----------------------------//
            //
            // Jacobian
            //
            //

            //
            // Term 1
            //
            Element_Jacobian += (Basis.transpose() * Basis) * (integrationWeight * Current_Bulk_Density_X_Capacity * timeStepInv);

            //
            // Term 3
            //
            Scaled_Grad_Basis2 = Scaled_Grad_Basis * Conductivity_Tensor;
            Element_Jacobian += (Scaled_Grad_Basis * Scaled_Grad_Basis2.transpose())* integrationWeight;
        }
      }
   }
}

void Eigen::EgnElementAssembly::basisFunction( double xi, double eta, double zeta, Vector8d &  basis, Matrix8x3d & gradBasis )
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

