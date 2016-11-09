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

#include "layer_iterators.h"
#include "Interface/SourceRock.h"

//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::constructor1"

Basin_Modelling::Layer_Iterator::Layer_Iterator () {

  Nullify ();

} // end constructor


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::constructor2"

Basin_Modelling::Layer_Iterator::Layer_Iterator
   ( const LayerList&           Basin_Layers,
     const Iterator_Direction   Which_Direction,
     const Layer_Range          Which_Range,
     const Activity_Range       Activity ) {

  Nullify ();
  Initialise_Iterator ( Basin_Layers, Which_Direction, Which_Range, Activity );
} // end constructor


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::destructor"

Basin_Modelling::Layer_Iterator::~Layer_Iterator () {

  if ( Iterated_Layers != nullptr ) {
    delete [] Iterated_Layers;
  } // end if

//    Iterated_Layers.clear ();
} // end destructor


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::Nullify"

void Basin_Modelling::Layer_Iterator::Nullify () {

  Number_Of_Iterator_Layers = 0;
  Current_Layer_Position = 0;
  Total_Number_Of_Layers = 0;
  Iterated_Layers = 0;

} // end Nullify


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::Add_Layer"

bool Basin_Modelling::Layer_Iterator::Add_Layer ( const LayerProps_Ptr       Current_Layer,
                                                  const Layer_Range          Which_Range,
                                                  const Activity_Range       Activity ) const {


   if ( Current_Layer == 0 ) {
      return false;
   }

  bool Include_Layer = false;
  //
  //
  // First, find if the layer is in the Layer_Range.
  //
  switch ( Which_Range ) {

    case All_Layers :

      Include_Layer = true;
      break;

    case Sediments_Only :

      Include_Layer = Current_Layer -> isSediment ();
      break;

    case Basement_Only :

      Include_Layer = Current_Layer -> isBasement ();
      break;

    case Basement_And_Sediments :

      Include_Layer = Current_Layer -> isSediment () || Current_Layer -> isBasement ();
      break;

    case Source_Rocks_Only :

       Include_Layer = Current_Layer -> isSourceRock ();
     break;

    case Shale_Gas_Only :
       
       Include_Layer = Current_Layer -> isSourceRock () && ( Current_Layer -> getSourceRock1() -> doApplyAdsorption() ||
                                                           ( Current_Layer -> getSourceRock2() ? Current_Layer -> getSourceRock2() -> doApplyAdsorption() : false ));
      break;

    case Reservoirs_Only :

//          Include_Layer = Current_Layer -> isReservoir ();
//          break;

    default :
      //
      //
      // Should never be in here. If we do end up in the 'default' then
      // should we raise an exception, give a warning,...
      // The simplest solution, is to NOT add any layer to the list, as 
      // it is not known what the user wants.
      //
      Include_Layer = false;

  } // end switch
  //
  //
  // Now check for the required active / in-active layers.
  //
  switch ( Activity ) {

    case Active_Layers_Only :

      Include_Layer = Include_Layer && Current_Layer  -> isActive ();
      break;

    case Inactive_Layers_Only :

      Include_Layer = Include_Layer && not Current_Layer  -> isActive ();
      break;

    case Active_And_Inactive_Layers :
      //
      //
      // There is nothing to do here, as isActive or isNotActive = isActive or not isActive = true always!
      //
//        Include_Layer = Include_Layer && ( Current_Layer->isActive () || 
//                                           not Current_Layer->isActive ());
      break;

    default :
      //
      //
      // There should be some error warning/exception here
      //
      Include_Layer = false;

  } // end switch

  return Include_Layer;
} // end Basin_Modelling::Layer_Iterator::Add_Layer


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::Initialise_Iterator"

