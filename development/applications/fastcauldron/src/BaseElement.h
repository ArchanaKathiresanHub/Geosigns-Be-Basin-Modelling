//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAULDRON__BASE_ELEMENT__H
#define FASTCAULDRON__BASE_ELEMENT__H

#include "LayerElement.h"
#include "BoundaryId.h"



/// \brief An element that is a part of a subdomain.
///
/// These exist mainly because the dof-, node- and element numbering
/// can be different for each subdomain, so we need to keep account of this.
template<class ParentElement>
class BaseElement : public ParentElement {

public :

   /// The number of faces the element has.
   ///
   /// Since all elements are hexahedron there are 6 faces for all elements.
   static const int NumberOfBoundaries = VolumeData::NumberOfBoundaries;


   BaseElement ();

   virtual ~BaseElement () {}


   /// Get the I position of the element within the subdomain.
   int getI () const;

   /// Get the J position of the element within the subdomain.
   int getJ () const;

   /// \brief Set K position within subdomain.
   void setK ( const int k );

   /// \brief Get K position of the element within subdomain.
   int getK () const;

   /// \name Node position.
   //@{

   /// \brief Set the k value for the node.
   ///
   /// \param node A node of the element.
   /// \param k    The k-index value of the node.
   /// \pre node in range [ 0 .. 7 ]
   void setNodeK ( const int node,
                   const int k );

   /// \brief Get the i value for the node.
   ///
   /// \param node A node of the element.
   /// \pre node in range [ 0 .. 7 ]
   int getNodeI ( const int node ) const;

   /// \brief Get the j value for the node.
   ///
   /// \param node A node of the element.
   /// \pre node in range [ 0 .. 7 ]
   int getNodeJ ( const int node ) const;

   /// \brief Get the k value for the node.
   ///
   /// \param node A node of the element.
   /// \pre node in range [ 0 .. 7 ]
   int getNodeK ( const int node ) const;

   /// \brief Get the positions of all nodes relative to the subdomain numbering system.
   void getNodePositions ( Nodal3DIndexArray& positions ) const;

   /// \brief Get the position of a single node relative to the subdomain numbering system.
   void getNode ( const int node, Mesh3DIndex& position ) const;

   //@}

   /// \name Boundary information.
   //@{

   /// \brief Indicate whether or not the element-boundary is on the domain-boundary.
   bool isOnDomainBoundary ( const VolumeData::BoundaryId id ) const;

   /// \brief Set whether or not GAMMA_1 is on the domain-boundary.
   void setShallowIsOnDomainBoundary ( const bool value );

   /// \brief Set whether or not GAMMA_6 is on the domain-boundary.
   void setDeepIsOnDomainBoundary ( const bool value );

   //@}

   /// \brief Return whether or not the subdomain-element is active.
   bool isActive () const;


   /// \brief Set the layer element.
   ///
   /// This value cannot be null.
   void setLayerElement ( const LayerElement& le );

   /// \brief Get the layer element.
   const LayerElement& getLayerElement () const;


   /// \name Neighbour information.
   //@{

   /// \brief Set all neighbours to null.
   void clearNeighbours ();

   /// \brief Return the neighbouring element.
   ///
   /// May return null if element does not exist.
   const BaseElement* getNeighbour ( const VolumeData::BoundaryId id ) const;

   /// \brief Return the neighbouring element.
   ///
   /// May return null if element does not exist.
   /// If lateral face has measure zero then a null will be returned.
   /// For vertical faces (i.e. shallow and deep) then some iteration 
   /// may be required to find an active element, if none is found then
   /// null will be returned.
   const BaseElement* getActiveNeighbour ( const VolumeData::BoundaryId id ) const;

   /// \brief Set the neighbour element on the boundary.
   void setNeighbour ( const VolumeData::BoundaryId id,
                       const BaseElement*      neighbour );

   //@}


private :

   /// \brief Return the active neighbouring element shallower than current element.
   ///
   /// May return null if element does not exist.
   const BaseElement* getShallowerActiveNeighbour () const;

   /// \brief Return the active neighbouring element deeper than current element.
   ///
   /// May return null if element does not exist.
   const BaseElement* getDeeperActiveNeighbour () const;



   /// \brief Array indicating if element boundary lies on sub-domain boundary.
   bool        m_onSubDomainBoundary [ NumberOfBoundaries ];

   /// \brief Array indicating if element boundary is active.
   bool        m_activeBoundary [ NumberOfBoundaries ];

   /// \brief Pointers to neighbouring elements across faces.
   const BaseElement* m_neighbours [ NumberOfBoundaries ];

   /// \brief Pointer to the corresponding layer-element.
   const LayerElement* m_layerElement;

   /// \brief The k-index for the subdomain to which this element belongs.
   int                 m_k;

   /// \brief The k-indicess of the nodes.
   int                 m_nodeKValues [ 8 ];

   /// \brief indicate whether or not on the top subdomain-boundary.
   bool m_onShallowBoundary;

   /// \brief indicate whether or not on the bottom subdomain-boundary.
   bool m_onDeepBoundary;


}; 

// /// \typedef BaseElementArray
// /// \brief A three-d array of subdomain-elements.
// typedef PETSc_Local_3D_Array<BaseElement> BaseElementArray;

//------------------------------------------------------------//
// Inline functions.
//------------------------------------------------------------//

template<class ParentElement>
inline int BaseElement<ParentElement>::getI () const {
   return m_layerElement->getIPosition ();
}

