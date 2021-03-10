//------------------------------------------------------------//

#include "fem_grid_auxiliary_functions.h"

#include "GeoPhysicalFunctions.h"

//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Initialise_Top_Depth"

void Basin_Modelling::Initialise_Top_Depth
   (       AppCtx*          Basin_Model,
     const double           Current_Time, 
     const int              X_Start, 
     const int              X_Count, 
     const int              Y_Start, 
     const int              Y_Count, 
           Double_Array_2D& Top_Depth ) {

  int I, J; 
  int IX, JY;

  for ( I = X_Start, IX = 0; I < X_Start + X_Count; I++, IX++ ) {

    for ( J = Y_Start, JY = 0; J < Y_Start + Y_Count; J++, JY++ ) {

       if ( Basin_Model->nodeIsDefined ( I, J )) {
        Top_Depth ( IX, JY ) = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( I, J, Current_Time );
      }

    }

  }

}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Initialise_Top_Properties"

void Basin_Modelling::Initialise_Top_Properties
   (       AppCtx*          Basin_Model,
     const FluidType*       Layer_Fluid,
     const double           Current_Time, 
     const int              X_Start, 
     const int              X_Count, 
     const int              Y_Start, 
     const int              Y_Count, 
           Double_Array_2D& Top_Hydrostatic_Pressure,
           Double_Array_2D& Top_Pore_Pressure,
           Double_Array_2D& Top_Lithostatic_Pressure,
           Double_Array_2D& Top_VES,
           Double_Array_2D& Top_Max_VES ) {

  int I, J; 
  int IX, JY;

  double Hydrostatic_Pressure;

  for ( I = X_Start, IX = 0; I < X_Start + X_Count; I++, IX++ ) {

    for ( J = Y_Start, JY = 0; J < Y_Start + Y_Count; J++, JY++ ) {

       if ( Basin_Model->nodeIsDefined ( I, J )) {

          computeHydrostaticPressure ( Layer_Fluid, 
                                       FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( I, J, Current_Time ), 
                                       FastcauldronSimulator::getInstance ().getSeaBottomDepth ( I, J, Current_Time ),
                                       Hydrostatic_Pressure );

          Top_Hydrostatic_Pressure ( IX, JY ) = Hydrostatic_Pressure;
          //
          //
          // The lithostatic pressure is the same as the hydrostatic at the top of the model
          //
          Top_Pore_Pressure        ( IX, JY ) = Hydrostatic_Pressure;
          Top_Lithostatic_Pressure ( IX, JY ) = Hydrostatic_Pressure;
          Top_VES                  ( IX, JY ) = 0.0;
          Top_Max_VES              ( IX, JY ) = 0.0;
       }

    }

  }

}

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::initialiseTopNodes"

void Basin_Modelling::initialiseTopNodes ( const int              X_Start, 
                                           const int              X_Count, 
                                           const int              Y_Start, 
                                           const int              Y_Count, 
                                           const int              Z_Top, 
                                           const Boolean2DArray&  Valid_Needle,
                                                 Double_Array_2D& Property_Above,
                                                 PETSC_3D_Array&  Layer_Property ) {

  int I, J;
  // The 2D array uses a different indexing than that used by the PETSc_3D_Array.
  int IX, JY;

  for ( I = X_Start, IX = 0; I < X_Start + X_Count; I++, IX++ ) {

    for ( J = Y_Start, JY = 0; J < Y_Start + Y_Count; J++, JY++ ) {

       if ( Valid_Needle ( I, J )) {
        Layer_Property ( Z_Top, J, I ) = Property_Above ( IX, JY );
      }

    }

  }

}

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::copyBottomNodes"

void Basin_Modelling::copyBottomNodes ( const int              X_Start, 
                                        const int              X_Count, 
                                        const int              Y_Start, 
                                        const int              Y_Count, 
                                        const Boolean2DArray&  Valid_Needle,
                                               Double_Array_2D& Property_Below,
                                               PETSC_3D_Array&  Layer_Property ) {

  int I, J;
  int IX, JY;

  for ( I = X_Start, IX = 0; I < X_Start + X_Count; I++, IX++ ) {

    for ( J = Y_Start, JY = 0; J < Y_Start + Y_Count; J++, JY++ ) {

      if ( Valid_Needle ( I, J )) {
        Property_Below ( IX, JY ) = Layer_Property ( 0, J, I );
      }

    }

  }

}

//------------------------------------------------------------//
