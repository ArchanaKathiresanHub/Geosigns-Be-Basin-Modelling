//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "temperature_solver.h"

#include "layer_iterators.h"
#include "fem_grid.h"

#include "utils.h"
#include "WallTime.h"

#include "ConstantsFastcauldron.h"
#include "element_contributions.h"
#include "FissionTrackCalculator.h"
#include "CrustFormation.h"
#include "MantleFormation.h"
#include "VitriniteReflectance.h"
#include "FastcauldronSimulator.h"
#include "Interface/RunParameters.h"

//Data access library
#include "Interface/GridMap.h"
#include "Interface/IgneousIntrusionEvent.h"
#include "PetscLogStages.h"

#include "BoundaryConditions.h"
#include "ElementThicknessActivityPredicate.h"
#include "CompositeElementActivityPredicate.h"
#include "LayerElement.h"
#include "ElementContributions.h"
#include "Lithology.h"

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;
#include "ConstantsMathematics.h"
using Utilities::Maths::MicroWattsToWatts;
using Utilities::Maths::MillionYearToSecond;
using Utilities::Maths::NegOne;
using Utilities::Maths::Zero;

//------------------------------------------------------------//

int Temperature_Solver::PlaneQuadratureDegrees [ NumberOfOptimisationLevels ] = { 2, 2, 2, 2, 3 };

int Temperature_Solver::DepthQuadratureDegrees [ NumberOfOptimisationLevels ] = { 2, 2, 2, 3, 4 };



//------------------------------------------------------------//

Temperature_Solver::Temperature_Solver( AppCtx* appctx ) : 
   m_SmectiteIlliteCalculator( appctx ),
   m_BiomarkersCalculator(appctx),
   m_FissionTrackCalculator(appctx)
{

  Basin_Model  = appctx;
 
}


//------------------------------------------------------------//


Temperature_Solver::~Temperature_Solver() 
{

  if ( Basin_Model -> IsCalculationCoupled || Basin_Model -> DoTemperature || Basin_Model -> Do_Iteratively_Coupled ) {
    Destroy_Petsc_Vector( Crust_Heat_Production );
  }
}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::Calculate_Crust_Heat_Production"

void Temperature_Solver::Compute_Crust_Heat_Production ( )
{

  using namespace Basin_Modelling;

  int X_Start;
  int Y_Start;
  int Z_Start;
  int X_Count;
  int Y_Count;
  int Z_Count;
  int Z_Node_Count;
  unsigned int I;
  unsigned int J;
  int K;

  // Get Crust Thickness at Start of Calculation
  double         Maximum_Crust_Thickness;
  double         Age_Of_Basin = FastcauldronSimulator::getInstance ().getAgeOfBasin ();
  // double         Age_Of_Basin = Basin_Model -> Age_Of_Basin ();

  CrustFormation* Crust_Layer = Basin_Model -> Crust();

  Maximum_Crust_Thickness = Crust_Layer->getCrustMaximumThicknessHistory ( Age_Of_Basin );

  //
  // Calculate Crust Heat Prod
  //
  DMCreateGlobalVector( Crust_Layer -> layerDA, &Crust_Heat_Production );
  VecSet( Crust_Heat_Production, Zero );

  double Crust_Segment_Thickness = Maximum_Crust_Thickness / Crust_Layer -> getNrOfActiveElements();
  double heatProductionDecayConstant = Crust_Layer->getHeatProductionDecayConstant ();

  DMDAGetCorners( Crust_Layer -> layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );


  PETSC_3D_Array Crust_Heat_Production_Array       ( Crust_Layer -> layerDA, Crust_Heat_Production );
  PETSC_3D_Array Crust_Layer_Heat_Production_Array ( Crust_Layer -> layerDA, Crust_Layer -> BulkHeatProd );


  for ( I = X_Start; I < X_Start + X_Count; I++ ) 
  {

    for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) 
    {
      Z_Node_Count = 0;

        for ( K = Z_Start + Z_Count - 1; K >= Z_Start; K-- ) 
        {
           Crust_Heat_Production_Array ( K, J, I ) = Crust_Layer->getHeatProduction ( I, J ) * exp( -( Z_Node_Count * Crust_Segment_Thickness ) / heatProductionDecayConstant );
           Z_Node_Count++;


           Crust_Layer_Heat_Production_Array ( K, J, I ) = Crust_Heat_Production_Array ( K, J, I ) * MicroWattsToWatts;
      }

    }

  }

}

//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::computeHeatProduction"

void Temperature_Solver::computeHeatProduction ( const double previousTime,
                                                 const double currentTime ) const {

   unsigned int i;
   unsigned int j;
   unsigned int k;

   int xStart;
   int yStart;
   int zStart;
   int xCount;
   int yCount;
   int zCount;

   LayerProps_Ptr currentLayer;
   Basin_Modelling::Layer_Iterator Layers;
   const CompoundLithology*  currentLithology;

   double heatProduction;

   Layers.Initialise_Iterator ( Basin_Model -> layers, Basin_Modelling::Descending, Basin_Modelling::Sediments_Only, Basin_Modelling::Active_Layers_Only );

   if ( Layers.Iteration_Is_Done ()) {
      return;
   }

   DMDAGetCorners ( *Basin_Model->mapDA, &xStart, &yStart, PETSC_NULL, &xCount, &yCount, PETSC_NULL );
   const Boolean2DArray& Valid_Needle = Basin_Model->getValidNeedles ();


   while ( ! Layers.Iteration_Is_Done () ) {
    
      currentLayer = Layers.Current_Layer ();

      PETSC_3D_Array layerPorosity ( currentLayer->layerDA, currentLayer->Porosity );
      PETSC_3D_Array layerHeatProduction ( currentLayer->layerDA, currentLayer->BulkHeatProd );
      DMDAGetCorners ( currentLayer->layerDA, PETSC_NULL, PETSC_NULL, &zStart, PETSC_NULL, PETSC_NULL, &zCount );
      
      //For everything except igneous intrusion at the time of intrusion
      if ( !(currentLayer->getIsIgneousIntrusion ()) or previousTime != currentLayer->getIgneousIntrusionAge ()) {
         
         for ( i = xStart; i < xStart + xCount; ++i ) {

            for ( j = yStart; j < yStart + yCount; ++j ) {

               if ( Valid_Needle ( i, j )) {
                  currentLithology = currentLayer->getLithology ( i, j );

                  for ( k = 0; k < zCount; k++ ) {
                     currentLithology->calcBulkHeatProd ( layerPorosity ( k, j, i ), heatProduction );
                     layerHeatProduction ( k, j, i ) = heatProduction;
                  }

               }

            }

         }
         

      } else { //For igneous intrusion at the time of intrusion

         
         #ifdef DEBUG_HEATPRODUCTIONFORINTRUSION
         if ( FastcauldronSimulator::getInstance ().getRank () == 0 ) 
         { //Display the intrusion temperature for the node 5, 5
           cout << " Setting igneous intrusion "  << previousTime << "  " << currentLayer -> getLithology ( 5, 5 ) -> igneousIntrusionTemperature () << endl;
         }
         #endif

         const bool   temperatureIsActive = currentLayer->Current_Properties.propertyIsActivated ( Basin_Modelling::Temperature );
         const bool   depthIsActive = currentLayer->Current_Properties.propertyIsActivated ( Basin_Modelling::Depth );
         const double timeStep = ( previousTime - currentTime ) * MillionYearToSecond;
         double       heatProductionRateForIntrusion;
         double       heatCapacity;

         if ( not temperatureIsActive ) {
            currentLayer->Current_Properties.Activate_Property ( Basin_Modelling::Temperature );
         }
         if ( not depthIsActive ) {
	           currentLayer->Current_Properties.Activate_Property ( Basin_Modelling::Depth );
         }

         for ( i = xStart; i < xStart + xCount; ++i ) {

            for ( j = yStart; j < yStart + yCount; ++j ) {

               if ( Valid_Needle ( i, j )) {
                  double thickness = currentLayer->Current_Properties ( Basin_Modelling::Depth, 0, j, i ) - currentLayer->Current_Properties ( Basin_Modelling::Depth, zCount-1 , j, i );

                  if (thickness <= 1 ) {
                     continue;
                  }
                  const double intrusionTemperature = currentLayer -> getLithology ( i, j ) -> igneousIntrusionTemperature ();
                  assert (("The temperature of intrusion cannot be no data value", intrusionTemperature != DataAccess::Interface::DefaultUndefinedScalarValue));
                  currentLithology = currentLayer->getLithology ( i, j );

                  heatCapacity = currentLithology->densityXheatcapacity ( intrusionTemperature, 0.0 );
                  
                  // The physical equation was not fitting the expected results, we determined a numerical factor in order to fit the benchmark
                  double factor = 41/thickness + 0.59;
                  heatProductionRateForIntrusion = factor * heatCapacity * ( intrusionTemperature - currentLayer->Current_Properties ( Basin_Modelling::Temperature, 0, j, i )) / timeStep;

                  
                  #ifdef DEBUG_HEATPRODUCTIONFORINTRUSION
                  // Some values printed to help in debug mode => to delete once the prototype is released
                  if ( i == 1 and j == 1 ) {
                     cout << " heatProductionForIntrusion " << heatProductionRateForIntrusion << "  " 
                          << heatProduction << "  "
                          << intrusionTemperature << "  "
                          << currentLayer->Current_Properties ( Basin_Modelling::Temperature, 0, j, i ) << "  "
                          << timeStep/MillionYearToSecond << "  "
                          << endl;
                  }
                  #endif

                  for ( k = 0; k < zCount; ++k ) {
                     layerHeatProduction ( k, j, i ) = heatProductionRateForIntrusion;
                  }

               }

            }

         }

         if ( not temperatureIsActive ) {
            currentLayer -> Current_Properties.Restore_Property ( Basin_Modelling::Temperature );
         }
         if ( not depthIsActive ) {
            currentLayer -> Current_Properties.Restore_Property ( Basin_Modelling::Depth );
         }
      }

      Layers++;
   }

} 

