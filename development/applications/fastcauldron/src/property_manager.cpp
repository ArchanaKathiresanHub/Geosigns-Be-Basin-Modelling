//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
//------------------------------------------------------------//

#include <memory>

#include "property_manager.h"
#include "layer_iterators.h"
#include "propinterface.h"
#include "milestones.h"
#include "utils.h"
#include "fem_grid_auxiliary_functions.h"
#include "ConstantsFastcauldron.h"
#include "ghost_array.h"
#include "FastcauldronSimulator.h"

//------------------------------------------------------------//

#include "GeoPhysicalFunctions.h"
#include "GeoPhysicalConstants.h"
#include "CompoundProperty.h"

#include "RunParameters.h"

//------------------------------------------------------------//

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;
#include "ConstantsMathematics.h"
using Utilities::Maths::PaToMegaPa;
using Utilities::Maths::MilliDarcyToM2;
#include "ConstantsPhysics.h"
using Utilities::Physics::AccelerationDueToGravity;

//------------------------------------------------------------//

const std::string& Basin_Modelling::fundamentalPropertyImage ( const Fundamental_Property property ) {

  static const string propertyNames [ NumberOfFundamentalProperties + 1 ] = { "Depth",
                                                                              "Thickness",
                                                                              "SolidThickness",
                                                                              "HydroStaticPressure",
                                                                              "LithoStaticPressure",
                                                                              "OverPressure",
                                                                              "Pressure", 
                                                                              "ChemicalCompaction",
                                                                              "Ves",
                                                                              "MaxVes",
                                                                              "Temperature",
                                                                              "No such property name" };

  if ( property >= Depth && property < No_Property ) {
    return propertyNames [ property ];
  } else {
    return propertyNames [ No_Property ];
  }

}


//------------------------------------------------------------//

const std::string& Basin_Modelling::fundamentalPropertyName ( const Fundamental_Property property ) {

  static const string propertyNames [ NumberOfFundamentalProperties + 1 ] = { "Depth",
                                                                              "Thickness",
                                                                              "UNKNOWN",
                                                                              "HydroStaticPressure",
                                                                              "LithoStaticPressure",
                                                                              "OverPressure",
                                                                              "Pressure", 
                                                                              "ChemicalCompaction",
                                                                              "Ves",
                                                                              "MaxVes",
                                                                              "Temperature",
                                                                              "UNKNOWN" };

  if ( property >= Depth && property < No_Property ) {
    return propertyNames [ property ];
  } else {
    return propertyNames [ No_Property ];
  }

}


//------------------------------------------------------------//


Basin_Modelling::Fundamental_Property Basin_Modelling::operator++ ( Fundamental_Property& Property ) {
  Property = Fundamental_Property ( int ( Property ) + 1 );
  return Property;
} // end 


//------------------------------------------------------------//


Basin_Modelling::Fundamental_Property Basin_Modelling::operator++ ( Fundamental_Property& Property, const int Dummy ) {

  Fundamental_Property Old_Property_Value = Property;

  Property = Fundamental_Property ( int ( Property ) + 1 );
  return Old_Property_Value;
} // end 


//------------------------------------------------------------//


void Basin_Modelling::Element_Positions::Set_Node_Position ( const int                Node_Number,
                                                             const Property_Position& Entry ) {

  Entries [ Node_Number ] = Entry;
}


//------------------------------------------------------------//


void Basin_Modelling::Element_Positions::Set_Node_Position ( const int nodeNumber,
                                                             const int newZPosition,
                                                             const int newYPosition,
                                                             const int newXPosition ) {

  Entries [ nodeNumber ].X_Position = newXPosition;
  Entries [ nodeNumber ].Y_Position = newYPosition;
  Entries [ nodeNumber ].Z_Position = newZPosition;
}


//------------------------------------------------------------//


Basin_Modelling::Fundamental_Property_Manager::Fundamental_Property_Manager () {

  Fundamental_Property Property;
  //
  //
  // This is only temporary until manager is fuilly implemented
  //
  for ( Property = Depth; Property < No_Property; Property++ ) {
    Vector_Properties [ Property ] = Vec_Ptr ( 0 ); // Initialise to Null
    Property_Active [ Property ] = false;
  }

} // 


//------------------------------------------------------------//


Basin_Modelling::Fundamental_Property_Manager::~Fundamental_Property_Manager () {

  Fundamental_Property Property;
  //
  //
  // This is only temporary until manager is fuilly implemented
  //
  for ( Property = Depth; Property < No_Property; Property++ ) {
    Vector_Properties [ Property ] = Vec_Ptr ( 0 ); // Initialise to Null
  }

} // 


//------------------------------------------------------------//


void Basin_Modelling::Fundamental_Property_Manager::Set_Layer_DA ( const DM* Layer_Array ) {
  //
  //
  // I dont know exactly what to do here. I dont know if having the const in the parameter
  // (indicating that this object will not change, which it will not) and having the
  // const_cast, OR removing the const and therefore the const_cast.
  //
  // In fact Layer_DA does not change within this object at all.
  //
  // Another option is to have the pointer type defined as being 
  // const (or pointing to const, which ever is more meaningful)
  //
  Layer_DA = const_cast<DA_Ptr>( Layer_Array );
} 


//------------------------------------------------------------//


void Basin_Modelling::Fundamental_Property_Manager::Set_Property_Vector ( const Fundamental_Property Property_Value, Vec* Property_Vector ) {
  Vector_Properties [ Property_Value ] = Property_Vector;
} 


//------------------------------------------------------------//


void Basin_Modelling::Fundamental_Property_Manager::Create_Properties () {

  Fundamental_Property Property;
  int Error;

  for ( Property = Depth; Property < No_Property; Property++ ) {

    if ( Vector_Properties [ Property ] != Vec_Ptr ( 0 )) {
      Error = DMCreateGlobalVector ( *Layer_DA, Vector_Properties [ Property ]);
    } 

    if ( Error != 0 ) {
      // Do Something
    }

  }


//    CHKERRQ ( Error );

}


