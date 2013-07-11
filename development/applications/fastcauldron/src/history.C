#include "history.h"
#include "globaldefs.h"
#include "utils.h"
#include <math.h>

#include "database.h"
#include "cauldronschemafuncs.h"

#include "FastcauldronSimulator.h"

using namespace database;

History::History( AppCtx *appl ){
  
  appctx  = appl;

  if ( appctx->DoHighResDecompaction && ! appctx->Do_Iteratively_Coupled ) {
    calculationModeFileNameExtension = "HRDecompaction";
  } else if ( appctx->DoHighResDecompaction && appctx->Do_Iteratively_Coupled ) {
    calculationModeFileNameExtension = "LCHRDecompaction";
  } else if ( appctx->DoDecompaction ) {
    calculationModeFileNameExtension = "Decompaction";
  } else if ( appctx->DoOverPressure && ! appctx->Do_Iteratively_Coupled ) {
    calculationModeFileNameExtension = "LCPressure";
  } else if ( appctx->DoTemperature && ! appctx->IsCalculationCoupled && ! appctx->Do_Iteratively_Coupled ) {
    calculationModeFileNameExtension = "HTemperature";
  } else if ( appctx->DoTemperature && appctx->IsCalculationCoupled  && ! appctx->Do_Iteratively_Coupled ) {
    calculationModeFileNameExtension = "LCTemperature";
  } else if ( appctx->Do_Iteratively_Coupled ) {
    calculationModeFileNameExtension = "PTCoupled";
  }

  Property_List.push_back ( DEPTH );
  Property_List.push_back ( HYDROSTATICPRESSURE );
  Property_List.push_back ( OVERPRESSURE );
  Property_List.push_back ( PRESSURE );
  Property_List.push_back ( LITHOSTATICPRESSURE );
  Property_List.push_back ( VES );
  Property_List.push_back ( MAXVES );
  Property_List.push_back ( POROSITYVEC );
  Property_List.push_back ( PERMEABILITYVEC );
  Property_List.push_back ( PERMEABILITYHVEC );
  Property_List.push_back ( THCONDVEC );
  Property_List.push_back ( TEMPERATURE );
  Property_List.push_back ( BULKDENSITYVEC );
  Property_List.push_back ( VR );

  Generate_Log_File_Header ();
  
}

Node_Info::~Node_Info(){

  HistoryProperties::iterator p_vec_it;

  for ( p_vec_it = Properties.begin (); p_vec_it != Properties.end (); p_vec_it++ ) {

    delete p_vec_it -> second;

  }

  Properties.clear();

}

Surface_Info::~Surface_Info(){

  unsigned int indx;

  for ( indx = 0; indx < Nodes.size(); indx++ ) {
    delete Nodes [ indx ];
  }
  Nodes.clear();

}

History::~History(){
  
  if ( Property_List.empty() != true ) {

    Property_List.clear();

  }

  if ( Time_Stamps.empty() != true ) {

    Time_Stamps.clear();

  }

  for ( surface_it = Surfaces.begin (); surface_it != Surfaces.end (); surface_it++ ) {

    delete surface_it -> second;

  }

  Surfaces.clear();

}
 
void History::Add_Time ( const double time ) {

  if ( ! IsDefined ()) {
    return;
  }

  current_time = time;
  Time_Stamps.insert( time );

  for ( r_surface_it = Surfaces.rbegin (); r_surface_it != Surfaces.rend (); r_surface_it++ ) {

     if ( time <= r_surface_it -> first or ( time != 0.0 and std::fabs ( time - r_surface_it -> first ) / time < DefaultAgeTolerance )) {
        Record_Properties ( r_surface_it -> second );
     }

  }

}

