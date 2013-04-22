#include "Well.h"

#include <string>
using namespace std;

#include "utils.h"
#include "GeoPhysicsFluidType.h"
#include "HydraulicFracturingManager.h"

#include "CompoundProperty.h"

#include "FastcauldronSimulator.h"

Well::Well ( AppCtx* Application_Context ) {

  Basin_Model = Application_Context;

  if ( Basin_Model->DoHighResDecompaction && ! Basin_Model->Do_Iteratively_Coupled ) {
    calculationModeFileNameExtension = "HRDecompaction";
  } else if ( Basin_Model->DoHighResDecompaction && Basin_Model->Do_Iteratively_Coupled ) {
    calculationModeFileNameExtension = "LCHRDecompaction";
  } else if ( Basin_Model->DoDecompaction ) {
    calculationModeFileNameExtension = "Decompaction";
  } else if ( Basin_Model->DoOverPressure && ! Basin_Model->Do_Iteratively_Coupled ) {
    calculationModeFileNameExtension = "LCPressure";
  } else if ( Basin_Model->DoTemperature && ! Basin_Model->IsCalculationCoupled && ! Basin_Model->Do_Iteratively_Coupled ) {
    calculationModeFileNameExtension = "HTemperature";
  } else if ( Basin_Model->DoTemperature && Basin_Model->IsCalculationCoupled && ! Basin_Model->Do_Iteratively_Coupled ) {
    calculationModeFileNameExtension = "LCTemperature";
  } else if ( Basin_Model->Do_Iteratively_Coupled ) {
    calculationModeFileNameExtension = "PTCoupled";
  }

}

Well::~Well () {

}

void Well::Save_Present_Day_Data ()
{

  string Related_Project_Name;
  size_t indx;

  for ( indx = 0; indx < Basin_Model -> Related_Projects.size (); indx++ )
  {
    Location needle;
    
    needle.X_Coord = Basin_Model -> Related_Projects [ indx ] -> X_Coord;
    needle.Y_Coord = Basin_Model -> Related_Projects [ indx ] -> Y_Coord;

    Related_Project_Name =  Basin_Model -> getOutputDirectory () + 
                            Basin_Model -> Related_Projects [ indx ] -> Name;

    Locate_Well( needle);

    Save_Well_Data ( needle, Related_Project_Name );

    MPI_Barrier(PETSC_COMM_WORLD);

  }

}

void Well::Locate_Well ( Location& needle ) {

   const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

  const double Origin_X = grid.originI;
  const double Origin_Y = grid.originJ;
  const int    Number_Of_X_Nodes = grid.nrI;
  const int    Number_Of_Y_Nodes = grid.nrJ;
  const double Delta_X = grid.deltaI;
  const double Delta_Y = grid.deltaJ;

  double X_Fraction;
  double Y_Fraction;



  if (( needle.X_Coord < Origin_X || Origin_X + Delta_X * Number_Of_X_Nodes < needle.X_Coord ) ||
      ( needle.Y_Coord < Origin_Y || Origin_Y + Delta_Y * Number_Of_Y_Nodes < needle.Y_Coord )) {

    // Outside the domain.

    needle.In_Processor_Range = false;
    needle.Exists    = false;
    return;
  }
      
  needle.X_Position = int (( needle.X_Coord - Origin_X ) / Delta_X );
  needle.Y_Position = int (( needle.Y_Coord - Origin_Y ) / Delta_Y );


  if ( needle.X_Position == Number_Of_X_Nodes - 1 ) 
  {
    needle.X_Step = 0;
  } 
  else 
  {
    needle.X_Step = 1;
  } 

  if ( needle.Y_Position == Number_Of_Y_Nodes - 1 ) 
  {
    needle.Y_Step = 0;
  } 
  else 
  {
    needle.Y_Step = 1;
  } 

#if 0
  DMDALocalInfo dainfo;
  int xMin;
  int xMax;
  int gxMin;
  int gxMax;

  int yMin;
  int yMax;
  int gyMin;
  int gyMax;


  DMDAGetLocalInfo( *Basin_Model -> mapDA, &dainfo );

  gxMin = dainfo.gxs;
  gxMax = dainfo.gxs + ( dainfo.gxm - 1 );
  gyMin = dainfo.gys;
  gyMax = dainfo.gys + ( dainfo.gym - 1 );
  xMin = dainfo.xs;
  xMax = dainfo.xs + ( dainfo.xm - 1 );
  yMin = dainfo.ys;
  yMax = dainfo.ys + ( dainfo.ym - 1 );
#endif

  needle.In_Processor_Range = false;
  needle.Exists    = false;

  needle.In_Processor_Range = Basin_Model -> In_Processor_Range ( needle.X_Position, needle.Y_Position );

  if ( needle.In_Processor_Range )
  {

    needle.Exists = (( Basin_Model->nodeIsDefined ( needle.X_Position,                 needle.Y_Position                 )) &&
                     ( Basin_Model->nodeIsDefined ( needle.X_Position,                 needle.Y_Position + needle.Y_Step )) &&
                     ( Basin_Model->nodeIsDefined ( needle.X_Position + needle.X_Step, needle.Y_Position + needle.Y_Step )) &&
                     ( Basin_Model->nodeIsDefined ( needle.X_Position + needle.X_Step, needle.Y_Position                 )));

    if ( needle.Exists )
    {
      X_Fraction = ( needle.X_Coord - ( Origin_X + needle.X_Position * Delta_X ) ) / Delta_X;
      Y_Fraction = ( needle.Y_Coord - ( Origin_Y + needle.Y_Position * Delta_Y ) ) / Delta_Y;
      
      needle.Fractions [ 0 ] = ( 1.0 - X_Fraction ) * ( 1.0 - Y_Fraction );
      needle.Fractions [ 1 ] = (       X_Fraction ) * ( 1.0 - Y_Fraction );
      needle.Fractions [ 2 ] = (       X_Fraction ) * (       Y_Fraction );
      needle.Fractions [ 3 ] = ( 1.0 - X_Fraction ) * (       Y_Fraction );
    }

  }

  MPI_Barrier(PETSC_COMM_WORLD);

}

