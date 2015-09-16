//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAULDRON__STRATIGRAPHIC_COLUMN__H
#define FASTCAULDRON__STRATIGRAPHIC_COLUMN__H

// STL library.
#include <vector>

// Geophysics and DataAccess library.
#include "Interface/Interface.h"

// Access to fastcauldron application code.
#include "layer.h"

/// \brief A sequence of layers.
///
/// The column is bounded by the top and bottom layers.
/// The top and bottom layer may be the same, in which case the column will consist
/// of a single layer.
/// The top layer need not be the top layer of the cauldron model.
/// Also, the bottom layer need not be the bottom layer of the cauldron model.
/// The layers are numbered such that 0 is at the top.
class StratigraphicColumn {

   /// \brief A simple 1d array of layers.
   typedef std::vector<LayerProps*> LayerArray;

public :

   /// \brief Constructor with top and bottom layers.
   ///
   /// The top and bottom layers or the StratigraphicColumn.
   /// The column will consists of all layer in between and including these layers.
   /// \param [in] topLayer The top layer of the computational domain.
   /// \param [in] bottomLayer The bottom layer of the computational domain.
   /// \pre topLayer should be the same age or younger than the bottomLayer.
   StratigraphicColumn ( const LayerProps& topLayer,
                         const LayerProps& bottomLayer );

   /// \brief Destructor.
   ~StratigraphicColumn ();

   /// \name Stratigraphic layer information.
   //@{

   /// \brief Return the number of layers in the column
   size_t getNumberOfLayers () const;

   /// \brief Get the layer at the position i.
   const LayerProps* getLayer ( const size_t i ) const;

   /// \brief Get the layer with the name.
   const LayerProps* getLayer ( const std::string& layerName ) const;

   /// \brief Get the layer at the position i.
   LayerProps* getLayer ( const size_t i );

   /// \brief Get the layer at the position i.
   size_t getLayerIndex ( const LayerProps* layer ) const;

   /// \brief Determine whether or not a formation is a part of this stratigraphic column.
   bool containsLayer ( const LayerProps* formation ) const;

   /// \brief Determine whether or not the stratigraphic column include basement formations.
   bool includesBasement () const;

   /// \brief Get the index of the top most layer at the particular age.
   ///
   /// If the age is earlier than the bottom most surface then the number to
   /// be returned  will be 1 greater than the number of layers in the column.
   size_t getTopLayerIndex ( const double age ) const;

   //@}

   /// \name Stratigraphic elements and nodes in depth.
   //@{

   /// \brief Count the number of elements in depth for the layers that are currently active.
   ///
   /// This number will also include all the elements that have zero thickness.
   int getNumberOfLogicalElementsInDepth ( const double age ) const;

   /// \brief Count the number of elements in depth for all layers.
   ///
   /// This number will also include all the elements that have zero thickness.
   int getMaximumNumberOfElementsInDepth () const;


   /// \brief Count the number of nodes in depth for the layers that are currently active.
   ///
   /// This number will also include all the nodes from elements that have zero thickness.
   int getNumberOfLogicalNodesInDepth ( const double age ) const;

   /// \brief Count the number of nodes in depth for all layers.
   ///
   /// This number will also include all the nodes from elements that have zero thickness.
   int getMaximumNumberOfNodesInDepth () const;

   //@}

private :

   // Disallow default construction.
   StratigraphicColumn (); // = delete;

   // Disallow copy construction.
   StratigraphicColumn ( const StratigraphicColumn& ); // = delete;

   // Disallow assignment.
   StratigraphicColumn& operator=( const StratigraphicColumn&); // = delete;


   /// list of layers in the column.
   LayerArray m_layers;

   /// \brief The maximum number of elements in the depth of the column.
   int        m_maximumNumberOfElements;

};

//------------------------------------------------------------//
// Inline functions

inline size_t StratigraphicColumn::getNumberOfLayers () const {
   return m_layers.size ();
}

inline const LayerProps* StratigraphicColumn::getLayer ( const size_t i ) const {
   return m_layers.at ( i );
}

inline LayerProps* StratigraphicColumn::getLayer ( const size_t i ) {
   return m_layers.at ( i );
}

inline bool StratigraphicColumn::includesBasement () const {
   // Need look only at the bottommost layer.
   return m_layers [ m_layers.size () - 1 ]->kind () == Interface::BASEMENT_FORMATION;
}

#endif // FASTCAULDRON__STRATIGRAPHIC_COLUMN__H