void Basin_Modelling::Layer_Iterator::Initialise_Iterator
   ( const LayerList&           Basin_Layers,
     const Iterator_Direction   Which_Direction,
     const Layer_Range          Which_Range,
     const Activity_Range       Activity ) {

  int  Layer_Index;

  //
  //
  // Here should only deallocate if the Total_Number_Of_Layers /= Basin_Layers.size ()
  // as the total number of layers CANNOT change within a run!
  //
  if ( Iterated_Layers != nullptr ) {
    delete [] Iterated_Layers;
  } // end if

//    Iterated_Layers.clear ();

  Direction = Which_Direction;

  Total_Number_Of_Layers = Basin_Layers.size ();
  Iterated_Layers = new LayerProps_Ptr [ Total_Number_Of_Layers ];
  //
  //
  // Number of layers that will be in the iterator, initialised at zero.
  // It will be incremented for each additional layer added to the iterator.
  //
  Number_Of_Iterator_Layers = 0;

  for ( Layer_Index = 0; Layer_Index < Total_Number_Of_Layers; Layer_Index++ ) {

    if ( this -> Add_Layer ( Basin_Layers [ Layer_Index ], Which_Range, Activity )) {
      //
      //
      // Only add the layer to the iterator IF range and activity requirements have been satisified.
      //
//        Iterated_Layers.push_back ( Basin_Layers [ Layer_Index ]);
      Iterated_Layers [ Number_Of_Iterator_Layers ] = Basin_Layers [ Layer_Index ];
      Number_Of_Iterator_Layers = Number_Of_Iterator_Layers + 1;
    } // end if

  } // end loop

  if ( Which_Direction == Ascending ) {
    //
    //
    // If Ascending is requested then the order of the array must be reversed
    //
    LayerProps_Ptr Temp;

    for ( Layer_Index = 0; Layer_Index < ( Number_Of_Iterator_Layers + 1 ) / 2; Layer_Index++ ) {
      Temp = Iterated_Layers [ Layer_Index ];
      Iterated_Layers [ Layer_Index ] = Iterated_Layers [ Number_Of_Iterator_Layers - Layer_Index - 1 ];
      Iterated_Layers [ Number_Of_Iterator_Layers - Layer_Index - 1 ] = Temp;
    } // end loop

  } // end if

  Current_Layer_Position = 0;
} // end Initialise_Iterator


//------------------------------------------------------------//


#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::Initialise_Iterator"

void Basin_Modelling::Layer_Iterator::Initialise_Iterator
   ( const LayerList&           Basin_Layers,
     const Iterator_Direction   Which_Direction,
     const LayerProps_Ptr       Current_Layer,
     const Iterator_Direction   Direction_From_Layer,
     const Layer_Range          Which_Range,
     const Activity_Range       Activity ) {

  int Layer_Index;
  int Start_Index;
  int End_Index;
  int Current_Layer_Index = -1;

  if ( Iterated_Layers == 0 ) {
    Total_Number_Of_Layers = Basin_Layers.size ();
    Iterated_Layers = new LayerProps_Ptr [ Total_Number_Of_Layers ];
  } // end if

  for ( Layer_Index = 0; Layer_Index < Total_Number_Of_Layers; Layer_Index++ ) {

    if ( Basin_Layers [ Layer_Index ] == Current_Layer ) {
      Current_Layer_Index = Layer_Index;
      break;
    }

  }

  if ( Current_Layer_Index == -1 ) {
    //
    //
    // Current layer could not be found in the basin.
    //
    Number_Of_Iterator_Layers = 0;
    return;
  }

  if ( Direction_From_Layer == Ascending ) {
    Start_Index = 0;
    End_Index   = Current_Layer_Index;
  } else {
    Start_Index = Current_Layer_Index;
    End_Index   = Total_Number_Of_Layers - 1;
  }

  Direction = Which_Direction;
  //
  //
  // Number of layers that will be in the iterator, initialised at zero.
  // It will be incremented for each additional layer added to the iterator.
  //
  Number_Of_Iterator_Layers = 0;

  for ( Layer_Index = Start_Index; Layer_Index <= End_Index; Layer_Index++ ) {

    if ( this -> Add_Layer ( Basin_Layers [ Layer_Index ], Which_Range, Activity )) {
      //
      //
      // Only add the layer to the iterator IF range and activity requirements have been satisified.
      //
//        Iterated_Layers.push_back ( Basin_Layers [ Layer_Index ]);
      Iterated_Layers [ Number_Of_Iterator_Layers ] = Basin_Layers [ Layer_Index ];
      Number_Of_Iterator_Layers = Number_Of_Iterator_Layers + 1;
    } // end if

  } // end loop

  if ( Which_Direction == Ascending ) {
    //
    //
    // If Ascending is requested then the order of the array must be reversed
    //
    LayerProps_Ptr Temp;

    for ( Layer_Index = 0; Layer_Index < ( Number_Of_Iterator_Layers + 1 ) / 2; Layer_Index++ ) {
      Temp = Iterated_Layers [ Layer_Index ];
      Iterated_Layers [ Layer_Index ] = Iterated_Layers [ Number_Of_Iterator_Layers - Layer_Index - 1 ];
      Iterated_Layers [ Number_Of_Iterator_Layers - Layer_Index - 1 ] = Temp;
    } // end loop

  } // end if

  Current_Layer_Position = 0;
} // end Initialise_Iterator