void History::Record_Properties ( Surface_Info* surface ) {

  PETSC_ASSERT ( surface -> Layer_Index );

  LayerProps * Layer_Below_Surface = appctx -> layers [ surface -> Layer_Index ];
  LayerProps * Layer_Above_Surface = appctx -> layers [ surface -> Layer_Index - 1];
  
  const int Top_K = Layer_Below_Surface -> getNrOfActiveElements();
  const int Bot_K = 0;

  property_it = Property_List.begin();

  do {

    string Property_Name = appctx -> timefilter.getPropertyName( *property_it );

    bool Property_Is_A_Vector = ( ( *property_it ) < DEPTH );

    Save_Property ( Property_Name, *property_it, Layer_Below_Surface, Top_K, surface -> Nodes );

    if ( Property_Is_A_Vector ) {

      Property_Name = Property_Name + "_Above";

      if ( not Layer_Above_Surface -> isActive() ) {

	Save_Depositional_Property ( Property_Name, *property_it, Layer_Above_Surface, surface -> Nodes );

      } else {

	Save_Property ( Property_Name, *property_it, Layer_Above_Surface, Bot_K, surface -> Nodes );

      }

    }

    ++property_it;
    
  } while ( property_it != Property_List.end() );
  
}

void History::Save_Property ( const string &   property_name,
                              const PropertyList propertyId,
			      LayerProps *     layer,
			      const int        K,
			      Node_Container & nodes ) {
  
  Node_Info *  Current_Node;

  bool         IncludeGhosts = true;

  Vec*         Current_Vector = layer -> vectorList.VecArray[ propertyId ];

  unsigned int          indx;

  double       Property_Value = 0.0;
  
  if ( Current_Vector != Null ) {
     PetscBool validVector;

     VecValid ( *Current_Vector, &validVector );

    if ( validVector && appctx->timefilter.propertyIsSelected ( propertyId )) {
      PETSC_3D_Array buffer( layer -> layerDA, *Current_Vector, INSERT_VALUES, IncludeGhosts );

      for ( indx = 0; indx < nodes.size(); indx++ ) {
      
        Current_Node = nodes [ indx ];
        Current_Node -> K_Index = K;
        Property_Value = Calculate_Interpolated_Value( Current_Node, buffer );
        Save_Property_Value_At_Node ( Current_Node, Property_Value, property_name );
      }

    } else {

      for ( indx = 0; indx < nodes.size(); indx++ ) {
        Save_Property_Value_At_Node ( nodes [ indx ], IBSNULLVALUE, property_name );
      }

    }


  } else {
    
    for ( indx = 0; indx < nodes.size(); indx++ ) {
      
      Current_Node = nodes [ indx ];

      Save_Property_Value_At_Node ( Current_Node, IBSNULLVALUE, property_name );

    }
    
  }
  
}

void History::Save_Property_Value_At_Node ( Node_Info *    node,
					    double         Property_Value,
					    const string & Property_Name ) {

  Double_Vector * pValues;
  
  prop_vec_it = node -> Properties.find( Property_Name );

  if ( prop_vec_it == node -> Properties.end() ) {
  
    pValues = new Double_Vector;
    node -> Properties[ Property_Name ] = pValues;

  } else {

    pValues = prop_vec_it -> second;

  }
  
  if ( Property_Value != IBSNULLVALUE && (( *property_it ) == VES || ( *property_it ) == MAXVES )) {

    Property_Value = Property_Value * Pa_To_MPa;

  }

  if (  Property_Value != IBSNULLVALUE && (( *property_it ) == PERMEABILITYVEC || ( *property_it ) == PERMEABILITYHVEC )) {
    
    Property_Value = log10 ( Property_Value );
    
  }

  pValues -> push_back ( Property_Value );

}

