//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAULDRON__FORMATION_ELEMENT_GRID__H
#define FASTCAULDRON__FORMATION_ELEMENT_GRID__H

// Access to fastcauldron application classes
#include "BoundaryId.h"
#include "ghost_array.h"
#include "globaldefs.h"
#include "layer.h"
#include "LayerElement.h"

/// \brief A three dimensional array of element-types.
///
/// \param ElementType The grid is to be made from elements of this type.
/// \pre ElementType must have functions to set a LayerElement reference.
/// \pre ElementType must have functions to set a neighbour ElementType, which may be null.
template<class ElementType>
class FormationElementGrid {

   /// \brief 3D-array of general-elements.
   typedef PETSc_Local_3D_Array <ElementType> FormationElementArray;

public :

   /// \brief Constructor.
   FormationElementGrid ( LayerProps& layer );

   /// \brief Destructor.
   virtual ~FormationElementGrid ();

   /// \brief Get whether or not the formation is active.
   bool isActive () const;

   /// \brief Get a constant reference to the element at the position.
   const ElementType& getElement ( const int i,
                                   const int j,
                                   const int k ) const;

   /// \brief Get a reference to the element at the position.
   ElementType& getElement ( const int i,
                             const int j,
                             const int k );

   /// \brief The first index in the x-direction.
   int firstI () const;

   /// \brief The first index in the y-direction.
   int firstJ () const;

   /// \brief The first index in the z-direction.
   ///
   /// Counting starts from bottom up, i.e. firstK is the bottom element in the stack of elements.
   int firstK () const;

   /// \brief The last index in the x-direction.
   int lastI () const;

   /// \brief The last index in the y-direction.
   int lastJ () const;

   /// \brief The last index in the z-direction.
   ///
   /// Counting starts from bottom up, i.e. lastK is the top element in the stack of elements.
   int lastK () const;

   /// \brief The number of items in the I-dimension.
   int lengthI () const;

   /// \brief The number of items in the J-dimension.
   int lengthJ () const;

   /// \brief The number of items in the K-dimension.
   int lengthK () const;


   /// \brief Get the formation associated with this object.
   const LayerProps& getFormation () const;

   /// \brief Get the formation associated with this object.
   LayerProps& getFormation ();


   /// \brief Set the node numbers for each element.
   ///
   /// These are relative to the regular 3d grid.
   void setElementNodeKValues ( const LocalIntegerArray3D& kValues,
                                const int                  topKValue,
                                const int                  globalKValue );

   /// \brief Get the indicies in depth for the nodes of the formation.
   ///
   /// These indices are relative to the subdomain to which this object belongs.
   const IntegerArray& getSubdomainNodeKIndices () const;


protected :

   // Disallow default constructor
   FormationElementGrid (); // = delete

   // Disallow copying of object of this type.
   FormationElementGrid ( const FormationElementGrid& copy ); // = delete

   FormationElementGrid& operator=( const FormationElementGrid& copy ); // = delete



   /// \brief Set the general-elements with the layer-element.
   void assignLayerElements ();

   /// \brief Link together all elements of the layer.
   void linkLayerElements ();


   /// \brief The formation associated with an object.
   LayerProps&     m_formation;

   /// \brief The general-elements for this object.
   FormationElementArray m_elements;

   /// \brief The index values in z-direction for the subdomain contained in this formation.
   IntegerArray          m_subdomainNodeKIndices;

};

//------------------------------------------------------------//

template<class ElementType>
FormationElementGrid<ElementType>::FormationElementGrid ( LayerProps& layer ) : m_formation ( layer ) {

   m_elements.create ( m_formation.getVolumeGrid ().getDa ());
   assignLayerElements ();
   linkLayerElements ();

}

//------------------------------------------------------------//

template<class ElementType>
FormationElementGrid<ElementType>::~FormationElementGrid () {
}

//------------------------------------------------------------//

template<class ElementType>
inline int FormationElementGrid<ElementType>::firstI () const {
   return m_elements.firstI ( false );
}

//------------------------------------------------------------//

template<class ElementType>
inline int FormationElementGrid<ElementType>::firstJ () const {
   return m_elements.firstJ ( false );
}

//------------------------------------------------------------//

template<class ElementType>
inline int FormationElementGrid<ElementType>::firstK () const {
   return m_elements.firstK ();
}

//------------------------------------------------------------//

template<class ElementType>
inline int FormationElementGrid<ElementType>::lastI () const {
   return m_elements.lastI ( false );
}

//------------------------------------------------------------//

template<class ElementType>
inline int FormationElementGrid<ElementType>::lastJ () const {
   return m_elements.lastJ ( false );
}

//------------------------------------------------------------//

template<class ElementType>
inline int FormationElementGrid<ElementType>::lastK () const {
   return m_elements.lastK ();
}

//------------------------------------------------------------//

template<class ElementType>
inline int FormationElementGrid<ElementType>::lengthI () const {
   return m_elements.lastI ( false ) - m_elements.firstI ( false ) + 1;
}

//------------------------------------------------------------//

template<class ElementType>
inline int FormationElementGrid<ElementType>::lengthJ () const {
   return m_elements.lastJ ( false ) - m_elements.firstJ ( false ) + 1;
}

//------------------------------------------------------------//

template<class ElementType>
inline int FormationElementGrid<ElementType>::lengthK () const {
   return m_elements.lastK () - m_elements.firstK () + 1;
}

//------------------------------------------------------------//

template<class ElementType>
inline ElementType& FormationElementGrid<ElementType>::getElement ( const int i,
                                                                    const int j,
                                                                    const int k ) {
   return m_elements ( i, j, k );
}

//------------------------------------------------------------//