//------------------------------------------------------------//


void Basin_Modelling::Fundamental_Property_Manager::Create_Property ( const Fundamental_Property Property ) {

  int Error;

  if ( Vector_Properties [ Property ] != Vec_Ptr ( 0 )) {
    Error = DMCreateGlobalVector ( *Layer_DA, Vector_Properties [ Property ]);
  } 

  if ( Error != 0 ) {
    // Do Something
  }
//    CHKERRQ ( Error );

}


//------------------------------------------------------------//

bool Basin_Modelling::Fundamental_Property_Manager::propertyIsActivated ( const Fundamental_Property Property ) const {
   return Property_Active [ Property ];
}

//------------------------------------------------------------//


void Basin_Modelling::Fundamental_Property_Manager::Activate_Property ( const Fundamental_Property Property,
                                                                        const InsertMode           Mode,
                                                                        const bool                 Include_Ghost_Values ) {


  if ( ! Property_Active [ Property ] && Vector_Properties [ Property ] != Vec_Ptr ( 0 )) {
    Properties [ Property ].Set_Global_Array ( *Layer_DA, *Vector_Properties [ Property ], Mode, Include_Ghost_Values );
    Property_Active [ Property ] = true;
  } 

} // 


//------------------------------------------------------------//


void Basin_Modelling::Fundamental_Property_Manager::Activate_Properties ( const InsertMode Mode,
                                                                          const bool       Include_Ghost_Values ) {


  Fundamental_Property Property;

  for ( Property = Depth; Property < No_Property; Property++ ) {

    if ( ! Property_Active [ Property ] && Vector_Properties [ Property ] != Vec_Ptr ( 0 )) {
      Properties [ Property ].Set_Global_Array ( *Layer_DA, *Vector_Properties [ Property ], Mode, Include_Ghost_Values );
      Property_Active [ Property ] = true;
    } 

  }

} // 


//------------------------------------------------------------//


void Basin_Modelling::Fundamental_Property_Manager::Restore_Properties () {


  Fundamental_Property Property;

  for ( Property = Depth; Property < No_Property; Property++ ) {

    if ( Property_Active [ Property ] && Vector_Properties [ Property ] != Vec_Ptr ( 0 )) {
      Properties [ Property ].Restore_Global_Array ();
    }

    Property_Active [ Property ] = false;
  }

} // 


//------------------------------------------------------------//


void Basin_Modelling::Fundamental_Property_Manager::Restore_Property ( const Fundamental_Property Property ) {

  if ( Property_Active [ Property ] && Vector_Properties [ Property ] != Vec_Ptr ( 0 )) {
    Properties [ Property ].Restore_Global_Array ();
    Property_Active [ Property ] = false;
  }

} // 


//------------------------------------------------------------//


void Basin_Modelling::Fundamental_Property_Manager::Extract_Property ( const Fundamental_Property Property, 
                                                                       const Element_Positions&   Positions, 
                                                                             ElementVector&       Property_Values ) const {

  int I;

  for ( I = 0; I < NumberOfElementNodes; I++ ) {
     Property_Values ( I + 1 ) = this -> operator ()( Property, Positions ( I ).Z_Position, Positions ( I ).Y_Position, Positions ( I ).X_Position );
    // Property_Values ( I + 1 ) = this -> operator ()( Property, Positions ( I ));
//      Property_Values ( I + 1 ) = ( Positions.Z_Position, Position.Y_Position, Position.X_Position );
//      Property_Values ( I + 1 ) = Properties [ Property ]( Positions ( I ));
  } //

} 



void Basin_Modelling::Fundamental_Property_Manager::Extract_Property ( const Fundamental_Property  property, 
                                                                       const Nodal3DIndexArray&    indices, 
                                                                             ElementVector&        propertyValues ) const {


   int i;

   for ( i = 0; i < NumberOfElementNodes; ++i ) {
      propertyValues ( i + 1 ) = operator ()( property, indices ( i ));
   }

}


//------------------------------------------------------------//


//  void Basin_Modelling::Fundamental_Property_Manager::Extract_Property ( const Fundamental_Property& Property, 
//                                                                         const int                   X_Position, 
//                                                                         const int                   Y_Position, 
//                                                                         const int                   Z_Position, 
//                                                                               PetscScalar&          Property_Value  ) const {

//  } 


//------------------------------------------------------------//


//      void Set_Property        ( const Fundamental_Property& Property, 
//                                 const int                   X_Position, 
//                                 const int                   Y_Position, 
//                                 const int                   Z_Position, 
//                                 const PetscScalar&          Property_Value  );
//------------------------------------------------------------//


#if 0
PetscScalar Basin_Modelling::Fundamental_Property_Manager::operator ()( const Fundamental_Property  Property,
                                                                        const Property_Position&    Position ) const {

  return Properties [ Property ]( Position.Z_Position, Position.Y_Position, Position.X_Position );

} // end operator ()
#endif


//------------------------------------------------------------//


#if 0
PetscScalar Basin_Modelling::Fundamental_Property_Manager::operator () ( const Fundamental_Property  Property,
                                                                         const int                   Z_Position, 
                                                                         const int                   Y_Position, 
                                                                         const int                   X_Position ) const {

  return Properties [ Property ]( Z_Position, Y_Position, X_Position );

} // end operator ()
#endif


//------------------------------------------------------------//


#if 0
PetscScalar& Basin_Modelling::Fundamental_Property_Manager::operator ()( const Fundamental_Property  Property,
                                                                         const Property_Position&    Position ) {

  return Properties [ Property ]( Position.Z_Position, Position.Y_Position, Position.X_Position );

} // end operator ()
#endif


//------------------------------------------------------------//


#if 0
PetscScalar& Basin_Modelling::Fundamental_Property_Manager::operator () ( const Fundamental_Property  Property,
                                                                          const int                   Z_Position, 
                                                                          const int                   Y_Position, 
                                                                          const int                   X_Position ) {

  return Properties [ Property ]( Z_Position, Y_Position, X_Position );

} // end operator ()
#endif


