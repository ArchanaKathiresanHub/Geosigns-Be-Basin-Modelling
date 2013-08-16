#include "temperature_solver.h"

#include "layer_iterators.h"
#include "fem_grid.h"

#include "utils.h"

#include "globaldefs.h"
#include "element_contributions.h"
#include "FissionTrackCalculator.h"
#include "CrustFormation.h"
#include "MantleFormation.h"

//Data access library
#include "Interface/GridMap.h"

#include "PetscLogStages.h"

//------------------------------------------------------------//

int Temperature_Solver::PlaneQuadratureDegrees [ NumberOfOptimisationLevels ] = { 2, 2, 2, 2, 3 };

int Temperature_Solver::DepthQuadratureDegrees [ NumberOfOptimisationLevels ] = { 2, 2, 2, 3, 4 };

//------------------------------------------------------------//


Temperature_Solver::Temperature_Solver( AppCtx* Application_Context ) 
  : 
Vitrinite_Calculator ( Application_Context ), 
m_SmectiteIlliteCalculator( Application_Context ),
m_BiomarkersCalculator(Application_Context),
m_FissionTrackCalculator(Application_Context)
{

  Basin_Model  = Application_Context;

  if ( FastcauldronSimulator::getInstance ().getBottomBoundaryConditions () == MANTLE_HEAT_FLOW ) {
    DMCreateGlobalVector( *Basin_Model -> mapDA, &Mantle_Heat_Flow );
  }
 
}


//------------------------------------------------------------//


Temperature_Solver::~Temperature_Solver() 
{

  if ( FastcauldronSimulator::getInstance ().getBottomBoundaryConditions () == MANTLE_HEAT_FLOW ) {
    Destroy_Petsc_Vector ( Mantle_Heat_Flow );
  }

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


           Crust_Layer_Heat_Production_Array ( K, J, I ) = Crust_Heat_Production_Array ( K, J, I ) * GeoPhysics::MicroWattsToWatts;
      }

    }

  }

}



//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::Compute_Heat_Flow_Boundary_Conditions"