//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::setSurfaceTemperature"

void Temperature_Solver::setSurfaceTemperature ( AppCtx*      basinModel,
                                                 const double Current_Time ) {

  using namespace Basin_Modelling;

  double surfaceTemperature;

  int I;
  int J;
  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;

  LayerProps_Ptr currentLayer;
  Layer_Iterator Layers;

  DMDAGetCorners ( *basinModel->mapDA, &xStart, &yStart, PETSC_NULL, &xCount, &yCount, PETSC_NULL );

  Double_Array_2D Temperature_Above ( xCount, yCount );
  
  Layers.Initialise_Iterator ( basinModel -> layers, Descending, Sediments_Only, Active_Layers_Only );

  if ( Layers.Iteration_Is_Done ()) {
    return;
  }

  // Current top layer
  currentLayer = Layers.Current_Layer ();

  currentLayer -> Current_Properties.Activate_Property ( Basin_Modelling::Temperature );

  DMDAGetCorners ( currentLayer->layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );


  for ( I = xStart; I < xStart + xCount; I++ ) {

    for ( J = yStart; J < yStart + yCount; J++ ) {

      if ( Basin_Model->nodeIsDefined ( I, J )) {
        surfaceTemperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( I, J, Current_Time ),
        currentLayer -> Current_Properties ( Basin_Modelling::Temperature, zCount - 1, J, I ) = surfaceTemperature;
      }

    }

  }

  currentLayer -> Current_Properties.Restore_Property ( Basin_Modelling::Temperature );

}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::Estimate_Basement_Temperature"

void Temperature_Solver::Estimate_Basement_Temperature ( )
{

  using namespace Basin_Modelling;

  int X_Start;
  int Y_Start;
  int X_Count;
  int Y_Count;
  int Mantle_Z_Nodes;
  int Crust_Z_Nodes;
  int I;
  int J;
  int K;

  double Sea_Bottom_Temperature;
  double Age_Of_Basin = FastcauldronSimulator::getInstance ().getAgeOfBasin ();
  // double Age_Of_Basin = Basin_Model -> Age_Of_Basin ();

  CrustFormation*  Crust_Layer   = Basin_Model -> Crust();
  MantleFormation* Mantle_Layer  = Basin_Model -> Mantle();

  double Top_Asthenospheric_Temperature = FastcauldronSimulator::getInstance ().getBottomMantleTemperature ();
  
  DMDAGetCorners( *Basin_Model -> mapDA, &X_Start, &Y_Start, PETSC_NULL, &X_Count, &Y_Count, PETSC_NULL );

  DMDAGetCorners( Crust_Layer -> layerDA, PETSC_NULL, PETSC_NULL, PETSC_NULL, 
                  PETSC_NULL, PETSC_NULL, &Crust_Z_Nodes );

  DMDAGetCorners( Mantle_Layer -> layerDA, PETSC_NULL, PETSC_NULL, PETSC_NULL, 
                  PETSC_NULL, PETSC_NULL, &Mantle_Z_Nodes );


  PETSC_3D_Array Crust_Depth( Crust_Layer -> layerDA, 
			      Crust_Layer -> Current_Properties ( Basin_Modelling::Depth ) );
  PETSC_3D_Array Crust_Temperature( Crust_Layer -> layerDA, 
				    Crust_Layer -> Current_Properties ( Basin_Modelling::Temperature ) );
  PETSC_3D_Array Crust_Previous_Temperature( Crust_Layer -> layerDA, 
                                             Crust_Layer -> Previous_Properties ( Basin_Modelling::Temperature ) );

  PETSC_3D_Array Mantle_Depth( Mantle_Layer -> layerDA, 
			       Mantle_Layer -> Current_Properties ( Basin_Modelling::Depth ) );
  PETSC_3D_Array Mantle_Temperature( Mantle_Layer -> layerDA, 
				     Mantle_Layer -> Current_Properties ( Basin_Modelling::Temperature ) );
  PETSC_3D_Array Mantle_Previous_Temperature( Mantle_Layer -> layerDA, 
                                              Mantle_Layer -> Previous_Properties ( Basin_Modelling::Temperature ) );


  for ( I = X_Start; I < X_Start + X_Count; I++ ) {

    for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

      if ( not Basin_Model->nodeIsDefined ( I, J )) continue;

      Polyfunction Temperature_Polyfunction;

      Sea_Bottom_Temperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( I, J, Age_Of_Basin ),

      Temperature_Polyfunction.AddPoint( Crust_Depth( Crust_Z_Nodes - 1,J,I ), Sea_Bottom_Temperature );
      Temperature_Polyfunction.AddPoint( Mantle_Depth( 0,J,I ), Top_Asthenospheric_Temperature );

      for ( K = 0; K < Crust_Z_Nodes; K++ ) {
         const double estimatedTemperature =  Temperature_Polyfunction.F( Crust_Depth( K,J,I ) );
         Crust_Temperature( K,J,I ) = estimatedTemperature;
         Crust_Previous_Temperature( K,J,I ) = estimatedTemperature;
      }

      for ( K = 0; K < Mantle_Z_Nodes; K++ ) {
         const double estimatedTemperature = Temperature_Polyfunction.F( Mantle_Depth( K,J,I ) );
         Mantle_Temperature( K,J,I ) = estimatedTemperature;
         Mantle_Previous_Temperature( K,J,I ) = estimatedTemperature;
      }

    }

  }

}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Temperature_Solver::Estimate_Temperature"

