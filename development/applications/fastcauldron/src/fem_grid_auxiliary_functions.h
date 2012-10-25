//------------------------------------------------------------//

#ifndef __FEM_Grid_Auxiliary_Functions_HH__
#define __FEM_Grid_Auxiliary_Functions_HH__

//------------------------------------------------------------//

#include "fem_grid.h"

//------------------------------------------------------------//

// #include "fluidprops.h"
#include "GeoPhysicsFluidType.h"
#include "propinterface.h"
#include "globaldefs.h"

//------------------------------------------------------------//


namespace Basin_Modelling {



  //!
  //!
  //!
  //!
  void Initialise_Top_Depth
     (       AppCtx*          Basin_Model,
       const double           Current_Time, 
       const int              X_Start, 
       const int              X_Count, 
       const int              Y_Start, 
       const int              Y_Count, 
             Double_Array_2D& Top_Depth );

  void Initialise_Top_Properties
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
             Double_Array_2D& Top_Max_VES );


  /// Initialise the top 'layer' of nodes in the 3D array with the values in the 2D array.
  void initialiseTopNodes
     ( const int              X_Start, 
       const int              X_Count, 
       const int              Y_Start, 
       const int              Y_Count, 
       const int              Z_Top, 
       const Boolean2DArray&  Valid_Needle,
             Double_Array_2D& Property_Above,
             PETSC_3D_Array&  Layer_Property );

  /// Copy the bottom 'layer' of nodes in the 3D array to the 2D array.
  void copyBottomNodes
     ( const int              X_Start, 
       const int              X_Count, 
       const int              Y_Start, 
       const int              Y_Count, 
       const Boolean2DArray&  Valid_Needle,
             Double_Array_2D& Property_Below,
             PETSC_3D_Array&  Layer_Property );


} // end namespace Basin_Modelling


//------------------------------------------------------------//

#endif // __FEM_Grid_Auxiliary_Functions_HH__

//------------------------------------------------------------//
