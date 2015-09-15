#include "PressureSolver.h"

#include "Interface/RunParameters.h"



#include "FiniteElementTypes.h"

#include "FastcauldronSimulator.h"
#include "HydraulicFracturingManager.h"

#include "element_contributions.h"
#include "layer_iterators.h"

#include "fem_grid_auxiliary_functions.h"

#include "Nodal3DIndexArray.h"
#include "MultiComponentFlowHandler.h"

#include "PetscLogStages.h"

using namespace FiniteElementMethod;


//------------------------------------------------------------//

const double PressureSolver::GeometricLoopThicknessTolerance = 0.001;


const int PressureSolver::DefaultMaximumPressureLinearSolverIterations = 1000;
const int PressureSolver::MaximumGMResRestartValue = 200;
const int PressureSolver::GMResRestartIncrementValue = 20;
const int PressureSolver::DefaultGMResRestartValue = 80;



int PressureSolver::PlaneQuadratureDegrees [ NumberOfOptimisationLevels ] = { 2, 2, 2, 2, 3 };

int PressureSolver::DepthQuadratureDegrees [ NumberOfOptimisationLevels ] = { 2, 2, 2, 3, 4 };

int PressureSolver::s_iterationsForiluFillLevelIncrease [ NumberOfOptimisationLevels ] = { 80, 80, 80, 80, 80 };

int PressureSolver::s_numberOfIterations [ NumberOfOptimisationLevels ] = { 8, 12, 15, 20, 30 };

double PressureSolver::NewtonSolverTolerances [ NumberOfOptimisationLevels ][ 3 ] = {{ 1.0e-2, 1.0e-2, 1.0e-2 },
                                                                                     { 1.0e-2, 1.0e-3, 1.0e-3 },
                                                                                     { 1.0e-2, 1.0e-3, 1.0e-3 },
                                                                                     { 5.0e-3, 1.0e-3, 1.0e-4 },
                                                                                     { 1.0e-4, 1.0e-4, 1.0e-5 }};


//------------------------------------------------------------//

PressureSolver::PressureSolver(AppCtx *appl) : CauldronCalculator ( appl ) {
  initialiseFctCorrection();
  basisFunctions = 0;
}


PressureSolver::~PressureSolver() {

   if ( basisFunctions != 0 ) {
      delete basisFunctions;
   }

}

//------------------------------------------------------------//

void PressureSolver::initialiseFctCorrection () {

  using namespace Basin_Modelling;

  cauldron->timefilter.setFilter ( "FCTCorrection", "SedimentsOnly" );
  cauldron->timefilter.setFilter ( "ThicknessError", "SedimentsOnly" );

  LayerProps_Ptr Current_Layer;
  
  Layer_Iterator Layers ( cauldron->layers, Ascending, Sediments_Only, 
			  Active_And_Inactive_Layers );


  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) 
  {
    Current_Layer = Layers.Current_Layer ();

    if ( ! cauldron->readFCTCorrectionFactor ) {
      DMCreateGlobalVector( *cauldron->mapDA, &Current_Layer -> FCTCorrection );
      VecSet( Current_Layer -> FCTCorrection, One );
    }

    DMCreateGlobalVector( *cauldron->mapDA, &Current_Layer -> Thickness_Error );
    VecSet( Current_Layer -> Thickness_Error, Zero );
  } 

}

//------------------------------------------------------------//

void PressureSolver::restorePressureSolution ( const DM  pressureFEMGrid,
                                               const Vec Pressure_DOF_Numbers,
                                                     Vec Overpressure ) {

  using namespace Basin_Modelling;
//    PetscFunctionBegin;

  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;
  int FEM_Grid_Index;
  int DOF_Index;

  int I, J, K;
  int*GlobalK;

  int Z_Node_Count = 0;
  int Number_Of_Segments;

  Layer_Iterator Layers ( cauldron->layers, Ascending, Sediments_Only, Active_Layers_Only );
  LayerProps_Ptr Current_Layer;

  PETSC_3D_Array New_Overpressure ( pressureFEMGrid, Overpressure );
  PETSC_3D_Array DOFs             ( pressureFEMGrid, Pressure_DOF_Numbers );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {
    Current_Layer = Layers.Current_Layer ();
    Number_Of_Segments = Current_Layer->getNrOfActiveElements ();
    GlobalK = new int [ Number_Of_Segments + 1 ];

    for ( K = 0; K <= Number_Of_Segments; K++ ) {
      GlobalK [ K ] = Z_Node_Count;
      Z_Node_Count = Z_Node_Count + 1;
    }

    Z_Node_Count = Z_Node_Count - 1;

    DMDAGetCorners ( Current_Layer->layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

    PETSC_3D_Array Layer_Po ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Overpressure ));

    for ( I = xStart; I < xStart + xCount; I++ ) {

      for ( J = yStart; J < yStart + yCount; J++ ) {

        if ( cauldron->nodeIsDefined ( I, J ) ) {

          for ( K = zStart; K < zStart + zCount; K++ ) {
            FEM_Grid_Index = GlobalK[ K ];
            DOF_Index = (int) DOFs ( FEM_Grid_Index, J, I );
            New_Overpressure ( DOF_Index, J, I ) = Layer_Po ( K, J, I ) * MPa_To_Pa;
          }

	}

      }

    }

    delete [] GlobalK;

  }

}

//------------------------------------------------------------//

void PressureSolver::storePressureSolution ( const DM  pressureFEMGrid,
                                             const Vec Pressure_DOF_Numbers,
                                             const Vec Overpressure ) {

  using namespace Basin_Modelling;
  //
  //
  // Minimum amount of Overpressure, 1Pa (1.0e-6 Mpa)
  //
  const double Minimum_Absolute_Overpressure_MPa = 1.0e-30;

  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;
  int FEM_Grid_Index;
  int DOF_Index;

  int I, J, K;
  int*GlobalK;

  int Z_Node_Count = 0;
  int Number_Of_Segments;

  PetscScalar Overpressure_Value;
  PetscScalar Fracture_Pressure;


  Layer_Iterator Layers ( cauldron->layers, Ascending, Sediments_Only, Active_Layers_Only );
  LayerProps_Ptr Current_Layer;

  DMDAGetCorners ( *cauldron->mapDA, &xStart, &yStart, PETSC_NULL, &xCount, &yCount, PETSC_NULL );

  PETSC_3D_Array New_Overpressure ( pressureFEMGrid, Overpressure );
  PETSC_3D_Array DOFs             ( pressureFEMGrid, Pressure_DOF_Numbers );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {
    Current_Layer = Layers.Current_Layer ();
    Number_Of_Segments = Current_Layer->getNrOfActiveElements ();
    GlobalK = new int [ Number_Of_Segments + 1 ];

    for ( K = 0; K <= Number_Of_Segments; K++ ) {
      GlobalK [ K ] = Z_Node_Count;
      Z_Node_Count = Z_Node_Count + 1;
    }

    Z_Node_Count = Z_Node_Count - 1;


    DMDAGetCorners ( Current_Layer->layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

    Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Overpressure );
    Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Pore_Pressure );
    Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Hydrostatic_Pressure );
    Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Lithostatic_Pressure );

    for ( I = xStart; I < xStart + xCount; I++ ) {

      for ( J = yStart; J < yStart + yCount; J++ ) {

        if ( cauldron->nodeIsDefined ( I, J ) ) {

          for ( K = zStart; K < zStart + zCount; K++ ) {
            FEM_Grid_Index = GlobalK[ K ];
            DOF_Index = (int) DOFs ( FEM_Grid_Index, J, I );

            Overpressure_Value = New_Overpressure ( DOF_Index, J, I ) * Pa_To_MPa;

            if ( fabs ( Overpressure_Value ) < Minimum_Absolute_Overpressure_MPa ) {
              Overpressure_Value = 0.0;
            } // 

            Current_Layer -> Current_Properties ( Basin_Modelling::Overpressure,  K, J, I ) = Overpressure_Value;
            Current_Layer -> Current_Properties ( Basin_Modelling::Pore_Pressure, K, J, I ) = Overpressure_Value + 
                                                                                              Current_Layer -> Current_Properties ( Basin_Modelling::Hydrostatic_Pressure, K, J, I );

            if ( Current_Layer -> Current_Properties ( Basin_Modelling::Pore_Pressure, K, J, I )> 
                              Current_Layer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure, K, J, I )) {

              Fracture_Pressure = Current_Layer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure, K, J, I ) -
                                  Current_Layer -> Current_Properties ( Basin_Modelling::Hydrostatic_Pressure, K, J, I );

              Current_Layer -> Current_Properties ( Basin_Modelling::Overpressure,  K, J, I ) = Fracture_Pressure;
              Current_Layer -> Current_Properties ( Basin_Modelling::Pore_Pressure, K, J, I ) = Current_Layer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure,  K, J, I );
            }

          }

	}

      }

    }

    Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Overpressure );
    Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Pore_Pressure );
    Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Hydrostatic_Pressure );
    Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Lithostatic_Pressure );

    delete [] GlobalK;
  }


}