void Temperature_Solver::Estimate_Temperature ( AppCtx*      basinModel,
                                                const double Current_Time ) {

  using namespace Basin_Modelling;

  int X_Start;
  int Y_Start;
  int Z_Start;
  int X_Count;
  int Y_Count;
  int Z_Count;
  int I, J, K;
  int globalI;
  int globalJ;

  double Top_Depth;
  double Surface_Temperature;
  double Estimated_Temperature;

  Layer_Iterator Pressure_Layers ( basinModel->layers, Ascending, Sediments_Only, Active_Layers_Only );
  LayerProps_Ptr Current_Layer;

  DMDAGetCorners ( *basinModel->mapDA, &X_Start, &Y_Start, PETSC_NULL, &X_Count, &Y_Count, PETSC_NULL );

  Double_Array_2D Seabottom_Temperature ( X_Count, Y_Count );
  Double_Array_2D Seabottom_Depth       ( X_Count, Y_Count );

  // Initialise Sea Bottom Temperature
  for ( I = 0, globalI = X_Start; I < X_Count; ++I, ++globalI ) {

    for ( J = 0, globalJ = Y_Start; J < Y_Count; ++J, ++globalJ ) {
      Seabottom_Temperature ( I, J ) = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( globalI, globalJ, Current_Time );
    }

  }

  // Initialise Sea Bottom Depth
  for ( I = X_Start; I < X_Start + X_Count; I++ ) {

    for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {
      Seabottom_Depth ( I - X_Start, J - Y_Start ) = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( I, J, Current_Time );
    }

  }

  // Now estimate the temperature based on the sea bottom depth, sea bottom temperature and the node depth.
  for ( Pressure_Layers.Initialise_Iterator (); ! Pressure_Layers.Iteration_Is_Done (); Pressure_Layers++ ) {
    Current_Layer = Pressure_Layers.Current_Layer ();

    // Get the size of the layer DA.
    DMDAGetCorners ( Current_Layer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );

    Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Temperature );
    Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Depth );

    for ( I = X_Start; I < X_Start + X_Count; I++ ) {

      for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

        if ( Basin_Model->nodeIsDefined ( I, J )) {
          Surface_Temperature = Seabottom_Temperature ( I - X_Start, J - Y_Start );
          Top_Depth           = Seabottom_Depth       ( I - X_Start, J - Y_Start );

          for ( K = Z_Start; K < Z_Start + Z_Count; K++ ) {

            // Copy Layer Depth into the Pressure FEM Depth vector
            Estimated_Temperature = basinModel->Estimate_Temperature_At_Depth ( Current_Layer->Current_Properties ( Basin_Modelling::Depth, K, J, I ),
                                                                                 Surface_Temperature,
                                                                                 Top_Depth );

            Current_Layer -> Current_Properties ( Basin_Modelling::Temperature, K, J, I ) = Estimated_Temperature;
          }

        }

      }

    }

    Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Temperature );
    Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Depth );
  }

}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Temperature_Solver::Store_Temperature_Solution"

void Temperature_Solver::correctTemperatureSolution ( const double Current_Time ) {

  using namespace Basin_Modelling;

  if ( not Basin_Model -> isALC ()) {
     return;
  }

  int X_Start;
  int Y_Start;
  int Z_Start;
  int X_Count;
  int Y_Count;
  int Z_Count;

  int I, J, K;

  PetscScalar Temperature_Value;
  PetscScalar SeaBottomTemperature;

  Layer_Iterator Layers ( Basin_Model -> layers, Ascending, Basement_And_Sediments, Active_Layers_Only );
  LayerProps_Ptr Current_Layer;

  DMDAGetCorners ( *Basin_Model->mapDA, &X_Start, &Y_Start, PETSC_NULL, &X_Count, &Y_Count, PETSC_NULL );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) 
  {
    Current_Layer = Layers.Current_Layer ();
    DMDAGetCorners ( Current_Layer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );
    Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Temperature );

    for ( I = X_Start; I < X_Start + X_Count; I++ ) {
          
       for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {
             
          if ( Basin_Model->nodeIsDefined ( I, J )) {
             SeaBottomTemperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( I, J, Current_Time );

             for ( K = Z_Start; K < Z_Start + Z_Count; K++ ) {
                Temperature_Value = Current_Layer -> Current_Properties ( Basin_Modelling::Temperature,  K, J, I );
                // Negative temperature could occur during the basalt emplacement. Cut off negative values at surface temperature.
                Current_Layer -> Current_Properties ( Basin_Modelling::Temperature,  K, J, I ) = ( Temperature_Value < 0.0 ? SeaBottomTemperature :  Temperature_Value );
             }
          }
       }
          
    }

    Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Temperature );
  }

}




//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::Maximum_Temperature_Difference"

PetscScalar Temperature_Solver::Maximum_Temperature_Difference ()
{

  using namespace Basin_Modelling;

  Vec Current_Temperature;
  Vec Previous_Temperature;
  Vec Temperature_Difference;

  Layer_Iterator Layers;
  LayerProps_Ptr Current_Layer;

  PetscScalar Maximum_Difference = -1.0e10;
  PetscScalar Maximum_Layer_Difference;

  Layers.Initialise_Iterator ( Basin_Model -> layers, Ascending, Sediments_Only, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done () ) 
  {
    
    Current_Layer = Layers.Current_Layer ();

    Current_Temperature  = Current_Layer->Current_Properties  ( Basin_Modelling::Temperature );
    Previous_Temperature = Current_Layer->Previous_Properties ( Basin_Modelling::Temperature );

    DMCreateGlobalVector ( Current_Layer->layerDA, & Temperature_Difference );

    VecWAXPY(Temperature_Difference, NegOne, Previous_Temperature, Current_Temperature );
    VecAbs( Temperature_Difference );
    VecMax( Temperature_Difference,PETSC_NULL,&Maximum_Layer_Difference );
    Destroy_Petsc_Vector( Temperature_Difference );

    Maximum_Difference = PetscMax ( Maximum_Difference, Maximum_Layer_Difference );

    if ( Basin_Model -> debug1 ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Temperature difference: %s  %3.4f  %3.4f \n",
                    Current_Layer -> layername.c_str (),
                    Maximum_Layer_Difference,
                    Maximum_Difference );
    }

    Layers++;

  }
  
  return Maximum_Difference;

}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::Maximum_Temperature_Difference_In_Source_Rocks"

PetscScalar Temperature_Solver::Maximum_Temperature_Difference_In_Source_Rocks ()
{

  using namespace Basin_Modelling;

  Vec Current_Temperature;
  Vec Previous_Temperature;
  Vec Temperature_Difference;

  Layer_Iterator Source_Rock_Layers;
  LayerProps_Ptr Current_Source_Rock;

  PetscScalar Maximum_Difference = -1.0e10;
  PetscScalar Maximum_Layer_Difference;

  Source_Rock_Layers.Initialise_Iterator ( Basin_Model -> layers, Ascending, Source_Rocks_Only, 
					   Active_Layers_Only );

  while ( ! Source_Rock_Layers.Iteration_Is_Done () ) 
  {
    
    Current_Source_Rock = Source_Rock_Layers.Current_Layer ();

    Current_Temperature  = Current_Source_Rock->Current_Properties  ( Basin_Modelling::Temperature );
    Previous_Temperature = Current_Source_Rock->Previous_Properties ( Basin_Modelling::Temperature );

    DMCreateGlobalVector ( Current_Source_Rock->layerDA, & Temperature_Difference );

    VecWAXPY(Temperature_Difference, NegOne, Previous_Temperature, Current_Temperature );
    VecAbs( Temperature_Difference );
    VecMax( Temperature_Difference,PETSC_NULL,&Maximum_Layer_Difference );
    Destroy_Petsc_Vector( Temperature_Difference );

    Maximum_Difference = PetscMax ( Maximum_Difference, Maximum_Layer_Difference );


    if ( Basin_Model -> debug1 ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Source rock difference: %s  %3.4f  %3.4f \n",
                    Current_Source_Rock->layername.c_str (),
                    Maximum_Layer_Difference,
                    Maximum_Difference );
    }


    Source_Rock_Layers++;

  }
  
  return Maximum_Difference;

}


//------------------------------------------------------------//


void Temperature_Solver::resetBiomarkerStateVectors() 
{

  if ( ! Basin_Model->filterwizard.IsBiomarkersCalculationNeeded()) {
    return;
  }

  using namespace Basin_Modelling;

  Layer_Iterator Layers;
  Layers.Initialise_Iterator ( Basin_Model -> layers, Descending, Sediments_Only, Active_Layers_Only );
    
  while ( ! Layers.Iteration_Is_Done () ) 
  {
    LayerProps_Ptr currentLayer = Layers.Current_Layer ();

    currentLayer->resetBiomarkerStateVectors();

    Layers++;
  }
}
void Temperature_Solver::resetSmectiteIlliteStateVectors( ) 
{

  if ( ! Basin_Model->filterwizard.IsSmectiteIlliteCalculationNeeded()) {
    return;
  }

  using namespace Basin_Modelling;

  Layer_Iterator Layers;
  Layers.Initialise_Iterator ( Basin_Model -> layers, Descending, Sediments_Only, Active_Layers_Only );
    
  while ( ! Layers.Iteration_Is_Done () ) 
  {
    LayerProps_Ptr currentLayer = Layers.Current_Layer ();

    currentLayer->resetSmectiteIlliteStateVectors();

    Layers++;
  }
}
//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::computeSmectiteIlliteIncrement"