PETSC_3D_Array* Well::getPropertyVector ( DM  layerDA,
                                          Vec propertyVector ) const {

  const bool IncludeGhosts = true;
  PETSC_3D_Array* array;

  PetscBool validVector;

  VecValid ( propertyVector, &validVector );

  if ( validVector ) {
    array = new PETSC_3D_Array ( layerDA, propertyVector, INSERT_VALUES, IncludeGhosts );
  } else {
    array = 0;
  }

  return array;
}


void Well::Save_Well_Data( Location& needle, const string& Related_Project_Name )
{
  
  int  K;
  int  xs, ys, zs, xm, ym, zm;
  
  int  Top_Z_Node_Index;
  int  Number_Of_Segment;
  int  Number_Of_Formation_Layers = Basin_Model -> layers.size() - 2;
  int  Layer_It;

  bool IncludeGhosts = true;
  bool includeChemicalCompactionTerm;

  ofstream    Well_Data_File;
  LayerProps* currentLayer;

  double seaTemperature;
  double nodeVRe;
  double normalPerm;
  double tangentialPerm;

  double nodeNormalPermeability;
  double nodeTangentialPermeability;
  double nodeThermalConductivityNormal;
  double nodeThermalConductivityTangential;
  double nodeTemperature;
  double nodeHydrostaticPressure;
  double nodeOverPressure;
  double nodePorePressure;
  double nodeLithostaticPressure;
  double nodeFracturePressure;
  double nodeVes;
  double nodeDepth;
  double nodeMaxVes;
  double nodeBulkDensity;
  double fluidDensity;
  double fluidViscosity;
  double chemicalCompactionTerm;

  GeoPhysics::CompoundProperty nodePorosity;

  if ( needle.Exists )
  {
    string Well_Data_File_Name = Related_Project_Name + '_' + calculationModeFileNameExtension + ".well";
    Well_Data_File.open( Well_Data_File_Name.c_str() );
    
    Well_Data_File.precision ( 10 );
    
    Well_Data_File << FastcauldronSimulator::getInstance ().getProjectName () << endl
                   << setw ( width ) << "X_m"
                   << setw ( width ) << "Y_m"
                   << setw ( width ) << "Depth_m"
                   << setw ( width ) << "Hydrostatic_MPa" 
                   << setw ( width ) << "Overpressure_MPa"
                   << setw ( width ) << "Pressure_MPa" 
                   << setw ( width ) << "Lithostatic_MPa"
                   << setw ( width ) << "FracPres_MPa"
		   << setw ( width ) << "VES_MPa"
                   << setw ( width ) << "MaxVES_MPa"
                   << setw ( width ) << "Porosity"
                   << setw ( width ) << "Kv_log10_mD"
                   << setw ( width ) << "Kh_log10_mD"
                   << setw ( width ) << "ThCondv_W/m/K"
                   << setw ( width ) << "ThCondh_W/m/K"
		   << setw ( width ) << "Temperature_C" 
		   << setw ( width ) << "VRe_%Ro" 
		   << setw ( width ) << "BulkDensity_kg/m3" 
		   << setw ( width ) << "Viscosity_Pa_s" 
		   << setw ( width + 1 ) << "Water_Density_Kg/m^3" 
                   << endl;

  }

  for ( Layer_It = 0; Layer_It < Number_Of_Formation_Layers; Layer_It++ ) {
    
    currentLayer = Basin_Model -> layers[ Layer_It ];
    
    includeChemicalCompactionTerm = Basin_Model->Do_Chemical_Compaction && currentLayer->Get_Chemical_Compaction_Mode ();
    
    Number_Of_Segment = currentLayer->getMaximumNumberOfElements ();
    
    if ( Number_Of_Segment <= 0 ) continue;

    DMDAGetCorners( currentLayer->layerDA, &xs, &ys, &zs, &xm, &ym, &zm ); 

    PETSC_3D_Array depth              ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Depth ),                INSERT_VALUES, IncludeGhosts );
    PETSC_3D_Array ves                ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::VES_FP ),               INSERT_VALUES, IncludeGhosts );
    PETSC_3D_Array maxves             ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Max_VES ),              INSERT_VALUES, IncludeGhosts );
    PETSC_3D_Array hydrostatic        ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Hydrostatic_Pressure ), INSERT_VALUES, IncludeGhosts );
    PETSC_3D_Array overpressure       ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Overpressure ),         INSERT_VALUES, IncludeGhosts );
    PETSC_3D_Array pressure           ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Pore_Pressure ),        INSERT_VALUES, IncludeGhosts );
    PETSC_3D_Array lithostatic        ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Lithostatic_Pressure ), INSERT_VALUES, IncludeGhosts );
    PETSC_3D_Array temperature        ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Temperature ),          INSERT_VALUES, IncludeGhosts );
    PETSC_3D_Array chemicalCompaction ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Chemical_Compaction ),  INSERT_VALUES, IncludeGhosts );

    PETSC_3D_Array* vre         = getPropertyVector ( currentLayer->layerDA, currentLayer->Vre );

    if ( needle.Exists )
    {
      const CompoundLithology* currentLithology = currentLayer->getLithology ( needle.X_Position, needle.Y_Position );
      const FluidType*   currentFluid     = currentLayer->fluid;
      Top_Z_Node_Index = zm - 1;

      for ( K = Top_Z_Node_Index; K >= 0; K-- ) {

	needle.Z_Position = K;

        if ( vre == 0 ) {
          nodeVRe = IBSNULLVALUE;
        } else {
          nodeVRe = Get_Well_Interpolated_Value( needle, *vre );
        }

        nodeDepth               = Get_Well_Interpolated_Value( needle, depth );
        nodeVes                 = Get_Well_Interpolated_Value( needle, ves );
        nodeMaxVes              = Get_Well_Interpolated_Value( needle, maxves );
        chemicalCompactionTerm  = Get_Well_Interpolated_Value( needle, chemicalCompaction );
        nodeHydrostaticPressure = Get_Well_Interpolated_Value( needle, hydrostatic );
        nodeOverPressure        = Get_Well_Interpolated_Value( needle, overpressure );
        nodePorePressure        = Get_Well_Interpolated_Value( needle, pressure );
        nodeLithostaticPressure = Get_Well_Interpolated_Value( needle, lithostatic );

        seaTemperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( needle.X_Position, needle.Y_Position, 0.0 );

        if ( HydraulicFracturingManager::getInstance ().fracturePressureSelection () == Interface::None ) {
          nodeFracturePressure    = IBSNULLVALUE;
        } else {
          nodeFracturePressure    = HydraulicFracturingManager::getInstance ().fracturePressure ( currentLithology,
                                                                                                  currentFluid,
                                                                                                  seaTemperature,
                                                                                                  surfaceDepth ( needle ),
                                                                                                  nodeDepth,
                                                                                                  nodeHydrostaticPressure,
                                                                                                  nodeLithostaticPressure );
        }


        currentLithology->getPorosity ( nodeVes, nodeMaxVes, includeChemicalCompactionTerm, chemicalCompactionTerm, nodePorosity );

        currentLithology->calcBulkPermeabilityNP ( nodeVes,
                                                   nodeMaxVes,
                                                   nodePorosity,
                                                   normalPerm,
                                                   tangentialPerm );

        nodeNormalPermeability     = normalPerm / MILLIDARCYTOM2;
        nodeTangentialPermeability = tangentialPerm / MILLIDARCYTOM2;            

        if ( Basin_Model->DoDecompaction ) {
          nodeTemperature = IBSNULLVALUE;
          fluidDensity = currentLayer->fluid->getConstantDensity ();
          fluidViscosity = IBSNULLVALUE;
          nodeThermalConductivityNormal = IBSNULLVALUE;
          nodeThermalConductivityTangential = IBSNULLVALUE;
        } else {
          nodeTemperature = Get_Well_Interpolated_Value( needle, temperature );
          currentLithology -> calcBulkThermCondNP ( currentLayer->fluid,
                                                    nodePorosity.mixedProperty (),
                                                    nodeTemperature,
                                                    nodeThermalConductivityNormal,
                                                    nodeThermalConductivityTangential );

          fluidViscosity = currentLayer->fluid->viscosity ( nodeTemperature );
          fluidDensity = currentLayer->fluid->density ( nodeTemperature, Get_Well_Interpolated_Value ( needle, pressure ));
        }

        nodeBulkDensity = nodePorosity.mixedProperty () * fluidDensity + ( 1.0 - nodePorosity.mixedProperty ()) * currentLithology->density ();

	Well_Data_File << setw ( width ) << needle.X_Coord 
		       << setw ( width ) << needle.Y_Coord 
		       << setw ( width ) << nodeDepth
		       << setw ( width ) << nodeHydrostaticPressure
		       << setw ( width ) << nodeOverPressure
		       << setw ( width ) << nodePorePressure
		       << setw ( width ) << nodeLithostaticPressure
		       << setw ( width ) << nodeFracturePressure
		       << setw ( width ) << nodeVes * Pa_To_MPa
		       << setw ( width ) << nodeMaxVes * Pa_To_MPa
		       << setw ( width ) << nodePorosity.mixedProperty ()
		       << setw ( width ) << log10 ( nodeNormalPermeability )
		       << setw ( width ) << log10 ( nodeTangentialPermeability )
                       << setw ( width ) << nodeThermalConductivityNormal
                       << setw ( width ) << nodeThermalConductivityTangential
		       << setw ( width ) << nodeTemperature
		       << setw ( width ) << nodeVRe
		       << setw ( width ) << nodeBulkDensity
		       << setw ( width ) << fluidViscosity
		       << setw ( width ) << fluidDensity
                       << endl;

      }
      
    }

    if ( vre != 0 ) {
      delete vre;
      vre = 0;
    }

  }

  if ( needle.Exists )
  {
  Well_Data_File.close();
  }

}