//------------------------------------------------------------//


void PressureSolver::assembleSystem ( const double  previousTime,
                                      const double  currentTime,
                                      const DM&     pressureFEMGrid,
                                      const Vec&    pressureDOFs,
                                      const Vec&    pressureNodeIncluded,
                                            Mat&    Jacobian,
                                            Vec&    Residual,
                                            double& elementContributionsTime ) {

  using namespace Basin_Modelling;

  
  PetscLogStages::push( PetscLogStages::PRESSURE_SYSTEM_ASSEMBLY );

  const int Plane_Quadrature_Degree = getPlaneQuadratureDegree ( cauldron->Optimisation_Level );
  const int Depth_Quadrature_Degree = getDepthQuadratureDegree ( cauldron->Optimisation_Level );


  PetscLogDouble Element_Start_Time;
  PetscLogDouble Element_End_Time;
  PetscLogDouble Start_Time;
  PetscLogDouble End_Time;

  PetscTime(&Start_Time);

  bool Include_Ghost_Values = true;

  const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

  double Delta_X  = grid.deltaI;
  double Delta_Y  = grid.deltaJ;
  double Origin_X = grid.originI;
  double Origin_Y = grid.originJ;

  int I;
  int J;
  int K;

  int FEM_K_Index;
  int Inode;

  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;
  int layerXCount;
  int layerYCount;
  int layerZCount;
  int globalXCount;
  int globalYCount;
  int globalZCount;
  int Element_Count = 0;
  int NumCoincidentNodes = 0;
  LayerProps_Ptr Current_Layer;
  LayerProps_Ptr Previous_Layer = LayerProps_Ptr ( 0 );
  const CompoundLithology* Element_Lithology;

  Layer_Iterator FEM_Layers ( cauldron->layers, Ascending, Sediments_Only, Active_Layers_Only );
  Layer_Iterator Layers_Above; // CHANGE THIS NAME!!!!!

  int I_Position;
  int J_Position;

  unsigned int Element_Index;

  PetscScalar Surface_Pressure_Value = 0.0;
                            
  MatStencil  col[8];
  MatStencil  row[8];
  MatStencil  a_diag[1];
  PetscScalar OneDiagonal[1] = {1.0};

#ifdef USE_EIGEN  
  Matrix8x8     Element_Jacobian;
#else
  ElementMatrix Element_Jacobian;
#endif

  ElementVector Element_Residual;

  ElementVector Element_Old_Po;
  ElementVector Element_Current_Po;
  ElementVector Element_VES;

  ElementVector Element_Old_VES;
  ElementVector Element_Old_Max_VES;
  ElementVector Element_Old_Hydrostatic_Pressure;
  ElementVector Element_Old_Lithostatic_Pressure;
  ElementVector Element_Old_Overpressure;

  ElementVector Element_Current_VES;
  ElementVector Element_Current_Max_VES;
  ElementVector Element_Current_Hydrostatic_Pressure;
  ElementVector Element_Current_Lithostatic_Pressure;
  ElementVector Element_Current_Overpressure;


  ElementVector Current_Chemical_Compaction;
  ElementVector Previous_Chemical_Compaction;

  ElementVector Previous_Element_Solid_Thickness;
  ElementVector Current_Element_Solid_Thickness;

  ElementVector Previous_Ph;
  ElementVector Current_Ph;
  ElementVector Current_Pl;

  ElementVector Previous_Po;
  ElementVector Current_Po;

  ElementVector Previous_Element_VES;
  ElementVector Current_Element_VES;

  ElementVector Previous_Element_Max_VES;
  ElementVector Current_Element_Max_VES;

  ElementVector Previous_Element_Temperature;
  ElementVector Current_Element_Temperature;

  double fracturePressure;
  ElementVector Exceeded_Fracture_Pressure;
  ElementVector preFractureScaling;
  BooleanVector Included_Nodes;

  Element_Positions Positions;

  double Constrained_Po_Value;
  bool   Overpressure_Is_Constrained;

  ElementGeometryMatrix Previous_Geometry_Matrix;
  ElementGeometryMatrix Geometry_Matrix;

  ElementVector Dirichlet_Boundary_Values;
  Boundary_Conditions Element_BCs [ 6 ];
  Boundary_Conditions Nodal_BCs[8];

  bool  degenerateElement;

  int *GlobalK;

  int Degenerate_Segments;

  DMDAGetInfo ( pressureFEMGrid, PETSC_NULL,&globalXCount,&globalYCount,&globalZCount, 
                PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL );

  PETSC_3D_Array residualVector ( pressureFEMGrid, Residual, INSERT_VALUES, Include_Ghost_Values );
  PETSC_3D_Array dof  ( pressureFEMGrid, pressureDOFs, INSERT_VALUES, Include_Ghost_Values);

  PETSC_3D_Array nodeIncluded  ( pressureFEMGrid, pressureNodeIncluded, INSERT_VALUES, Include_Ghost_Values );


  int Z_Node_Count = 0;
  int Number_Of_Layer_Segments;

  int localI;
  int localJ;

  bool Include_Chemical_Compaction;

  Saturation currentSaturation;
  Saturation previousSaturation;

  // double waterSaturation;
  // double previousWaterSaturation;

  bool includeWaterSaturation = FastcauldronSimulator::getInstance ().getMcfHandler ().includeWaterSaturationInOp ();

  bool includedInDarcySimulation = FastcauldronSimulator::getInstance ().getMcfHandler ().solveFlowEquations ();

#if 0
  bool bottomOfModel = true;
#endif

  VecSet ( Residual, Zero );

  Element_BCs [ Gamma_1 ] = Interior_Boundary;

  elementContributionsTime = 0.0;

  if ( basisFunctions == 0 ) {
     basisFunctions = new FiniteElementMethod::BasisFunctionCache ( Plane_Quadrature_Degree, Plane_Quadrature_Degree, Depth_Quadrature_Degree );
  }

  for ( FEM_Layers.Initialise_Iterator (); ! FEM_Layers.Iteration_Is_Done (); FEM_Layers++ ) {
    Current_Layer  = FEM_Layers.Current_Layer ();
    Previous_Layer = FEM_Layers.Layer_Above ();

    double fluidDensityForP0_1andT0 = Current_Layer->fluid->density ( 0,  0.1 ); 


    PetscBlockVector<Saturation> layerSaturations;
    PetscBlockVector<Saturation> previousLayerSaturations;

    if ( includedInDarcySimulation ) {
       layerSaturations.setVector ( Current_Layer->getVolumeGrid ( Saturation::NumberOfPhases ), Current_Layer->getPhaseSaturationVec (), INSERT_VALUES, false );
       previousLayerSaturations.setVector ( Current_Layer->getVolumeGrid ( Saturation::NumberOfPhases ), Current_Layer->getPreviousPhaseSaturationVec (), INSERT_VALUES, false );
    }

    Include_Chemical_Compaction = (( cauldron->Do_Chemical_Compaction ) && ( Current_Layer -> Get_Chemical_Compaction_Mode ()));

    Layers_Above.Initialise_Iterator ( cauldron->layers, Ascending, Current_Layer, Ascending, Sediments_Only, Active_Layers_Only );

    Current_Layer->getConstrainedOverpressure ( currentTime,
                                                Constrained_Po_Value,
                                                Overpressure_Is_Constrained );

    Number_Of_Layer_Segments = Current_Layer->getNrOfActiveElements();
    GlobalK = new int [ Number_Of_Layer_Segments + 1 ];

    for ( K = 0; K <= Number_Of_Layer_Segments; K++ ){
      GlobalK [ K ] = Z_Node_Count;
      Z_Node_Count = Z_Node_Count + 1;
    } 

    Z_Node_Count = Z_Node_Count - 1;

    Current_Layer->Current_Properties.Activate_Properties  ( INSERT_VALUES, Include_Ghost_Values );
    Current_Layer->Previous_Properties.Activate_Properties ( INSERT_VALUES, Include_Ghost_Values );

    DMDAGetInfo(Current_Layer->layerDA,0, &layerXCount, &layerYCount, &layerZCount,0,0,0,0,0,0,0,0,0);
    DMDAGetCorners ( Current_Layer->layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

    for ( K = zStart; K < zStart + zCount; K++ ) {

      if ( K != layerZCount - 1 ) {
        FEM_K_Index = GlobalK [ K ];

        for ( Element_Index = 0; Element_Index < cauldron->mapElementList.size (); Element_Index++ ) {
          Exceeded_Fracture_Pressure.fill ( 0.0 );
          preFractureScaling.fill ( 0.0 );
//            Exceeded_Fracture_Pressure.fill ( false );
          Included_Nodes.fill ( false );

          Element_Count = Element_Count + 1;
          Degenerate_Segments = 0;

          Element_Residual.zero ();

#ifndef USE_EIGEN
          Element_Jacobian.zero ();
#else
          Element_Jacobian.setZero ();
#endif
          if ( cauldron->mapElementList [ Element_Index ].exists ) {
            I_Position = cauldron->mapElementList [ Element_Index ].i [ 0 ];
            J_Position = cauldron->mapElementList [ Element_Index ].j [ 0 ];

            if ( includedInDarcySimulation ) {
               currentSaturation = layerSaturations ( K, J_Position, I_Position );
               previousSaturation = previousLayerSaturations ( K, J_Position, I_Position );
            } else {
               // If Darcy simulation has not been enabled then set the saturation to ( Water => 1, Vapour => 0, Liquid => 0 )
               currentSaturation.initialise ();
               previousSaturation.initialise ();
            }

#if 0
            // Assign Neumann type boundary conditions.
            if ( bottomOfModel and K == zCount - 1 ) {
              Element_BCs [ Gamma_6 ] = Bottom_Neumann_Boundary;
            } else {
              Element_BCs [ Gamma_6 ] = Interior_Boundary;
            }

            if ( cauldron->mapElementList [ Element_Index ].onDomainBoundary ( 0 )) {
              Element_BCs [ Gamma_2 ] = Side_Neumann_Boundary;
            } else {
              Element_BCs [ Gamma_2 ] = Interior_Boundary;
            }

            if ( cauldron->mapElementList [ Element_Index ].onDomainBoundary ( 1 )) {
              Element_BCs [ Gamma_3 ] = Side_Neumann_Boundary;
            } else {
              Element_BCs [ Gamma_3 ] = Interior_Boundary;
            }

            if ( cauldron->mapElementList [ Element_Index ].onDomainBoundary ( 2 )) {
              Element_BCs [ Gamma_4 ] = Side_Neumann_Boundary;
            } else {
              Element_BCs [ Gamma_4 ] = Interior_Boundary;
            }

            if ( cauldron->mapElementList [ Element_Index ].onDomainBoundary ( 3 )) {
              Element_BCs [ Gamma_5 ] = Side_Neumann_Boundary;
            } else {
              Element_BCs [ Gamma_5 ] = Interior_Boundary;
            }
#endif

            // Copy segment lithology
            Element_Lithology = Current_Layer->getLithology ( I_Position, J_Position );

            // if element hase fluid density more than matrix density, we assuming the solid is ice in this case. 
            bool isIceSheetLayer = Current_Layer->fluid->SwitchPermafrost() && fluidDensityForP0_1andT0 > Element_Lithology->density();

            degenerateElement = true;

            for ( Inode = 0; Inode < 4; Inode++ ) {
              I_Position = cauldron->mapElementList [ Element_Index ].i [ Inode ];
              J_Position = cauldron->mapElementList [ Element_Index ].j [ Inode ];

              Current_Element_Solid_Thickness  ( Inode + 1 ) = Current_Layer -> Current_Properties  ( Basin_Modelling::Solid_Thickness, K, J_Position, I_Position );
              Previous_Element_Solid_Thickness ( Inode + 1 ) = Current_Layer -> Previous_Properties ( Basin_Modelling::Solid_Thickness, K, J_Position, I_Position );

              if ( Current_Element_Solid_Thickness ( Inode + 1 ) == IBSNULLVALUE ) {
                Current_Element_Solid_Thickness ( Inode + 1 ) = 0.0;
              }

              if ( Previous_Element_Solid_Thickness ( Inode + 1 ) == IBSNULLVALUE ) {
                Previous_Element_Solid_Thickness ( Inode + 1 ) = 0.0;
              }

              degenerateElement = degenerateElement && ( Current_Element_Solid_Thickness ( Inode + 1 ) == 0.0 );
            }

            if ( degenerateElement || Element_Lithology->surfacePorosity () == 0.0 ) {
              continue;
            }


            for ( Inode = 0; Inode < 8; Inode++ ) {
              int LidxZ = K + ( Inode < 4 ? 1 : 0 );
              int GidxZ = FEM_K_Index + ( Inode < 4 ? 1 : 0 );
              int GidxY = cauldron->mapElementList [ Element_Index ].j[Inode%4];
              int GidxX = cauldron->mapElementList [ Element_Index ].i[Inode%4];

              localI = Integer_Min ( Integer_Max ( GidxX, xStart ), xStart + xCount - 1 ) - xStart;
              localJ = Integer_Min ( Integer_Max ( GidxY, yStart ), yStart + yCount - 1 ) - yStart;

              Exceeded_Fracture_Pressure ( Inode + 1 ) = Current_Layer -> fracturedPermeabilityScaling ( GidxX, GidxY, LidxZ );
              preFractureScaling ( Inode + 1 ) = double ( Current_Layer -> preFractureScaling ( GidxX, GidxY, LidxZ ));

              Included_Nodes ( Inode + 1 ) = Current_Layer -> includedNodes ( GidxX, GidxY, LidxZ );
              Positions.Set_Node_Position ( Inode, LidxZ, GidxY, GidxX );

              Dirichlet_Boundary_Values ( Inode + 1 ) = 0.0;

              if ( globalZCount-1 == int( dof(GidxZ,GidxY,GidxX) ) ) {
                //
                // A surface boundary node is any node having the same DOF number as the surface boundary node.
                //
                Nodal_BCs[Inode] = Surface_Boundary; 
                Dirichlet_Boundary_Values ( Inode + 1 ) = Surface_Pressure_Value;

              } else if ( Overpressure_Is_Constrained && ( Current_Element_Solid_Thickness ( Inode % 4 + 1 ) > EPS1 )) {

                Nodal_BCs[Inode] = Interior_Constrained_Overpressure; 
                Dirichlet_Boundary_Values ( Inode + 1 ) = Constrained_Po_Value;

              } else if ( K == zCount - 2 and
                          Exceeded_Fracture_Pressure ( Inode + 1 ) > 0.0 and
                          Previous_Layer != NULL and
                          Previous_Layer->getLithology ( I_Position, J_Position ) -> surfacePorosity () < 0.01 ) {

                // In this branch if fracture pressure has been exceeded and the 
                // lithology of the layer above is, for example, a salt.

                Nodal_BCs[Inode] = Interior_Constrained_Overpressure; 

                fracturePressure = HydraulicFracturingManager::getInstance().fracturePressure( Element_Lithology,
                                                                                               Current_Layer->fluid,
                                                                                               FastcauldronSimulator::getInstance().getSeaBottomTemperature( GidxX, GidxY, currentTime ),
                                                                                               FastcauldronSimulator::getInstance().getSeaBottomDepth( GidxX, GidxY, currentTime ),
                                                                                               Current_Layer->Current_Properties( Basin_Modelling::Depth, LidxZ, GidxY, GidxX ),
                                                                                               Current_Layer->Current_Properties( Basin_Modelling::Hydrostatic_Pressure, LidxZ, GidxY, GidxX ),
                                                                                               Current_Layer->Current_Properties( Basin_Modelling::Lithostatic_Pressure, LidxZ, GidxY, GidxX )
                                                                                             );

                // If the lithology above is a salt then scaling the permeabilities will not help.
                // So the best method is to make the node a Dirichlet type boundary condition.
                Dirichlet_Boundary_Values ( Inode + 1 ) = fracturePressure 
                                                          - Current_Layer->Current_Properties ( Basin_Modelling::Hydrostatic_Pressure, LidxZ, GidxY, GidxX );

                Dirichlet_Boundary_Values ( Inode + 1 ) = NumericFunctions::Maximum ( Dirichlet_Boundary_Values ( Inode + 1 ), 0.0 );

              } else if ( HydraulicFracturingManager::getInstance ().isNonConservativeFractureModel () and Current_Layer->nodeIsTemporarilyDirichlet ( GidxX, GidxY, LidxZ ) ) {

                fracturePressure = HydraulicFracturingManager::getInstance().fracturePressure( Element_Lithology,
                                                                                               Current_Layer->fluid,
                                                                                               FastcauldronSimulator::getInstance().getSeaBottomTemperature( GidxX, GidxY, currentTime ),
                                                                                               FastcauldronSimulator::getInstance().getSeaBottomDepth ( GidxX, GidxY, currentTime ),
                                                                                               Current_Layer->Current_Properties( Basin_Modelling::Depth, LidxZ, GidxY, GidxX ),
                                                                                               Current_Layer->Current_Properties( Basin_Modelling::Hydrostatic_Pressure, LidxZ, GidxY, GidxX ),
                                                                                               Current_Layer->Current_Properties( Basin_Modelling::Lithostatic_Pressure, LidxZ, GidxY, GidxX )
                                                                                             );

                Nodal_BCs[Inode] = Interior_Constrained_Overpressure; 
                Dirichlet_Boundary_Values ( Inode + 1 ) = NumericFunctions::Maximum ( fracturePressure - Current_Layer->Current_Properties ( Basin_Modelling::Hydrostatic_Pressure, LidxZ, GidxY, GidxX ), 0.0 );
              } else if ( GidxZ == 0 ) {
                
                 Nodal_BCs[Inode] = Bottom_Boundary_Flux;

              } else if ( isIceSheetLayer ) {
                 // For the ice sheet with Permafrost taking in account, we do noy want to "compute" the overpressure in the ice lithology - we want to impose it.
                 Nodal_BCs[Inode] = Interior_Constrained_Overpressure;
                 Dirichlet_Boundary_Values ( Inode + 1 ) = Current_Layer->Current_Properties( Basin_Modelling::Lithostatic_Pressure, LidxZ, GidxY, GidxX ) - 
                                                           Current_Layer->Current_Properties( Basin_Modelling::Hydrostatic_Pressure, LidxZ, GidxY, GidxX );
              }
              else {
                Nodal_BCs[Inode] = Interior_Boundary;
              }

              col[Inode].i = GidxX;
              col[Inode].j = GidxY;
              col[Inode].k = GidxZ;

              row[Inode].i = GidxX;
              row[Inode].j = GidxY;
              row[Inode].k = GidxZ;

              if (GidxZ != int ( dof(GidxZ,GidxY,GidxX ))) {
                Degenerate_Segments = Degenerate_Segments + 1;
                NumCoincidentNodes = max(NumCoincidentNodes,((int)dof(GidxZ,GidxY,GidxX)-GidxZ));

                col[Inode].k = (int) dof(GidxZ,GidxY,GidxX);
                row[Inode].k = col[Inode].k;
              }

              Geometry_Matrix ( 1, Inode + 1 ) = Origin_X + (Delta_X * GidxX);
              Geometry_Matrix ( 2, Inode + 1 ) = Origin_Y + (Delta_Y * GidxY);
              Geometry_Matrix ( 3, Inode + 1 ) = Current_Layer->Current_Properties ( Basin_Modelling::Depth, LidxZ, GidxY, GidxX );

              Previous_Geometry_Matrix ( 1, Inode + 1 ) = Origin_X + (Delta_X * GidxX);
              Previous_Geometry_Matrix ( 2, Inode + 1 ) = Origin_Y + (Delta_Y * GidxY);
              Previous_Geometry_Matrix ( 3, Inode + 1 ) = Current_Layer->Previous_Properties ( Basin_Modelling::Depth, LidxZ, GidxY, GidxX );
            }

            int realI = cauldron->mapElementList [ Element_Index ].i[0];
            int realJ = cauldron->mapElementList [ Element_Index ].j[0];

            const Nodal3DIndexArray& indices = Current_Layer->getLayerElement ( realI, realJ, K ).getNodePositions ();

            // Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Hydrostatic_Pressure, indices, Current_Ph );
            // Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Lithostatic_Pressure, indices, Current_Pl );
            // Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Overpressure,         indices, Current_Po );
            // Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::VES_FP,               indices, Current_Element_VES );
            // Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Max_VES,              indices, Current_Element_Max_VES );
            // Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Temperature,          indices, Current_Element_Temperature );
            // Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Chemical_Compaction,  indices, Current_Chemical_Compaction );

            // Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::Hydrostatic_Pressure, indices, Previous_Ph );
            // Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::Overpressure,         indices, Previous_Po );
            // Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::VES_FP,               indices, Previous_Element_VES );
            // Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::Max_VES,              indices, Previous_Element_Max_VES );
            // Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::Temperature,          indices, Previous_Element_Temperature );
            // Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::Chemical_Compaction,  indices, Previous_Chemical_Compaction );

            Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Hydrostatic_Pressure, Positions, Current_Ph );
            Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Lithostatic_Pressure, Positions, Current_Pl );
            Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Overpressure,         Positions, Current_Po );
            Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::VES_FP,               Positions, Current_Element_VES );
            Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Max_VES,              Positions, Current_Element_Max_VES );
            Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Temperature,          Positions, Current_Element_Temperature );
            Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Chemical_Compaction,  Positions, Current_Chemical_Compaction );

            Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::Hydrostatic_Pressure, Positions, Previous_Ph );
            Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::Overpressure,         Positions, Previous_Po );
            Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::VES_FP,               Positions, Previous_Element_VES );
            Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::Max_VES,              Positions, Previous_Element_Max_VES );
            Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::Temperature,          Positions, Previous_Element_Temperature );
            Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::Chemical_Compaction,  Positions, Previous_Chemical_Compaction );

            PetscTime(&Element_Start_Time);

            Assemble_Element_Pressure_System ( *basisFunctions,
                                               currentTime,
                                               previousTime - currentTime,
                                               Element_BCs,
                                               Nodal_BCs,
                                               Dirichlet_Boundary_Values,
                                               isIceSheetLayer,
                                               Element_Lithology,
                                               Current_Layer->fluid,
                                               Include_Chemical_Compaction,
                                               HydraulicFracturingManager::getInstance ().getModel (),
                                               Previous_Geometry_Matrix,
                                               Geometry_Matrix,
                                               Previous_Element_Solid_Thickness,
                                               Current_Element_Solid_Thickness,
                                               Previous_Ph,
                                               Current_Ph,
                                               Previous_Po,
                                               Current_Po,
                                               Current_Pl,
                                               Previous_Element_VES,
                                               Current_Element_VES,
                                               Previous_Element_Max_VES,
                                               Current_Element_Max_VES,
                                               Previous_Element_Temperature,
                                               Current_Element_Temperature,
                                               Previous_Chemical_Compaction,
                                               Current_Chemical_Compaction,
                                               Exceeded_Fracture_Pressure,
                                               preFractureScaling,
                                               Included_Nodes,
                                               includeWaterSaturation,
                                               currentSaturation,
                                               previousSaturation,
                                               Element_Jacobian,
                                               Element_Residual );

            PetscTime(&Element_End_Time);
            elementContributionsTime = elementContributionsTime + Element_End_Time - Element_Start_Time;