void Temperature_Solver::computeSmectiteIlliteIncrement ( const double Previous_Time, const double Current_Time ) 
{

   if ( false == Basin_Model -> filterwizard.IsSmectiteIlliteCalculationNeeded()) 
   {
      return;
   }

   MPI_Barrier(PETSC_COMM_WORLD);

   m_SmectiteIlliteCalculator.computeSmectiteIlliteIncrement ( Current_Time, Previous_Time - Current_Time );

}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::computeSnapShotSmectiteIllite"

void Temperature_Solver::computeSnapShotSmectiteIllite ( const double Current_Time, const Boolean2DArray& validNeedle ) 
{
   if ( false == Basin_Model -> filterwizard.IsSmectiteIlliteCalculationNeeded()) 
   {
      return;
   }

   m_SmectiteIlliteCalculator.computeSnapShotSmectiteIllite ( Current_Time, validNeedle );
}
//------------------------------------------------------------//
#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::deleteSmectiteIlliteVector"
void Temperature_Solver::deleteSmectiteIlliteVector ()
{
   if ( Basin_Model -> filterwizard.IsSmectiteIlliteCalculationNeeded()) 
   {
      return;
   }

   using namespace Basin_Modelling;

   Layer_Iterator Layers;
   PetscBool     validVector;

   Layers.Initialise_Iterator ( Basin_Model -> layers, Descending, Sediments_Only, Active_Layers_Only );

   while ( ! Layers.Iteration_Is_Done ()) 
   {
      VecValid ( Layers.Current_Layer () -> m_IlliteFraction, &validVector );

      if ( validVector ) 
      {
         VecDestroy (&( Layers.Current_Layer () -> m_IlliteFraction ));
         Layers.Current_Layer () -> m_IlliteFraction = Vec ( 0 );
      }

      Layers++;
   }


}
//------------------------------------------------------------//
#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::computeBiomarkersIncrement"

void Temperature_Solver::computeBiomarkersIncrement ( const double Previous_Time, const double Current_Time ) 
{

   if ( false == Basin_Model -> filterwizard.IsBiomarkersCalculationNeeded()) 
   {
      return;
   }

   MPI_Barrier(PETSC_COMM_WORLD);

   m_BiomarkersCalculator.computeBiomarkersIncrement ( Current_Time, Previous_Time - Current_Time );

}
#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::collectFissionTrackSampleData"
void  Temperature_Solver::collectFissionTrackSampleData(const double time)
{
   m_FissionTrackCalculator.collectSampleTrackingData(time);
}
#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::computeFissionTracks(void)"
void  Temperature_Solver::computeFissionTracks(void)
{
   m_FissionTrackCalculator.compute();
}
#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::computeFissionTracks(void)"
void  Temperature_Solver:: resetFissionTrackCalculator(void)
{
   m_FissionTrackCalculator.clearSampleInputHistory();
}

//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::computeSnapShotBiomarkers"

void Temperature_Solver::computeSnapShotBiomarkers ( const double Current_Time, const Boolean2DArray& validNeedle ) 
{
   if ( false == Basin_Model -> filterwizard.IsBiomarkersCalculationNeeded()) 
   {
      return;
   }

   m_BiomarkersCalculator.computeSnapShotBiomarkers ( Current_Time, validNeedle );
}
//------------------------------------------------------------//
#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::deleteBiomarkersVectors"
void Temperature_Solver::deleteBiomarkersVectors ( )
{

   if ( false == Basin_Model -> filterwizard.IsBiomarkersCalculationNeeded()) 
   {
      return;
   }

   using namespace Basin_Modelling;

   Layer_Iterator Layers;
   PetscBool     validVector;

   Layers.Initialise_Iterator ( Basin_Model -> layers, Descending, Sediments_Only, Active_Layers_Only );

   while ( ! Layers.Iteration_Is_Done ()) 
   {
      VecValid ( Layers.Current_Layer () -> m_HopaneIsomerisation, &validVector );

      if ( validVector ) 
      {
         VecDestroy (&( Layers.Current_Layer () -> m_HopaneIsomerisation ));
         Layers.Current_Layer () -> m_HopaneIsomerisation = Vec ( 0 );
      }

      VecValid ( Layers.Current_Layer () -> m_SteraneIsomerisation, &validVector );

      if ( validVector ) 
      {
         VecDestroy (&( Layers.Current_Layer () -> m_SteraneIsomerisation ));
         Layers.Current_Layer () -> m_SteraneIsomerisation = Vec ( 0 );
      }
      
      VecValid ( Layers.Current_Layer () -> m_SteraneAromatisation, &validVector );
      
      if ( validVector ) 
      {
         VecDestroy (&( Layers.Current_Layer () -> m_SteraneAromatisation ));
         Layers.Current_Layer () -> m_SteraneAromatisation = Vec ( 0 );
      }
      
      Layers++;
   }
}

//------------------------------------------------------------//

int Temperature_Solver::getPlaneQuadratureDegree ( const int optimisationLevel ) const {

  return PlaneQuadratureDegrees [ optimisationLevel - 1 ];

}

//------------------------------------------------------------//

int Temperature_Solver::getDepthQuadratureDegree ( const int optimisationLevel ) const {

  return DepthQuadratureDegrees [ optimisationLevel - 1 ];

}

//------------------------------------------------------------//

void Temperature_Solver::setPlaneQuadratureDegree ( const int optimisationLevel,
                                                    const int newDegree ) {

  PlaneQuadratureDegrees [ optimisationLevel - 1 ] = newDegree;

}

//------------------------------------------------------------//

void Temperature_Solver::setDepthQuadratureDegree ( const int optimisationLevel,
                                                    const int newDegree ) {

  DepthQuadratureDegrees [ optimisationLevel - 1 ] = newDegree;

}

//------------------------------------------------------------//


int Temperature_Solver::maximumNumberOfNonlinearIterations ( const int optimisationLevel ) const {

  static const int Number_Of_Iterations [ 5 ] = { 1, 2, 5, 5, 5 };

  return Number_Of_Iterations [ optimisationLevel - 1 ];
}

//------------------------------------------------------------//

double Temperature_Solver::linearSolverTolerance ( const int optimisationLevel ) const {

  static const double linearSolverTolerances [ 5 ] = { 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-7 };

  return linearSolverTolerances [ optimisationLevel - 1 ];

}


void Temperature_Solver::writeFissionTrackResultsToDatabase(void)
{
   Basin_Model->writeFissionTrackResultsToDatabase(m_FissionTrackCalculator);
}




//------------------------------------------------------------//

void Temperature_Solver::getBoundaryConditions ( const GeneralElement& element,
                                                 const double          currentTime,
                                                 const int             topIndex,
                                                 BoundaryConditions&   bcs ) const {

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();

   const LayerElement& layerElement = element.getLayerElement ();
   const GeneralElement* bottomNeighbour = 0;
   bool bottomOfDomain = false;

   if ( layerElement.getFormation ()->isMantle ()) {
      bottomNeighbour = element.getActiveNeighbour ( VolumeData::DeepFace );

      if ( bottomNeighbour == 0 ) {
         bottomOfDomain = true;
      }
   }


   bcs.reset ();

   for ( int n = 0; n < 8; ++n ) {

      // node numbers (n \in {4,5,6,7}) are at the bottom of the element.
      // When we have crustal thinning then the bottom node may not have a getNodeK ( n ) == 0
      if ( element.getNodeK ( n ) == 0 or ( bottomOfDomain and n >= 4 )) {
         Interface::BottomBoundaryConditions bottomBcs = fc.getBottomBoundaryConditions ();

         if ( bottomBcs == Interface::MANTLE_HEAT_FLOW ) {
            bcs.setBoundaryConditions ( n, Bottom_Boundary_Flux,
                                        fc.getMantleHeatFlow ( element.getNodeI ( n ), element.getNodeJ ( n ), currentTime ));
         } else if ( bottomBcs == Interface::FIXED_BASEMENT_TEMPERATURE or
                     bottomBcs == Interface::ADVANCED_LITHOSPHERE_CALCULATOR) {
            bcs.setBoundaryConditions ( n, Surface_Boundary, fc.getBottomMantleTemperature ());
         }

      } else if ( element.getNodeK ( n ) == topIndex ) {
         bcs.setBoundaryConditions ( n, Surface_Boundary,
                                     fc.getSeaBottomTemperature ( element.getNodeI ( n ),
                                                                  element.getNodeJ ( n ),
                                                                  currentTime ));
      }

   }

}

//------------------------------------------------------------//

