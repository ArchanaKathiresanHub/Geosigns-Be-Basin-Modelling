//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "StratigraphicGrids.h"

#include "petsc.h"
#include "petscdm.h"

//------------------------------------------------------------//

StratigraphicGrids::StratigraphicGrids () : m_numberOfElements ( 0 ), m_numberOfNodes ( 0 ) {
}

//------------------------------------------------------------//

StratigraphicGrids::~StratigraphicGrids () {

   for ( size_t i = 0; i < m_elementVolumeGrids.size (); ++i ) {

      if ( m_elementVolumeGrids [ i ] != 0 ) {
         delete m_elementVolumeGrids [ i ];
      }

   }

   for ( size_t i = 0; i < m_nodalGrids.size (); ++i ) {

      if ( m_nodalGrids [ i ] != 0 ) {
         delete m_nodalGrids [ i ];
      }

   }

}

//------------------------------------------------------------//

void StratigraphicGrids::resizeGrids ( const int elementCount,
                                       const int nodeCount ) {

   if ( elementCount <= 0 or nodeCount <= 1 ) {
      return;
   }

   m_numberOfElements = elementCount;
   m_numberOfNodes = nodeCount;

   for ( size_t i = 0; i < m_elementVolumeGrids.size (); ++i ) {

      if ( m_elementVolumeGrids [ i ] != 0 ) {
         m_elementVolumeGrids [ i ]->resizeInZDirection ( m_numberOfElements );
      }

   }

   for ( size_t i = 0; i < m_nodalGrids.size (); ++i ) {

      if ( m_nodalGrids [ i ] != 0 ) {
         m_nodalGrids [ i ]->resizeInZDirection ( m_numberOfNodes );
      }

   }

}

//------------------------------------------------------------//

void StratigraphicGrids::allocateElementGrid ( const int numberOfDofs ) const {

   // Resize the array if array is too small, filling extra values with the null value.
   if ( m_elementVolumeGrids.size () < numberOfDofs ) {
      m_elementVolumeGrids.resize ( numberOfDofs, 0 );
   }

   if ( m_numberOfElements <= 0 ) {
      return;
   }

   // If the element-grid does not exist then create one.
   if ( m_elementVolumeGrids [ numberOfDofs - 1 ] == 0 ) {
      m_elementVolumeGrids [ numberOfDofs - 1 ] = new ElementVolumeGrid ( FastcauldronSimulator::getInstance ().getElementGrid (),
                                                                          m_numberOfElements,
                                                                          numberOfDofs );
   }

}

//------------------------------------------------------------//

void StratigraphicGrids::allocateNodeGrid ( const int numberOfDofs ) const {

   // Resize the array if array is too small, filling extra values with the null value.
   if ( m_nodalGrids.size () < numberOfDofs ) {
      m_nodalGrids.resize ( numberOfDofs, 0 );
   }

   if ( m_numberOfNodes == 0 ) {
      return;
   }

   // If the element-grid does not exist then create one.
   if ( m_nodalGrids [ numberOfDofs - 1 ] == 0 ) {
      m_nodalGrids [ numberOfDofs - 1 ] = new NodalVolumeGrid ( FastcauldronSimulator::getInstance ().getNodalGrid (),
                                                                m_numberOfNodes,
                                                                numberOfDofs );
   }

}

//------------------------------------------------------------//

ElementVolumeGrid& StratigraphicGrids::getElementGrid ( const int numberOfDofs ) {
   allocateElementGrid ( numberOfDofs );
   return *m_elementVolumeGrids [ numberOfDofs - 1 ];
}

//------------------------------------------------------------//

const ElementVolumeGrid& StratigraphicGrids::getElementGrid ( const int numberOfDofs ) const {
   allocateElementGrid ( numberOfDofs );
   return *m_elementVolumeGrids [ numberOfDofs - 1 ];
}

//------------------------------------------------------------//

Vec StratigraphicGrids::createElementVector ( const int numberOfDofs ) const {

   Vec result;

   DMCreateGlobalVector ( getElementGrid ( numberOfDofs ).getDa (), &result );
   return result;
}

//------------------------------------------------------------//

Vec StratigraphicGrids::createNodeVector ( const int numberOfDofs ) const {

   Vec result;

   DMCreateGlobalVector ( getNodeGrid ( numberOfDofs ).getDa (), &result );
   return result;
}

//------------------------------------------------------------//

NodalVolumeGrid& StratigraphicGrids::getNodeGrid ( const int numberOfDofs ) {
   allocateNodeGrid ( numberOfDofs );
   return *m_nodalGrids [ numberOfDofs - 1 ];
}

//------------------------------------------------------------//

const NodalVolumeGrid& StratigraphicGrids::getNodeGrid ( const int numberOfDofs ) const {
   allocateNodeGrid ( numberOfDofs );
   return *m_nodalGrids [ numberOfDofs - 1 ];
}

//------------------------------------------------------------//