#ifdef USE_EIGEN
            MatSetValuesStencil( Jacobian, 8, row, 8, col, Element_Jacobian.data(), ADD_VALUES);
#else
            MatSetValuesStencil( Jacobian, 8, row, 8, col, Element_Jacobian.C_Array (), ADD_VALUES);
#endif
            for (Inode = 0; Inode<8; Inode++) {
              int irow = row[Inode].i;
              int jrow = row[Inode].j;
              int krow = row[Inode].k;

              residualVector(krow,jrow,irow) = residualVector(krow,jrow,irow) - Element_Residual ( Inode + 1 );
            }

          }

        }

      }

    } 

    Current_Layer->Current_Properties.Restore_Properties ();
    Current_Layer->Previous_Properties.Restore_Properties ();

#if 0
    bottomOfModel = false;
#endif

    delete[] GlobalK;

  }

  DMDAGetCorners ( pressureFEMGrid, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

  //
  // Now set the dummy nodes on the diagonal of the Jacobian matrix to 1.
  //
  for ( K = zStart; K < zStart + zCount; K++ ) {

    for ( J = yStart; J < yStart + yCount; J++ ) {

      for ( I = xStart; I < xStart + xCount; I++ ) {

        if ( (int) ( nodeIncluded ( K, J, I )) == 0 ) {
          a_diag[0].k = K;
          a_diag[0].j = J;
          a_diag[0].i = I; 

          MatSetValuesStencil( Jacobian, 1, a_diag, 1, a_diag, OneDiagonal, ADD_VALUES );
        }

      }

    }

  }

  residualVector.Restore_Global_Array ( Update_Including_Ghosts );  
  MatAssemblyBegin ( Jacobian, MAT_FINAL_ASSEMBLY );
  MatAssemblyEnd   ( Jacobian, MAT_FINAL_ASSEMBLY );

  PetscTime(&End_Time);

  PetscLogStages::pop();
}