template<class ParentElement>
inline int BaseElement<ParentElement>::getJ () const {
   return m_layerElement->getJPosition ();
}

template<class ParentElement>
inline int BaseElement<ParentElement>::getK () const {
   return m_k;
}

template<class ParentElement>
inline bool BaseElement<ParentElement>::isActive () const {
   return getLayerElement ().isActive ();
}

template<class ParentElement>
inline int BaseElement<ParentElement>::getNodeI ( const int node ) const {
   return getLayerElement ().getNodeIPosition ( node );
}

template<class ParentElement>
inline int BaseElement<ParentElement>::getNodeJ ( const int node ) const {
   return getLayerElement ().getNodeJPosition ( node );
}

template<class ParentElement>
inline int BaseElement<ParentElement>::getNodeK ( const int node ) const {
   return m_nodeKValues [ node ];
}

template<class ParentElement>
inline const BaseElement<ParentElement>* BaseElement<ParentElement>::getNeighbour ( const VolumeData::BoundaryId id ) const {
   return m_neighbours [ id ];
}

template<class ParentElement>
inline const LayerElement& BaseElement<ParentElement>::getLayerElement () const {
   return *m_layerElement;
}

template<class ParentElement>
inline bool BaseElement<ParentElement>::isOnDomainBoundary ( const VolumeData::BoundaryId id ) const {

   if ( id == VolumeData::GAMMA_1 ) {
      return m_onShallowBoundary;
   } else if ( id == VolumeData::GAMMA_6 ) {
      return m_onDeepBoundary;
   } else {
      return getLayerElement ().isOnDomainBoundary ( id );
   }

}

//------------------------------------------------------------//

template<class ParentElement>
BaseElement<ParentElement>::BaseElement () {

   int i;

   m_layerElement = 0;
   m_k = 0;

   for ( i = 0; i < NumberOfBoundaries; ++i ) {
      m_onSubDomainBoundary [ i ] = false;
      m_activeBoundary [ i ] = false;
      m_neighbours [ i ] = 0;
   }

   m_onShallowBoundary = false;
   m_onDeepBoundary = false;

}

//------------------------------------------------------------//

template<class ParentElement>
void BaseElement<ParentElement>::setLayerElement ( const LayerElement& ve ) {
   m_layerElement = &ve;
}

//------------------------------------------------------------//

template<class ParentElement>
void BaseElement<ParentElement>::setK ( const int k ) {
   m_k = k;
}

//------------------------------------------------------------//

template<class ParentElement>
void BaseElement<ParentElement>::setShallowIsOnDomainBoundary ( const bool value ) {
   m_onShallowBoundary = value;
}

//------------------------------------------------------------//

template<class ParentElement>
void BaseElement<ParentElement>::setDeepIsOnDomainBoundary ( const bool value ) {
   m_onDeepBoundary = value;
}

//------------------------------------------------------------//

template<class ParentElement>
void BaseElement<ParentElement>::setNeighbour ( const VolumeData::BoundaryId id,
                                      const BaseElement*      neighbour ) {
   m_neighbours [ id ] = neighbour;
}

//------------------------------------------------------------//

template<class ParentElement>
void BaseElement<ParentElement>::clearNeighbours () {
   m_neighbours [ 0 ] = 0;
   m_neighbours [ 1 ] = 0;
   m_neighbours [ 2 ] = 0;
   m_neighbours [ 3 ] = 0;
   m_neighbours [ 4 ] = 0;
   m_neighbours [ 5 ] = 0;
}

//------------------------------------------------------------//

template<class ParentElement>
const BaseElement<ParentElement>* BaseElement<ParentElement>::getActiveNeighbour ( const VolumeData::BoundaryId id ) const {

   const BaseElement* neighbour;

   if ( id == VolumeData::GAMMA_1 ) {
      neighbour = getShallowerActiveNeighbour ();
   } else if ( id == VolumeData::GAMMA_6 ) {
      neighbour = getDeeperActiveNeighbour ();
   } else {
      // Otherwise get the neighbour and do not recursivly seek for an element.

      if ( m_layerElement->isActiveBoundary ( id )) {
         neighbour = getNeighbour ( id );
      } else {
         neighbour = 0;
      }

   }

   return neighbour;
}

//------------------------------------------------------------//

template<class ParentElement>
const BaseElement<ParentElement>* BaseElement<ParentElement>::getShallowerActiveNeighbour () const {

   const BaseElement* neighbour = getNeighbour ( VolumeData::ShallowFace );

   while ( neighbour != 0 and not neighbour->m_layerElement->isActive ()) {
      neighbour = neighbour->getNeighbour ( VolumeData::ShallowFace );
   } 

   // neighbour is either null-pointer or active-element.
   return neighbour;
}

//------------------------------------------------------------//

template<class ParentElement>
const BaseElement<ParentElement>* BaseElement<ParentElement>::getDeeperActiveNeighbour () const {

   const BaseElement* neighbour = getNeighbour ( VolumeData::DeepFace );

   while ( neighbour != 0 and not neighbour->m_layerElement->isActive ()) {
      neighbour = neighbour->getNeighbour ( VolumeData::DeepFace );
   } 

   // neighbour is either null-pointer or active-element.
   return neighbour;
}

//------------------------------------------------------------//

template<class ParentElement>
void BaseElement<ParentElement>::setNodeK ( const int node,
                             const int k ) {
   m_nodeKValues [ node ] = k;
}

//------------------------------------------------------------//


#endif // FASTCAULDRON__BASE_ELEMENT__H