//------------------------------------------------------------//


void Basin_Modelling::Fundamental_Property_Manager::Copy ( const Fundamental_Property_Manager& Manager,
                                                           const Boolean2DArray&               Valid_Needle ) {

  Fundamental_Property Property;
  int X_Start;
  int Y_Start;
  int Z_Start;
  int X_End;
  int Y_End;
  int Z_End;
  int X_Count;
  int Y_Count;
  int Z_Count;

  DMDAGetCorners ( *Layer_DA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );
  X_End = X_Start + X_Count;
  Y_End = Y_Start + Y_Count;
  Z_End = Z_Start + Z_Count;

  for ( Property = Depth; Property <= Temperature; Property++ ) {

    if (( this -> Vector_Properties [ Property ] != Vec_Ptr ( 0 )) &&
        ( Manager.Vector_Properties [ Property ] != Vec_Ptr ( 0 ))) {
      Copy_Vector ( Valid_Needle,
                    X_Start, X_End,
                    Y_Start, Y_End,
                    Z_Start, Z_End,
                    *Layer_DA,
                    *Manager.Vector_Properties [ Property ],
                    *Vector_Properties [ Property ]);
    }
    
  }

} 


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::Fundamental_Property_Manager::Copy_Vector"

void Basin_Modelling::Fundamental_Property_Manager::Copy_Vector ( const Boolean2DArray& Valid_Needle,
                                                                  const int             X_Start,
                                                                  const int             X_End,
                                                                  const int             Y_Start,
                                                                  const int             Y_End,
                                                                  const int             Z_Start,
                                                                  const int             Z_End,
                                                                  const DM              Layer_DA,
                                                                  const Vec             Current_Property,
                                                                        Vec&            Previous_Property ) const {

  int I, J, K;

  PETSC_3D_Array Current  ( Layer_DA, Current_Property );
  PETSC_3D_Array Previous ( Layer_DA, Previous_Property );

  for ( I = X_Start; I < X_End; I++ ) {

    for ( J = Y_Start; J < Y_End; J++ ) {

      if ( Valid_Needle ( I, J ) ) {

        for ( K = Z_Start; K < Z_End; K++ ) {
          Previous ( K, J, I ) = Current ( K, J, I );
        } // end loop

      } // end if

    } // end loop

  } // end loop

} // end Basin_Modelling::Copy_Vector


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::computeBulkDensityVectors"

void Basin_Modelling::computeBulkDensityVectors ( AppCtx* Basin_Model ) {

  if ( ! Basin_Model -> filterwizard.SetDerivedProperties &&
       ! Basin_Model -> filterwizard.bBulkDensities ) {
    return;
  }

  int I;
  int J;
  int K;
  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;

  Layer_Iterator Layers;
  LayerProps_Ptr currentLayer;
  const CompoundLithology*  currentLithology;


  Layers.Initialise_Iterator ( Basin_Model->layers, Descending, Basement_And_Sediments, Active_Layers_Only );
  DMDAGetCorners ( *Basin_Model->mapDA, &xStart, &yStart, PETSC_NULL, &xCount, &yCount, PETSC_NULL );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {
    currentLayer = Layers.Current_Layer ();

    PETSC_3D_Array layerBulkDensity;
    PETSC_3D_Array layerPorosity     ( currentLayer -> layerDA, currentLayer -> Porosity );
    PETSC_3D_Array layerPorePressure ( currentLayer -> layerDA, currentLayer -> Current_Properties ( Basin_Modelling::Pore_Pressure ) );
    PETSC_3D_Array layerLithoPressure( currentLayer -> layerDA, currentLayer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure ) );
    PETSC_3D_Array layerTemperature  ( currentLayer -> layerDA, currentLayer -> Current_Properties ( Basin_Modelling::Temperature ));

    DMDAGetCorners ( currentLayer->layerDA, PETSC_NULL, PETSC_NULL, &zStart, PETSC_NULL, PETSC_NULL, &zCount );
    DMCreateGlobalVector ( currentLayer -> layerDA, &currentLayer -> BulkDensity );
    VecSet ( currentLayer -> BulkDensity, CauldronNoDataValue );
    layerBulkDensity.Set_Global_Array ( currentLayer -> layerDA, 
                                        currentLayer -> BulkDensity );

    for ( I = xStart; I < xStart + xCount; I++ ) {

      for ( J = yStart; J < yStart + yCount; J++ ) {

        if ( Basin_Model->nodeIsDefined ( I, J )) {

          for ( K = zStart; K < zStart + zCount; K++ ) {
             currentLithology = currentLayer -> getLithology ( I, J, K );
             currentLithology -> calcBulkDensity ( currentLayer->fluid,
                                                   layerPorosity ( K, J, I ),
                                                   layerPorePressure ( K, J, I ),
                                                   layerTemperature ( K, J, I ),
                                                   layerLithoPressure( K, J, I ),
                                                   layerBulkDensity ( K, J, I ));
	  }

	}

      }

    }

  }

} // end Basin_Modelling::computeBulkDensity


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::deleteBulkDensityVectors"

void Basin_Modelling::deleteBulkDensityVectors ( AppCtx* Basin_Model ) {

  if ( !Basin_Model -> filterwizard.SetDerivedProperties && 
       !Basin_Model -> filterwizard.bBulkDensities ) {
    return;
  }

  PetscBool     validVector;
  Layer_Iterator Layers;

  Layers.Initialise_Iterator ( Basin_Model->layers, Descending, Basement_And_Sediments, Active_Layers_Only );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {

    VecValid ( Layers.Current_Layer () -> BulkDensity, &validVector );

    if ( validVector ) {
       VecDestroy (&( Layers.Current_Layer () -> BulkDensity ));
       Layers.Current_Layer () -> BulkDensity = Vec ( 0 );
    }

  }

} // end Basin_Modelling::deleteBulkDensityVectors


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::computeThermalConductivityVectors"