//------------------------------------------------------------//

#if 0
void PressureSolver::assembleResidual ( const double  previousTime,
                                        const double  currentTime,
                                        const DM&     pressureFEMGrid,
                                        const Vec&    pressureDOFs,
                                              Vec&    Residual,
                                              double& elementContributionsTime ) {
  using namespace Basin_Modelling;

  const int Plane_Quadrature_Degree = getPlaneQuadratureDegree ( cauldron->Optimisation_Level );
  const int Depth_Quadrature_Degree = getDepthQuadratureDegree ( cauldron->Optimisation_Level );
                                               

  PetscLogDouble Element_Start_Time;
  PetscLogDouble Element_End_Time;
  PetscLogDouble Start_Time;
  PetscLogDouble End_Time;
  PetscTime(&Start_Time);

  bool Include_Ghost_Values = true;

  const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

  double Delta_X  = grid.deltaI;
  double Delta_Y  = grid.deltaJ;
  double Origin_X = grid.originI;
  double Origin_Y = grid.originJ;

  int K;
  int FEM_K_Index;
  int Inode;

  int xStart, yStart, zStart, xCount, yCount, zCount;
  int layerXCount, layerYCount, layerZCount;
  int globalXCount, globalYCount, globalZCount;
  int Element_Count = 0;
  int NumCoincidentNodes = 0;

  LayerProps_Ptr Current_Layer;
  LayerProps_Ptr Previous_Layer = LayerProps_Ptr ( 0 );
  const CompoundLithology* Element_Lithology;

  Layer_Iterator FEM_Layers ( cauldron->layers, Ascending, Sediments_Only, Active_Layers_Only );

  int I_Position;
  int J_Position;

  int localI;
  int localJ;

  unsigned int Element_Index;

  PetscScalar Surface_Pressure_Value = 0.0;
                            
  MatStencil   col[8];
  MatStencil   row[8];
  ElementVector Element_Residual;

  ElementVector Element_Old_Po;
  ElementVector Element_Current_Po;
  ElementVector Element_VES;

  ElementVector Element_Old_VES;
  ElementVector Element_Old_Max_VES;
  ElementVector Element_Old_Hydrostatic_Pressure;
  ElementVector Element_Old_Lithostatic_Pressure;
  ElementVector Element_Old_Overpressure;

  ElementVector Element_Current_VES;
  ElementVector Element_Current_Max_VES;
  ElementVector Element_Current_Hydrostatic_Pressure;
  ElementVector Element_Current_Lithostatic_Pressure;
  ElementVector Element_Current_Overpressure;


  ElementVector Current_Chemical_Compaction;
  ElementVector Previous_Chemical_Compaction;

  ElementVector Previous_Element_Solid_Thickness;
  ElementVector Current_Element_Solid_Thickness;

  ElementVector Previous_Ph;
  ElementVector Current_Ph;
  ElementVector Current_Pl;

  ElementVector Previous_Po;
  ElementVector Current_Po;

  ElementVector Previous_Element_VES;
  ElementVector Current_Element_VES;

  ElementVector Previous_Element_Max_VES;
  ElementVector Current_Element_Max_VES;

  ElementVector Previous_Element_Temperature;
  ElementVector Current_Element_Temperature;

  Element_Positions Positions;

  double Constrained_Po_Value;
  bool   Overpressure_Is_Constrained;
  bool   Include_Chemical_Compaction;
  bool   degenerateElement;

  ElementGeometryMatrix Previous_Geometry_Matrix;
  ElementGeometryMatrix Geometry_Matrix;


  double fracturePressure;
  ElementVector preFractureScaling;
  ElementVector Exceeded_Fracture_Pressure;
  ElementVector Dirichlet_Boundary_Values;
  Boundary_Conditions Element_BCs[6];
  Boundary_Conditions Nodal_BCs[8];

  int *GlobalK;

  int Degenerate_Segments;

  DMDAGetInfo ( pressureFEMGrid, PETSC_NULL,&globalXCount, &globalYCount, &globalZCount, 
              PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL );


  PETSC_3D_Array residualVector ( pressureFEMGrid, Residual, INSERT_VALUES, Include_Ghost_Values );

  PETSC_3D_Array dof  ( pressureFEMGrid, pressureDOFs, INSERT_VALUES, Include_Ghost_Values);

  int Z_Node_Count = 0;
  int Number_Of_Layer_Segments;

  VecSet ( Residual, Zero );

  Element_BCs [ Gamma_2 ] = Interior_Boundary;
  Element_BCs [ Gamma_3 ] = Interior_Boundary;
  Element_BCs [ Gamma_4 ] = Interior_Boundary;
  Element_BCs [ Gamma_5 ] = Interior_Boundary;

  elementContributionsTime = 0.0;

  for ( FEM_Layers.Initialise_Iterator (); ! FEM_Layers.Iteration_Is_Done (); FEM_Layers++ ) {
    Current_Layer = FEM_Layers.Current_Layer ();

    Include_Chemical_Compaction = (( cauldron->Do_Chemical_Compaction ) && ( Current_Layer -> Get_Chemical_Compaction_Mode ()));

    Current_Layer->getConstrainedOverpressure ( currentTime,
                                                Constrained_Po_Value,
                                                Overpressure_Is_Constrained );

    Number_Of_Layer_Segments = Current_Layer->getNrOfActiveElements();
    GlobalK = new int [ Number_Of_Layer_Segments + 1 ];

    for ( K = 0; K <= Number_Of_Layer_Segments; K++ ){
      GlobalK [ K ] = Z_Node_Count;
      Z_Node_Count = Z_Node_Count + 1;
    } 

    Z_Node_Count = Z_Node_Count - 1;

    Current_Layer->Current_Properties.Activate_Properties  ( INSERT_VALUES, Include_Ghost_Values );
    Current_Layer->Previous_Properties.Activate_Properties ( INSERT_VALUES, Include_Ghost_Values );

    DMDAGetInfo(Current_Layer->layerDA,0,&layerXCount,&layerYCount,&layerZCount,0,0,0,0,0,0,0);
    DMDAGetCorners ( Current_Layer->layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

    for ( K = zStart; K < zStart + zCount; K++ ) {

      if ( K != layerZCount - 1 ) {
        FEM_K_Index = GlobalK [ K ];

        for ( Element_Index = 0; Element_Index < cauldron->mapElementList.size (); Element_Index++ ) {
          Exceeded_Fracture_Pressure.fill ( 0.0 );
//            Exceeded_Fracture_Pressure.fill ( false );

          Element_Count = Element_Count + 1;
          Degenerate_Segments = 0;

          Element_Residual.zero ();

          if ( cauldron->mapElementList [ Element_Index ].exists ) {
            //
            //
            // Copy segment lithologies
            //
            I_Position = cauldron->mapElementList [ Element_Index ].i [ 0 ];
            J_Position = cauldron->mapElementList [ Element_Index ].j [ 0 ];

            Element_Lithology = Current_Layer->getLithology ( I_Position, J_Position );
            Element_BCs [ Gamma_6 ] = Interior_Boundary;

            degenerateElement = true;

            for ( Inode = 0; Inode < 4; Inode++ ) {
              I_Position = cauldron->mapElementList [ Element_Index ].i [ Inode ];
              J_Position = cauldron->mapElementList [ Element_Index ].j [ Inode ];


              Current_Element_Solid_Thickness  ( Inode + 1 ) = Current_Layer -> Current_Properties  ( Basin_Modelling::Solid_Thickness, K, J_Position, I_Position );
              Previous_Element_Solid_Thickness ( Inode + 1 ) = Current_Layer -> Previous_Properties ( Basin_Modelling::Solid_Thickness, K, J_Position, I_Position );

              if ( Current_Element_Solid_Thickness ( Inode + 1 ) == IBSNULLVALUE ) {
                Current_Element_Solid_Thickness ( Inode + 1 ) = 0.0;
              }

              if ( Previous_Element_Solid_Thickness ( Inode + 1 ) == IBSNULLVALUE ) {
                Previous_Element_Solid_Thickness ( Inode + 1 ) = 0.0;
              }

              degenerateElement = degenerateElement && ( Current_Element_Solid_Thickness ( Inode + 1 ) == 0.0 );
            } 

            if ( degenerateElement || Element_Lithology->surfacePorosity () == 0.0 ) {
              continue;
            }

            for (Inode = 0; Inode<8; Inode++) {
              int LidxZ = K + (Inode<4 ? 1 : 0);
              int GidxZ = FEM_K_Index + (Inode<4 ? 1 : 0);
              int GidxY = cauldron->mapElementList [ Element_Index ].j[Inode%4];
              int GidxX = cauldron->mapElementList [ Element_Index ].i[Inode%4];

              localI = Integer_Min ( Integer_Max ( GidxX, xStart ), xStart + xCount - 1 ) - xStart;
              localJ = Integer_Min ( Integer_Max ( GidxY, yStart ), yStart + yCount - 1 ) - yStart;

              Dirichlet_Boundary_Values ( Inode + 1 )= 0.0;
              Positions.Set_Node_Position ( Inode, LidxZ, GidxY, GidxX );
              Exceeded_Fracture_Pressure ( Inode + 1 ) = Current_Layer -> fracturedPermeabilityScaling ( GidxX, GidxY, LidxZ );

              if ( globalZCount-1 == int(dof(GidxZ,GidxY,GidxX))) {
                //
                // A surface boundary node is any node having the same DOF number as the surface boundary node.
                //
                Nodal_BCs[Inode] = Surface_Boundary; 
                Dirichlet_Boundary_Values ( Inode + 1 ) = Surface_Pressure_Value;

              } else if ( Overpressure_Is_Constrained ) {

                Nodal_BCs[Inode] = Interior_Constrained_Overpressure; 
                Dirichlet_Boundary_Values ( Inode + 1 ) = Constrained_Po_Value;

              } else if ( K == zCount - 2 and
                          Exceeded_Fracture_Pressure ( Inode + 1 ) > 0.0 and
                          Previous_Layer != NULL and
                          ( Previous_Layer->getLithology ( I_Position, J_Position ) -> surfacePorosity () < 0.03 )) {

                Nodal_BCs[Inode] = Interior_Constrained_Overpressure; 

                Dirichlet_Boundary_Values ( Inode + 1 ) = HydraulicFracturingManager::getInstance ().fracturePressure ( Element_Lithology,
                                                                                                                        Current_Layer->fluid,
                                                                                                                        FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( GidxX, GidxY, currentTime ),
                                                                                                                        FastcauldronSimulator::getInstance ().getSeaBottomDepth ( GidxX, GidxY, currentTime ),
                                                                                                                        Current_Layer->Previous_Properties ( Basin_Modelling::Depth, LidxZ, GidxY, GidxX ),
                                                                                                                        Current_Layer->Previous_Properties ( Basin_Modelling::Hydrostatic_Pressure, LidxZ, GidxY, GidxX ),
                                                                                                                        Current_Layer->Previous_Properties ( Basin_Modelling::Lithostatic_Pressure, LidxZ, GidxY, GidxX ))
                                                                                                                        - Current_Layer->Previous_Properties ( Basin_Modelling::Hydrostatic_Pressure, LidxZ, GidxY, GidxX );
                Dirichlet_Boundary_Values ( Inode + 1 ) = NumericFunctions::Maximum ( Dirichlet_Boundary_Values ( Inode + 1 ), 0.0 );


                // fracturePressure = HydraulicFracturingManager::getInstance ().fracturePressure ( Element_Lithology,
                //                                                                                  Current_Layer->fluid,
                //                                                                                  FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( GidxX, GidxY, currentTime ),
                //                                                                                  FastcauldronSimulator::getInstance ().getSeaBottomDepth ( GidxX, GidxY, currentTime ),
                //                                                                                  Current_Layer->Previous_Properties ( Basin_Modelling::Depth, LidxZ, GidxY, GidxX ),
                //                                                                                  Current_Layer->Previous_Properties ( Basin_Modelling::Hydrostatic_Pressure, LidxZ, GidxY, GidxX ),
                //                                                                                  Current_Layer->Previous_Properties ( Basin_Modelling::Lithostatic_Pressure, LidxZ, GidxY, GidxX ));
                // Dirichlet_Boundary_Values ( Inode + 1 ) = NumericFunctions::Maximum ( fracturePressure - Current_Layer->Current_Properties ( Basin_Modelling::Hydrostatic_Pressure, LidxZ, GidxY, GidxX ), 0.0 );

              } else if ( HydraulicFracturingManager::getInstance ().isNonConservativeFractureModel () and Current_Layer->nodeIsTemporarilyDirichlet ( GidxX, GidxY, LidxZ )) {

                fracturePressure = HydraulicFracturingManager::getInstance ().fracturePressure ( Element_Lithology,
                                                                                                 Current_Layer->fluid,
                                                                                                 FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( GidxX, GidxY, currentTime ),
                                                                                                 FastcauldronSimulator::getInstance ().getSeaBottomDepth ( GidxX, GidxY, currentTime ),
                                                                                                 Current_Layer->Current_Properties ( Basin_Modelling::Depth, LidxZ, GidxY, GidxX ),
                                                                                                 Current_Layer->Current_Properties ( Basin_Modelling::Hydrostatic_Pressure, LidxZ, GidxY, GidxX ),
                                                                                                 Current_Layer->Current_Properties ( Basin_Modelling::Lithostatic_Pressure, LidxZ, GidxY, GidxX ));

                Nodal_BCs[Inode] = Interior_Constrained_Overpressure; 
                Dirichlet_Boundary_Values ( Inode + 1 ) = NumericFunctions::Maximum ( fracturePressure - Current_Layer->Current_Properties ( Basin_Modelling::Hydrostatic_Pressure, LidxZ, GidxY, GidxX ), 0.0 );

              } else {
                Nodal_BCs[Inode] = Interior_Boundary;
              }

              col[Inode].i = GidxX;
              col[Inode].j = GidxY;
              col[Inode].k = GidxZ;

              row[Inode].i = GidxX;
              row[Inode].j = GidxY;
              row[Inode].k = GidxZ;

              if (GidxZ != int ( dof(GidxZ,GidxY,GidxX ))) {
                Degenerate_Segments = Degenerate_Segments + 1;
                NumCoincidentNodes = max(NumCoincidentNodes,((int)dof(GidxZ,GidxY,GidxX)-GidxZ));

                col[Inode].k = (int) dof(GidxZ,GidxY,GidxX);
                row[Inode].k = col[Inode].k;
              }

              Geometry_Matrix ( 1, Inode + 1 ) = Origin_X + (Delta_X * GidxX);
              Geometry_Matrix ( 2, Inode + 1 ) = Origin_Y + (Delta_Y * GidxY);
              Geometry_Matrix ( 3, Inode + 1 ) = Current_Layer->Current_Properties ( Basin_Modelling::Depth, LidxZ, GidxY, GidxX );

              Previous_Geometry_Matrix ( 1, Inode + 1 ) = Origin_X + (Delta_X * GidxX);
              Previous_Geometry_Matrix ( 2, Inode + 1 ) = Origin_Y + (Delta_Y * GidxY);
              Previous_Geometry_Matrix ( 3, Inode + 1 ) = Current_Layer->Previous_Properties ( Basin_Modelling::Depth, LidxZ, GidxY, GidxX );
            }

            Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Hydrostatic_Pressure, Positions, Current_Ph );
            Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Lithostatic_Pressure, Positions, Current_Pl );
            Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Overpressure,         Positions, Current_Po );
            Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::VES_FP,               Positions, Current_Element_VES );
            Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Max_VES,              Positions, Current_Element_Max_VES );
            Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Temperature,          Positions, Current_Element_Temperature );
            Current_Layer->Current_Properties.Extract_Property ( Basin_Modelling::Chemical_Compaction,  Positions, Current_Chemical_Compaction );

            Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::Hydrostatic_Pressure, Positions, Previous_Ph );
            Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::Overpressure,         Positions, Previous_Po );
            Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::VES_FP,               Positions, Previous_Element_VES );
            Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::Max_VES,              Positions, Previous_Element_Max_VES );
            Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::Temperature,          Positions, Previous_Element_Temperature );
            Current_Layer->Previous_Properties.Extract_Property ( Basin_Modelling::Chemical_Compaction,  Positions, Previous_Chemical_Compaction );

            PetscTime(&Element_Start_Time);

            Assemble_Element_Pressure_Residual ( Plane_Quadrature_Degree,
                                                 Depth_Quadrature_Degree,
                                                 currentTime,
                                                 previousTime - currentTime,
                                                 Element_BCs,
                                                 Nodal_BCs,
                                                 Dirichlet_Boundary_Values,
                                                 Element_Lithology,
                                                 Current_Layer->fluid,
                                                 Include_Chemical_Compaction,
                                                 HydraulicFracturingManager::getInstance ().getModel (),
                                                 Previous_Geometry_Matrix,
                                                 Geometry_Matrix,
                                                 Previous_Element_Solid_Thickness,
                                                 Current_Element_Solid_Thickness,
                                                 Previous_Ph,
                                                 Current_Ph,
                                                 Previous_Po,
                                                 Current_Po,
                                                 Current_Pl,
                                                 Previous_Element_VES,
                                                 Current_Element_VES,
                                                 Previous_Element_Max_VES,
                                                 Current_Element_Max_VES,
                                                 Previous_Element_Temperature,
                                                 Current_Element_Temperature,
                                                 Previous_Chemical_Compaction,
                                                 Current_Chemical_Compaction,
                                                 Exceeded_Fracture_Pressure,
                                                 Element_Residual );


            PetscTime(&Element_End_Time);
            elementContributionsTime = elementContributionsTime + Element_End_Time - Element_Start_Time;

            for (Inode = 0; Inode<8; Inode++) {
              int irow = row[Inode].i;
              int jrow = row[Inode].j;
              int krow = row[Inode].k;

              if ( irow >= 0 && jrow >= 0 && krow >= 0 ) {
                residualVector(krow,jrow,irow) = residualVector(krow,jrow,irow) - Element_Residual ( Inode + 1 );
              }

            }

          }

        }

      }

    }

    Current_Layer->Current_Properties.Restore_Properties ();
    Current_Layer->Previous_Properties.Restore_Properties ();

    delete[] GlobalK;

  }

  residualVector.Restore_Global_Array ( Update_Including_Ghosts );  
  PetscTime(&End_Time);

}
#endif