template<class ElementType>
inline const ElementType& FormationElementGrid<ElementType>::getElement ( const int i,
                                                                          const int j,
                                                                          const int k ) const {
   return m_elements ( i, j, k );
}

//------------------------------------------------------------//

template<class ElementType>
inline const LayerProps& FormationElementGrid<ElementType>::getFormation () const {
   return m_formation;
}

//------------------------------------------------------------//

template<class ElementType>
inline LayerProps& FormationElementGrid<ElementType>::getFormation () {
   return m_formation;
}

//------------------------------------------------------------//

template<class ElementType>
const IntegerArray& FormationElementGrid<ElementType>::getSubdomainNodeKIndices () const {
   return m_subdomainNodeKIndices;
}

//------------------------------------------------------------//

template<class ElementType>
void FormationElementGrid<ElementType>::assignLayerElements () {

   const LayerElementArray& layerElements = m_formation.getLayerElements ();

   int i;
   int j;
   int k;

   for ( i = layerElements.firstI ( true ); i <= layerElements.lastI ( true ); ++i ) {

      for ( j = layerElements.firstJ ( true ); j <= layerElements.lastJ ( true ); ++j ) {

         for ( k = layerElements.firstK (); k <= layerElements.lastK (); ++k ) {
            m_elements ( i, j, k ).setLayerElement ( layerElements ( i, j, k ));
         }

      }

   }

}

//------------------------------------------------------------//

template<class ElementType>
void FormationElementGrid<ElementType>::linkLayerElements () {

   const MapElementArray& mapElements = FastcauldronSimulator::getInstance ().getMapElementArray ();

   unsigned int i;
   unsigned int j;
   unsigned int k;

   for ( i = mapElements.firstI ( true ); i <= mapElements.lastI ( true ); ++i ) {

      for ( j = mapElements.firstJ ( true ); j <= mapElements.lastJ ( true ); ++j ) {

         const MapElement& mapElement = mapElements ( i, j );

         for ( k = 0; k < m_formation.getMaximumNumberOfElements (); ++k ) {
            m_elements ( i, j, k ).clearNeighbours ();

            if ( mapElement.isOnProcessor ()) {

               if ( not mapElement.isOnDomainBoundary ( MapElement::Front )) {
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::Front, &m_elements ( i, j - 1, k ));
               }

               if ( not mapElement.isOnDomainBoundary ( MapElement::Right )) {
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::Right, &m_elements ( i + 1, j, k ));
               }

               if ( not mapElement.isOnDomainBoundary ( MapElement::Back )) {
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::Back, &m_elements ( i, j + 1, k ));
               }

               if ( not mapElement.isOnDomainBoundary ( MapElement::Left )) {
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::Left, &m_elements ( i - 1, j, k ));
               }

               if ( k > 0 ) {
                  // Add element below to bottom face.
                  // The elments (in the thickness of the layer) are numbered 0 = deepest, n == shallowest.
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::DeepFace, &m_elements ( i, j, k - 1 ));
               }

               if ( k < m_formation.getMaximumNumberOfElements () - 1 ) {
                  // Add element above to top face.
                  // The elments (in the thickness of the layer) are numbered 0 = deepest, n == shallowest.
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::ShallowFace, &m_elements ( i, j, k + 1 ));
               }

            }

         }

      }

   }

}

//------------------------------------------------------------//

template<class ElementType>
void FormationElementGrid<ElementType>::setElementNodeKValues ( const LocalIntegerArray3D& kValues,
                                                                const int                  topKValue,
                                                                const int                  globalKValue ) {

   m_subdomainNodeKIndices.clear ();
   m_subdomainNodeKIndices.resize ( m_formation.getMaximumNumberOfElements () + 1, 0 );
   int globalK = globalKValue;

   for ( int k = m_formation.getMaximumNumberOfElements (); k >= 0; --k, --globalK ) {
      m_subdomainNodeKIndices [ k ] = globalK;
   }

   for ( int i = firstI (); i <= lastI (); ++i ) {

      for ( int j = firstJ (); j <= lastJ (); ++j ) {
         globalK = globalKValue;

         for ( int k = m_formation.getMaximumNumberOfElements () - 1; k >= 0; --k, --globalK ) {

            ElementType& element = getElement ( i, j, k );

            int n1 = static_cast<int>( kValues ( i,     j,     globalK - 1 ));
            int n2 = static_cast<int>( kValues ( i + 1, j,     globalK - 1 ));
            int n3 = static_cast<int>( kValues ( i + 1, j + 1, globalK - 1 ));
            int n4 = static_cast<int>( kValues ( i,     j + 1, globalK - 1 ));

            int n5 = static_cast<int>( kValues ( i,     j,     globalK ));
            int n6 = static_cast<int>( kValues ( i + 1, j,     globalK ));
            int n7 = static_cast<int>( kValues ( i + 1, j + 1, globalK ));
            int n8 = static_cast<int>( kValues ( i,     j + 1, globalK ));

            // Set node numbers.
            element.setNodeK ( 0, n1 );
            element.setNodeK ( 1, n2 );
            element.setNodeK ( 2, n3 );
            element.setNodeK ( 3, n4 );

            element.setNodeK ( 4, n5 );
            element.setNodeK ( 5, n6 );
            element.setNodeK ( 6, n7 );
            element.setNodeK ( 7, n8 );

            element.setShallowIsOnDomainBoundary ( n1 == topKValue and n2 == topKValue and n3 == topKValue and n4 == topKValue );
            element.setDeepIsOnDomainBoundary ( n5 == 0 and n6 == 0 and n7 == 0 and n8 == 0 );
         }

      }

   }

}

//------------------------------------------------------------//

#endif // FASTCAULDRON__FORMATION_ELEMENT_GRID__H