void Temperature_Solver::getAlcBcsAndLithology ( const GeneralElement&     element,
                                                 const double              previousTime,
                                                 const double              currentTime,
                                                 const CompoundLithology*& elementLithology,
                                                 PETSC_2D_Array&           topBasaltDepth,
                                                 PETSC_2D_Array&           bottomBasaltDepth,
                                                 BoundaryConditions&       bcs ) const {

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();

   const LayerElement& layerElement = element.getLayerElement ();
   LayerProps* currentFormation = const_cast<LayerProps*>( layerElement.getFormation ());

   const double constrainedTempValue = fc.getConstrainedBasaltTemperature();

   ElementGeometryMatrix geometryMatrixOffset;
   const PETSC_3D_Array& depth = currentFormation->Current_Properties.getArray ( Basin_Modelling::Depth );
   int layerTopIndex = currentFormation->getMaximumNumberOfElements ();

   // First step determine if the lithology should be a basalt or not
   for ( int n = 0; n < 8; ++n ) {
      int i = layerElement.getNodeIPosition ( n );
      int j = layerElement.getNodeJPosition ( n );
      int k = layerElement.getNodeLocalKPosition ( n );

      geometryMatrixOffset ( 3, n + 1 ) = depth ( k, j, i ) - depth ( layerTopIndex, j, i );
   }

   // Set the boundary conditions for the mantle.
   if ( currentFormation->isMantle ()) {
      const PETSC_3D_Array& temperature = currentFormation->Current_Properties.getArray ( Basin_Modelling::Temperature );

      for ( int n = 0; n < 8; ++n ) {
         double Hfix = fc.getLithosphereThicknessMod ( element.getNodeI ( n ), element.getNodeJ ( n ), currentTime );
         int i = layerElement.getNodeIPosition ( n );
         int j = layerElement.getNodeJPosition ( n );

         if ( geometryMatrixOffset ( 3, n + 1 ) > Hfix ) {
            // Is this just the bottom BC (1333)?
            bcs.setBoundaryConditions ( n, Surface_Boundary, fc.getBottomMantleTemperature ());
         }
         
      }

   }

   double offsetMidPointDepth = geometryMatrixOffset.getMidPoint ();

   elementLithology = currentFormation->getLithology ( currentTime, layerElement.getIPosition (), layerElement.getJPosition (), offsetMidPointDepth );

   // If we are in a basalt then determine other bcs.
   // Can the setting of the top and bottom basalt depth be moved to somewhere before we assemble the global matrix.
   if ( currentFormation->isBasalt ()) {

      for ( int n = 0; n < 8; ++n ) {
         int i = layerElement.getNodeIPosition ( n );
         int j = layerElement.getNodeJPosition ( n );
         int k = layerElement.getNodeLocalKPosition ( n );

         currentFormation->setBasaltLitho ( i, j, k );

         if ( n > 3 ) {

            if ( bottomBasaltDepth ( j, i ) == CauldronNoDataValue ) {
               bottomBasaltDepth ( j, i ) = depth ( k, j, i );
            }

         } else {
            topBasaltDepth ( j, i ) = depth ( k, j, i );
         }

         if ( not fc.getCauldron ()->bottomBasaltTemp ) {

            if ( ! currentFormation->getPreviousBasaltLitho( i, j, k ) && 
                 currentTime > fc.getEndOfRiftEvent( layerElement.getIPosition (), layerElement.getJPosition ())) {
               bcs.setBoundaryConditions ( n, Interior_Constrained_Temperature, constrainedTempValue );
            }

            // set constraied temperature only for the bottom basalt element
         } else if (( bottomBasaltDepth( j, i ) == CauldronNoDataValue ) && 
                    ( fc.getBasaltThickness( i, j, currentTime ) > 
                      fc.getBasaltThickness( i, j, previousTime )) &&
                    currentTime >  fc.getEndOfRiftEvent( layerElement.getIPosition (), layerElement.getJPosition ())) {

            bcs.setBoundaryConditions ( n, Interior_Constrained_Temperature, constrainedTempValue );
         }

      }

   }

}

//------------------------------------------------------------//

void Temperature_Solver::assembleElementTemperatureStiffnessMatrix ( const GeneralElement&     element,
                                                                     const PETSC_3D_Array&     bulkHeatProd,
                                                                     const int                 planeQuadratureDegree,
                                                                     const int                 depthQuadratureDegree,
                                                                     const double              currentTime,
                                                                     const double              timeStep,
                                                                     const bool                includeAdvectiveTerm,
                                                                     const BoundaryConditions& bcs,
                                                                     const CompoundLithology*  elementLithology,
                                                                     const bool                includeChemicalCompaction,
                                                                     ElementMatrix&            elementStiffnessMatrix,
                                                                     ElementVector&            elementLoadVector ) const {


   const LayerElement& layerElement = element.getLayerElement ();

   ElementGeometryMatrix geometryMatrix;

   ElementVector currentPh;
   ElementVector currentPo;
   ElementVector previousPp;
   ElementVector currentPp;
   ElementVector previousPl;
   ElementVector currentPl;
   ElementVector previousVes;
   ElementVector currentVes;
   ElementVector previousMaxVes;
   ElementVector currentMaxVes;
   ElementVector previousTemperature;
   ElementVector currentTemperature;
   ElementVector previousChemicalCompaction;
   ElementVector currentChemicalCompaction;
   ElementVector elementHeatProduction;

   getGeometryMatrix ( layerElement, geometryMatrix );
   getCoefficients ( layerElement, Basin_Modelling::Hydrostatic_Pressure, currentPh );
   getCoefficients ( layerElement, Basin_Modelling::Overpressure,         currentPo );
   getCoefficients ( layerElement, Basin_Modelling::Pore_Pressure,        currentPp );
   getCoefficients ( layerElement, Basin_Modelling::Lithostatic_Pressure, currentPl );
   getCoefficients ( layerElement, Basin_Modelling::VES_FP,               currentVes );
   getCoefficients ( layerElement, Basin_Modelling::Max_VES,              currentMaxVes );
   getCoefficients ( layerElement, Basin_Modelling::Temperature,          currentTemperature );
   getCoefficients ( layerElement, Basin_Modelling::Chemical_Compaction,  currentChemicalCompaction );

   getPreviousCoefficients ( layerElement, Basin_Modelling::Pore_Pressure,        previousPp );
   getPreviousCoefficients ( layerElement, Basin_Modelling::Lithostatic_Pressure, previousPl );
   getPreviousCoefficients ( layerElement, Basin_Modelling::VES_FP,               previousVes );
   getPreviousCoefficients ( layerElement, Basin_Modelling::Max_VES,              previousMaxVes );
   getPreviousCoefficients ( layerElement, Basin_Modelling::Temperature,          previousTemperature );
   getPreviousCoefficients ( layerElement, Basin_Modelling::Chemical_Compaction,  previousChemicalCompaction );

   getCoefficients ( layerElement, bulkHeatProd, elementHeatProduction );

   Basin_Modelling::assembleElementTemperatureStiffnessMatrix ( layerElement.getFormation ()->kind() == Interface::BASEMENT_FORMATION,
                                                                planeQuadratureDegree,
                                                                depthQuadratureDegree,
                                                                currentTime,
                                                                timeStep,
                                                                includeAdvectiveTerm,
                                                                bcs,
                                                                elementLithology,
                                                                layerElement.getFormation ()->fluid,
                                                                includeChemicalCompaction,
                                                                geometryMatrix,
                                                                elementHeatProduction,
                                                                currentPh,
                                                                currentPo,
                                                                previousPp,
                                                                currentPp,
                                                                previousPl,
                                                                currentPl,
                                                                previousVes,
                                                                currentVes,
                                                                previousMaxVes,
                                                                currentMaxVes,
                                                                previousTemperature,
                                                                currentTemperature,
                                                                previousChemicalCompaction,
                                                                currentChemicalCompaction,
                                                                elementStiffnessMatrix,
                                                                elementLoadVector );

}


//------------------------------------------------------------//