//------------------------------------------------------------//


PetscScalar PressureSolver::maximumPressureDifference () {

  using namespace Basin_Modelling;

  int xStart;
  int yStart;
  int zStart;

  int xCount;
  int yCount;
  int zCount;

  Vec Current_Overpressure;
  Vec Previous_Overpressure;
  Vec Pressure_Difference;

  Layer_Iterator Layers;
  LayerProps_Ptr Current_Layer;

  PetscScalar Maximum_Difference = -1.0e10;
  PetscScalar Maximum_Layer_Difference;

  Layers.Initialise_Iterator ( cauldron->layers, Descending, Sediments_Only, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done ()) {
    Current_Layer = Layers.Current_Layer ();
    //
    //
    // Get the size of the layer DA.
    //
    DMDAGetCorners ( Current_Layer->layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

    Current_Overpressure  = Current_Layer->Current_Properties  ( Basin_Modelling::Overpressure );
    Previous_Overpressure = Current_Layer->Previous_Properties ( Basin_Modelling::Overpressure );

    DMCreateGlobalVector ( Current_Layer->layerDA, & Pressure_Difference );

    // Check this!!
    // VecWAXPY(&NegOne, Previous_Overpressure, Current_Overpressure, Pressure_Difference );
    VecWAXPY( Pressure_Difference, NegOne, Previous_Overpressure, Current_Overpressure );
    VecAbs( Pressure_Difference );
    VecMax( Pressure_Difference,PETSC_NULL,&Maximum_Layer_Difference );
    Destroy_Petsc_Vector( Pressure_Difference );

    Maximum_Difference = PetscMax ( Maximum_Difference, Maximum_Layer_Difference );

    Layers++;
  }

  return Maximum_Difference;
}

//------------------------------------------------------------//


PetscScalar PressureSolver::maximumPressureDifference2 () {

  using namespace Basin_Modelling;

  int xStart;
  int yStart;
  int zStart;

  int xCount;
  int yCount;
  int zCount;

  int I;
  int J;
  int K;

  int maxI;
  int maxJ;
  int maxK;

  int iMin;
  int iMax;
  int jMin;
  int jMax;

  Layer_Iterator Layers;
  LayerProps_Ptr currentLayer;
  LayerProps_Ptr layerAbove;
  LayerProps_Ptr layerBelow;

  PetscScalar pressureDifference;
  PetscScalar maximumLayerPressureDifference;
  PetscScalar localMaximumPressureDifference = -1.0;
  PetscScalar globalMaximumPressureDifference;

  Layers.Initialise_Iterator ( cauldron->layers, Descending, Sediments_Only, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done ()) {
    currentLayer = Layers.Current_Layer ();
    maximumLayerPressureDifference = -1.0;

    maxI = -1;
    maxJ = -1;
    maxK = -1;


    if ( Layers.Layer_Above () == 0 ) {
      layerAbove = currentLayer;
    } else {
      layerAbove = Layers.Layer_Above ();
    }

    if ( Layers.Layer_Below () == 0 ) {
      layerBelow = currentLayer;
    } else {
      layerBelow = Layers.Layer_Below ();
    }

    //
    // Get the size of the layer DA.
    //
    DMDAGetCorners ( currentLayer->layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

    PETSC_3D_Array currentPressure  ( currentLayer->layerDA, currentLayer->Current_Properties  ( Basin_Modelling::Overpressure ));
    PETSC_3D_Array previousPressure ( currentLayer->layerDA, currentLayer->Previous_Properties ( Basin_Modelling::Overpressure ));

    for ( I = xStart; I < xStart + xCount; I++ ) {

      if ( I == xStart ) {
        iMin = xStart;
      } else {
        iMin = I - 1;
      }

      if ( I == xStart + xCount - 1 ) {
        iMax = xStart + xCount - 1;
      } else {
        iMax = I + 1;
      }

      for ( J = yStart; J < yStart + yCount; J++ ) {

        if ( J == yStart ) {
          jMin = yStart;
        } else {
          jMin = J - 1;
        }

        if ( J == yStart + yCount - 1 ) {
          jMax = yStart + yCount - 1;
        } else {
          jMax = J + 1;
        }

        if ( cauldron->nodeIsDefined ( I, J ) && ! currentLayer->getLithologyHasSwitched ( I, J )) {

          for ( K = zStart; K < zStart + zCount; K++ ) {

            pressureDifference = fabs ( currentPressure ( K, J, I ) - previousPressure ( K, J, I ));

            if ( pressureDifference > maximumLayerPressureDifference ) {
              maxI = I;
              maxJ = J;
              maxK = K;
              maximumLayerPressureDifference = pressureDifference;
            }

          }

        }

      }

    }

    localMaximumPressureDifference = NumericFunctions::Maximum ( localMaximumPressureDifference, maximumLayerPressureDifference );

    if ( cauldron->debug2 ) {
      PetscSynchronizedPrintf ( PETSC_COMM_WORLD, " Maximum difference for layer: %d  %s   %d %d %d   %3.4f %3.4f %3.4f \n", 
                                FastcauldronSimulator::getInstance ().getRank (),
                                currentLayer -> layername.c_str (),
                                maxI,  maxJ,  maxK, 
                                maximumLayerPressureDifference,
                                currentPressure ( maxK, maxJ, maxI ),
                                previousPressure ( maxK, maxJ, maxI ));

      PetscSynchronizedFlush ( PETSC_COMM_WORLD );
    }

    Layers++;
  }

  // Now find the maximum pressure difference of all processes
  MPI_Allreduce( &localMaximumPressureDifference, &globalMaximumPressureDifference, 1, 
                 MPI_DOUBLE, MPI_MAX, PETSC_COMM_WORLD );


  return globalMaximumPressureDifference;
}

//------------------------------------------------------------//

void PressureSolver::setBasementDepths ( const double           Current_Time,
                                               Double_Array_2D& Depth_Above,
                                         const Boolean2DArray&  Valid_Needle ) {

  // As the depth values in the basement are computed before the overpressure is calculated they 
  // will probably be incorrect. Therefore, they must be set to the correct value.
  //
  // Should only be called for iteratively coupled problems.

  const double presentDayMantleThickness = FastcauldronSimulator::getInstance ().getMantleFormation ()->getPresentDayThickness ();

  int X_Start;
  int Y_Start;
  int Z_Start;
  int X_Count;
  int Y_Count;
  int Z_Count;
  int Z_Top;

  int I;
  int J;
  int K;

  double BasementThickness;
  double CrustThinningRatio;
  double MantleThickness;
  double Mantle_Segment_Height;
  double Max_Depth;
  double currentCrustThickness;

  double mantleMaximumElementThickness = FastcauldronSimulator::getInstance ().getRunParameters ()->getBrickHeightMantle ();

  DMDAGetCorners ( *cauldron->mapDA, &X_Start, &Y_Start, PETSC_NULL, &X_Count, &Y_Count, PETSC_NULL );

  LayerProps_Ptr Crust_Layer  = cauldron->Crust ();
  LayerProps_Ptr Mantle_Layer = cauldron->Mantle ();

  PETSC_3D_Array Crust_Depth     ( Crust_Layer->layerDA, Crust_Layer->Current_Properties ( Basin_Modelling::Depth ));
  PETSC_3D_Array Mantle_Depth    ( Mantle_Layer->layerDA, Mantle_Layer->Current_Properties ( Basin_Modelling::Depth ));

//   PETSC_2D_Array maxCrustThickns ( *cauldron->mapDA, cauldron->Maximum_Crust_Thickness );

  // Get the size of the layer DA.
  DMDAGetCorners ( Crust_Layer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );
  Z_Top = Z_Start + Z_Count - 1;

  Basin_Modelling::initialiseTopNodes ( X_Start, X_Count, Y_Start, Y_Count, Z_Top, Valid_Needle, Depth_Above, Crust_Depth );

  // Compute CRUST depths
  for ( I = X_Start; I < X_Start + X_Count; I++ ) {

    for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

      if ( Valid_Needle ( I, J )) {

        for ( K = Z_Start + Z_Count - 2; K >= 0; K-- ) {
          Crust_Depth ( K, J, I ) = Crust_Depth ( K + 1, J, I ) + Crust_Layer->getSolidThickness ( I, J, K, Current_Time );
        }

      } else {
 
        for ( K = Z_Start + Z_Count - 2; K >= 0; K-- ) {
          Crust_Depth ( K, J, I ) = CAULDRONIBSNULLVALUE;
        }

      }

    }

  }

  Basin_Modelling::copyBottomNodes ( X_Start, X_Count, Y_Start, Y_Count, Valid_Needle, Depth_Above, Crust_Depth );

  DMDAGetCorners ( Mantle_Layer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );
  Z_Top = Z_Start + Z_Count - 1;

  Basin_Modelling::initialiseTopNodes ( X_Start, X_Count, Y_Start, Y_Count, Z_Top, Valid_Needle, Depth_Above, Mantle_Depth );

  // Compute MANTLE depths
  for ( I = X_Start; I < X_Start + X_Count; I++ ) {

    for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

      if ( Valid_Needle ( I, J )) {
            currentCrustThickness = FastcauldronSimulator::getInstance ().getCrustThickness ( I, J, Current_Time );            
            CrustThinningRatio = cauldron->Crust ()->getCrustMaximumThickness ( I, J ) / currentCrustThickness;

         if( cauldron->isALC() ) {
            BasementThickness = cauldron->Mantle()->getInitialLithosphericMantleThickness () + cauldron->Crust()->getInitialCrustalThickness();
            Mantle_Segment_Height = cauldron->Mantle()->m_mantleElementHeight0 / CrustThinningRatio;
         } else {
            BasementThickness = presentDayMantleThickness + FastcauldronSimulator::getInstance ().getCrustThickness ( I, J, 0.0 );
            Mantle_Segment_Height = mantleMaximumElementThickness / CrustThinningRatio;
         }
         MantleThickness = BasementThickness - currentCrustThickness;
         Max_Depth = Mantle_Depth ( Z_Top, J, I ) + MantleThickness;

         for ( K = Z_Top - 1; K >= 0; K-- ) {
            Mantle_Depth ( K, J, I ) = PetscMin ( Max_Depth, Mantle_Depth ( K + 1, J, I ) + Mantle_Segment_Height );
         }

      } else {
 
        for ( K = Z_Top - 1; K >= 0; K-- ) {
          Mantle_Depth ( K, J, I ) = CAULDRONIBSNULLVALUE;
        }

      }

    }  

  }  

}