void Basin_Modelling::computeThermalConductivityVectors ( AppCtx*        Basin_Model ) {

  if ( ! Basin_Model -> filterwizard.SetDerivedProperties &&
       ! Basin_Model -> filterwizard.IsBulkThCondCalculationNeeded ()) {
    return;
  }

  int I;
  int J;
  int K;
  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;
  const CompoundLithology*  currentLithology;

  Layer_Iterator Layers;
  LayerProps_Ptr currentLayer;

  Layers.Initialise_Iterator ( Basin_Model->layers, Descending, Basement_And_Sediments, Active_Layers_Only );
  DMDAGetCorners ( *Basin_Model->mapDA, &xStart, &yStart, PETSC_NULL, &xCount, &yCount, PETSC_NULL );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {
    currentLayer = Layers.Current_Layer ();

    PETSC_3D_Array layerThermalConductivityNormal;
    PETSC_3D_Array layerThermalConductivityTangential;

    PETSC_3D_Array layerPorosity    ( currentLayer -> layerDA, currentLayer -> Porosity );
    PETSC_3D_Array layerTemperature ( currentLayer -> layerDA, currentLayer -> Current_Properties ( Basin_Modelling::Temperature ));
    PETSC_3D_Array layerLithoPressure ( currentLayer -> layerDA, currentLayer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure ));
    PETSC_3D_Array layerPorePressure ( currentLayer -> layerDA, currentLayer -> Current_Properties ( Basin_Modelling::Pore_Pressure ));

    DMDAGetCorners ( currentLayer->layerDA, PETSC_NULL, PETSC_NULL, &zStart, PETSC_NULL, PETSC_NULL, &zCount );
    DMCreateGlobalVector ( currentLayer -> layerDA, &currentLayer -> BulkTHCondN );
    VecSet ( currentLayer -> BulkTHCondN, CauldronNoDataValue );

    DMCreateGlobalVector ( currentLayer -> layerDA, &currentLayer -> BulkTHCondP );
    VecSet ( currentLayer -> BulkTHCondP, CauldronNoDataValue );

    layerThermalConductivityNormal.Set_Global_Array ( currentLayer -> layerDA, 
                                                      currentLayer -> BulkTHCondN );

    layerThermalConductivityTangential.Set_Global_Array ( currentLayer -> layerDA, 
                                                          currentLayer -> BulkTHCondP );

    for ( I = xStart; I < xStart + xCount; I++ ) {

      for ( J = yStart; J < yStart + yCount; J++ ) {

         if ( Basin_Model->nodeIsDefined ( I, J )) {

            for ( K = zStart; K < zStart + zCount; K++ ) {
               currentLithology = currentLayer->getLithology ( I, J, K );
               if( currentLayer->kind() == Interface::BASEMENT_FORMATION ) {
                  currentLithology -> calcBulkThermCondNPBasement ( currentLayer->fluid,
                                                                    layerPorosity ( K, J, I ),
                                                                    layerTemperature ( K, J, I ),
                                                                    layerLithoPressure ( K, J, I ),
                                                                    layerThermalConductivityNormal ( K, J, I ),
                                                                    layerThermalConductivityTangential ( K, J, I ));
               } else {
                  currentLithology -> calcBulkThermCondNP ( currentLayer->fluid,
                                                            layerPorosity ( K, J, I ),
                                                            layerTemperature ( K, J, I ),
                                                            layerPorePressure ( K, J, I ),
                                                            layerThermalConductivityNormal ( K, J, I ),
                                                            layerThermalConductivityTangential ( K, J, I ));
               }
          }

        }

      }

    }

  }

} // end Basin_Modelling::computeThermalConductivity


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::deleteThermalConductivityVectors"

void Basin_Modelling::deleteThermalConductivityVectors ( AppCtx* Basin_Model ) {

  if ( ! Basin_Model -> filterwizard.SetDerivedProperties && 
       ! Basin_Model -> filterwizard.IsBulkThCondCalculationNeeded ()) {
    return;
  }

  Layer_Iterator Layers;
  PetscBool     validVector;

  Layers.Initialise_Iterator ( Basin_Model->layers, Descending, Basement_And_Sediments, Active_Layers_Only );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {
    VecValid ( Layers.Current_Layer () -> BulkTHCondN, &validVector );

    if ( validVector ) {
       VecDestroy (&( Layers.Current_Layer () -> BulkTHCondN ));
       Layers.Current_Layer () -> BulkTHCondN = Vec ( 0 );
    }

  }

} // end Basin_Modelling::deleteThermalConductivityVectors


//------------------------------------------------------------//


#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::computeThicknessVectors"

void Basin_Modelling::computeThicknessVectors ( AppCtx*        Basin_Model ) {

  if ( ! Basin_Model -> filterwizard.SetDerivedProperties &&
       ! Basin_Model -> filterwizard.thicknessCalculationNeeded ()) {
    return;
  }

  int I;
  int J;
  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;

  double needleThickness;

  Layer_Iterator Layers;
  LayerProps_Ptr currentLayer;

  Layers.Initialise_Iterator ( Basin_Model->layers, Descending, Basement_And_Sediments, Active_Layers_Only );
  DMDAGetCorners ( *Basin_Model->mapDA, &xStart, &yStart, PETSC_NULL, &xCount, &yCount, PETSC_NULL );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {
    currentLayer = Layers.Current_Layer ();

    PETSC_3D_Array layerThickness;

    PETSC_3D_Array layerDepth       ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Depth ));

    DMDAGetCorners ( currentLayer->layerDA, PETSC_NULL, PETSC_NULL, &zStart, PETSC_NULL, PETSC_NULL, &zCount );

    //
    // The full 3D array must be allocated here, even though its only partially filled.
    // Output of this vector depends on the layerDA which has the full 3D dimension.
    //
    DMCreateGlobalVector ( currentLayer -> layerDA, &currentLayer -> layerThickness );
    VecSet ( currentLayer -> layerThickness, CauldronNoDataValue );

    layerThickness.Set_Global_Array ( currentLayer -> layerDA, 
                                      currentLayer -> layerThickness );

    for ( I = xStart; I < xStart + xCount; I++ ) {

      for ( J = yStart; J < yStart + yCount; J++ ) {

         if ( Basin_Model->nodeIsDefined ( I, J )) {
          needleThickness = layerDepth ( 0, J, I ) - layerDepth ( zCount - 1, J, I );
          layerThickness ( zCount - 1, J, I ) = needleThickness;
        }

      }

    }

  }

} // end Basin_Modelling::computeThickness


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::deleteThicknessVectors"