//------------------------------------------------------------//


#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::Initialise_Iterator"

void Basin_Modelling::Layer_Iterator::Initialise_Iterator
   ( const LayerList&           Basin_Layers,
     const std::string&         topFormationName,
     const std::string&         bottomFormationName ) {

  const Layer_Range    Which_Range = Sediments_Only;
  const Activity_Range Activity    = Active_And_Inactive_Layers;

  int layerIndex;
  int startIndex = -1;
  int endIndex = -1;


  if ( Iterated_Layers == 0 ) {
    Total_Number_Of_Layers = Basin_Layers.size ();
    Iterated_Layers = new LayerProps_Ptr [ Total_Number_Of_Layers ];
  } // end if

  for ( layerIndex = 0; layerIndex < Total_Number_Of_Layers; layerIndex++ ) {

    if ( Basin_Layers [ layerIndex ]->layername == topFormationName ) {
      startIndex = layerIndex;
      break;
    }

  }

  for ( layerIndex = 0; layerIndex < Total_Number_Of_Layers; layerIndex++ ) {

    if ( Basin_Layers [ layerIndex ]->layername == bottomFormationName ) {
      endIndex = layerIndex;
      break;
    }

  }

  if ( startIndex == -1 || endIndex == -1 ) {
    //
    //
    // One of the layers could not be found in the basin.
    //
    Number_Of_Iterator_Layers = 0;
    return;
  }

  if ( startIndex > endIndex ) {
    int temp = startIndex;
    startIndex = endIndex;
    endIndex = temp;
  }

  Direction = Descending;

  ///
  /// Number of layers that will be in the iterator, initialised at zero.
  /// It will be incremented for each additional layer added to the iterator.
  ///
  Number_Of_Iterator_Layers = 0;

  for ( layerIndex = startIndex; layerIndex <= endIndex; layerIndex++ ) {

    if ( this -> Add_Layer ( Basin_Layers [ layerIndex ], Which_Range, Activity )) {
      //
      //
      // Only add the layer to the iterator IF range and activity requirements have been satisified.
      //
//        Iterated_Layers.push_back ( Basin_Layers [ Layer_Index ]);
      Iterated_Layers [ Number_Of_Iterator_Layers ] = Basin_Layers [ layerIndex ];
      Number_Of_Iterator_Layers = Number_Of_Iterator_Layers + 1;
    } // end if

  } // end loop

  Current_Layer_Position = 0;
} // end Initialise_Iterator


//------------------------------------------------------------//

//  void Initialise () {

//    if ( Direction == Ascending ) { 

//      for ( I = N - 1; I >= 0; I-- ) {

//        if ( Layer_Should_Be_Added ( layers [ I ], Which_Range, Activity )) {
//          Iterated_Layers [ Layer_Index ] = layer [ I ];
//          Layer_Index = Layer_Index + 1;
//        } 

//      }

//    } else {

//      for ( I = 0; I <= N - 1; I++ ) {

//        if ( Layer_Should_Be_Added ( layers [ I ], Which_Range, Activity )) {
//          Iterated_Layers [ Layer_Index ] = layer [ I ];
//          Layer_Index = Layer_Index + 1;
//        }

//      }

//    }

//  } // 

//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::Number_Of_Layers"

int Basin_Modelling::Layer_Iterator::Number_Of_Layers () const {
  return Number_Of_Iterator_Layers;
} // end Top_Layer


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::Top_Layer"

LayerProps_Ptr Basin_Modelling::Layer_Iterator::Top_Layer () const {

  LayerProps_Ptr Result_Layer;

  if ( Number_Of_Iterator_Layers > 0 ) {

    if ( Direction == Ascending ) {
      Result_Layer = Iterated_Layers [ Number_Of_Iterator_Layers - 1 ];
    } else {
      Result_Layer = Iterated_Layers [ 0 ];
    }

  } else {
    Result_Layer = 0;
  } // end if

  return Result_Layer;
} // end Top_Layer


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::Bottom_Layer"