void Temperature_Solver::assembleElementNonLinearSystem ( const GeneralElement&     element,
                                                          const PETSC_3D_Array&     bulkHeatProd,
                                                          const int                 planeQuadratureDegree,
                                                          const int                 depthQuadratureDegree,
                                                          const double              currentTime,
                                                          const double              timeStep,
                                                          const bool                includeAdvectiveTerm,
                                                          const BoundaryConditions& bcs,
                                                          const CompoundLithology*  elementLithology,
                                                          const bool                includeChemicalCompaction,
                                                          ElementMatrix&            elementJacobian,
                                                          ElementVector&            elementResidual ) const {

   const LayerElement& layerElement = element.getLayerElement ();

   ElementGeometryMatrix geometryMatrix;

   ElementVector previousPh;
   ElementVector currentPh;
   ElementVector previousPo;
   ElementVector currentPo;
   ElementVector previousPp;
   ElementVector currentPp;
   ElementVector previousPl;
   ElementVector currentPl;
   ElementVector previousVes;
   ElementVector currentVes;
   ElementVector previousMaxVes;
   ElementVector currentMaxVes;
   ElementVector previousTemperature;
   ElementVector currentTemperature;
   ElementVector previousChemicalCompaction;
   ElementVector currentChemicalCompaction;
   ElementVector elementHeatProduction;

   getGeometryMatrix ( layerElement, geometryMatrix );
   getCoefficients ( layerElement, Basin_Modelling::Hydrostatic_Pressure, currentPh );
   getCoefficients ( layerElement, Basin_Modelling::Overpressure,         currentPo );
   getCoefficients ( layerElement, Basin_Modelling::Lithostatic_Pressure, currentPl );
   getCoefficients ( layerElement, Basin_Modelling::VES_FP,               currentVes );
   getCoefficients ( layerElement, Basin_Modelling::Max_VES,              currentMaxVes );
   getCoefficients ( layerElement, Basin_Modelling::Temperature,          currentTemperature );
   getCoefficients ( layerElement, Basin_Modelling::Chemical_Compaction,  currentChemicalCompaction );

   getPreviousCoefficients ( layerElement, Basin_Modelling::Hydrostatic_Pressure, previousPh );
   getPreviousCoefficients ( layerElement, Basin_Modelling::Overpressure,         previousPo );
   getPreviousCoefficients ( layerElement, Basin_Modelling::VES_FP,               previousVes );
   getPreviousCoefficients ( layerElement, Basin_Modelling::Max_VES,              previousMaxVes );
   getPreviousCoefficients ( layerElement, Basin_Modelling::Temperature,          previousTemperature );
   getPreviousCoefficients ( layerElement, Basin_Modelling::Chemical_Compaction,  previousChemicalCompaction );

   getCoefficients ( layerElement, bulkHeatProd, elementHeatProduction );

   Basin_Modelling::assembleElementTemperatureSystem ( layerElement.getFormation ()->kind() == Interface::BASEMENT_FORMATION,
                                                       planeQuadratureDegree,
                                                       depthQuadratureDegree,
                                                       currentTime,
                                                       timeStep,
                                                       includeAdvectiveTerm,
                                                       bcs,
                                                       elementLithology,
                                                       layerElement.getFormation ()->fluid,
                                                       includeChemicalCompaction,
                                                       geometryMatrix,
                                                       elementHeatProduction,
                                                       previousPh,
                                                       currentPh,
                                                       previousPo,
                                                       currentPo,
                                                       currentPl,
                                                       previousVes,
                                                       currentVes,
                                                       previousMaxVes,
                                                       currentMaxVes,
                                                       previousTemperature,
                                                       currentTemperature,
                                                       previousChemicalCompaction,
                                                       currentChemicalCompaction,
                                                       elementJacobian,
                                                       elementResidual );

}

//------------------------------------------------------------//

void Temperature_Solver::assembleElementNonLinearResidual ( const GeneralElement&     element,
                                                            const PETSC_3D_Array&     bulkHeatProd,
                                                            const int                 planeQuadratureDegree,
                                                            const int                 depthQuadratureDegree,
                                                            const double              currentTime,
                                                            const double              timeStep,
                                                            const bool                includeAdvectiveTerm,
                                                            const BoundaryConditions& bcs,
                                                            const CompoundLithology*  elementLithology,
                                                            const bool                includeChemicalCompaction,
                                                            ElementVector&            elementResidual ) const {

   const LayerElement& layerElement = element.getLayerElement ();

   ElementGeometryMatrix geometryMatrix;

   ElementVector previousPh;
   ElementVector currentPh;
   ElementVector previousPo;
   ElementVector currentPo;
   ElementVector currentPl;
   ElementVector previousVes;
   ElementVector currentVes;
   ElementVector previousMaxVes;
   ElementVector currentMaxVes;
   ElementVector previousTemperature;
   ElementVector currentTemperature;
   ElementVector previousChemicalCompaction;
   ElementVector currentChemicalCompaction;
   ElementVector elementHeatProduction;

   getGeometryMatrix ( layerElement, geometryMatrix );
   getCoefficients ( layerElement, Basin_Modelling::Hydrostatic_Pressure, currentPh );
   getCoefficients ( layerElement, Basin_Modelling::Overpressure,         currentPo );
   getCoefficients ( layerElement, Basin_Modelling::Lithostatic_Pressure, currentPl );
   getCoefficients ( layerElement, Basin_Modelling::VES_FP,               currentVes );
   getCoefficients ( layerElement, Basin_Modelling::Max_VES,              currentMaxVes );
   getCoefficients ( layerElement, Basin_Modelling::Temperature,          currentTemperature );
   getCoefficients ( layerElement, Basin_Modelling::Chemical_Compaction,  currentChemicalCompaction );

   getPreviousCoefficients ( layerElement, Basin_Modelling::Hydrostatic_Pressure, previousPh );
   getPreviousCoefficients ( layerElement, Basin_Modelling::Overpressure,         previousPo );
   getPreviousCoefficients ( layerElement, Basin_Modelling::VES_FP,               previousVes );
   getPreviousCoefficients ( layerElement, Basin_Modelling::Max_VES,              previousMaxVes );
   getPreviousCoefficients ( layerElement, Basin_Modelling::Temperature,          previousTemperature );
   getPreviousCoefficients ( layerElement, Basin_Modelling::Chemical_Compaction,  previousChemicalCompaction );

   getCoefficients ( layerElement, bulkHeatProd, elementHeatProduction );

   Basin_Modelling::assembleElementTemperatureResidual ( layerElement.getFormation ()->kind() == Interface::BASEMENT_FORMATION,
                                                         planeQuadratureDegree,
                                                         depthQuadratureDegree,
                                                         currentTime,
                                                         timeStep,
                                                         includeAdvectiveTerm,
                                                         bcs,
                                                         elementLithology,
                                                         layerElement.getFormation ()->fluid,
                                                         includeChemicalCompaction,
                                                         geometryMatrix,
                                                         elementHeatProduction,
                                                         previousPh,
                                                         currentPh,
                                                         previousPo,
                                                         currentPo,
                                                         currentPl,
                                                         previousVes,
                                                         currentVes,
                                                         previousMaxVes,
                                                         currentMaxVes,
                                                         previousTemperature,
                                                         currentTemperature,
                                                         previousChemicalCompaction,
                                                         currentChemicalCompaction,
                                                         elementResidual );

}

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Temperature_Solver::assembleStiffnessMatrix"