//------------------------------------------------------------//

int PressureSolver::getPlaneQuadratureDegree ( const int optimisationLevel ) const {

  return PlaneQuadratureDegrees [ optimisationLevel - 1 ];
}

//------------------------------------------------------------//

int PressureSolver::getDepthQuadratureDegree ( const int optimisationLevel ) const {

  return DepthQuadratureDegrees [ optimisationLevel - 1 ];
}

//------------------------------------------------------------//

void PressureSolver::setPlaneQuadratureDegree ( const int optimisationLevel,
                                                const int newDegree ) {

  PlaneQuadratureDegrees [ optimisationLevel - 1 ] = newDegree;

}

//------------------------------------------------------------//

void PressureSolver::setDepthQuadratureDegree ( const int optimisationLevel,
                                                const int newDegree ) {
  
  DepthQuadratureDegrees [ optimisationLevel - 1 ] = newDegree;

}

//------------------------------------------------------------//

void PressureSolver::setMaximumNumberOfNonlinearIterations ( const int optimisationLevel,
                                                             const int maximumIterationCount ) {
  s_numberOfIterations [ optimisationLevel - 1 ] = maximumIterationCount;
}

//------------------------------------------------------------//

int PressureSolver::getMaximumNumberOfNonlinearIterations ( const int optimisationLevel ) {
  return s_numberOfIterations [ optimisationLevel - 1 ];
}