LayerProps_Ptr Basin_Modelling::Layer_Iterator::Bottom_Layer () const {

  LayerProps_Ptr Result_Layer;

  if ( Number_Of_Iterator_Layers > 0 ) {

    if ( Direction == Ascending ) {
      Result_Layer = Iterated_Layers [ 0 ];
    } else {
      Result_Layer = Iterated_Layers [ Number_Of_Iterator_Layers - 1 ];
    }

  } else {
    Result_Layer = 0;
  } // end if

  return Result_Layer;
} // end Bottom_Layer


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::Layer_Above"

LayerProps_Ptr Basin_Modelling::Layer_Iterator::Layer_Above () const {

  LayerProps_Ptr Result_Layer;

  if ( Number_Of_Iterator_Layers > 1 ) {

    if ( Direction == Ascending && Current_Layer_Position < Number_Of_Iterator_Layers - 1 ) {
      Result_Layer = Iterated_Layers [ Current_Layer_Position + 1 ];
    } else if ( Direction == Descending && Current_Layer_Position > 0 ) {
      Result_Layer = Iterated_Layers [ Current_Layer_Position - 1 ];
    } else {
      Result_Layer = 0;
    }

  } else {
    Result_Layer = 0;
  } // end if

  return Result_Layer;
} // end Layer_Above


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::Layer_Below"

LayerProps_Ptr Basin_Modelling::Layer_Iterator::Layer_Below () const {

  LayerProps_Ptr Result_Layer;

  if ( Number_Of_Iterator_Layers > 1 ) {

    if ( Direction == Descending && Current_Layer_Position < Number_Of_Iterator_Layers - 1 ) {
      Result_Layer = Iterated_Layers [ Current_Layer_Position + 1 ];
    } else if ( Direction == Ascending && Current_Layer_Position > 0 ) {
      Result_Layer = Iterated_Layers [ Current_Layer_Position - 1 ];
    } else {
      Result_Layer = 0;
    }

  } else {
    Result_Layer = 0;
  } // end if

  return Result_Layer;
} // end Layer_Below


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::Current_Layer"

LayerProps_Ptr Basin_Modelling::Layer_Iterator::Current_Layer () const {

  LayerProps_Ptr Result_Layer;

  if ( Current_Layer_Position < Number_Of_Iterator_Layers ) {
    Result_Layer = Iterated_Layers [ Current_Layer_Position ];
  } else {
    Result_Layer = 0;
  } // end if

  return Result_Layer;
} // end Current_Layer


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::operator++"

Basin_Modelling::Layer_Iterator& Basin_Modelling::Layer_Iterator::operator++ ( const int Dummy ) {

  if ( Current_Layer_Position < Number_Of_Iterator_Layers ) {
    Current_Layer_Position = Current_Layer_Position + 1;
  } // end if

  return *this;
} // end operator++


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::Next"

void Basin_Modelling::Layer_Iterator::Next () {

  if ( Current_Layer_Position < Number_Of_Iterator_Layers ) {
    Current_Layer_Position = Current_Layer_Position + 1;
  } // end if

} // end Next


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::Iteration_Is_Done"

bool Basin_Modelling::Layer_Iterator::Iteration_Is_Done () const {
  return Number_Of_Iterator_Layers == Current_Layer_Position;
} // end Iteration_Is_Done


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::Initialise_Iterator"

void Basin_Modelling::Layer_Iterator::Initialise_Iterator () {
  Current_Layer_Position = 0;
} // end Initialise_Iterator


//------------------------------------------------------------//

namespace Basin_Modelling {

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::Layer_Iterator::operator<<"

   ostream& operator<< ( ostream& Output_Stream, const Layer_Iterator Layers ) {

      int I;

      Output_Stream << " There are " << Layers.Number_Of_Iterator_Layers << " layers " << endl;

      for ( I = 0; I < Layers.Number_Of_Iterator_Layers; I++ ) {
         Output_Stream << " Layer " << I + 1 << ": " << Layers.Iterated_Layers [ I ]->layername << endl;
      } // end loop

      return Output_Stream;
   } // end operator <<

}

//============================================================//