void History::Output_Properties () {

  if ( ! IsDefined ()) {
    return;
  }

  Surface_Info  * Current_Surface;
  Node_Info     * Current_Node;
  Double_Vector * pValues;
  int             Time_Count;
  double          Depo_Age;
  string          Property_Name;

  for ( surface_it = Surfaces.begin(); surface_it != Surfaces.end(); surface_it++ ) {

    Depo_Age        = surface_it -> first;
    Current_Surface = surface_it -> second;

    unsigned int indx;

    for ( indx = 0; indx < Current_Surface -> Nodes.size(); indx++ ) {

      Current_Node = Current_Surface -> Nodes [ indx ];

      ofstream History_Data_File;
      string   History_Data_File_Name = appctx->getOutputDirectory ();

      History_Data_File_Name = History_Data_File_Name + removeNonUsableCharacters ( surface_it -> second -> Name ) + "_"
	+ IntegerToString( int ( Current_Node->X_Coord ) ) + "_east_" + IntegerToString( int ( Current_Node->Y_Coord )) + "_north_"
        + calculationModeFileNameExtension + ".hist";
      History_Data_File.open( History_Data_File_Name.c_str() );

      createLogFileHeader ( History_Data_File );
    
//        History_Data_File << Log_File_Header;

      Time_Count = 0;
      dble_it = findTimeStamp ( Depo_Age );

      // Should this be Time_Stamps.begin ()? Perhaps Depo_Age is the first value in the vector.
      for ( dble_it = findTimeStamp ( Depo_Age ); dble_it != Time_Stamps.end(); dble_it-- ) {
	
	History_Data_File << setw( width ) << *dble_it; 
	History_Data_File << setw( width ) << Current_Node->X_Coord; 
	History_Data_File << setw( width ) << Current_Node->Y_Coord; 

	property_it = Property_List.begin();
	
	do {
	  
	  Property_Name = appctx -> timefilter.getPropertyName( *property_it );

	  bool  Property_Is_A_Vector = ( ( *property_it ) < DEPTH );

	  prop_vec_it = Current_Node -> Properties.find( Property_Name );
	  pValues = prop_vec_it -> second;
	  
	  History_Data_File << setw( width ) << ( * pValues ) [ Time_Count ];

	  if ( Property_Is_A_Vector ) {

	    Property_Name = Property_Name + "_Above";
	    prop_vec_it = Current_Node -> Properties.find( Property_Name );
	    pValues = prop_vec_it -> second;
	    
	    History_Data_File << setw( width ) << ( * pValues ) [ Time_Count ];	    
	    
	  }
	  
	  ++property_it;
	  
	} while ( property_it != Property_List.end() );

	++Time_Count;	  
	History_Data_File << endl;

      }

      History_Data_File.close();

    }

  }

}

void History::clearProperties () {

  if ( ! IsDefined ()) {
    return;
  }

  unsigned int indx;
  SurfaceManager::iterator surfaceIter;
  Surface_Info*  currentSurface;

  Time_Stamps.clear ();

  for ( surfaceIter = Surfaces.begin(); surfaceIter != Surfaces.end(); ++surfaceIter ) {
    currentSurface = surfaceIter -> second;

    for ( indx = 0; indx < currentSurface -> Nodes.size(); indx++ ) {
      currentSurface->Nodes[indx]->Properties.clear ();
    }

  }

}

void History::Read_Spec_File () {

  Has_Nodes = false;


  string Spec_File_Name = FastcauldronSimulator::getInstance ().getProjectName ();

  // string Spec_File_Name = appctx -> getProjectFileName ();
  removeExtension ( Spec_File_Name );
  Spec_File_Name += ".hist";

  if ( File_Exists( Spec_File_Name.c_str() ) ) {

    ifstream Spec_File;
    Spec_File.open( Spec_File_Name.c_str() );
    
    double Deposition_Age;
    double X_Coordinate, Y_Coordinate;
    int    Layer_Index;
    SurfaceManager::iterator surfaceIt;    

    while ( Spec_File.good() ) {
      
      Surface_Info * pSurface_Info = NULL;
      Node_Info *    pNode_Info    = NULL;

      Spec_File >> Deposition_Age >> X_Coordinate >> Y_Coordinate;

      if ( !Spec_File.eof()) {

        Layer_Index = appctx -> Find_Layer_Index ( Deposition_Age );

        if ( 0 <= Layer_Index && Layer_Index <= static_cast<int>(appctx -> layers.size ()) - 3 ) {

          Locate_Point ( X_Coordinate, Y_Coordinate, pNode_Info );

          surfaceIt = findSurface ( Deposition_Age );

          if ( surfaceIt == Surfaces.end () ) {

            pSurface_Info         = new Surface_Info;
            LayerProps* pLayer    = appctx -> layers [ static_cast<unsigned int>(Layer_Index) ];
            pSurface_Info -> Layer_Index = int ( Layer_Index );
            pSurface_Info -> Name = pLayer -> TopSurfaceName;
            //pSurface_Info -> Nodes.push_back( pNode_Info ); 
            
            Surfaces [ Deposition_Age ] = pSurface_Info;
          
          } else {
          
            pSurface_Info = surfaceIt -> second;
            //pSurface_Info -> Nodes.push_back( pNode_Info );
          
          }

          if ( pNode_Info != NULL ) {
            pSurface_Info -> Nodes.push_back( pNode_Info );
          }

        }

      }

    }
    
    Spec_File.close();

    Has_Nodes = true;

  }

}