double Well::interpolateValue ( const Location& needle,
                                const double*   values ) const {

  int        Node_Count;
  int        Divisor = 0;
  double     Sum     = 0.0;


  for ( Node_Count = 0; Node_Count < 4; Node_Count++ )
  {
    if ( values [ Node_Count ] != IBSNULLVALUE && values [ Node_Count ] != CAULDRONIBSNULLVALUE   ) 
    {
      Sum = Sum + values [ Node_Count ] * needle.Fractions [ Node_Count ];
      Divisor = Divisor + 1;
    }
  }

  if ( Divisor == 4 ) 
  {
    return Sum;
  }
  else
  {
    return IBSNULLVALUE;
  }


}

double Well::surfaceDepth ( const Location& needle ) const {

  const double presentDay = 0.0;

  double     Values [4];


  Values [ 0 ] = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( needle.X_Position,                 needle.Y_Position,                 presentDay );
  Values [ 1 ] = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( needle.X_Position + needle.X_Step, needle.Y_Position,                 presentDay );
  Values [ 2 ] = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( needle.X_Position + needle.X_Step, needle.Y_Position + needle.Y_Step, presentDay );
  Values [ 3 ] = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( needle.X_Position,                 needle.Y_Position + needle.Y_Step, presentDay );

  return interpolateValue ( needle, Values );

}

double Well::Get_Well_Interpolated_Value( const Location& needle, PETSC_3D_Array& property ) const {

  double     Values [4];

  Values [ 0 ] = property( needle.Z_Position, needle.Y_Position, needle.X_Position );
  Values [ 1 ] = property( needle.Z_Position, needle.Y_Position, needle.X_Position + needle.X_Step );
  Values [ 2 ] = property( needle.Z_Position, needle.Y_Position + needle.Y_Step , needle.X_Position  + needle.X_Step );
  Values [ 3 ] = property( needle.Z_Position, needle.Y_Position + needle.Y_Step , needle.X_Position );

  return interpolateValue ( needle, Values );

}