void Basin_Modelling::deleteThicknessVectors ( AppCtx* Basin_Model ) {

  if ( ! Basin_Model -> filterwizard.SetDerivedProperties && 
       ! Basin_Model -> filterwizard.thicknessCalculationNeeded ()) {
    return;
  }

  Layer_Iterator Layers;
  PetscBool     validVector;

  Layers.Initialise_Iterator ( Basin_Model->layers, Descending, Basement_And_Sediments, Active_Layers_Only );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {
    VecValid ( Layers.Current_Layer () -> layerThickness, &validVector );

    //
    // The vector should be valid. some other comment
    //
    if ( validVector ) {
       VecDestroy (&( Layers.Current_Layer () -> layerThickness ));
       Layers.Current_Layer () -> layerThickness = Vec ( 0 );
    }

  }

} // end Basin_Modelling::deleteThicknessVectors


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::computePermeabilityVectors"

void Basin_Modelling::computePermeabilityVectors ( AppCtx*        Basin_Model ) {

//    if ( ! Basin_Model -> filterwizard.SetDerivedProperties ) {
//      return;
//    }

  int I;
  int J;
  int K;
  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;

  bool includeChemicalCompactionTerm;

  double VES;
  double maxVES;
  double normalPerm;
  double tangentialPerm;

  GeoPhysics::CompoundProperty porosity;

  const CompoundLithology*  currentLithology;
  Layer_Iterator Layers;
  LayerProps_Ptr currentLayer;

  Layers.Initialise_Iterator ( Basin_Model->layers, Descending, Basement_And_Sediments, Active_Layers_Only );
  DMDAGetCorners ( *Basin_Model->mapDA, &xStart, &yStart, PETSC_NULL, &xCount, &yCount, PETSC_NULL );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {
    currentLayer = Layers.Current_Layer ();

    includeChemicalCompactionTerm = Basin_Model->Do_Chemical_Compaction && currentLayer->Get_Chemical_Compaction_Mode ();

    PETSC_3D_Array layerNormalPermeability;
    PETSC_3D_Array layerTangentialPermeability;

    PETSC_3D_Array layerChemicalCompaction ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Chemical_Compaction ));
    PETSC_3D_Array layerVES                ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::VES_FP ));
    PETSC_3D_Array layerMaxVES             ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Max_VES ));

    DMDAGetCorners ( currentLayer->layerDA, PETSC_NULL, PETSC_NULL, &zStart, PETSC_NULL, PETSC_NULL, &zCount );

    //
    // The full 3D array must be allocated here, even though its only partially filled.
    // Output of this vector depends on the layerDA which has the full 3D dimension.
    //
//      DMCreateGlobalVector ( currentLayer -> layerDA, &currentLayer -> PermeabilityV );
//      DMCreateGlobalVector ( currentLayer -> layerDA, &currentLayer -> PermeabilityH );

    VecSet ( currentLayer -> PermeabilityV, CauldronNoDataValue );
    VecSet ( currentLayer -> PermeabilityH, CauldronNoDataValue );

    layerNormalPermeability.Set_Global_Array ( currentLayer -> layerDA, 
                                               currentLayer -> PermeabilityV );

    layerTangentialPermeability.Set_Global_Array ( currentLayer -> layerDA, 
                                                   currentLayer -> PermeabilityH );

    for ( I = xStart; I < xStart + xCount; I++ ) {

      for ( J = yStart; J < yStart + yCount; J++ ) {

         if ( Basin_Model->nodeIsDefined ( I, J )) {

          currentLithology = currentLayer->getLithology ( I, J );

          for ( K = zStart; K < zStart + zCount; K++ ) {
            VES = layerVES ( K, J, I );
            maxVES = layerMaxVES ( K, J, I );

            currentLithology->getPorosity ( VES, maxVES, includeChemicalCompactionTerm, layerChemicalCompaction ( K, J, I ), porosity );
            currentLithology->calcBulkPermeabilityNP ( VES, maxVES, porosity, normalPerm, tangentialPerm );
            layerNormalPermeability     ( K, J, I )= normalPerm / MilliDarcyToM2;
            layerTangentialPermeability ( K, J, I )= tangentialPerm / MilliDarcyToM2;            
	  }

        }

      }

    }

  }

} // end Basin_Modelling::computePermeability

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::collectAndSaveIsoValues"