void Temperature_Solver::assembleStiffnessMatrix ( const ComputationalDomain& computationalDomain,
                                                   const double previousTime,
                                                   const double currentTime,
                                                   Mat&         stiffnessMatrix,
                                                   Vec&         loadVector,
                                                   double&      elementContributionsTime ) {

  using namespace Basin_Modelling;

  PetscLogStages::push( PetscLogStages :: TEMPERATURE_SYSTEM_ASSEMBLY);

  const double timeStep = previousTime - currentTime;

  const int planeQuadratureDegree = getPlaneQuadratureDegree ( Basin_Model -> Optimisation_Level );
  const int depthQuadratureDegree = getDepthQuadratureDegree ( Basin_Model -> Optimisation_Level );

  const bool SteadyStateCalculation = ( previousTime == currentTime );

  const bool includeAdvectiveTerm = Basin_Model->includeAdvectiveTerm and 
                                    not SteadyStateCalculation and
                                   ( Basin_Model->Do_Iteratively_Coupled or
                                     Basin_Model->IsCalculationCoupled );

  const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

  PetscLogDouble Element_Start_Time;
  PetscLogDouble Element_End_Time;
			    
  bool IncludeGhosts = true;

  BoundaryConditions bcs;

  const GeoPhysics::CompoundLithology*  elementLithology;
  ElementMatrix elementStiffnessMatrix;
  ElementVector elementLoadVector;

  bool includeChemicalCompaction;

  Layer_Iterator Layers;
  Layers.Initialise_Iterator ( Basin_Model -> layers, Ascending, Basement_And_Sediments, 
			       Active_Layers_Only );

  elementContributionsTime = 0.0;

  const CompositeElementActivityPredicate& activityPredicate = computationalDomain.getActivityPredicate ();
  int topIndex = computationalDomain.getStratigraphicColumn ().getNumberOfLogicalNodesInDepth ( currentTime ) - 1;

  PETSC_2D_Array topBasaltDepth;
  PETSC_2D_Array bottomBasaltDepth;

  if ( Basin_Model -> isALC()) {
     CrustFormation*   crustLayer = dynamic_cast<CrustFormation*>(Basin_Model -> Crust ());   
     MantleFormation*  mantleLayer = dynamic_cast<MantleFormation*>(Basin_Model -> Mantle ());
     crustLayer->cleanVectors();
     mantleLayer->cleanVectors();

     topBasaltDepth.Set_Global_Array ( * Basin_Model ->mapDA, crustLayer -> TopBasaltDepth, INSERT_VALUES, IncludeGhosts );
     bottomBasaltDepth.Set_Global_Array ( * Basin_Model ->mapDA, crustLayer -> BottomBasaltDepth, INSERT_VALUES, IncludeGhosts );
  }

  MatZeroEntries ( stiffnessMatrix );
  
  WallTime::Time startAss = WallTime::clock ();

  while ( ! Layers.Iteration_Is_Done () ) {

    LayerProps* currentLayer = Layers.Current_Layer ();

    const ComputationalDomain::FormationGeneralElementGrid* formationGrid = computationalDomain.getFormationGrid ( currentLayer );

    currentLayer->Current_Properties.Activate_Properties  ( INSERT_VALUES, IncludeGhosts );
    currentLayer->Previous_Properties.Activate_Properties ( INSERT_VALUES, IncludeGhosts );

    PETSC_3D_Array bulkHeatProd ( currentLayer-> layerDA, 
                                  currentLayer -> BulkHeatProd,
                                  INSERT_VALUES, IncludeGhosts );

    // The order is important here (k,i,j) in order to retain the same basalt thickness.
    for ( int k = formationGrid->firstK (); k <= formationGrid->lastK (); ++k ) {

       for ( int i = formationGrid->firstI (); i <= formationGrid->lastI (); ++i ) {

          for ( int j = formationGrid->firstJ (); j <= formationGrid->lastJ (); ++j ) {

             const GeneralElement& gridElement = formationGrid->getElement ( i, j, k );
             const LayerElement& layerElement = gridElement.getLayerElement ();

             if ( activityPredicate.isActive ( layerElement )) {

                includeChemicalCompaction = (( Basin_Model -> Do_Chemical_Compaction ) && ( currentLayer -> Get_Chemical_Compaction_Mode ()));

                elementLithology  = layerElement.getLithology ();
                getBoundaryConditions ( gridElement, currentTime, topIndex, bcs );

                if ( Basin_Model->isALC () and currentLayer->isBasement ()) {
                   getAlcBcsAndLithology ( gridElement, previousTime, currentTime, elementLithology, topBasaltDepth, bottomBasaltDepth, bcs );
                }

                PetscTime(&Element_Start_Time);

                assembleElementTemperatureStiffnessMatrix ( gridElement,
                                                            bulkHeatProd,
                                                            planeQuadratureDegree,
                                                            depthQuadratureDegree,
                                                            currentTime,
                                                            timeStep,
                                                            includeAdvectiveTerm,
                                                            bcs,
                                                            elementLithology,
                                                            includeChemicalCompaction,
                                                            elementStiffnessMatrix,
                                                            elementLoadVector );

                PetscTime(&Element_End_Time);
                elementContributionsTime = elementContributionsTime + Element_End_Time - Element_Start_Time;

                MatSetValues ( stiffnessMatrix,
                               8, gridElement.getDofs ().data (),
                               8, gridElement.getDofs ().data (),
                               elementStiffnessMatrix.C_Array (),
                               ADD_VALUES );

                VecSetValues ( loadVector,
                               8, gridElement.getDofs ().data (),
                               elementLoadVector.data (),
                               ADD_VALUES );
             }

          }

       }

    }

    currentLayer->Current_Properties.Restore_Properties ();
    currentLayer->Previous_Properties.Restore_Properties ();

    Layers++;
  }


  if ( Basin_Model -> isALC ()) {
     topBasaltDepth.Restore_Global_Array( Update_Excluding_Ghosts );  
     bottomBasaltDepth.Restore_Global_Array( Update_Excluding_Ghosts ); 
  }

  VecAssemblyBegin ( loadVector );
  VecAssemblyEnd ( loadVector );

  MatAssemblyBegin ( stiffnessMatrix, MAT_FINAL_ASSEMBLY );
  MatAssemblyEnd ( stiffnessMatrix, MAT_FINAL_ASSEMBLY );

  PetscLogStages::pop();
}

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Temperature_Solver::assembleStiffnessMatrix"

void Temperature_Solver::assembleSystem ( const ComputationalDomain& computationalDomain,
                                          const double               previousTime,
                                          const double               currentTime,
                                          Mat&                       jacobian,
                                          Vec&                       residual,
                                          double&                    elementContributionsTime ) {

  using namespace Basin_Modelling;

  PetscLogStages::push( PetscLogStages::TEMPERATURE_INITIALISATION_SYSTEM_ASSEMBLY);

  const double timeStep = previousTime - currentTime;

  const int planeQuadratureDegree = getPlaneQuadratureDegree ( Basin_Model -> Optimisation_Level );
  const int depthQuadratureDegree = getDepthQuadratureDegree ( Basin_Model -> Optimisation_Level );

  const bool SteadyStateCalculation = ( previousTime == currentTime );

  const bool includeAdvectiveTerm = Basin_Model->includeAdvectiveTerm and 
                                    not SteadyStateCalculation and
                                   ( Basin_Model->Do_Iteratively_Coupled or
                                     Basin_Model->IsCalculationCoupled );

  const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

  PetscLogDouble Element_Start_Time;
  PetscLogDouble Element_End_Time;
			    
  bool IncludeGhosts = true;

  BoundaryConditions bcs;

  const GeoPhysics::CompoundLithology*  elementLithology;
  ElementMatrix elementJacobian;
  ElementVector elementResidual;

  bool includeChemicalCompaction;

  Layer_Iterator Layers;
  Layers.Initialise_Iterator ( Basin_Model -> layers, Ascending, Basement_And_Sediments, 
			       Active_Layers_Only );

  elementContributionsTime = 0.0;

  const CompositeElementActivityPredicate& activityPredicate = computationalDomain.getActivityPredicate ();
  int topIndex = computationalDomain.getStratigraphicColumn ().getNumberOfLogicalNodesInDepth ( currentTime ) - 1;

  PETSC_2D_Array topBasaltDepth;
  PETSC_2D_Array bottomBasaltDepth;

  if( Basin_Model -> isALC() ) {
     CrustFormation*   crustLayer = dynamic_cast<CrustFormation*>(Basin_Model -> Crust ());   
     MantleFormation*  mantleLayer = dynamic_cast<MantleFormation*>(Basin_Model -> Mantle ());
     crustLayer->cleanVectors();
     mantleLayer->cleanVectors();
     topBasaltDepth.Set_Global_Array ( * Basin_Model ->mapDA, crustLayer -> TopBasaltDepth, INSERT_VALUES, IncludeGhosts );
     bottomBasaltDepth.Set_Global_Array ( * Basin_Model ->mapDA, crustLayer -> BottomBasaltDepth, INSERT_VALUES, IncludeGhosts );
  }

  MatZeroEntries ( jacobian );
  
  WallTime::Time startAss = WallTime::clock ();

  while ( ! Layers.Iteration_Is_Done () ) {

    LayerProps* currentLayer = Layers.Current_Layer ();

    const ComputationalDomain::FormationGeneralElementGrid* formationGrid = computationalDomain.getFormationGrid ( currentLayer );

    currentLayer->Current_Properties.Activate_Properties  ( INSERT_VALUES, IncludeGhosts );
    currentLayer->Previous_Properties.Activate_Properties ( INSERT_VALUES, IncludeGhosts );

    PETSC_3D_Array bulkHeatProd ( currentLayer-> layerDA, 
                                  currentLayer -> BulkHeatProd,
                                  INSERT_VALUES, IncludeGhosts );

    // The order is important here (k,i,j) in order to retain the same basalt thickness.
    for ( int k = formationGrid->firstK (); k <= formationGrid->lastK (); ++k ) {

       for ( int i = formationGrid->firstI (); i <= formationGrid->lastI (); ++i ) {

          for ( int j = formationGrid->firstJ (); j <= formationGrid->lastJ (); ++j ) {

             const GeneralElement& gridElement = formationGrid->getElement ( i, j, k );
             const LayerElement& layerElement = gridElement.getLayerElement ();

             if ( activityPredicate.isActive ( layerElement )) {

                includeChemicalCompaction = (( Basin_Model -> Do_Chemical_Compaction ) && ( currentLayer -> Get_Chemical_Compaction_Mode ()));

                elementLithology  = layerElement.getLithology ();
                getBoundaryConditions ( gridElement, currentTime, topIndex, bcs );

                if ( Basin_Model->isALC () and currentLayer->isBasement ()) {
                   getAlcBcsAndLithology ( gridElement, previousTime, currentTime, elementLithology, topBasaltDepth, bottomBasaltDepth, bcs );
                }

                PetscTime(&Element_Start_Time);

                assembleElementNonLinearSystem ( gridElement,
                                                 bulkHeatProd,
                                                 planeQuadratureDegree,
                                                 depthQuadratureDegree,
                                                 currentTime,
                                                 timeStep,
                                                 includeAdvectiveTerm,
                                                 bcs,
                                                 elementLithology,
                                                 includeChemicalCompaction,
                                                 elementJacobian,
                                                 elementResidual );

                PetscTime(&Element_End_Time);
                elementContributionsTime = elementContributionsTime + Element_End_Time - Element_Start_Time;

                MatSetValues ( jacobian,
                               8, gridElement.getDofs ().data (),
                               8, gridElement.getDofs ().data (),
                               elementJacobian.C_Array (),
                               ADD_VALUES );

                VecSetValues ( residual,
                               8, gridElement.getDofs ().data (),
                               elementResidual.data (),
                               ADD_VALUES );
             }

          }

       }

    }

    currentLayer->Current_Properties.Restore_Properties ();
    currentLayer->Previous_Properties.Restore_Properties ();

    Layers++;
  }

  if ( Basin_Model -> isALC ()) {
     topBasaltDepth.Restore_Global_Array( Update_Excluding_Ghosts );  
     bottomBasaltDepth.Restore_Global_Array( Update_Excluding_Ghosts ); 
  }

  VecAssemblyBegin ( residual );
  VecAssemblyEnd ( residual );

  MatAssemblyBegin ( jacobian, MAT_FINAL_ASSEMBLY );
  MatAssemblyEnd ( jacobian, MAT_FINAL_ASSEMBLY );

  PetscLogStages::pop();
}

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Temperature_Solver::assembleResidual"