void Temperature_Solver::Compute_Heat_Flow_Boundary_Conditions ( const double Current_Time ) {


  if ( FastcauldronSimulator::getInstance ().getBottomBoundaryConditions () != MANTLE_HEAT_FLOW) {
    return;
  }

  using namespace Basin_Modelling;

  int X_Start;
  int Y_Start;
  int Z_Start;
  int X_Count;
  int Y_Count;
  int Z_Count;
  int I;
  int J;

  LayerProps_Ptr mantleLayer  = Basin_Model->Mantle ();

  // Calculate Mantle heat flow.
  DMDAGetCorners( mantleLayer -> layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );

  PETSC_2D_Array MHF ( *Basin_Model -> mapDA, Mantle_Heat_Flow );

  for ( I = X_Start; I < X_Start + X_Count; I++ ) {

    for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

      if ( Basin_Model->nodeIsDefined ( I, J )) {
        MHF ( J, I ) = FastcauldronSimulator::getInstance ().getMantleHeatFlow ( I, J, Current_Time );
      }

    }

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

  PETSC_3D_Array Mantle_Depth( Mantle_Layer -> layerDA, 
			       Mantle_Layer -> Current_Properties ( Basin_Modelling::Depth ) );
  PETSC_3D_Array Mantle_Temperature( Mantle_Layer -> layerDA, 
				     Mantle_Layer -> Current_Properties ( Basin_Modelling::Temperature ) );


  for ( I = X_Start; I < X_Start + X_Count; I++ ) {

    for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

      if ( not Basin_Model->nodeIsDefined ( I, J )) continue;

      Polyfunction Temperature_Polyfunction;

      Sea_Bottom_Temperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( I, J, Age_Of_Basin ),

      Temperature_Polyfunction.AddPoint( Crust_Depth( Crust_Z_Nodes - 1,J,I ), Sea_Bottom_Temperature );
      Temperature_Polyfunction.AddPoint( Mantle_Depth( 0,J,I ), Top_Asthenospheric_Temperature );

      for ( K = 0; K < Crust_Z_Nodes; K++ ) {
	Crust_Temperature( K,J,I ) = Temperature_Polyfunction.F( Crust_Depth( K,J,I ) );
      }

      for ( K = 0; K < Mantle_Z_Nodes; K++ ) {
	Mantle_Temperature( K,J,I ) = Temperature_Polyfunction.F( Mantle_Depth( K,J,I ) );
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
#define __FUNCT__ "Temperature_Solver::Assemble_System"

void Temperature_Solver::Assemble_System ( const double  Previous_Time,
                                           const double  Current_Time,
                                           const DM&     Temperature_FEM_Grid,
                                           const Vec&    Temperature_DOFs,
                                                 Mat&    Jacobian,
                                                 Vec&    Residual,
                                                 double& Element_Contributions_Time ) {

  using namespace Basin_Modelling;

  PetscLogStages::push( PetscLogStages::TEMPERATURE_INITIALISATION_SYSTEM_ASSEMBLY);

  const double Time_Step = Previous_Time - Current_Time;

  const int Plane_Quadrature_Degree = getPlaneQuadratureDegree ( Basin_Model -> Optimisation_Level );
  const int Depth_Quadrature_Degree = getDepthQuadratureDegree ( Basin_Model -> Optimisation_Level );

  const bool SteadyStateCalculation = ( Previous_Time == Current_Time );

  const bool IncludeAdvectiveTerm = Basin_Model->includeAdvectiveTerm and 
                                    not SteadyStateCalculation and
                                   ( Basin_Model->Do_Iteratively_Coupled or
                                     Basin_Model->IsCalculationCoupled );

  const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

  double Delta_X  = grid.deltaI;
  double Delta_Y  = grid.deltaJ;
  double Origin_X = grid.originI;
  double Origin_Y = grid.originJ;

  PetscLogDouble Element_Start_Time;
  PetscLogDouble Element_End_Time;

  int kkk;
  int Layer_K;
  int xs, ys, zs, xm, ym, zm;
  int layerMx, layerMy, layerMz;
  int gridMx, gridMy, gridMz;
  int k;
  int NumElement = 0;
  int NumCoincidentNodes = 0;
			    
  bool IncludeGhosts = true;

  MatStencil   col[8],row[8];
  MatStencil   a_diag[1];
  PetscScalar  OneDiagonal[1] = {1.0};
//    PetscScalar  EltRHS[8];
  int          *GlobalK;

  int    CollapsedNodes = 0;


  DMDAGetInfo( Temperature_FEM_Grid,0,&gridMx,&gridMy,&gridMz,0,0,0,0,0,0,0,0,0);

  PETSC_2D_Array MantleHeatFlow;

  if ( FastcauldronSimulator::getInstance ().getBottomBoundaryConditions () == MANTLE_HEAT_FLOW ) {

    MantleHeatFlow.Set_Global_Array ( *Basin_Model -> mapDA, Mantle_Heat_Flow,
				      INSERT_VALUES,IncludeGhosts );

  }


  PETSC_3D_Array rhsF( Temperature_FEM_Grid, Residual, INSERT_VALUES, IncludeGhosts);

  PETSC_3D_Array dof( Temperature_FEM_Grid, Temperature_DOFs, INSERT_VALUES, IncludeGhosts);

  ElementGeometryMatrix Previous_Geometry_Matrix;
  ElementGeometryMatrix Geometry_Matrix;

  ElementVector Previous_Ph;
  ElementVector Current_Ph;

  ElementVector Previous_Po;
  ElementVector Current_Po;
  ElementVector Current_Lp;

  ElementVector Previous_VES;
  ElementVector Current_VES;

  ElementVector Previous_Max_VES;
  ElementVector Current_Max_VES;

  ElementVector Previous_Temperature;
  ElementVector Current_Temperature;

  ElementVector Current_Chemical_Compaction;
  ElementVector Previous_Chemical_Compaction;

  ElementVector Element_Heat_Production;

  ElementVector BC_Values;
  Boundary_Conditions Nodal_BCs[8];

  ElementMatrix Element_Jacobian;
  ElementVector Element_Residual;
  const CompoundLithology*  Element_Lithology;

  int NSegmZ;

  bool Is_Steady_State = false; // remove this soon
  bool Include_Chemical_Compaction;

  const double Constrained_Temp_Value = FastcauldronSimulator::getInstance ().getConstrainedBasaltTemperature();

  int NodeZDirCount = 0;

  Layer_Iterator Layers;
  Layers.Initialise_Iterator ( Basin_Model -> layers, Ascending, Basement_And_Sediments, 
			       Active_Layers_Only );

  Element_Contributions_Time = 0.0;

  int globalXNodes;
  int globalYNodes;
  if( Basin_Model -> isALC() ) {
     CrustFormation*  crustLayer = dynamic_cast<CrustFormation*>(Basin_Model -> Crust ());
     MantleFormation*  mantleLayer = dynamic_cast<MantleFormation*>(Basin_Model -> Mantle ());
     crustLayer->cleanVectors();
     mantleLayer->cleanVectors();
     DMDAGetInfo( *Basin_Model -> mapDA, PETSC_NULL, &globalXNodes, &globalYNodes,
                  PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, 
                  PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL );
  }
  while ( ! Layers.Iteration_Is_Done () ) {

    LayerProps_Ptr Current_Layer = Layers.Current_Layer ();

    Current_Layer -> Previous_Properties.Activate_Property ( Basin_Modelling::Depth, INSERT_VALUES, IncludeGhosts );

    Include_Chemical_Compaction = (( Basin_Model -> Do_Chemical_Compaction ) && ( Current_Layer -> Get_Chemical_Compaction_Mode ()));

    NSegmZ = Current_Layer -> getNrOfActiveElements();
    
    string EventName = "setUpGM" + Current_Layer -> layername;

    GlobalK = new int[NSegmZ+1];

    for (kkk = 0; kkk <=NSegmZ; kkk++){
      GlobalK[kkk] = NodeZDirCount;
      NodeZDirCount++;
    }

    NodeZDirCount--;

    PETSC_3D_Array temp  ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Temperature ), INSERT_VALUES, IncludeGhosts );
    PETSC_3D_Array depth ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Depth       ), INSERT_VALUES, IncludeGhosts );

    PETSC_3D_Array bulkHeatProd( Current_Layer -> layerDA, 
				 Current_Layer -> BulkHeatProd,
				 INSERT_VALUES, IncludeGhosts );

    PETSC_3D_Array oldtemp (Current_Layer -> layerDA, 
				 Current_Layer -> Previous_Properties ( Basin_Modelling::Temperature ),
				 INSERT_VALUES, IncludeGhosts);


    PETSC_3D_Array Layer_Previous_Ph ( Current_Layer -> layerDA, 
		      Current_Layer -> Previous_Properties ( Basin_Modelling::Hydrostatic_Pressure ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Previous_Po ( Current_Layer -> layerDA, 
		      Current_Layer -> Previous_Properties ( Basin_Modelling::Overpressure ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Previous_VES ( Current_Layer -> layerDA, 
		      Current_Layer -> Previous_Properties ( Basin_Modelling::VES_FP ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Previous_Max_VES ( Current_Layer -> layerDA, 
		      Current_Layer -> Previous_Properties ( Basin_Modelling::Max_VES ),
		      INSERT_VALUES, IncludeGhosts);


    PETSC_3D_Array Layer_Current_Ph ( Current_Layer -> layerDA, 
		      Current_Layer -> Current_Properties ( Basin_Modelling::Hydrostatic_Pressure ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Current_Po ( Current_Layer -> layerDA, 
		      Current_Layer -> Current_Properties ( Basin_Modelling::Overpressure ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Current_Lp ( Current_Layer -> layerDA, 
		      Current_Layer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Current_VES ( Current_Layer -> layerDA, 
		      Current_Layer -> Current_Properties ( Basin_Modelling::VES_FP ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Current_Max_VES ( Current_Layer -> layerDA, 
		      Current_Layer -> Current_Properties ( Basin_Modelling::Max_VES ),
		      INSERT_VALUES, IncludeGhosts);



    PETSC_3D_Array Layer_Previous_Chemical_Compaction ( Current_Layer -> layerDA, 
		      Current_Layer -> Previous_Properties ( Basin_Modelling::Chemical_Compaction ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Current_Chemical_Compaction ( Current_Layer -> layerDA, 
		      Current_Layer -> Current_Properties ( Basin_Modelling::Chemical_Compaction ),
		      INSERT_VALUES, IncludeGhosts);


    PETSC_2D_Array topBasaltDepth;
    PETSC_2D_Array bottomBasaltDepth;
    if( Basin_Model -> isALC() && Current_Layer -> isBasement() ) {
       CrustFormation*  crustLayer = dynamic_cast<CrustFormation*>(Basin_Model -> Crust ());
       topBasaltDepth.Set_Global_Array ( * Basin_Model ->mapDA, crustLayer -> TopBasaltDepth, INSERT_VALUES, IncludeGhosts );
       bottomBasaltDepth.Set_Global_Array ( * Basin_Model ->mapDA, crustLayer -> BottomBasaltDepth, INSERT_VALUES, IncludeGhosts );
    }

    DMDAGetInfo(Current_Layer -> layerDA,0,&layerMx,&layerMy,&layerMz,0,0,0,0,0,0,0,0,0);
    DMDAGetCorners(Current_Layer -> layerDA,&xs,&ys,&zs,&xm,&ym,&zm);

    for ( Layer_K = zs; Layer_K < zs+zm; Layer_K++ ) {

      if ( Layer_K != layerMz-1) {
	k = GlobalK[Layer_K];
	unsigned int EltCount;
        int Inode;

	for (EltCount=0; EltCount<Basin_Model -> mapElementList.size(); EltCount++) {
	  NumElement++;
	  CollapsedNodes = 0;

	  if (Basin_Model -> mapElementList[EltCount].exists) {

            Element_Residual.zero ();
            Element_Jacobian.zero ();
            Element_Lithology = Current_Layer->getLithology ( Basin_Model -> mapElementList[EltCount].i[ 0 ],
                                                             Basin_Model -> mapElementList[EltCount].j[ 0 ] );

	    for (Inode = 0; Inode<8; Inode++) {
	      int LidxZ = Layer_K + (Inode<4 ? 1 : 0);
	      int GidxZ = k + (Inode<4 ? 1 : 0);
	      int GidxY = Basin_Model -> mapElementList[EltCount].j[Inode%4];
	      int GidxX = Basin_Model -> mapElementList[EltCount].i[Inode%4];

              Nodal_BCs [ Inode ] = Interior_Boundary;

	      if ( Current_Layer -> isMantle() && GidxZ == 0 ) {

		if ( FastcauldronSimulator::getInstance ().getBottomBoundaryConditions () == MANTLE_HEAT_FLOW ) {
                  Nodal_BCs [Inode]  = Bottom_Boundary_Flux;
                  BC_Values ( Inode + 1 ) = MantleHeatFlow ( GidxY, GidxX );
		} else if ( FastcauldronSimulator::getInstance ().getBottomBoundaryConditions () == FIXED_BASEMENT_TEMPERATURE ||
                            FastcauldronSimulator::getInstance ().getBottomBoundaryConditions () == Interface::ADVANCED_LITHOSPHERE_CALCULATOR ) {
                  Nodal_BCs [ Inode ] = Surface_Boundary;
                  BC_Values ( Inode + 1 ) = temp(LidxZ,GidxY,GidxX);
		}

	      } else if ( GidxZ == gridMz-1 ) {
                Nodal_BCs [ Inode ] = Surface_Boundary;
                BC_Values ( Inode + 1 ) = temp(LidxZ,GidxY,GidxX);
	      }

	      col[Inode].k = GidxZ; row[Inode].k = GidxZ;
	      col[Inode].j = GidxY; row[Inode].j = GidxY;
	      col[Inode].i = GidxX; row[Inode].i = GidxX;

	      if (GidxZ != dof(GidxZ,GidxY,GidxX)) {
		CollapsedNodes++;
		NumCoincidentNodes = max(NumCoincidentNodes,((int)dof(GidxZ,GidxY,GidxX)-GidxZ));

		col[Inode].k = (int) dof(GidxZ,GidxY,GidxX); row[Inode].k = col[Inode].k;
		a_diag[0].k = GidxZ;
                a_diag[0].j = GidxY;
                a_diag[0].i = GidxX;
		MatSetValuesStencil( Jacobian,1,a_diag,1,a_diag,OneDiagonal,ADD_VALUES);
	      }

              Element_Heat_Production ( Inode + 1 ) =  bulkHeatProd(LidxZ,GidxY,GidxX);

              Geometry_Matrix ( 1, Inode + 1 ) = Origin_X + (Delta_X * GidxX);
              Geometry_Matrix ( 2, Inode + 1 ) = Origin_Y + (Delta_Y * GidxY);
              Geometry_Matrix ( 3, Inode + 1 ) = depth(LidxZ,GidxY,GidxX);

              Previous_Geometry_Matrix ( 1, Inode + 1 ) = Origin_X + (Delta_X * GidxX);
              Previous_Geometry_Matrix ( 2, Inode + 1 ) = Origin_Y + (Delta_Y * GidxY);
              Previous_Geometry_Matrix ( 3, Inode + 1 ) = Current_Layer->Previous_Properties ( Basin_Modelling::Depth, LidxZ, GidxY, GidxX );

              Previous_Ph ( Inode + 1 ) = Layer_Previous_Ph ( LidxZ, GidxY, GidxX );
              Current_Ph  ( Inode + 1 ) = Layer_Current_Ph  ( LidxZ, GidxY, GidxX );

              Previous_Po ( Inode + 1 ) = Layer_Previous_Po ( LidxZ, GidxY, GidxX );
              Current_Po  ( Inode + 1 ) = Layer_Current_Po  ( LidxZ, GidxY, GidxX );

              Current_Lp  ( Inode + 1 ) = Layer_Current_Lp  ( LidxZ, GidxY, GidxX );
              Previous_VES ( Inode + 1 ) = Layer_Previous_VES ( LidxZ, GidxY, GidxX );
              Current_VES  ( Inode + 1 ) = Layer_Current_VES  ( LidxZ, GidxY, GidxX );

              Previous_Max_VES ( Inode + 1 ) = Layer_Previous_Max_VES ( LidxZ, GidxY, GidxX );
              Current_Max_VES  ( Inode + 1 ) = Layer_Current_Max_VES  ( LidxZ, GidxY, GidxX );

              Previous_Temperature ( Inode + 1 ) = oldtemp ( LidxZ, GidxY, GidxX );
              Current_Temperature  ( Inode + 1 ) = temp  ( LidxZ, GidxY, GidxX );

              Previous_Chemical_Compaction ( Inode + 1 ) = Layer_Previous_Chemical_Compaction ( LidxZ, GidxY, GidxX );
              Current_Chemical_Compaction  ( Inode + 1 ) = Layer_Current_Chemical_Compaction  ( LidxZ, GidxY, GidxX );
	    }

            if( Current_Layer -> isBasement() && Basin_Model -> isALC() ) {
               ElementGeometryMatrix Geometry_Matrix1;
               for ( Inode = 0; Inode < 8; Inode ++ ) {
                  int LidxZ = Layer_K + (Inode < 4 ? 1 : 0);
                  int GidxY = Basin_Model -> mapElementList[EltCount].j[Inode%4];
                  int GidxX = Basin_Model -> mapElementList[EltCount].i[Inode%4];
                  
                  Geometry_Matrix1 ( 3, Inode + 1 ) = depth(LidxZ,GidxY,GidxX) - depth( zs + zm - 1, GidxY, GidxX );
                  if( Current_Layer -> isMantle() ) {
                     
                     double Hfix = FastcauldronSimulator::getInstance ().getLithosphereThicknessMod( GidxX, GidxY, Current_Time);
                     if ( Geometry_Matrix1 ( 3, Inode + 1 ) > Hfix ) {
                        Nodal_BCs [ Inode ] = Surface_Boundary;
                        BC_Values ( Inode + 1 ) = temp(0, GidxY,  GidxX);
                     }
                  }
               }
               double midPointDepth = Geometry_Matrix1.getMidPoint();

               int xY = Basin_Model -> mapElementList[EltCount].j[0];
               int xX = Basin_Model -> mapElementList[EltCount].i[0];

               Element_Lithology = Current_Layer -> getLithology( Current_Time, xX, xY, midPointDepth );
               
               if( Current_Layer->isBasalt() ) {
                  for ( Inode = 0; Inode < 8; Inode ++ ) {
                     int LidxZ = Layer_K + (Inode < 4 ? 1 : 0);
                     int GidxY = Basin_Model -> mapElementList[EltCount].j[Inode%4];
                     int GidxX = Basin_Model -> mapElementList[EltCount].i[Inode%4];
                     Current_Layer->setBasaltLitho ( GidxX, GidxY, LidxZ );                   

                     if ( Inode > 3 ) {
                        if( bottomBasaltDepth( GidxY, GidxX ) == CAULDRONIBSNULLVALUE ) {
                           bottomBasaltDepth( GidxY, GidxX ) = depth( LidxZ, GidxY, GidxX ); 
                        }
                        
                     } else {
                        topBasaltDepth( GidxY, GidxX ) = depth( LidxZ, GidxY, GidxX );                       
                     }

                     if( ! Basin_Model->bottomBasaltTemp ) {
                        if( !Current_Layer->getPreviousBasaltLitho( GidxX, GidxY, LidxZ ) && 
                            Current_Time > FastcauldronSimulator::getInstance ().getEndOfRiftEvent( xX, xY )) {
                           if( Current_Layer->isMantle() || 
                               ( Current_Layer->isCrust() && ( Geometry_Matrix1 ( 3, Inode + 1 ) > FastcauldronSimulator::getInstance ().getMinimumHCBL() ))) {
                              Nodal_BCs [ Inode ] = Interior_Constrained_Temperature; 
                              BC_Values ( Inode + 1 ) = Constrained_Temp_Value;
                           }
                        } 
                     } else {
                        // set constraied temperature only for the bottom basalt element
                        if(( bottomBasaltDepth( GidxY, GidxX ) == CAULDRONIBSNULLVALUE ) && 
                           ( FastcauldronSimulator::getInstance ().getBasaltThickness( GidxX, GidxY, Current_Time ) > 
                             FastcauldronSimulator::getInstance ().getBasaltThickness( GidxX, GidxY, Previous_Time )) &&
                           Current_Time >  FastcauldronSimulator::getInstance ().getEndOfRiftEvent( xX, xY )) {
                           
                           Nodal_BCs [ Inode ] = Interior_Constrained_Temperature; 
                           BC_Values ( Inode + 1 ) = Constrained_Temp_Value;
                        }
                     }
                  }
               }
            }
            PetscTime(&Element_Start_Time);
            Assemble_Element_Temperature_System ( Current_Layer->kind() == Interface::BASEMENT_FORMATION,
                                                  Plane_Quadrature_Degree, Depth_Quadrature_Degree,
                                                  Current_Time, Time_Step, Is_Steady_State, 
                                                  IncludeAdvectiveTerm,
                                                  Nodal_BCs, BC_Values,
                                                  Element_Lithology,
                                                  Current_Layer->fluid,
                                                  Include_Chemical_Compaction,
                                                  Previous_Geometry_Matrix,
                                                  Geometry_Matrix,
                                                  Element_Heat_Production,
                                                  Previous_Ph,
                                                  Current_Ph,
                                                  Previous_Po,
                                                  Current_Po,
                                                  Current_Lp,
                                                  Previous_VES,
                                                  Current_VES,
                                                  Previous_Max_VES,
                                                  Current_Max_VES,
                                                  Previous_Temperature,
                                                  Current_Temperature,
                                                  Previous_Chemical_Compaction,
                                                  Current_Chemical_Compaction,
                                                  Element_Jacobian,
                                                  Element_Residual );

            PetscTime(&Element_End_Time);
            Element_Contributions_Time = Element_Contributions_Time + Element_End_Time - Element_Start_Time;

	  } else {

	    for (Inode = 0; Inode<8; Inode++) {
	      int GidxZ = k + (Inode<4 ? 0 : 1);
	      int GidxY = Basin_Model -> mapElementList[EltCount].j[Inode%4];
	      int GidxX = Basin_Model -> mapElementList[EltCount].i[Inode%4];

	      if ( not Basin_Model->nodeIsDefined ( GidxX, GidxY )) {
		a_diag[0].k = GidxZ; a_diag[0].j = GidxY; a_diag[0].i = GidxX; 
		MatSetValuesStencil( Jacobian,1,a_diag,1,a_diag,OneDiagonal,ADD_VALUES);
	      }  

	    }

	    continue;
	  }

	  MatSetValuesStencil( Jacobian,8,row,8,col, Element_Jacobian.C_Array (),ADD_VALUES);

	  for (Inode = 0; Inode<8; Inode++) {
	    int irow = row[Inode].i;
	    int jrow = row[Inode].j;
	    int krow = row[Inode].k;
            rhsF(krow,jrow,irow) = rhsF(krow,jrow,irow) - Element_Residual ( Inode + 1 );
	  }

	}

      }

    }

    delete[] GlobalK;

    if( Basin_Model -> isALC() && Current_Layer -> isBasement() ) {
       topBasaltDepth.Restore_Global_Array( Update_Excluding_Ghosts );  
       bottomBasaltDepth.Restore_Global_Array( Update_Excluding_Ghosts ); 
    }
    
    Current_Layer -> Previous_Properties.Restore_Property ( Basin_Modelling::Depth );

    Layers++;
  }

  rhsF.Restore_Global_Array( Update_Including_Ghosts );  

  MatAssemblyBegin( Jacobian,MAT_FINAL_ASSEMBLY);
  MatAssemblyEnd( Jacobian,MAT_FINAL_ASSEMBLY);

  PetscLogStages::pop();
}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Temperature_Solver::Assemble_Residual"

void Temperature_Solver::Assemble_Residual ( const double  Previous_Time,
                                             const double  Current_Time,
                                             const DM&     Temperature_FEM_Grid,
                                             const Vec&    Temperature_DOFs,
                                                   Vec&    Residual,
                                                   double& Element_Contributions_Time ) {

  using namespace Basin_Modelling;

  PetscLogStages::push( PetscLogStages::TEMPERATURE_INITIALISATION_SYSTEM_ASSEMBLY );

  const double Time_Step = Previous_Time - Current_Time;

  const int Plane_Quadrature_Degree = getPlaneQuadratureDegree ( Basin_Model -> Optimisation_Level );
  const int Depth_Quadrature_Degree = getDepthQuadratureDegree ( Basin_Model -> Optimisation_Level );

  const bool SteadyStateCalculation = ( Previous_Time == Current_Time );

  const bool IncludeAdvectiveTerm = Basin_Model->includeAdvectiveTerm and 
                                    not SteadyStateCalculation and
                                   ( Basin_Model->Do_Iteratively_Coupled or
                                     Basin_Model->IsCalculationCoupled );

  const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

  double Delta_X  = grid.deltaI;
  double Delta_Y  = grid.deltaJ;
  double Origin_X = grid.originI;
  double Origin_Y = grid.originJ;

  PetscLogDouble Element_Start_Time;
  PetscLogDouble Element_End_Time;


  int kkk;
  int Layer_K;
  int xs, ys, zs, xm, ym, zm;
  int layerMx, layerMy, layerMz;
  int gridMx, gridMy, gridMz;
  int k;
  int NumElement = 0;
  int NumCoincidentNodes = 0;
			    
  bool IncludeGhosts = true;

  MatStencil   row[8];
  int          *GlobalK;

  int    CollapsedNodes = 0;

  DMDAGetInfo( Temperature_FEM_Grid,0,&gridMx,&gridMy,&gridMz,0,0,0,0,0,0,0,0,0);

  PETSC_2D_Array MantleHeatFlow;

  if ( FastcauldronSimulator::getInstance ().getBottomBoundaryConditions () == MANTLE_HEAT_FLOW ) {
    MantleHeatFlow.Set_Global_Array ( *Basin_Model -> mapDA, Mantle_Heat_Flow,
				      INSERT_VALUES,IncludeGhosts );

  }


  PETSC_3D_Array rhsF( Temperature_FEM_Grid, Residual, INSERT_VALUES, IncludeGhosts);

  PETSC_3D_Array dof( Temperature_FEM_Grid, Temperature_DOFs, INSERT_VALUES, IncludeGhosts);

  ElementGeometryMatrix Previous_Geometry_Matrix;
  ElementGeometryMatrix Geometry_Matrix;

  ElementVector Previous_Ph;
  ElementVector Current_Ph;

  ElementVector Previous_Po;
  ElementVector Current_Po;
  ElementVector Current_Lp;

  ElementVector Previous_VES;
  ElementVector Current_VES;

  ElementVector Previous_Max_VES;
  ElementVector Current_Max_VES;

  ElementVector Previous_Temperature;
  ElementVector Current_Temperature;

  ElementVector Current_Chemical_Compaction;
  ElementVector Previous_Chemical_Compaction;

  ElementVector Element_Heat_Production;

  ElementVector BC_Values;
  Boundary_Conditions Nodal_BCs[8];

  ElementVector Element_Residual;
  const CompoundLithology*  Element_Lithology;

  int NSegmZ;

  bool Is_Steady_State = false; // remove this soon
  bool Include_Chemical_Compaction;

  const double Constrained_Temp_Value = FastcauldronSimulator::getInstance ().getConstrainedBasaltTemperature();

  int NodeZDirCount = 0;
  Layer_Iterator Layers;
  Layers.Initialise_Iterator ( Basin_Model -> layers, Ascending, Basement_And_Sediments, 
			       Active_Layers_Only );

  Element_Contributions_Time = 0.0;

  int globalXNodes;
  int globalYNodes;
  if( Basin_Model -> isALC() ) {
     CrustFormation*   crustLayer = dynamic_cast<CrustFormation*>(Basin_Model -> Crust ());   
     MantleFormation*  mantleLayer = dynamic_cast<MantleFormation*>(Basin_Model -> Mantle ());
     crustLayer->cleanVectors();
     mantleLayer->cleanVectors();
     DMDAGetInfo( *Basin_Model -> mapDA, PETSC_NULL, &globalXNodes, &globalYNodes,
                  PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, 
                  PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL );
  }
  while ( ! Layers.Iteration_Is_Done () ) {

    LayerProps_Ptr Current_Layer = Layers.Current_Layer ();

    Include_Chemical_Compaction = (( Basin_Model -> Do_Chemical_Compaction ) && ( Current_Layer -> Get_Chemical_Compaction_Mode ()));

    Current_Layer -> Previous_Properties.Activate_Property ( Basin_Modelling::Depth, INSERT_VALUES, IncludeGhosts );

    NSegmZ = Current_Layer -> getNrOfActiveElements();
    
    string EventName = "setUpGM" + Current_Layer -> layername;

    GlobalK = new int[NSegmZ+1];

    for (kkk = 0; kkk <=NSegmZ; kkk++){
      GlobalK[kkk] = NodeZDirCount;
      NodeZDirCount++;
    }

    NodeZDirCount--;


    PETSC_3D_Array temp  ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Temperature ), INSERT_VALUES, IncludeGhosts );
    PETSC_3D_Array depth ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Depth       ), INSERT_VALUES, IncludeGhosts );

    PETSC_3D_Array bulkHeatProd( Current_Layer -> layerDA, 
				 Current_Layer -> BulkHeatProd,
				 INSERT_VALUES, IncludeGhosts );

    PETSC_3D_Array oldtemp (Current_Layer -> layerDA, 
				 Current_Layer -> Previous_Properties ( Basin_Modelling::Temperature ),
				 INSERT_VALUES, IncludeGhosts);


    PETSC_3D_Array Layer_Previous_Ph ( Current_Layer -> layerDA, 
		      Current_Layer -> Previous_Properties ( Basin_Modelling::Hydrostatic_Pressure ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Previous_Po ( Current_Layer -> layerDA, 
		      Current_Layer -> Previous_Properties ( Basin_Modelling::Overpressure ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Previous_VES ( Current_Layer -> layerDA, 
		      Current_Layer -> Previous_Properties ( Basin_Modelling::VES_FP ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Previous_Max_VES ( Current_Layer -> layerDA, 
		      Current_Layer -> Previous_Properties ( Basin_Modelling::Max_VES ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Current_Ph ( Current_Layer -> layerDA, 
		      Current_Layer -> Current_Properties ( Basin_Modelling::Hydrostatic_Pressure ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Current_Po ( Current_Layer -> layerDA, 
		      Current_Layer -> Current_Properties ( Basin_Modelling::Overpressure ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Current_Lp ( Current_Layer -> layerDA, 
		      Current_Layer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Current_VES ( Current_Layer -> layerDA, 
		      Current_Layer -> Current_Properties ( Basin_Modelling::VES_FP ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Current_Max_VES ( Current_Layer -> layerDA, 
		      Current_Layer -> Current_Properties ( Basin_Modelling::Max_VES ),
		      INSERT_VALUES, IncludeGhosts);


    PETSC_3D_Array Layer_Previous_Chemical_Compaction ( Current_Layer -> layerDA, 
		      Current_Layer -> Previous_Properties ( Basin_Modelling::Chemical_Compaction ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Current_Chemical_Compaction ( Current_Layer -> layerDA, 
		      Current_Layer -> Current_Properties ( Basin_Modelling::Chemical_Compaction ),
		      INSERT_VALUES, IncludeGhosts);

    PETSC_2D_Array topBasaltDepth;
    PETSC_2D_Array bottomBasaltDepth;
    if( Basin_Model -> isALC() && Current_Layer -> isBasement() ) {
       CrustFormation*  crustLayer = dynamic_cast<CrustFormation*>(Basin_Model -> Crust ());
       topBasaltDepth.Set_Global_Array ( * Basin_Model ->mapDA, crustLayer -> TopBasaltDepth, INSERT_VALUES, IncludeGhosts );
       bottomBasaltDepth.Set_Global_Array ( * Basin_Model ->mapDA, crustLayer -> BottomBasaltDepth, INSERT_VALUES, IncludeGhosts );
    }

    DMDAGetInfo(Current_Layer -> layerDA,0,&layerMx,&layerMy,&layerMz,0,0,0,0,0,0,0,0,0);
    DMDAGetCorners(Current_Layer -> layerDA,&xs,&ys,&zs,&xm,&ym,&zm);

    for ( Layer_K = zs; Layer_K < zs + zm; Layer_K++) {
      if ( Layer_K == layerMz-1) {
	continue;
      } else {
	k = GlobalK[Layer_K];
	unsigned int EltCount;
        int Inode;
	for (EltCount=0; EltCount<Basin_Model -> mapElementList.size(); EltCount++) {
	  NumElement++;
	  CollapsedNodes = 0;

	  if (Basin_Model -> mapElementList[EltCount].exists) {

            Element_Residual.zero ();
            Element_Lithology = Current_Layer->getLithology ( Basin_Model -> mapElementList[EltCount].i[ 0 ],
                                                              Basin_Model -> mapElementList[EltCount].j[ 0 ] );

	    for (Inode = 0; Inode<8; Inode++) {
	      int LidxZ = Layer_K + (Inode<4 ? 1 : 0);
	      int GidxZ = k + (Inode<4 ? 1 : 0);
	      int GidxY = Basin_Model -> mapElementList[EltCount].j[Inode%4];
	      int GidxX = Basin_Model -> mapElementList[EltCount].i[Inode%4];

              Nodal_BCs [ Inode ] = Interior_Boundary;

	      if ( Current_Layer -> isMantle() && GidxZ == 0 ) {

		if ( FastcauldronSimulator::getInstance ().getBottomBoundaryConditions () == MANTLE_HEAT_FLOW) {
                  Nodal_BCs [Inode]  = Bottom_Boundary_Flux;
                  BC_Values ( Inode + 1 ) = MantleHeatFlow ( GidxY, GidxX );
		} else if ( FastcauldronSimulator::getInstance ().getBottomBoundaryConditions () == FIXED_BASEMENT_TEMPERATURE ||
                            FastcauldronSimulator::getInstance ().getBottomBoundaryConditions () == Interface::ADVANCED_LITHOSPHERE_CALCULATOR ) {
                  Nodal_BCs [ Inode ] = Surface_Boundary;
                  BC_Values ( Inode + 1 ) = temp(LidxZ,GidxY,GidxX);
		}

	      } else if ( GidxZ == gridMz-1 ) {
                Nodal_BCs [ Inode ] = Surface_Boundary;
                BC_Values ( Inode + 1 ) = temp(LidxZ,GidxY,GidxX);
	      }

              row[Inode].k = GidxZ;
              row[Inode].j = GidxY;
              row[Inode].i = GidxX;

	      if (GidxZ != dof(GidxZ,GidxY,GidxX)) {
		CollapsedNodes++;
		NumCoincidentNodes = max(NumCoincidentNodes,((int)dof(GidxZ,GidxY,GidxX)-GidxZ));

		row[Inode].k = (int) dof(GidxZ,GidxY,GidxX); 
	      }

              Element_Heat_Production ( Inode + 1 ) =  bulkHeatProd(LidxZ,GidxY,GidxX);
              Geometry_Matrix ( 1, Inode + 1 ) = Origin_X + (Delta_X * GidxX);
              Geometry_Matrix ( 2, Inode + 1 ) = Origin_Y + (Delta_Y * GidxY);
              Geometry_Matrix ( 3, Inode + 1 ) = depth(LidxZ,GidxY,GidxX);

              Previous_Geometry_Matrix ( 1, Inode + 1 ) = Origin_X + (Delta_X * GidxX);
              Previous_Geometry_Matrix ( 2, Inode + 1 ) = Origin_Y + (Delta_Y * GidxY);
              Previous_Geometry_Matrix ( 3, Inode + 1 ) = Current_Layer->Previous_Properties ( Basin_Modelling::Depth, LidxZ, GidxY, GidxX );

              Previous_Ph ( Inode + 1 ) = Layer_Previous_Ph ( LidxZ, GidxY, GidxX );
              Current_Ph  ( Inode + 1 ) = Layer_Current_Ph  ( LidxZ, GidxY, GidxX );

              Previous_Po ( Inode + 1 ) = Layer_Previous_Po ( LidxZ, GidxY, GidxX );
              Current_Po  ( Inode + 1 ) = Layer_Current_Po  ( LidxZ, GidxY, GidxX );
              Current_Lp  ( Inode + 1 ) = Layer_Current_Lp  ( LidxZ, GidxY, GidxX );

              Previous_VES ( Inode + 1 ) = Layer_Previous_VES ( LidxZ, GidxY, GidxX );
              Current_VES  ( Inode + 1 ) = Layer_Current_VES  ( LidxZ, GidxY, GidxX );

              Previous_Max_VES ( Inode + 1 ) = Layer_Previous_Max_VES ( LidxZ, GidxY, GidxX );
              Current_Max_VES  ( Inode + 1 ) = Layer_Current_Max_VES  ( LidxZ, GidxY, GidxX );

              Previous_Temperature ( Inode + 1 ) = oldtemp ( LidxZ, GidxY, GidxX );
              Current_Temperature  ( Inode + 1 ) = temp  ( LidxZ, GidxY, GidxX );

              Previous_Chemical_Compaction ( Inode + 1 ) = Layer_Previous_Chemical_Compaction ( LidxZ, GidxY, GidxX );
              Current_Chemical_Compaction  ( Inode + 1 ) = Layer_Current_Chemical_Compaction  ( LidxZ, GidxY, GidxX );
	    }

            if( Current_Layer -> isBasement() && Basin_Model -> isALC() ) {
               ElementGeometryMatrix Geometry_Matrix1;
               for ( Inode = 0; Inode < 8; Inode ++ ) {
                  int LidxZ = Layer_K + (Inode < 4 ? 1 : 0);
                  int GidxY = Basin_Model -> mapElementList[EltCount].j[Inode%4];
                  int GidxX = Basin_Model -> mapElementList[EltCount].i[Inode%4];
                  Geometry_Matrix1 ( 3, Inode + 1 ) = depth(LidxZ,GidxY,GidxX) - depth( zs + zm - 1, GidxY, GidxX );
                  if( Current_Layer -> isMantle() ) {
                     double Hfix = FastcauldronSimulator::getInstance ().getLithosphereThicknessMod( GidxX, GidxY, Current_Time);
                     if ( Geometry_Matrix1 ( 3, Inode + 1 ) > Hfix ) {
                        Nodal_BCs [ Inode ] = Surface_Boundary;
                        BC_Values ( Inode + 1 ) = temp(0, GidxY,  GidxX);
                     }
                  }
               }
               double midPointDepth = Geometry_Matrix1.getMidPoint();
               
               int xY = Basin_Model -> mapElementList[EltCount].j[0];
               int xX = Basin_Model -> mapElementList[EltCount].i[0];
               Element_Lithology = Current_Layer -> getLithology( Current_Time, xX, xY, midPointDepth );

               if( Current_Layer->isBasalt() ) {
                  for ( Inode = 0; Inode < 8; Inode ++ ) {
                     int LidxZ = Layer_K + (Inode < 4 ? 1 : 0);
                     int GidxY = Basin_Model -> mapElementList[EltCount].j[Inode%4];
                     int GidxX = Basin_Model -> mapElementList[EltCount].i[Inode%4];
                     Current_Layer->setBasaltLitho ( GidxX, GidxY, LidxZ );

                     if ( Inode > 3 ) {
                        if( bottomBasaltDepth( GidxY, GidxX ) == CAULDRONIBSNULLVALUE ) {
                           bottomBasaltDepth( GidxY, GidxX ) = depth( LidxZ, GidxY, GidxX ); 
                        }
                        
                     } else {
                        topBasaltDepth( GidxY, GidxX ) = depth( LidxZ, GidxY, GidxX);
                        
                     }

                    if( ! Basin_Model->bottomBasaltTemp ) {
                        if( !Current_Layer->getPreviousBasaltLitho( GidxX, GidxY, LidxZ ) &&
                            Current_Time > FastcauldronSimulator::getInstance ().getEndOfRiftEvent( xX, xY ) ) {
                           if( Current_Layer->isMantle() || 
                               ( Current_Layer->isCrust() && ( Geometry_Matrix1 ( 3, Inode + 1 ) >  FastcauldronSimulator::getInstance ().getMinimumHCBL() ))) {
                              Nodal_BCs [ Inode ] = Interior_Constrained_Temperature; 
                              BC_Values ( Inode + 1 ) = Constrained_Temp_Value;
                           }
                        }
                     } else {
                        // set constraied temperature only for the bottom basalt element
                        if(( bottomBasaltDepth( GidxY, GidxX ) == CAULDRONIBSNULLVALUE ) && 
                           ( FastcauldronSimulator::getInstance ().getBasaltThickness( GidxX, GidxY, Current_Time ) > 
                             FastcauldronSimulator::getInstance ().getBasaltThickness( GidxX, GidxY, Previous_Time )) &&
                           Current_Time >  FastcauldronSimulator::getInstance ().getEndOfRiftEvent( xX, xY )) {
                           Nodal_BCs [ Inode ] = Interior_Constrained_Temperature; 
                           BC_Values ( Inode + 1 ) = Constrained_Temp_Value;
                        }
                     }
                  }
               }
            }
            PetscTime(&Element_Start_Time);

            Assemble_Element_Temperature_Residual ( Current_Layer->kind() == Interface::BASEMENT_FORMATION,
                                                    Plane_Quadrature_Degree, Depth_Quadrature_Degree,
                                                    Current_Time, Time_Step, Is_Steady_State,
                                                    IncludeAdvectiveTerm,
                                                    Nodal_BCs, BC_Values,
                                                    Element_Lithology,
                                                    Current_Layer->fluid,
                                                    Include_Chemical_Compaction,
                                                    Previous_Geometry_Matrix,
                                                    Geometry_Matrix,
                                                    Element_Heat_Production,
                                                    Previous_Ph,
                                                    Current_Ph,
                                                    Previous_Po,
                                                    Current_Po,
                                                    Current_Lp,
                                                    Previous_VES,
                                                    Current_VES,
                                                    Previous_Max_VES,
                                                    Current_Max_VES,
                                                    Previous_Temperature,
                                                    Current_Temperature,
                                                    Previous_Chemical_Compaction,
                                                    Current_Chemical_Compaction,
                                                    Element_Residual );

            PetscTime(&Element_End_Time);
            Element_Contributions_Time = Element_Contributions_Time + Element_End_Time - Element_Start_Time;

	  } else {

	    continue;
	  }

	  for (Inode = 0; Inode<8; Inode++) {
	    int irow = row[Inode].i;
	    int jrow = row[Inode].j;
	    int krow = row[Inode].k;
            rhsF(krow,jrow,irow) = rhsF(krow,jrow,irow) - Element_Residual ( Inode + 1 );
	  }

	}

      }

    }

    delete[] GlobalK;

    if( Basin_Model -> isALC() && Current_Layer -> isBasement() ) {
       topBasaltDepth.Restore_Global_Array( Update_Excluding_Ghosts );  
       bottomBasaltDepth.Restore_Global_Array( Update_Excluding_Ghosts ); 
    }

    Current_Layer -> Previous_Properties.Restore_Property ( Basin_Modelling::Depth );

    Layers++;
  }

  rhsF.Restore_Global_Array( Update_Including_Ghosts );  

  PetscLogStages::pop();
}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Temperature_Solver::Restore_Temperature_Solution"

void Temperature_Solver::Restore_Temperature_Solution ( const DM  Temperature_FEM_Grid,
							const Vec Temperature_DOF_Numbers,
                                                              Vec Temperature ) {

  using namespace Basin_Modelling;

  int X_Start;
  int Y_Start;
  int Z_Start;
  int X_Count;
  int Y_Count;
  int Z_Count;
  int FEM_Grid_Index;
  int DOF_Index;

  int I, J, K;
  int*GlobalK;

  int Z_Node_Count = 0;
  int Number_Of_Segments;

  Layer_Iterator Layers ( Basin_Model -> layers, Ascending, Basement_And_Sediments, Active_Layers_Only );
  LayerProps_Ptr Current_Layer;

  PETSC_3D_Array New_Temperature ( Temperature_FEM_Grid, Temperature );
  PETSC_3D_Array DOFs            ( Temperature_FEM_Grid, Temperature_DOF_Numbers );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) 
  {
    Current_Layer = Layers.Current_Layer ();
    Number_Of_Segments = Current_Layer->getNrOfActiveElements ();
    GlobalK = new int [ Number_Of_Segments + 1 ];

    for ( K = 0; K <= Number_Of_Segments; K++ ) 
    {
      GlobalK [ K ] = Z_Node_Count;
      Z_Node_Count = Z_Node_Count + 1;
    }

    Z_Node_Count = Z_Node_Count - 1;

    DMDAGetCorners ( Current_Layer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );

    PETSC_3D_Array Layer_Temperature ( Current_Layer->layerDA, 
				       Current_Layer->Current_Properties ( Basin_Modelling::Temperature ) );

    for ( I = X_Start; I < X_Start + X_Count; I++ ) {

      for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

        if ( Basin_Model->nodeIsDefined ( I, J ) ) {

          for ( K = Z_Start; K < Z_Start + Z_Count; K++ ) 
	  {
            FEM_Grid_Index = GlobalK[ K ];
            DOF_Index = (int) DOFs ( FEM_Grid_Index, J, I );

            New_Temperature ( DOF_Index, J, I ) = Layer_Temperature ( K, J, I );
          }

	}

      }

    }

    delete [] GlobalK;
  }

}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Temperature_Solver::Store_Temperature_Solution"

void Temperature_Solver::Store_Temperature_Solution ( const DM  Temperature_FEM_Grid,
                                                      const Vec Temperature_DOF_Numbers,
                                                      const Vec Temperature,
                                                      const double Current_Time ) {

  using namespace Basin_Modelling;

  int X_Start;
  int Y_Start;
  int Z_Start;
  int X_Count;
  int Y_Count;
  int Z_Count;
  int FEM_Grid_Index;
  int DOF_Index;

  int I, J, K;
  int*GlobalK;

  int Z_Node_Count = 0;
  int Number_Of_Segments;

  PetscScalar Temperature_Value, SeaBottomTemperature;

  Layer_Iterator Layers ( Basin_Model -> layers, Ascending, Basement_And_Sediments, Active_Layers_Only );
  LayerProps_Ptr Current_Layer;

  DMDAGetCorners ( *Basin_Model->mapDA, &X_Start, &Y_Start, PETSC_NULL, &X_Count, &Y_Count, PETSC_NULL );

  PETSC_3D_Array New_Temperature  ( Temperature_FEM_Grid, Temperature );
  PETSC_3D_Array DOFs             ( Temperature_FEM_Grid, Temperature_DOF_Numbers );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) 
  {
    Current_Layer = Layers.Current_Layer ();
    Number_Of_Segments = Current_Layer->getNrOfActiveElements ();
    GlobalK = new int [ Number_Of_Segments + 1 ];

    for ( K = 0; K <= Number_Of_Segments; K++ ) 
    {
      GlobalK [ K ] = Z_Node_Count;
      Z_Node_Count = Z_Node_Count + 1;
    }

    Z_Node_Count = Z_Node_Count - 1;

    DMDAGetCorners ( Current_Layer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );

    Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Temperature );

    if( Basin_Model -> isALC() ) { 
       
       for ( I = X_Start; I < X_Start + X_Count; I++ ) {
          
          for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {
             
             if ( Basin_Model->nodeIsDefined ( I, J )) {
                SeaBottomTemperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( I, J, Current_Time );
                for ( K = Z_Start; K < Z_Start + Z_Count; K++ ) {
                   FEM_Grid_Index = GlobalK[ K ];
                   DOF_Index = (int) DOFs ( FEM_Grid_Index, J, I );
                   
                   Temperature_Value = New_Temperature ( DOF_Index, J, I );
                   // Negative temperature could occur during the basalt emplacement. Cut off negative values at surface temperature.
                   Current_Layer -> Current_Properties ( Basin_Modelling::Temperature,  K, J, I ) = ( Temperature_Value < 0.0 ? SeaBottomTemperature :  Temperature_Value );
                }
             }
          }
          
       }
    } else {
        for ( I = X_Start; I < X_Start + X_Count; I++ ) {
          
          for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {
             
             if ( Basin_Model->nodeIsDefined ( I, J )) {
                
                for ( K = Z_Start; K < Z_Start + Z_Count; K++ ) {
                   FEM_Grid_Index = GlobalK[ K ];
                   DOF_Index = (int) DOFs ( FEM_Grid_Index, J, I );
                   
                   Temperature_Value = New_Temperature ( DOF_Index, J, I );

                   Current_Layer -> Current_Properties ( Basin_Modelling::Temperature,  K, J, I ) = Temperature_Value;
                   
                }
                
             }
             
          }
          
       }
   }

    Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Temperature );

    delete [] GlobalK;

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

#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::initialiseVReVectors"

void Temperature_Solver::initialiseVReVectors ( AppCtx* basinModel ) {

  if ( ! basinModel->filterwizard.IsVreCalculationNeeded()) {
    return;
  }

  using namespace Basin_Modelling;

  Layer_Iterator Layers;
  Layers.Initialise_Iterator ( basinModel -> layers, Descending, Source_Rocks_Only, 
                               Active_Layers_Only );
    
  while ( ! Layers.Iteration_Is_Done () ) {
    LayerProps_Ptr currentLayer = Layers.Current_Layer ();

    VecSet ( currentLayer->VreIntValue, Zero );

    Layers++;
  }

  Vitrinite_Calculator.initialiseVectors ();
}

//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::computeVReIncrement"

void Temperature_Solver::computeVReIncrement ( AppCtx*      basinModel,
                                               const double Previous_Time,
                                               const double Current_Time ) {

  MPI_Barrier(PETSC_COMM_WORLD);

  Vitrinite_Calculator.CalcStep ( Current_Time, Previous_Time - Current_Time );

}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::computeSnapShotVRe"

void Temperature_Solver::computeSnapShotVRe (  AppCtx*         basinModel,
                                               const double    Current_Time ) {

  using namespace Basin_Modelling;

  if ( Current_Time == Present_Day ) {
    
    Layer_Iterator Layers;
    Layers.Initialise_Iterator ( basinModel -> layers, Descending, Source_Rocks_Only, 
                                 Active_Layers_Only );
    
    while ( ! Layers.Iteration_Is_Done () ) {
      
      LayerProps_Ptr Current_Layer = Layers.Current_Layer ();
      
      DMCreateGlobalVector( *basinModel -> mapDA, 
                            &Current_Layer->Present_Day_VRE );
      VecSet ( Current_Layer->Present_Day_VRE, CAULDRONIBSNULLVALUE );

      Layers++;
    }
    
  }

  Vitrinite_Calculator.CalcSnaptimeVr ( Current_Time, basinModel->getValidNeedles ());
}

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
#undef  __FUNCT__
#define __FUNCT__ "Temperature_Solver::deleteVReVectors"

void Temperature_Solver::deleteVReVectors ( AppCtx* basinModel ) {

  using namespace Basin_Modelling;

  Layer_Iterator Layers;
  PetscBool     validVector;

  Layers.Initialise_Iterator ( basinModel -> layers, Descending, Source_Rocks_Only, 
                               Active_Layers_Only );
    
  while ( ! Layers.Iteration_Is_Done ()) {
    VecValid ( Layers.Current_Layer () -> Vre, &validVector );

    if ( validVector ) {
       VecDestroy (&( Layers.Current_Layer () -> Vre ));
       Layers.Current_Layer () -> Vre = Vec ( 0 );
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

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Temperature_Solver::Assemble_Stiffness_Matrix"

void Temperature_Solver::Assemble_Stiffness_Matrix ( const double  Previous_Time,
                                                     const double  Current_Time,
                                                     const DM&     Temperature_FEM_Grid,
                                                     const Vec&    Temperature_DOFs,
                                                           Mat&    Stiffness_Matrix,
                                                           Vec&    Load_Vector,
                                                           double& Element_Contributions_Time ) {

  using namespace Basin_Modelling;

  PetscLogStages::push( PetscLogStages :: TEMPERATURE_SYSTEM_ASSEMBLY);

  const double Time_Step = Previous_Time - Current_Time;

  const int Plane_Quadrature_Degree = getPlaneQuadratureDegree ( Basin_Model -> Optimisation_Level );
  const int Depth_Quadrature_Degree = getDepthQuadratureDegree ( Basin_Model -> Optimisation_Level );

  const bool SteadyStateCalculation = ( Previous_Time == Current_Time );

  const bool IncludeAdvectiveTerm = Basin_Model->includeAdvectiveTerm and 
                                    not SteadyStateCalculation and
                                   ( Basin_Model->Do_Iteratively_Coupled or
                                     Basin_Model->IsCalculationCoupled );

  const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

  double Delta_X  = grid.deltaI;
  double Delta_Y  = grid.deltaJ;
  double Origin_X = grid.originI;
  double Origin_Y = grid.originJ;

  PetscLogDouble Element_Start_Time;
  PetscLogDouble Element_End_Time;

  int kkk;
  int Layer_K;
  int xs, ys, zs, xm, ym, zm;
  int layerMx, layerMy, layerMz;
  int gridMx, gridMy, gridMz;
  int k;
  int NumElement = 0;
  int NumCoincidentNodes = 0;
			    
  bool IncludeGhosts = true;

  MatStencil   col[8],row[8];
  MatStencil   a_diag[1];
  PetscScalar  OneDiagonal[1] = {1.0};
//    PetscScalar  EltRHS[8];
  int          *GlobalK;

  int    CollapsedNodes = 0;

  DMDAGetInfo( Temperature_FEM_Grid,0,&gridMx,&gridMy,&gridMz,0,0,0,0,0,0,0,0,0);

  PETSC_2D_Array MantleHeatFlow;

  if ( FastcauldronSimulator::getInstance ().getBottomBoundaryConditions () == MANTLE_HEAT_FLOW ) {

    MantleHeatFlow.Set_Global_Array ( *Basin_Model -> mapDA, Mantle_Heat_Flow,
				      INSERT_VALUES,IncludeGhosts );

  }


  PETSC_3D_Array rhsF( Temperature_FEM_Grid, Load_Vector, INSERT_VALUES, IncludeGhosts);

  PETSC_3D_Array dof( Temperature_FEM_Grid, Temperature_DOFs, INSERT_VALUES, IncludeGhosts);

  ElementGeometryMatrix Geometry_Matrix;

  ElementVector Current_Ph;
  ElementVector Current_Po;
  ElementVector Previous_Pp;
  ElementVector Current_Pp;
  ElementVector Previous_Lp;
  ElementVector Current_Lp;
  ElementVector Previous_VES;
  ElementVector Current_VES;
  ElementVector Previous_Max_VES;
  ElementVector Current_Max_VES;
  ElementVector Previous_Temperature;
  ElementVector Current_Temperature;
  ElementVector Previous_Chemical_Compaction;
  ElementVector Current_Chemical_Compaction;
  ElementVector Element_Heat_Production;

  ElementVector BC_Values;
  Boundary_Conditions Nodal_BCs[8];

  ElementMatrix Element_Stiffness_Matrix;
  ElementVector Element_Load_Vector;
  const CompoundLithology*  Element_Lithology;

  int NSegmZ;

  bool Is_Steady_State = false; // remove this soon
  bool Include_Chemical_Compaction;

  const double Constrained_Temp_Value = FastcauldronSimulator::getInstance ().getConstrainedBasaltTemperature();

  int NodeZDirCount = 0;

  Layer_Iterator Layers;
  Layers.Initialise_Iterator ( Basin_Model -> layers, Ascending, Basement_And_Sediments, 
			       Active_Layers_Only );

  Element_Contributions_Time = 0.0;

  int globalXNodes;
  int globalYNodes;
  if( Basin_Model -> isALC() ) {
     CrustFormation*   crustLayer = dynamic_cast<CrustFormation*>(Basin_Model -> Crust ());   
     MantleFormation*  mantleLayer = dynamic_cast<MantleFormation*>(Basin_Model -> Mantle ());
     crustLayer->cleanVectors();
     mantleLayer->cleanVectors();
     DMDAGetInfo( *Basin_Model -> mapDA, PETSC_NULL, &globalXNodes, &globalYNodes,
                  PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, 
                  PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL );
  }
  
  while ( ! Layers.Iteration_Is_Done () ) {

    LayerProps_Ptr Current_Layer = Layers.Current_Layer ();

    Current_Layer -> Previous_Properties.Activate_Property ( Basin_Modelling::Depth, INSERT_VALUES, IncludeGhosts );

    Include_Chemical_Compaction = (( Basin_Model -> Do_Chemical_Compaction ) && ( Current_Layer -> Get_Chemical_Compaction_Mode ()));

    NSegmZ = Current_Layer -> getNrOfActiveElements();
    
    string EventName = "setUpGM" + Current_Layer -> layername;

    GlobalK = new int[NSegmZ+1];

    for (kkk = 0; kkk <=NSegmZ; kkk++){
      GlobalK[kkk] = NodeZDirCount;
      NodeZDirCount++;
    }

    NodeZDirCount--;

    PETSC_3D_Array temp  ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Temperature ), INSERT_VALUES, IncludeGhosts );
    PETSC_3D_Array depth ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Depth       ), INSERT_VALUES, IncludeGhosts );

    PETSC_3D_Array bulkHeatProd( Current_Layer -> layerDA, 
				 Current_Layer -> BulkHeatProd,
				 INSERT_VALUES, IncludeGhosts );

    PETSC_3D_Array oldtemp (Current_Layer -> layerDA, 
				 Current_Layer -> Previous_Properties ( Basin_Modelling::Temperature ),
				 INSERT_VALUES, IncludeGhosts);


    PETSC_3D_Array Layer_Previous_Ph ( Current_Layer -> layerDA, 
                                       Current_Layer -> Previous_Properties ( Basin_Modelling::Hydrostatic_Pressure ),
                                       INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Previous_Pp ( Current_Layer -> layerDA, 
                                       Current_Layer -> Previous_Properties ( Basin_Modelling::Pore_Pressure ),
                                       INSERT_VALUES, IncludeGhosts );

    PETSC_3D_Array Layer_Previous_Lp ( Current_Layer -> layerDA, 
                                       Current_Layer -> Previous_Properties ( Basin_Modelling::Lithostatic_Pressure ),
        	                           INSERT_VALUES, IncludeGhosts);
									   
    PETSC_3D_Array Layer_Previous_Po ( Current_Layer -> layerDA, 
                                       Current_Layer -> Previous_Properties ( Basin_Modelling::Overpressure ),
                                       INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Previous_VES ( Current_Layer -> layerDA, 
                                        Current_Layer -> Previous_Properties ( Basin_Modelling::VES_FP ),
                                        INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Previous_Max_VES ( Current_Layer -> layerDA, 
                                            Current_Layer -> Previous_Properties ( Basin_Modelling::Max_VES ),
                                            INSERT_VALUES, IncludeGhosts);


    PETSC_3D_Array Layer_Current_Ph ( Current_Layer -> layerDA, 
                                      Current_Layer -> Current_Properties ( Basin_Modelling::Hydrostatic_Pressure ),
                                      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Current_Pp ( Current_Layer -> layerDA, 
                                      Current_Layer -> Current_Properties ( Basin_Modelling::Pore_Pressure ),
                                      INSERT_VALUES, IncludeGhosts );

     PETSC_3D_Array Layer_Current_Lp ( Current_Layer -> layerDA, 
                                       Current_Layer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure ),
                                       INSERT_VALUES, IncludeGhosts );

    PETSC_3D_Array Layer_Current_Po ( Current_Layer -> layerDA, 
                                      Current_Layer -> Current_Properties ( Basin_Modelling::Overpressure ),
                                      INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Current_VES ( Current_Layer -> layerDA, 
                                       Current_Layer -> Current_Properties ( Basin_Modelling::VES_FP ),
                                       INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Current_Max_VES ( Current_Layer -> layerDA, 
                                           Current_Layer -> Current_Properties ( Basin_Modelling::Max_VES ),
                                           INSERT_VALUES, IncludeGhosts);


    PETSC_3D_Array Layer_Previous_Chemical_Compaction ( Current_Layer -> layerDA, 
                                                        Current_Layer -> Previous_Properties ( Basin_Modelling::Chemical_Compaction ),
                                                        INSERT_VALUES, IncludeGhosts);

    PETSC_3D_Array Layer_Current_Chemical_Compaction ( Current_Layer -> layerDA, 
                                                       Current_Layer -> Current_Properties ( Basin_Modelling::Chemical_Compaction ),
                                                       INSERT_VALUES, IncludeGhosts);

    PETSC_2D_Array topBasaltDepth;
    PETSC_2D_Array bottomBasaltDepth;
    if( Basin_Model -> isALC() && Current_Layer -> isBasement() ) {
       CrustFormation*  crustLayer = dynamic_cast<CrustFormation*>(Basin_Model -> Crust ());
       topBasaltDepth.Set_Global_Array ( * Basin_Model ->mapDA, crustLayer -> TopBasaltDepth, INSERT_VALUES, IncludeGhosts );
       bottomBasaltDepth.Set_Global_Array ( * Basin_Model ->mapDA, crustLayer -> BottomBasaltDepth, INSERT_VALUES, IncludeGhosts );
    }

    DMDAGetInfo(Current_Layer -> layerDA,0,&layerMx,&layerMy,&layerMz,0,0,0,0,0,0,0,0,0);
    DMDAGetCorners(Current_Layer -> layerDA,&xs,&ys,&zs,&xm,&ym,&zm);

    for ( Layer_K = zs; Layer_K < zs+zm; Layer_K++ ) {

      if ( Layer_K != layerMz-1) {
	k = GlobalK[Layer_K];
	unsigned int EltCount;
        int Inode;

	for (EltCount=0; EltCount<Basin_Model -> mapElementList.size(); EltCount++) {
	  NumElement++;
	  CollapsedNodes = 0;

	  if (Basin_Model -> mapElementList[EltCount].exists) {

            Element_Lithology = Current_Layer->getLithology ( Basin_Model -> mapElementList[EltCount].i[ 0 ],
                                                              Basin_Model -> mapElementList[EltCount].j[ 0 ] );


	    for (Inode = 0; Inode<8; Inode++) {
	      int LidxZ = Layer_K + (Inode<4 ? 1 : 0);
	      int GidxZ = k + (Inode<4 ? 1 : 0);
	      int GidxY = Basin_Model -> mapElementList[EltCount].j[Inode%4];
	      int GidxX = Basin_Model -> mapElementList[EltCount].i[Inode%4];

              Nodal_BCs [ Inode ] = Interior_Boundary;

	      if ( Current_Layer -> isMantle() && GidxZ == 0 ) {

		if ( FastcauldronSimulator::getInstance ().getBottomBoundaryConditions () == MANTLE_HEAT_FLOW ) {
                  Nodal_BCs [Inode]  = Bottom_Boundary_Flux;
                  BC_Values ( Inode + 1 ) = MantleHeatFlow ( GidxY, GidxX );
		} else if ( FastcauldronSimulator::getInstance ().getBottomBoundaryConditions () == FIXED_BASEMENT_TEMPERATURE ||
                            FastcauldronSimulator::getInstance ().getBottomBoundaryConditions () == Interface::ADVANCED_LITHOSPHERE_CALCULATOR) {
                  Nodal_BCs [ Inode ] = Surface_Boundary;
                  BC_Values ( Inode + 1 ) = temp(LidxZ,GidxY,GidxX);
		}

	      } else if ( GidxZ == gridMz-1 ) {

                Nodal_BCs [ Inode ] = Surface_Boundary;
                BC_Values ( Inode + 1 ) = temp(LidxZ,GidxY,GidxX);
	      }

	      col[Inode].k = GidxZ; row[Inode].k = GidxZ;
	      col[Inode].j = GidxY; row[Inode].j = GidxY;
	      col[Inode].i = GidxX; row[Inode].i = GidxX;

	      if (GidxZ != dof(GidxZ,GidxY,GidxX)) {
		CollapsedNodes++;
		NumCoincidentNodes = max(NumCoincidentNodes,((int)dof(GidxZ,GidxY,GidxX)-GidxZ));

		col[Inode].k = (int) dof(GidxZ,GidxY,GidxX); row[Inode].k = col[Inode].k;
		a_diag[0].k = GidxZ;
                a_diag[0].j = GidxY;
                a_diag[0].i = GidxX;
		MatSetValuesStencil( Stiffness_Matrix,1,a_diag,1,a_diag,OneDiagonal,ADD_VALUES);
	      }

              Element_Heat_Production ( Inode + 1 ) =  bulkHeatProd(LidxZ,GidxY,GidxX);

              Geometry_Matrix ( 1, Inode + 1 ) = Origin_X + (Delta_X * GidxX);
              Geometry_Matrix ( 2, Inode + 1 ) = Origin_Y + (Delta_Y * GidxY);
              Geometry_Matrix ( 3, Inode + 1 ) = depth(LidxZ,GidxY,GidxX);

              Current_Ph ( Inode + 1 ) = Layer_Current_Ph ( LidxZ, GidxY, GidxX );
              Current_Po ( Inode + 1 ) = Layer_Current_Po ( LidxZ, GidxY, GidxX );
              Current_Pp ( Inode + 1 ) = Layer_Current_Pp ( LidxZ, GidxY, GidxX );
              Current_Lp ( Inode + 1 ) = Layer_Current_Lp ( LidxZ, GidxY, GidxX );
              Current_VES ( Inode + 1 ) = Layer_Current_VES ( LidxZ, GidxY, GidxX );
              Current_Max_VES ( Inode + 1 ) = Layer_Current_Max_VES ( LidxZ, GidxY, GidxX );

              Previous_Pp ( Inode + 1 ) = Layer_Previous_Pp ( LidxZ, GidxY, GidxX );
              Previous_Lp ( Inode + 1 ) = Layer_Previous_Lp ( LidxZ, GidxY, GidxX );
              Previous_VES ( Inode + 1 ) = Layer_Previous_VES ( LidxZ, GidxY, GidxX );
              Previous_Max_VES ( Inode + 1 ) = Layer_Previous_Max_VES ( LidxZ, GidxY, GidxX );

              Previous_Temperature ( Inode + 1 ) = oldtemp ( LidxZ, GidxY, GidxX );
              Current_Temperature  ( Inode + 1 ) = temp  ( LidxZ, GidxY, GidxX );

              Previous_Chemical_Compaction  ( Inode + 1 ) = Layer_Previous_Chemical_Compaction  ( LidxZ, GidxY, GidxX );
              Current_Chemical_Compaction  ( Inode + 1 ) = Layer_Current_Chemical_Compaction  ( LidxZ, GidxY, GidxX );
            }
            if( Current_Layer -> isBasement() && Basin_Model -> isALC() ) {
               ElementGeometryMatrix Geometry_Matrix1;

               for ( Inode = 0; Inode < 8; Inode ++ ) {
                  int LidxZ = Layer_K + (Inode < 4 ? 1 : 0);
                  int GidxY = Basin_Model -> mapElementList[EltCount].j[Inode%4];
                  int GidxX = Basin_Model -> mapElementList[EltCount].i[Inode%4];

                  Geometry_Matrix1 ( 3, Inode + 1 ) = depth( LidxZ, GidxY, GidxX ) - depth( zs + zm - 1, GidxY, GidxX );

                  if( Current_Layer -> isMantle() ) {
                     double Hfix  = FastcauldronSimulator::getInstance ().getLithosphereThicknessMod( GidxX, GidxY, Current_Time);
                     if ( Geometry_Matrix1 ( 3, Inode + 1 ) > Hfix ) {
                        Nodal_BCs [ Inode ] = Surface_Boundary;
                        BC_Values ( Inode + 1 ) = temp(0, GidxY,  GidxX);
                     }
                  }
               }
               double midPointDepth = Geometry_Matrix1.getMidPoint();

               int xY = Basin_Model -> mapElementList[EltCount].j[0];
               int xX = Basin_Model -> mapElementList[EltCount].i[0];

               Element_Lithology = Current_Layer -> getLithology( Current_Time, xX, xY, midPointDepth );

               if( Current_Layer->isBasalt() ) {
               
                  for ( Inode = 0; Inode < 8; Inode ++ ) {
                     int LidxZ = Layer_K + (Inode < 4 ? 1 : 0);
                     int GidxY = Basin_Model -> mapElementList[EltCount].j[Inode%4];
                     int GidxX = Basin_Model -> mapElementList[EltCount].i[Inode%4];
                     Current_Layer->setBasaltLitho ( GidxX, GidxY, LidxZ );
                     
                     if ( Inode > 3 ) {
                        if( bottomBasaltDepth( GidxY,  GidxX ) == CAULDRONIBSNULLVALUE ) {
                           bottomBasaltDepth( GidxY,  GidxX ) = depth(LidxZ,GidxY,GidxX); 
                        }
                     } else {
                        topBasaltDepth( GidxY, GidxX ) = depth(LidxZ,GidxY,GidxX); 
                     }
                     if( ! Basin_Model->bottomBasaltTemp ) {
                        if( !Current_Layer->getPreviousBasaltLitho( GidxX, GidxY, LidxZ ) && 
                            Current_Time > FastcauldronSimulator::getInstance ().getEndOfRiftEvent( xX, xY )) {
                           if( Current_Layer->isMantle() || 
                               ( Current_Layer->isCrust() && ( Geometry_Matrix1 ( 3, Inode + 1 ) > FastcauldronSimulator::getInstance ().getMinimumHCBL() ))) {
                              Nodal_BCs [ Inode ] = Interior_Constrained_Temperature; 
                              BC_Values ( Inode + 1 ) = Constrained_Temp_Value;
                           } 
                        }
                     } else {
                        // set constraied temperature only for the bottom basalt element
                        if(( bottomBasaltDepth( GidxY, GidxX ) == CAULDRONIBSNULLVALUE ) && 
                           ( FastcauldronSimulator::getInstance ().getBasaltThickness( GidxX, GidxY, Current_Time ) > 
                             FastcauldronSimulator::getInstance ().getBasaltThickness( GidxX, GidxY, Previous_Time )) &&
                           Current_Time >  FastcauldronSimulator::getInstance ().getEndOfRiftEvent( xX, xY )) {
                           Nodal_BCs [ Inode ] = Interior_Constrained_Temperature; 
                           BC_Values ( Inode + 1 ) = Constrained_Temp_Value;
                        }
                     }

                   
                  }
               }
            }
            PetscTime(&Element_Start_Time);
            Assemble_Element_Temperature_Stiffness_Matrix ( Current_Layer->kind() == Interface::BASEMENT_FORMATION,
                                                            Plane_Quadrature_Degree, Depth_Quadrature_Degree,
                                                            Current_Time, Time_Step, Is_Steady_State, 
                                                            IncludeAdvectiveTerm,
                                                            Nodal_BCs, BC_Values,
                                                            Element_Lithology,
                                                            Current_Layer->fluid,
                                                            Include_Chemical_Compaction,
                                                            Geometry_Matrix,
                                                            Element_Heat_Production,
                                                            Current_Ph,
                                                            Current_Po,
                                                            Previous_Pp,
                                                            Current_Pp,
                                                            Previous_Lp,
                                                            Current_Lp,
                                                            Previous_VES,
                                                            Current_VES,
                                                            Previous_Max_VES,
                                                            Current_Max_VES,
                                                            Previous_Temperature,
                                                            Current_Temperature,
                                                            Previous_Chemical_Compaction,
                                                            Current_Chemical_Compaction,
                                                            Element_Stiffness_Matrix,
                                                            Element_Load_Vector );

            PetscTime(&Element_End_Time);
            Element_Contributions_Time = Element_Contributions_Time + Element_End_Time - Element_Start_Time;

	  } else {
	    for (Inode = 0; Inode<8; Inode++) {
	      int GidxZ = k + (Inode<4 ? 0 : 1);
	      int GidxY = Basin_Model -> mapElementList[EltCount].j[Inode%4];
	      int GidxX = Basin_Model -> mapElementList[EltCount].i[Inode%4];
	      if ( not Basin_Model->nodeIsDefined( GidxX, GidxY )) {
		a_diag[0].k = GidxZ; a_diag[0].j = GidxY; a_diag[0].i = GidxX; 
		MatSetValuesStencil( Stiffness_Matrix,1,a_diag,1,a_diag,OneDiagonal,ADD_VALUES);
	      }  
	    }
	    continue;
	  }

	  MatSetValuesStencil( Stiffness_Matrix,8,row,8,col, Element_Stiffness_Matrix.C_Array (),ADD_VALUES);

	  for (Inode = 0; Inode<8; Inode++) {
	    int irow = row[Inode].i;
	    int jrow = row[Inode].j;
	    int krow = row[Inode].k;
            rhsF(krow,jrow,irow) = rhsF(krow,jrow,irow) + Element_Load_Vector ( Inode + 1 );
	  }

	}

      }

    }

    delete[] GlobalK;

    if( Basin_Model -> isALC() && Current_Layer -> isBasement() ) {
       topBasaltDepth.Restore_Global_Array( Update_Excluding_Ghosts );  
       bottomBasaltDepth.Restore_Global_Array( Update_Excluding_Ghosts ); 
    }
    Current_Layer -> Previous_Properties.Restore_Property ( Basin_Modelling::Depth );

    Layers++;
  }

  rhsF.Restore_Global_Array( Update_Including_Ghosts );  

  MatAssemblyBegin( Stiffness_Matrix,MAT_FINAL_ASSEMBLY);
  MatAssemblyEnd( Stiffness_Matrix,MAT_FINAL_ASSEMBLY);

  PetscLogStages::pop();
}

void Temperature_Solver::writeFissionTrackResultsToDatabase(void)
{
   Basin_Model->writeFissionTrackResultsToDatabase(m_FissionTrackCalculator);


}




