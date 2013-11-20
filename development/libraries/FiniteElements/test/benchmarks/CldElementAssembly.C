#include "../../src/BasisFunction.h"

#include "CldElementAssembly.h"


///////////////////////////////////////////////////////////////////////////////
// CldElementAssembly methods
///////////////////////////////////////////////////////////////////////////////
void FiniteElementMethod::CldElementAssembly::AssembleElement()
{
   Element_Jacobian.zero();
   Element_Residual.zero();

   ElementVector      Basis;
   GradElementVector  Grad_Basis;
   GradElementVector  Scaled_Grad_Basis;
   GradElementVector  Scaled_Grad_Basis2;

   BasisFunction      basisFunction;
 
   Matrix3x3          Jacobian;
   Matrix3x3          Jacobian_Inverse;
   Matrix3x3          Previous_Jacobian;
   Matrix3x3          Conductivity_Tensor;

   double Current_Bulk_Density_X_Capacity;
   double timeStepInv = 1.0;

   ThreeVector        Heat_Flow;
   ElementVector      Term_3;

   for ( int I = 0; I < Number_Of_X_Points; I++ )
   {
      for ( int J = 0; J < Number_Of_Y_Points; J++ )
      {
         for ( int K = 0; K < Number_Of_Z_Points; K++ )
         {
            basisFunction( X_Quadrature_Points[I], Y_Quadrature_Points[J], Z_Quadrature_Points[K], Basis, Grad_Basis );
         
            matrixMatrixProduct ( geometryMatrix, Grad_Basis, Jacobian );
            matrixMatrixProduct ( previousGeometryMatrix, Grad_Basis, Previous_Jacobian );
            
            invert ( Jacobian, Jacobian_Inverse );

            matrixMatrixProduct ( Grad_Basis, Jacobian_Inverse, Scaled_Grad_Basis );

            double integrationWeight = X_Quadrature_Weights[I] * Y_Quadrature_Weights [J] * Z_Quadrature_Weights [K] * determinant( Jacobian );

            double Current_VES                   = innerProduct ( Basis, Current_Element_VES );
            double Current_Max_VES               = innerProduct ( Basis, Current_Element_Max_VES );
            double Current_Hydrostatic_Pressure  = innerProduct ( Basis, Current_Ph );
            double Current_Overpressure          = innerProduct ( Basis, Current_Po );
            double Current_LithostaticPressure   = innerProduct ( Basis, Current_Lp );
            double Current_Pore_Pressure         = Current_Hydrostatic_Pressure  + Current_Overpressure;
            double Current_Temperature           = innerProduct ( Basis, Current_Element_Temperature );
            double Previous_Temperature          = innerProduct ( Basis, Previous_Element_Temperature );

            double Current_Chemical_Compaction_Term = innerProduct ( Basis, Current_Element_Chemical_Compaction );

            //lithology->getPorosity ( Current_VES, Current_Max_VES, includeChemicalCompaction, Current_Chemical_Compaction_Term, Current_Compound_Porosity );
            double Current_Porosity = 0.0; //Current_Compound_Porosity.mixedProperty();

            //
            // Term 1
            //
            //lithology->calcBulkDensXHeatCapacity( Fluid, Current_Porosity, Current_Pore_Pressure, Current_Temperature, Current_LithostaticPressure, Current_Bulk_Density_X_Capacity );
            Current_Bulk_Density_X_Capacity = 4.4458451220825845e+06;

            double Scaling = Current_Bulk_Density_X_Capacity * Current_Temperature * integrationWeight * timeStepInv;
            Increment ( -Scaling, Basis, Element_Residual );

            //
            // Term 2
            //
            Scaling = Current_Bulk_Density_X_Capacity * Previous_Temperature * integrationWeight * timeStepInv;
            Increment ( Scaling, Basis, Element_Residual );

            //
            // Term 3
            //
            //Compute_Heat_Flow( isBasementFormation, lithology, Fluid, Current_Element_Temperature, Current_Temperature, Current_Porosity, Current_Pore_Pressure,
            //                   Current_LithostaticPressure, Jacobian, Scaled_Grad_Basis, Heat_Flow, Conductivity_Tensor );
            Heat_Flow(1) = 0;
            Heat_Flow(2) = 0;
            Heat_Flow(3) = 3.5413333e-2;

            Conductivity_Tensor.zero();
            Conductivity_Tensor(1,1) = 3.2;
            Conductivity_Tensor(2,2) = 3.2;
            Conductivity_Tensor(3,3) = 3.2;


            matrixVectorProduct ( Scaled_Grad_Basis, Heat_Flow, Term_3 );
            Increment ( -integrationWeight, Term_3, Element_Residual );

            //
            // Term 4
            //
            double Heat_Source_Term = innerProduct ( Element_Heat_Production, Basis );
            Increment ( integrationWeight * Heat_Source_Term, Basis, Element_Residual );

            //----------------------------//
            //
            // Jacobian
            //
            //

            //
            // Term 1
            //
            addOuterProduct ( integrationWeight * Current_Bulk_Density_X_Capacity * timeStepInv, Basis, Basis, Element_Jacobian );

            //
            // Term 3
            //
            matrixMatrixProduct ( Scaled_Grad_Basis, Conductivity_Tensor, Scaled_Grad_Basis2 );
            addOuterProduct ( integrationWeight, Scaled_Grad_Basis, Scaled_Grad_Basis2, Element_Jacobian );
          }
      }
   }
}