void Basin_Modelling::collectAndSaveIsoValues(const double Current_Time, AppCtx *basinModel) 
{
   int K;
   int xStart;
   int yStart;
   int zStart;
   int xCount;
   int yCount;
   int zCount;

   Layer_Iterator Layers;
   LayerProps_Ptr currentLayer;

   Layers.Initialise_Iterator ( basinModel->layers, Descending, Basement_And_Sediments, Active_Layers_Only );
   DMDAGetCorners ( *basinModel->mapDA, &xStart, &yStart, PETSC_NULL, &xCount, &yCount, PETSC_NULL );
   
   std::unique_ptr<Polyfunction> temperatureDepthFunction (new Polyfunction);
   std::unique_ptr<Polyfunction> vreDepthFunction (new Polyfunction);
   
   static EqualityTest<double> doubleEqualityTest;
   //collect data
   //cout << "milestone time " << Current_Time << endl;
   for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) 
   {
      currentLayer = Layers.Current_Layer ();
      
      PETSC_3D_Array* layerVrePtr = 0;
      if ( currentLayer->isSediment() ) 
         layerVrePtr = new  PETSC_3D_Array( currentLayer -> layerDA, currentLayer -> Vre );
      
      //PETSC_3D_Array layerVre    ( currentLayer -> layerDA, currentLayer -> Vre );
      PETSC_3D_Array layerTemperature ( currentLayer -> layerDA, currentLayer -> Current_Properties ( Basin_Modelling::Temperature ));
      PETSC_3D_Array layerDepth ( currentLayer -> layerDA, currentLayer -> Current_Properties ( Basin_Modelling::Depth ));
      
      DMDAGetCorners ( currentLayer->layerDA, PETSC_NULL, PETSC_NULL, &zStart, PETSC_NULL, PETSC_NULL, &zCount );

      int xPosition = xStart ;//1d case only, assumption for lower left corner
      int yPosition = yStart ;//1d case only, assumption for lower left corner
      int zEnd = (zStart + zCount);
		
      //add the triple temperature, vre, depth
      double currentDepth, previousDepth,currentTemperature, currentVre; 
      currentDepth = previousDepth = currentTemperature = currentVre = -9999;	
      
      if ( basinModel->nodeIsDefined ( xPosition, yPosition )) 
      {   
         for ( K = zEnd - 1; K >= 0; --K ) 
         {
            currentDepth = layerDepth ( K, yStart, xStart );
                 
            if ( false == doubleEqualityTest(currentDepth ,previousDepth) ) // if not equal, case of zero thickness layer
            { 
               //cout << " T " << layerTemperature( K,  yPosition, xPosition ) << " depth " << currentDepth <<endl;
               temperatureDepthFunction->AddPoint ( layerTemperature( K,  yPosition, xPosition ), currentDepth );
               if (layerVrePtr)
               {
                  double vreMilestone = (*layerVrePtr)( K,  yPosition, xPosition );
                  vreDepthFunction->AddPoint ( vreMilestone, currentDepth );
               }
            }
            previousDepth = currentDepth;
         }
      }
      if (layerVrePtr) delete layerVrePtr;
   }
   updateSedimentBottomSurfaceCurves(Current_Time, basinModel);
   //update the data model
   const AppCtx::DoubleVector & temperatureMilestoneValues = basinModel->getContourMilestones(AppCtx::ISOTEMPERATURE);
   const AppCtx::DoubleVector & vreMilestoneValues 		  = basinModel->getContourMilestones(AppCtx::ISOVRE);
   
   const Polyfunction::Point* low = 0;
   const Polyfunction::Point* high = 0;
   low = high = 0;

   AppCtx::DoubleVector::const_iterator itTemp 	= temperatureMilestoneValues.begin(); 
   AppCtx::DoubleVector::const_iterator itTempEnd = temperatureMilestoneValues.end();

   while(itTemp != itTempEnd)
   {
      double tempMilestone = (*itTemp);
      if( temperatureDepthFunction->seedPoints(tempMilestone,low, high) )
      {
         //double tempMilestoneDepth = temperatureDepthFunction->F(tempMilestone);
         double tempMilestoneDepth = temperatureDepthFunction->F(tempMilestone);
         IsolinePoint *thePoint = new IsolinePoint(Current_Time, tempMilestoneDepth, tempMilestone);
         basinModel->addIsolinePoint(AppCtx::ISOTEMPERATURE, thePoint);
      }  
      ++itTemp;
   }

   AppCtx::DoubleVector::const_iterator itVre 	= vreMilestoneValues.begin(); 
   AppCtx::DoubleVector::const_iterator itVreEnd = vreMilestoneValues.end();

   while(itVre != itVreEnd)
   {
      double vreMilestone = (*itVre);
      if( vreDepthFunction->seedPoints(vreMilestone,low, high))
      {
         double vreMilestoneDepth = vreDepthFunction->F(vreMilestone);
         IsolinePoint *thePoint = new IsolinePoint(Current_Time, vreMilestoneDepth, vreMilestone);
         basinModel->addIsolinePoint(AppCtx::ISOVRE, thePoint);
      }
      ++itVre;
   }	
}


#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::updateSedimentBottomSurfaceCurves"
void Basin_Modelling::updateSedimentBottomSurfaceCurves(const double Current_Time, AppCtx *basinModel)
{
	int xStart;
	int yStart;
	int zStart;
	int xCount;
	int yCount;
	int zCount;

   Layer_Iterator Layers;
	LayerProps_Ptr currentLayer;

   Layers.Initialise_Iterator ( basinModel->layers, Descending, Sediments_Only, Active_Layers_Only );

   if(Layers.Iteration_Is_Done ())
	{ 
           return;
	}
	DMDAGetCorners ( *basinModel->mapDA, &xStart, &yStart, PETSC_NULL, &xCount, &yCount, PETSC_NULL );

   currentLayer = Layers.Bottom_Layer();

   PETSC_3D_Array layerVre    ( currentLayer -> layerDA, currentLayer -> Vre );
	PETSC_3D_Array layerTemperature ( currentLayer -> layerDA, currentLayer -> Current_Properties ( Basin_Modelling::Temperature ));
	PETSC_3D_Array layerDepth ( currentLayer -> layerDA, currentLayer -> Current_Properties ( Basin_Modelling::Depth ));
   DMDAGetCorners ( currentLayer->layerDA, PETSC_NULL, PETSC_NULL, &zStart, PETSC_NULL, PETSC_NULL, &zCount );

   int xPosition = xStart ;//1d case only, assumption for lower left corner
	int yPosition = yStart ;//1d case only, assumption for lower left corner
	//int zEnd = (zStart + zCount);
   //int K = zEnd - 1;
   int K = 0;
   
   //minus Current_Time to conform with the spec of Polyfunction for ascending X values...
   basinModel->m_bottomSedimentSurfaceAgeTemperatureCurve.AddPoint(( -Current_Time), layerTemperature( K,  yPosition, xPosition ) );
   basinModel->m_bottomSedimentSurfaceAgeVreCurve.AddPoint(( -Current_Time), layerVre( K,  yPosition, xPosition ) );
   basinModel->m_bottomSedimentSurfaceAgeDepthCurve.AddPoint(( -Current_Time), layerDepth( K,  yPosition, xPosition ) );
}

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::computeErosionFactorMaps"

