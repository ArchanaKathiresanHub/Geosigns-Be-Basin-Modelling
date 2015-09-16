//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "StratigraphicColumn.h"

// Access to fastcauldron application code.
#include "FastcauldronSimulator.h"
#include "propinterface.h"

//------------------------------------------------------------//

StratigraphicColumn::StratigraphicColumn ( const LayerProps& topLayer,
                                           const LayerProps& bottomLayer ) {

   const AppCtx* cauldron = FastcauldronSimulator::getInstance ().getCauldron ();


   int topIndex    = cauldron->getLayerIndex ( topLayer.getName ());
   int bottomIndex = cauldron->getLayerIndex ( bottomLayer.getName ());
   int i;

   if ( topIndex > bottomIndex ) {
      std::cerr << " Top layer '"
                << topLayer.getName ()
                << "' is deeper than the bottom layer '"
                << bottomLayer.getName () << "'"
                << std::endl;
      // Should throw exception here.
   }

   m_maximumNumberOfElements = 0;

   for ( i = topIndex; i <= bottomIndex; ++i ) {
      m_layers.push_back ( cauldron->layers [ i ]);
      m_maximumNumberOfElements += cauldron->layers [ i ]->getMaximumNumberOfElements ();
   }

}

//------------------------------------------------------------//

StratigraphicColumn::~StratigraphicColumn () {
}

//------------------------------------------------------------//

bool StratigraphicColumn::containsLayer ( const LayerProps* formation ) const {
   LayerArray::const_iterator iter = std::find ( m_layers.begin (), m_layers.end (), formation );
   return iter != m_layers.end ();
}

//------------------------------------------------------------//

const LayerProps* StratigraphicColumn::getLayer ( const std::string& layerName ) const {

   for ( size_t i = 0; i < m_layers.size (); ++i ) {

      if ( m_layers [ i ]->getName () == layerName ) {
         return m_layers [ i ];
      }

   }

   return 0;
}

//------------------------------------------------------------//

size_t StratigraphicColumn::getLayerIndex ( const LayerProps* layer ) const {

   for ( size_t i = 0; i < m_layers.size (); ++i ) {

      if ( m_layers [ i ] == layer ) {
         return i;
      }

   }

   return m_layers.size () + 1;
}

//------------------------------------------------------------//

int StratigraphicColumn::getNumberOfLogicalElementsInDepth ( const double age ) const {

   if ( age < 0.0 ) {
      // Is this an error case?
      return 0;
   }

   int elementCount = 0;

   for ( size_t i = 0; i < m_layers.size (); ++i ) {
      
      if ( m_layers [ i ]->depositionStartAge > age ) {
         elementCount += m_layers [ i ]->getMaximumNumberOfElements ();
      }

   }

   return elementCount;
}

//------------------------------------------------------------//

int StratigraphicColumn::getMaximumNumberOfElementsInDepth () const {
   return m_maximumNumberOfElements;
}

//------------------------------------------------------------//

int StratigraphicColumn::getNumberOfLogicalNodesInDepth ( const double age ) const {

   int numberOfElements = getNumberOfLogicalElementsInDepth ( age );

   if ( numberOfElements > 0 ) {
      return numberOfElements + 1;
   } else {
      return numberOfElements;
   }

}

//------------------------------------------------------------//

int StratigraphicColumn::getMaximumNumberOfNodesInDepth () const {
   return m_maximumNumberOfElements + 1;
}

//------------------------------------------------------------//

size_t StratigraphicColumn::getTopLayerIndex ( const double age ) const {

   for ( size_t i = 0; i < m_layers.size (); ++i ) {

      if ( m_layers [ i ]->depoage <= age and age < m_layers [ i ]->depositionStartAge ) {
         return i;
      }

   }

   return m_layers.size () + 1;
}

//------------------------------------------------------------//