SurfaceManager::iterator History::findSurface ( const double age,
                                                const double relativeTolerance ) {

   SurfaceManager::iterator surface;

   for ( surface = Surfaces.begin (); surface != Surfaces.end (); ++surface ) {

      if (( age != 0.0 and ( std::fabs ( age - surface->first ) / age ) < relativeTolerance ) or
          ( age == 0.0 and ( std::fabs ( age - surface->first )) < relativeTolerance )) {
         return surface;
      }

   }

   return Surfaces.end ();
}

Double_Container::iterator History::findTimeStamp ( const double age,
                                                    const double relativeTolerance ) {


   Double_Container::iterator timeIt;

   for ( timeIt = Time_Stamps.begin (); timeIt != Time_Stamps.end (); ++timeIt ) {

      if (( age != 0.0 and ( std::fabs ( age - (*timeIt)) / age ) < relativeTolerance ) or
          ( age == 0.0 and ( std::fabs ( age - (*timeIt))) < relativeTolerance )) {
         return timeIt;
      }

   }

   return Time_Stamps.end ();

}


void History::Locate_Point( const double X_Coord, const double Y_Coord, Node_Info * &node )
{

   const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

  const double Origin_X = grid.originI;
  const double Origin_Y = grid.originJ;
  const int    Number_Of_X_Nodes = grid.nrI;
  const int    Number_Of_Y_Nodes = grid.nrJ;
  const double Delta_X = grid.deltaI;
  const double Delta_Y = grid.deltaJ;

  double X_Fraction;
  double Y_Fraction;

  int    X_Position;
  int    Y_Position;
  int    X_Step;
  int    Y_Step;

  if (( X_Coord < Origin_X || Origin_X + Delta_X * Number_Of_X_Nodes < X_Coord ) ||
      ( Y_Coord < Origin_Y || Origin_Y + Delta_Y * Number_Of_Y_Nodes < Y_Coord )) {

    node = 0;
    return;
  }

  X_Position = int (( X_Coord - Origin_X ) / Delta_X );
  Y_Position = int (( Y_Coord - Origin_Y ) / Delta_Y );

  if ( X_Position == Number_Of_X_Nodes - 1 ) 
  {
    X_Step = 0;
  } 
  else 
  {
    X_Step = 1;
  } 

  if ( Y_Position == Number_Of_Y_Nodes - 1 ) 
  {
    Y_Step = 0;
  } 
  else 
  {
    Y_Step = 1;
  } 

  bool Location_Exists    = false;
  bool In_Processor_Range = appctx -> In_Processor_Range ( X_Position, Y_Position );

  if ( In_Processor_Range )
  {


    Location_Exists = (( appctx->nodeIsDefined ( X_Position,          Y_Position          )) &&
                       ( appctx->nodeIsDefined ( X_Position,          Y_Position + Y_Step )) &&
                       ( appctx->nodeIsDefined ( X_Position + X_Step, Y_Position + Y_Step )) &&
                       ( appctx->nodeIsDefined ( X_Position + X_Step, Y_Position          )));

    if ( Location_Exists )
    {

      node = new Node_Info;
      node -> X_Coord = X_Coord;
      node -> Y_Coord = Y_Coord;
      node -> I_Index = X_Position;
      node -> J_Index = Y_Position;
      node -> I_Step  = X_Step;
      node -> J_Step  = Y_Step;

      X_Fraction = ( X_Coord - ( Origin_X + X_Position * Delta_X ) ) / Delta_X;
      Y_Fraction = ( Y_Coord - ( Origin_Y + Y_Position * Delta_Y ) ) / Delta_Y;
      
      node -> Fractions [ 0 ] = ( 1.0 - X_Fraction ) * ( 1.0 - Y_Fraction );
      node -> Fractions [ 1 ] = (       X_Fraction ) * ( 1.0 - Y_Fraction );
      node -> Fractions [ 2 ] = (       X_Fraction ) * (       Y_Fraction );
      node -> Fractions [ 3 ] = ( 1.0 - X_Fraction ) * (       Y_Fraction );

    } else {
      node = 0;
    }
    
  } else {
    node = 0;
  }

}