//------------------------------------------------------------//

double PressureSolver::getNewtonSolverTolerance ( const int  optimisationLevel,
                                                  const bool isGeometricLoop,
                                                  const int  geometricLoopNumber ) {

  int overpressureRun;

  if ( isGeometricLoop and not FastcauldronSimulator::getInstance ().getCauldron ()->readFCTCorrectionFactor ) {
    overpressureRun = int ( PetscMin ( geometricLoopNumber, 3 ));
  } else {
    overpressureRun = 3;
  } 

  return NewtonSolverTolerances [ optimisationLevel - 1 ][ overpressureRun - 1 ];

}

//------------------------------------------------------------//

void PressureSolver::setNewtonSolverTolerance ( const int    optimisationLevel,
                                                const double newTolerance ) {
  
  NewtonSolverTolerances [ optimisationLevel - 1 ][ 0 ] = newTolerance;
  NewtonSolverTolerances [ optimisationLevel - 1 ][ 1 ] = newTolerance;
  NewtonSolverTolerances [ optimisationLevel - 1 ][ 2 ] = newTolerance;

}

//------------------------------------------------------------//

double PressureSolver::linearSolverTolerance ( const int optimisationLevel ) const {

  static const double LinearSolverTolerances [ NumberOfOptimisationLevels ] = { 1.0e-5, 1.0e-5, 1.0e-5, 1.0e-6, 1.0e-7 };

  return LinearSolverTolerances [ optimisationLevel - 1 ];

}