void Basin_Modelling::computeErosionFactorMaps ( AppCtx* basinModel,
                                                 const double    age ) {

  // This property is always required.

  Layer_Iterator Layers;

  Layers.Initialise_Iterator ( basinModel->layers, Descending, Sediments_Only, Active_Layers_Only );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {
    Layers.Current_Layer ()->setErosionFactorMap ( basinModel, basinModel->getValidNeedles (), age );
  }

}


//------------------------------------------------------------//


#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::deleteErosionFactorMaps"

void Basin_Modelling::deleteErosionFactorMaps ( AppCtx* basinModel ) {

  // This property is always required.

  Layer_Iterator Layers;

  Layers.Initialise_Iterator ( basinModel->layers, Descending, Sediments_Only, Active_Layers_Only );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {
    Layers.Current_Layer ()->deleteErosionFactorMap ();
  }

}

//------------------------------------------------------------//

void Basin_Modelling::computeBasementLithostaticPressureForCurrentTimeStep ( AppCtx* basinModel,
                                                                             const double age ) {

   int i, j, k;

   size_t layer;
   int xStart;
   int yStart;
   int zStart;
   int xCount;
   int yCount;
   int zCount;

   // The bottom of the sediment pile. The layer that lies directly above the crust.
   int bottomSedimentIndex =  basinModel->layers.size () - 3;
   LayerProps_Ptr bottomLayer = basinModel->layers [ bottomSedimentIndex ];
   LayerProps_Ptr currentLayer;
      
   double segmentThickness;
      
   DMDAGetCorners ( *basinModel->mapDA,
                    &xStart, &yStart, PETSC_NULL, &xCount, &yCount, PETSC_NULL );
      
   Double_Array_2D lithostaticPressureAbove ( xCount, yCount );
      
   // Initialise the top lithostatic-pressure values.
   if ( age < basinModel->Crust()->depoage ) {
      PETSC_3D_Array sedimentLithostaticPressure ( bottomLayer->layerDA, 
                                                   bottomLayer->Current_Properties ( Basin_Modelling::Lithostatic_Pressure ));
         
      DMDAGetCorners ( bottomLayer->layerDA,
                       PETSC_NULL, PETSC_NULL, &zStart, PETSC_NULL, PETSC_NULL, &zCount );
         
      // Copy lithostatic pressure from the layer array to the local 2D array, for the top of the crust.
      copyBottomNodes ( xStart, xCount, yStart, yCount, basinModel->getValidNeedles (), lithostaticPressureAbove, sedimentLithostaticPressure );
   } else {
      //global indexing
      for ( i = xStart; i < xStart + xCount; ++i ) {
            
         for ( j = yStart; j < yStart + yCount; ++j ) {
               
            double hydrostaticPressureAtSeaBottom;
            computeHydrostaticPressure ( 0, // no fluid in the basement
                                         FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( i, j, age ),
                                         FastcauldronSimulator::getInstance ().getSeaBottomDepth ( i, j, age ),
                                         hydrostaticPressureAtSeaBottom );
               
            lithostaticPressureAbove ( i - xStart, j - yStart ) = hydrostaticPressureAtSeaBottom; 
         }
            
      }
         
   }
   double density;
      
   for ( layer = basinModel->layers.size () - 2; layer < basinModel->layers.size (); ++layer ) {
         
      currentLayer = basinModel->layers [ layer ];
           
      // Get the lithostatic pressure and depth of the current layer.
      PETSC_3D_Array lithostaticPressure ( currentLayer->layerDA,
                                           currentLayer->Current_Properties ( Basin_Modelling::Lithostatic_Pressure ));
            
      PETSC_3D_Array temp ( currentLayer->layerDA,
                            currentLayer->Current_Properties ( Basin_Modelling::Temperature ));
         
      PETSC_3D_Array depth ( currentLayer->layerDA,
                             currentLayer->Current_Properties ( Basin_Modelling::Depth ));
            
      DMDAGetCorners ( currentLayer->layerDA,
                       &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );
           
      initialiseTopNodes ( xStart, xCount, yStart, yCount, zCount - 1, basinModel->getValidNeedles (), lithostaticPressureAbove, lithostaticPressure );
            
      for ( i = xStart; i < xStart + xCount; ++i ) {
               
         for ( j = yStart; j < yStart + yCount; ++j ) {
                  
            if ( basinModel->nodeIsDefined ( i, j )) {

               for (k = zCount-2; k >= 0 ; --k  ) {
                  density = currentLayer->getLithology ( i, j, k )->getSimpleLithology()->getDensity ( temp( k, j, i ), lithostaticPressure ( k + 1, j, i ) );
                  segmentThickness = depth ( k, j, i ) - depth  ( k + 1, j, i );
                  lithostaticPressure ( k, j, i ) = lithostaticPressure ( k + 1, j, i ) + density * AccelerationDueToGravity * PaToMegaPa * segmentThickness;
               }

            }
                  
         }
               
      }         

      copyBottomNodes   ( xStart, xCount, yStart, yCount, basinModel->getValidNeedles (), lithostaticPressureAbove, lithostaticPressure );
   } 
}

//------------------------------------------------------------//