bool History::IsDefined () {

  return Has_Nodes;

}

void History::Generate_Log_File_Header () {

  std::ostringstream Header;

  Header << FastcauldronSimulator::getInstance ().getProjectName () << endl
  // Header << appctx->getProjectName () << endl
         << setw ( width ) << "Age_Ma"
         << setw ( width ) << "X_m"
         << setw ( width ) << "Y_m"
         << setw ( width ) << "Depth_m"
         << setw ( width ) << "Hydrostatic_MPa"
         << setw ( width ) << "Overpressure_MPa"
         << setw ( width ) << "Pressure_MPa"
         << setw ( width ) << "Lithostatic_MPa"
         << setw ( width ) << "VES_MPa"
         << setw ( width ) << "MaxVES_MPa"
         << setw ( width ) << "Porosity_b"
         << setw ( width ) << "Porosity_a"
         << setw ( width ) << "Kv_log10_b_mD"
         << setw ( width ) << "Kv_log10_a_mD"
         << setw ( width ) << "Kh_log10_b_mD"
         << setw ( width ) << "Kh_log10_a_mD"
         << setw ( width ) << "ThCondv_b_W/m/K"
         << setw ( width ) << "ThCondv_a_W/m/K"
         << setw ( width ) << "Temperature_C"
         << setw ( width ) << "BulkDensity_b_kg/m"
         << setw ( width ) << "BulkDensity_a_kg/m"
         << setw ( width ) << "VRe_a_%Ro"
         << setw ( width ) << "VRe_b_%Ro"
         << endl;

  Log_File_Header = Header.str();

}

void History::createLogFileHeader ( ofstream& historyDataFile ) const {

  historyDataFile << FastcauldronSimulator::getInstance ().getProjectName () << endl
  // historyDataFile << appctx->getProjectName () << endl
                  << setw ( width ) << "Age_Ma"
                  << setw ( width ) << "X_m"
                  << setw ( width ) << "Y_m"
                  << setw ( width ) << "Depth_m"
                  << setw ( width ) << "Hydrostatic_MPa"
                  << setw ( width ) << "Overpressure_MPa"
                  << setw ( width ) << "Pressure_MPa"
                  << setw ( width ) << "Lithostatic_MPa"
                  << setw ( width ) << "VES_MPa"
                  << setw ( width ) << "MaxVES_MPa"
                  << setw ( width ) << "Porosity_b"
                  << setw ( width ) << "Porosity_a"
                  << setw ( width ) << "Kv_log10_b_mD"
                  << setw ( width ) << "Kv_log10_a_mD"
                  << setw ( width ) << "Kh_log10_b_mD"
                  << setw ( width ) << "Kh_log10_a_mD"
                  << setw ( width ) << "ThCondv_b_W/m/K"
                  << setw ( width ) << "ThCondv_a_W/m/K"
                  << setw ( width ) << "Temperature_C"
                  << setw ( width ) << "BulkDensity_b_kg/m"
                  << setw ( width ) << "BulkDensity_a_kg/m"
                  << setw ( width ) << "VRe_a_%Ro"
                  << setw ( width ) << "VRe_b_%Ro"
                  << endl;

}