void Temperature_Solver::assembleResidual ( const ComputationalDomain& computationalDomain,
                                            const double               previousTime,
                                            const double               currentTime,
                                            Vec&                       residual,
                                            double&                    elementContributionsTime ) {

  using namespace Basin_Modelling;

  PetscLogStages::push( PetscLogStages::TEMPERATURE_INITIALISATION_SYSTEM_ASSEMBLY);

  const double timeStep = previousTime - currentTime;

  const int planeQuadratureDegree = getPlaneQuadratureDegree ( Basin_Model -> Optimisation_Level );
  const int depthQuadratureDegree = getDepthQuadratureDegree ( Basin_Model -> Optimisation_Level );

  const bool SteadyStateCalculation = ( previousTime == currentTime );

  const bool includeAdvectiveTerm = Basin_Model->includeAdvectiveTerm and 
                                    not SteadyStateCalculation and
                                   ( Basin_Model->Do_Iteratively_Coupled or
                                     Basin_Model->IsCalculationCoupled );

  const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

  PetscLogDouble Element_Start_Time;
  PetscLogDouble Element_End_Time;
		    
  bool IncludeGhosts = true;


  BoundaryConditions bcs;

  const GeoPhysics::CompoundLithology*  elementLithology;
  ElementVector elementResidual;

  bool includeChemicalCompaction;

  Layer_Iterator Layers;
  Layers.Initialise_Iterator ( Basin_Model -> layers, Ascending, Basement_And_Sediments, 
			       Active_Layers_Only );

  elementContributionsTime = 0.0;

  const CompositeElementActivityPredicate& activityPredicate = computationalDomain.getActivityPredicate ();
  int topIndex = computationalDomain.getStratigraphicColumn ().getNumberOfLogicalNodesInDepth ( currentTime ) - 1;

  PETSC_2D_Array topBasaltDepth;
  PETSC_2D_Array bottomBasaltDepth;

  if( Basin_Model -> isALC() ) {
     CrustFormation*   crustLayer = dynamic_cast<CrustFormation*>(Basin_Model -> Crust ());   
     MantleFormation*  mantleLayer = dynamic_cast<MantleFormation*>(Basin_Model -> Mantle ());
     crustLayer->cleanVectors();
     mantleLayer->cleanVectors();

     topBasaltDepth.Set_Global_Array ( * Basin_Model ->mapDA, crustLayer -> TopBasaltDepth, INSERT_VALUES, IncludeGhosts );
     bottomBasaltDepth.Set_Global_Array ( * Basin_Model ->mapDA, crustLayer -> BottomBasaltDepth, INSERT_VALUES, IncludeGhosts );
  }
  
  WallTime::Time startAss = WallTime::clock ();


  while ( ! Layers.Iteration_Is_Done () ) {

    LayerProps* currentLayer = Layers.Current_Layer ();

    const ComputationalDomain::FormationGeneralElementGrid* formationGrid = computationalDomain.getFormationGrid ( currentLayer );

    currentLayer->Current_Properties.Activate_Properties  ( INSERT_VALUES, IncludeGhosts );
    currentLayer->Previous_Properties.Activate_Properties ( INSERT_VALUES, IncludeGhosts );

    PETSC_3D_Array bulkHeatProd ( currentLayer-> layerDA, 
                                  currentLayer -> BulkHeatProd,
                                  INSERT_VALUES, IncludeGhosts );

    // The order is important here (k,i,j) in order to retain the same basalt thickness.
    for ( int k = formationGrid->firstK (); k <= formationGrid->lastK (); ++k ) {

       for ( int i = formationGrid->firstI (); i <= formationGrid->lastI (); ++i ) {

          for ( int j = formationGrid->firstJ (); j <= formationGrid->lastJ (); ++j ) {

             const GeneralElement& gridElement = formationGrid->getElement ( i, j, k );
             const LayerElement& layerElement = gridElement.getLayerElement ();

             if ( activityPredicate.isActive ( layerElement )) {

                includeChemicalCompaction = (( Basin_Model -> Do_Chemical_Compaction ) && ( currentLayer -> Get_Chemical_Compaction_Mode ()));

                elementLithology  = layerElement.getLithology ();
                getBoundaryConditions ( gridElement, currentTime, topIndex, bcs );

                if ( Basin_Model->isALC () and currentLayer->isBasement ()) {
                   getAlcBcsAndLithology ( gridElement, previousTime, currentTime, elementLithology, topBasaltDepth, bottomBasaltDepth, bcs );
                }

                PetscTime(&Element_Start_Time);

                assembleElementNonLinearResidual ( gridElement,
                                                   bulkHeatProd,
                                                   planeQuadratureDegree,
                                                   depthQuadratureDegree,
                                                   currentTime,
                                                   timeStep,
                                                   includeAdvectiveTerm,
                                                   bcs,
                                                   elementLithology,
                                                   includeChemicalCompaction,
                                                   elementResidual );

                PetscTime(&Element_End_Time);
                elementContributionsTime = elementContributionsTime + Element_End_Time - Element_Start_Time;

                VecSetValues ( residual,
                               8, gridElement.getDofs ().data (),
                               elementResidual.data (),
                               ADD_VALUES );
             }

          }

       }

    }

    currentLayer->Current_Properties.Restore_Properties ();
    currentLayer->Previous_Properties.Restore_Properties ();

    Layers++;
  }


  if( Basin_Model -> isALC ()) {
     topBasaltDepth.Restore_Global_Array( Update_Excluding_Ghosts );  
     bottomBasaltDepth.Restore_Global_Array( Update_Excluding_Ghosts ); 
  }


  VecAssemblyBegin ( residual );
  VecAssemblyEnd ( residual );

  PetscLogStages::pop();
}