//------------------------------------------------------------//

double PressureSolver::getRelativeThicknessTolerance ( const int optimisationLevel ) const {

  static const double ThicknessTolerances [ NumberOfOptimisationLevels ] = { 0.05, 0.03, 0.01, 0.01, 0.01 };

  return ThicknessTolerances [ optimisationLevel - 1 ];

}

//------------------------------------------------------------//

double PressureSolver::getAbsoluteThicknessTolerance ( const int optimisationLevel ) const {

  static const double ThicknessTolerances [ NumberOfOptimisationLevels ] = { 5.0, 3.0, 1.0, 1.0, 1.0 };

  return ThicknessTolerances [ optimisationLevel - 1 ];

}

//------------------------------------------------------------//

int PressureSolver::numberOfStepsForCompactionEquation ( const int optimisationLevel ) const {

  static const int NumberOfSteps [ NumberOfOptimisationLevels ] = { 3, 3, 4, 4, 5 };

  return NumberOfSteps [ optimisationLevel - 1 ];

}

//------------------------------------------------------------//

int PressureSolver::getIterationsForIluFillLevelIncrease ( const int optimisationLevel,
                                                           const int currentIluFillLevel ) {

   return s_iterationsForiluFillLevelIncrease [ optimisationLevel ];

} 

//------------------------------------------------------------//

void PressureSolver::setIterationsForIluFillLevelIncrease ( const int newIluFillLevelIterations ) {

   int i;

   // Since the optimisation-level is not known at this time, 
   // set all of the values to the new value.
   for ( i = 0; i < NumberOfOptimisationLevels; ++i ) {
      s_iterationsForiluFillLevelIncrease [ i ]= newIluFillLevelIterations;
   }

}

//------------------------------------------------------------//