double History::Calculate_Interpolated_Value( Node_Info * node, PETSC_3D_Array& property )
{

  int        Node_Count;
  int        Divisor = 0;
  double     Values [4];
  double     Sum     = 0.0;

  Values [ 0 ] = property( node -> K_Index, 
                           node -> J_Index, 
                           node -> I_Index );

  Values [ 1 ] = property( node -> K_Index, 
                           node -> J_Index, 
                           node -> I_Index + node -> I_Step );

  Values [ 2 ] = property( node -> K_Index, 
                           node -> J_Index + node -> J_Step , 
                           node -> I_Index  + node -> I_Step );

  Values [ 3 ] = property( node -> K_Index, 
                           node -> J_Index + node -> J_Step , 
                           node -> I_Index );

  for ( Node_Count = 0; Node_Count < 4; Node_Count++ )
  {
    if ( Values [ Node_Count ] != IBSNULLVALUE ) 
    {
      Sum = Sum + Values [ Node_Count ] * node -> Fractions [ Node_Count ];
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

void History::Save_Depositional_Property ( const string&      property_name,
                                           const PropertyList propertyId,
                                           LayerProps *       layer,
                                           Node_Container&    nodes ) {


  Node_Info *  Current_Node;

  unsigned int          indx;

  double       Property_Value;

  for ( indx = 0; indx < nodes.size(); indx++ ) {
      
    Current_Node = nodes [ indx ];

    if ( appctx->timefilter.propertyIsSelected ( propertyId )) {
      Property_Value = Interpolate_Depositional_Property( layer, Current_Node );
    } else {
      Property_Value = IBSNULLVALUE;
    }

    Save_Property_Value_At_Node ( Current_Node, Property_Value, property_name );

  }

}

double History::Interpolate_Depositional_Property( LayerProps * layer,
                                                   Node_Info *  node )
{

  int  XS, YS;

  DMDAGetCorners( * appctx -> mapDA, &XS, &YS, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL );

  return Calculate_Surface_Property( layer,
                                     node -> I_Index,
                                     node -> J_Index );

}

double History::Calculate_Surface_Property( LayerProps * layer,
                                            const int    I, 
                                            const int    J )
{

  double Dummy_Value = 0.0;
  double Property_Value;

  double sea_bottom_temperature;
  double surfacePorosityScalar;
  CompoundProperty surfacePorosityCompound;

  int xStart;
  int yStart;

  switch ( *property_it ) {
    
  case POROSITYVEC:
    
    Property_Value = layer->getLithology ( I,J ) -> surfacePorosity();
    break;
    
  case THCONDVEC:

    DMDAGetCorners ( *appctx->mapDA, &xStart, &yStart, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL );

    sea_bottom_temperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( I, J, current_time );
    
    surfacePorosityScalar = layer->getLithology ( I,J ) -> surfacePorosity();
    
    layer->getLithology ( I,J ) -> calcBulkThermCondNP( layer -> fluid,
                                                        surfacePorosityScalar,
                                                        sea_bottom_temperature,
                                                        1.0,
                                                        Property_Value, 
                                                        Dummy_Value );
    
    break;
    
  case PERMEABILITYVEC:
    
    layer->getLithology ( I,J )->getSurfacePorosity( surfacePorosityCompound );
    
    layer->getLithology ( I,J ) -> calcBulkPermeabilityNP( 0.0, 0.0,
                                                           surfacePorosityCompound, 
                                                           Property_Value, 
                                                           Dummy_Value );
    Property_Value = Property_Value / MILLIDARCYTOM2;
    
    break;
    
  case PERMEABILITYHVEC:

    
    layer->getLithology ( I,J )->getSurfacePorosity( surfacePorosityCompound );
    layer->getLithology( I,J ) -> calcBulkPermeabilityNP ( 0.0, 0.0,
                                                           surfacePorosityCompound, 
                                                           Dummy_Value,
                                                           Property_Value );
    Property_Value = Property_Value / MILLIDARCYTOM2;
    
    break;
 
  case BULKDENSITYVEC :

    surfacePorosityScalar = layer->getLithology ( I,J ) -> surfacePorosity();
    DMDAGetCorners ( *appctx->mapDA, &xStart, &yStart, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL );


    sea_bottom_temperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( I, J, current_time );

    layer->getLithology ( I,J )->calcBulkDensity ( layer->fluid,
                                                   surfacePorosityScalar, 
                                                   0.1,
                                                   sea_bottom_temperature, 
                                                   0.0, 
                                                   Property_Value );
    break;

  default:
    
    PETSC_ASSERT(0);
    
  }

  return Property_Value;

}
