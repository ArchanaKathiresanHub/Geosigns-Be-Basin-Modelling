//------------------------------------------------------------//

#ifndef __Layer_Iterators_H__
#define __Layer_Iterators_H__

//------------------------------------------------------------//

#include<iostream>
using namespace std;
#include <vector>

//------------------------------------------------------------//

#include "layer.h"

//------------------------------------------------------------//


namespace Basin_Modelling {

  ///
  /// Direction in the basin the iterator should traverse. 
  /// Ascending is from the bottom of the basin to the top ie. The mantle (if included), 
  /// crust (if included) through to the layer that is currently being deposited
  /// Decending is the reverse of this.
  ///
  enum Iterator_Direction {
    ///
    ///
    /// From the bottom of the basin to the top
    ///
    Ascending,
    ///
    ///
    /// From the top of the basin to the bottom
    ///
    Descending };


  enum Layer_Range {
    ///
    ///
    /// All layers, that is: Sea Level, Sediments and Basement
    ///
    All_Layers,
    ///
    ///
    /// Only layers that form a part of the sediment (above the basement) excluding Sea Level
    ///
    Sediments_Only,
    ///
    ///
    /// Only the basement layers (Crust and Mantle)
    ///
    Basement_Only,
    ///
    ///
    /// All of the layers in the basin
    ///
    Basement_And_Sediments,
    ///
    ///
    /// Only layers that contain source rocks
    ///
    Source_Rocks_Only,
    ///
    ///
    /// Only layers that contain reservoirs
    ///
    Reservoirs_Only };


  enum Activity_Range {
    ///
    ///
    /// Only layers that are currently active, that is: Layers that have been deposited 
    /// or are in the process of being deposited and have not been entirely eroded.
    ///
    Active_Layers_Only,
    ///
    ///
    /// Only layers that are currently not active, that is: Layers that are 
    /// not yet deposited or ones that have been entirely eroded.
    ///
    Inactive_Layers_Only,
    ///
    ///
    /// All layers
    ///
    Active_And_Inactive_Layers };


//------------------------------------------------------------//

  ///
  ///
  /// An iterator over the layers that are in the basin
  ///
  class Layer_Iterator {

  public :

    Layer_Iterator ();

    Layer_Iterator ( const LayerList&           Basin_Layers,
                     const Iterator_Direction   Which_Direction,
                     const Layer_Range          Which_Range,
                     const Activity_Range       Activity );

    ~Layer_Iterator ();


    void Initialise_Iterator  ( const LayerList&           Basin_Layers,
                                const Iterator_Direction   Which_Direction,
                                const Layer_Range          Which_Range,
                                const Activity_Range       Activity );

    void Initialise_Iterator  ( const LayerList&           Basin_Layers,
                                const Iterator_Direction   Which_Direction,
                                const LayerProps_Ptr       Current_Layer,
                                const Iterator_Direction   Direction_From_Layer,
                                const Layer_Range          Which_Range,
                                const Activity_Range       Activity );

    void Initialise_Iterator  ( const LayerList&           Basin_Layers,
                                const std::string&         topFormationName,
                                const std::string&         bottomFormationName );





    ///
    /// The topmost layer in the list of layers that satisfies the range and activity requirements.
    ///
    LayerProps_Ptr Top_Layer () const;

    ///
    /// The bottommost layer in the list of layers that satisfies the range and activity requirements.
    ///
    LayerProps_Ptr Bottom_Layer () const;

    ///
    /// Layer above the current layer.
    ///
    LayerProps_Ptr Layer_Above () const;

    ///
    /// Layer below the current layer.
    ///
    LayerProps_Ptr Layer_Below () const;

    ///
    /// Current layer iterator is pointing to.
    ///
    LayerProps_Ptr Current_Layer () const;

    ///
    /// Number of layers that satisfy the range and activity requirements.
    ///
    int Number_Of_Layers () const;

    ///
    /// Resets iterator, but keeps the current list of layers.
    ///
    void Initialise_Iterator ();

    ///
    /// Determines if iteration is complete.
    ///
    bool Iteration_Is_Done () const;

    ///
    /// Advances iterator.
    ///
    void Next ();

    ///
    /// Advances iterator.
    ///
    Layer_Iterator& operator++( const int Dummy );

    ///
    /// Prints the number of layres and the name of each layer.
    ///
    friend ostream& operator<< ( ostream& Output_Stream, const Layer_Iterator Layers );

  private :


    bool Add_Layer ( const LayerProps_Ptr       Current_Layer,
                     const Layer_Range          Which_Range,
                     const Activity_Range  Activity ) const;
    //
    //
    // This should not be called
    //
    Layer_Iterator& operator= ( const Layer_Iterator& Copy ) const;
    //
    //
    //
    //
    void Nullify ();


    int                Number_Of_Iterator_Layers;
    int                Current_Layer_Position;
    int                Total_Number_Of_Layers;
    Iterator_Direction Direction;


//      typedef vector<LayerProps_Ptr> Layer_Vector;

//      Layer_Vector Iterated_Layers;
    LayerProps_Ptr* Iterated_Layers;

  }; // end class Layer_Iterator


} // end namespace Basin_Modelling


//------------------------------------------------------------//

#endif // __Layer_Iterators_H__

//------------------------------------------------------------//