void Basin_Modelling::computeBasementLithostaticPressure ( AppCtx* basinModel,
                                                           const double age ) {

   if ( ( basinModel->DoOverPressure || basinModel->timefilter.PropertyOutputOption [ LITHOSTATICPRESSURE ] != SEDIMENTSPLUSBASEMENT ) &&
        ( not basinModel->isALC() )) {
    return;
  }
 
  int i;
  int j;
  int k;
  size_t layer;
  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;

  // The bottom of the sediment pile. The layer that lies directly above the crust.
  int bottomSedimentIndex =  basinModel->layers.size () - 3;
  LayerProps_Ptr bottomLayer = basinModel->layers [ bottomSedimentIndex ];
  LayerProps_Ptr currentLayer;

  double segmentThickness;

  DMDAGetCorners ( *basinModel->mapDA,
                   &xStart, &yStart, PETSC_NULL, &xCount, &yCount, PETSC_NULL );

  Double_Array_2D lithostaticPressureAbove ( xCount, yCount );

  // Initialise the top lithostatic-pressure values.
  if ( age < basinModel->Crust()->depoage ) {
    PETSC_3D_Array sedimentLithostaticPressure ( bottomLayer->layerDA, 
                                                 bottomLayer->Current_Properties ( Basin_Modelling::Lithostatic_Pressure ));

    DMDAGetCorners ( bottomLayer->layerDA,
                     PETSC_NULL, PETSC_NULL, &zStart, PETSC_NULL, PETSC_NULL, &zCount );

    // Copy lithostatic pressure from the layer array to the local 2D array, for the top of the crust.
    copyBottomNodes ( xStart, xCount, yStart, yCount, basinModel->getValidNeedles (), lithostaticPressureAbove, sedimentLithostaticPressure );
  } else {
     //global indexing
     for ( i = xStart; i < xStart + xCount; ++i ) {

       for ( j = yStart; j < yStart + yCount; ++j ) {
          
          double hydrostaticPressureAtSeaBottom;
          computeHydrostaticPressure ( 0, // no fluid in the basement
                                       FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( i, j, age ),
                                       FastcauldronSimulator::getInstance ().getSeaBottomDepth ( i, j, age ),
                                       hydrostaticPressureAtSeaBottom );

         lithostaticPressureAbove ( i - xStart, j - yStart ) = hydrostaticPressureAtSeaBottom; 
      }

    }

  }

  if( basinModel->isALC() ) {
     double density;
        
     for ( layer = basinModel->layers.size () - 2; layer < basinModel->layers.size (); ++layer ) {
           
        currentLayer = basinModel->layers [ layer ];

        // initialize lithostatic pressure for the first timestep
              
        // Get the lithostatic pressure and depth of the current layer.
        PETSC_3D_Array lithostaticPressure ( currentLayer->layerDA,
                                             currentLayer->Current_Properties ( Basin_Modelling::Lithostatic_Pressure ));
           
        PETSC_3D_Array temp ( currentLayer->layerDA,
                              currentLayer->Current_Properties ( Basin_Modelling::Temperature ));
           
        PETSC_3D_Array depth ( currentLayer->layerDA,
                               currentLayer->Current_Properties ( Basin_Modelling::Depth ));
              
        DMDAGetCorners ( currentLayer->layerDA,
                         &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );
              
        initialiseTopNodes ( xStart, xCount, yStart, yCount, zCount - 1, basinModel->getValidNeedles (), lithostaticPressureAbove, lithostaticPressure );
        
        for ( i = xStart; i < xStart + xCount; ++i ) {
           
           for ( j = yStart; j < yStart + yCount; ++j ) {
              
              if ( basinModel->nodeIsDefined ( i, j )) {

                 for (k = zCount-2; k >= 0 ; --k  ) {
                    density = currentLayer->getLithology ( i, j, k )->getSimpleLithology()->getDensity ( temp( k, j, i ), lithostaticPressure ( k + 1, j, i ) );
                    segmentThickness = depth ( k, j, i ) - depth  ( k + 1, j, i );
                    lithostaticPressure ( k, j, i ) = lithostaticPressure ( k + 1, j, i ) + density * AccelerationDueToGravity * PaToMegaPa * segmentThickness;
                 }

              }
              
           }
           
        }         

        copyBottomNodes   ( xStart, xCount, yStart, yCount, basinModel->getValidNeedles (), lithostaticPressureAbove, lithostaticPressure );
     }
  } else {
     for ( layer = basinModel->layers.size () - 2; layer < basinModel->layers.size (); ++layer ) {
        
        currentLayer = basinModel->layers [ layer ];
        
        // Get the lithostatic pressure and depth of the current layer.
        PETSC_3D_Array lithostaticPressure ( currentLayer->layerDA,
                                             currentLayer->Current_Properties ( Basin_Modelling::Lithostatic_Pressure ));
        
        PETSC_3D_Array depth ( currentLayer->layerDA,
                               currentLayer->Current_Properties ( Basin_Modelling::Depth ));
        
        DMDAGetCorners ( currentLayer->layerDA,
                         &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );
        
        initialiseTopNodes ( xStart, xCount, yStart, yCount, zCount - 1, basinModel->getValidNeedles (), lithostaticPressureAbove, lithostaticPressure );
        
        for ( i = xStart; i < xStart + xCount; ++i ) {
           
           for ( j = yStart; j < yStart + yCount; ++j ) {
              
              if ( basinModel->nodeIsDefined ( i, j )) {
                 
                 for (k = zCount-2; k >= 0 ; --k  )
                 {
                    segmentThickness = depth ( k, j, i ) - depth ( k + 1, j, i );
                    
                    // The intermediate nodal values for the litho-static pressure are not computed.
                    lithostaticPressure ( k, j, i ) = lithostaticPressure ( k + 1, j, i ) + currentLayer->getLithology ( i, j )->density () * AccelerationDueToGravity * PaToMegaPa * segmentThickness;
                 }
              }
              
           }
           
        }
        
        copyBottomNodes   ( xStart, xCount, yStart, yCount, basinModel->getValidNeedles (), lithostaticPressureAbove, lithostaticPressure );
     }
  }

}

//------------------------------------------------------------//
