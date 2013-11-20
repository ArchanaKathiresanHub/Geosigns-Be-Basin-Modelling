#include <time.h>
#include <iostream>

#include "../src/FiniteElementTypes.h"
#include "../src/BasisFunction.h"

#include <Eigen/Dense>
#include <armadillo>

int  ElementsNumber = 100 * 100 * 20;
const int  Number_Of_X_Points = 2;
const int  Number_Of_Y_Points = 2;
const int  Number_Of_Z_Points = 3;

double X_Quadrature_Weights[] = {1.0, 1.0};
double Y_Quadrature_Weights[] = {1.0, 1.0};
double Z_Quadrature_Weights[] = {0.55555556, 0.8888889, 0.55555556};

double X_Quadrature_Points[] = {-5.7735026918962573e-01, 5.7735026918962573e-01};
double Y_Quadrature_Points[] = {-5.7735026918962573e-01, 5.7735026918962573e-01};
double Z_Quadrature_Points[] = {-7.7459666924148340e-01, 0.0, 7.7459666924148340e-01};

namespace FiniteElementMethod
{
  ///////////////////////////////////////////////////////////////////////////////
  /// @brief Class which implements element matrix assembly using cauldron FiniteElements library
  ///////////////////////////////////////////////////////////////////////////////
  class CldElementAssembly
   {
   public:
      ElementMatrix         Element_Jacobian;
      ElementVector         Element_Residual;
      ElementGeometryMatrix geometryMatrix;
      ElementGeometryMatrix previousGeometryMatrix;

      ElementVector         Current_Element_VES;
      ElementVector         Current_Element_Max_VES;
      ElementVector         Current_Ph;
      ElementVector         Current_Po;
      ElementVector         Current_Lp;
      ElementVector         Previous_Element_Temperature;
      ElementVector         Current_Element_Temperature;
      ElementVector         Current_Element_Chemical_Compaction;
      ElementVector         Element_Heat_Production;

      void AssembleElement();
   };
};

namespace Eigen
{
   ///////////////////////////////////////////////////////////////////////////////
   /// @brief Class which implements element matrix assembly using Eigen library
   ///////////////////////////////////////////////////////////////////////////////
   class EgnElementAssembly
   {
   public:
      typedef Matrix<double,3,3> Matrix3d;
      typedef Matrix<double,8,8> Matrix8d;
      typedef Matrix<double,1,3> Vector3d;
      typedef Matrix<double,3,1> VectorC3d;
      typedef Matrix<double,1,8> Vector8d;
      typedef Matrix<double,8,3> Matrix8x3d;
      typedef Matrix<double,3,8> Matrix3x8d;

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
};

namespace arma
{
   ///////////////////////////////////////////////////////////////////////////////
   /// @brief Class which implements element matrix assembly using Aramdillo library
   ///////////////////////////////////////////////////////////////////////////////
   class ArmaElementAssembly
   {
   public:
      ArmaElementAssembly() : Element_Jacobian( 8, fill::zeros ),
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
                              Element_Heat_Production( 8, fill::zeros ) {;}

      ~ArmaElementAssembly(){;}

      vec Element_Jacobian;
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

   private:
      void basisFunction( double xi, double eta, double zeta, vec & basis, mat & gradBasis );
   };
};


// Run benchamrks in main()
void InitObjects( FiniteElementMethod::CldElementAssembly & cld, Eigen::EgnElementAssembly & egn, arma::ArmaElementAssembly & arma );

int main( int argc, char ** argv )
{
   FiniteElementMethod::CldElementAssembly  cld;
   Eigen::EgnElementAssembly                egn;
   arma::ArmaElementAssembly                arm;
   
   InitObjects( cld, egn, arm );

   clock_t t1 = clock();

   for ( int el = 0; el < ElementsNumber; ++el )
   {
      cld.AssembleElement();
   }
   double cldTotTime = static_cast<double>( (clock() - t1 ) ) / static_cast<double>( CLOCKS_PER_SEC );

   std::cout << "Cauldron time: " << cldTotTime << " sec.\n"; 


   t1 = clock();

   for ( int el = 0; el < ElementsNumber; ++el )
   {
      egn.AssembleElement();
   }
   double egnTotTime = static_cast<double>( (clock() - t1 ) ) / static_cast<double>( CLOCKS_PER_SEC );

   std::cout << "Eigen time: " << egnTotTime << " sec.\n"; 


   t1 = clock();

   for ( int el = 0; el < ElementsNumber; ++el )
   {
      arm.AssembleElement();
   }
   double armTotTime = static_cast<double>( (clock() - t1 ) ) / static_cast<double>( CLOCKS_PER_SEC );

   std::cout << "Armadillo time: " << armTotTime << " sec.\n"; 

   std::cout << "Eigen     / Cauldtron time relation: " << egnTotTime/cldTotTime << std::endl;
   std::cout << "Armadillo / Cauldtron time relation: " << armTotTime/cldTotTime << std::endl;
   std::cout << "Eigen     / Armadillo time relation: " << egnTotTime/armTotTime << std::endl;
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
            Element_Jacobian += (Basis.t() * Basis) * (integrationWeight * Current_Bulk_Density_X_Capacity * timeStepInv);

            //
            // Term 3
            //
            Scaled_Grad_Basis2 = Scaled_Grad_Basis * Conductivity_Tensor;
            Element_Jacobian += (Scaled_Grad_Basis * Scaled_Grad_Basis2.t())* integrationWeight;
        }
      }
   }
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



