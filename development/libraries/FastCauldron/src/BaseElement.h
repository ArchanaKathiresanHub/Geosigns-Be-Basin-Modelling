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

#include "BoundaryId.h"
#include "LayerElement.h"


/// \brief An element that is a part of a subdomain.
///
/// These exist mainly because the dof-, node- and element numbering
/// can be different for each subdomain, so we need to keep account of this.
/// This is intended to be instantiated with 
template<class ParentElement>
class BaseElement : public ParentElement {

public :

   /// The number of faces the element has.
   ///
   /// Since all elements are hexahedron there are 6 faces for all elements.
   static const int NumberOfBoundaries = VolumeData::NumberOfBoundaries;


   BaseElement ();

   virtual ~BaseElement () {}

   /// \name Element position.
   //@{

   /// Get the I position of the element within the subdomain.
   int getI () const;

   /// Get the J position of the element within the subdomain.
   int getJ () const;

   /// \brief Set K position within subdomain.
   void setK ( const int k );

   /// \brief Get K position of the element within subdomain.
   int getK () const;

   //@}

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

   //@}

   /// \name Boundary information.
   //@{

   /// \brief Indicate whether or not the element-boundary is on the domain-boundary.
   bool isOnDomainBoundary ( const VolumeData::BoundaryId id ) const;

   /// \brief Set whether or not the top-most face of the element is on the domain-boundary.
   void setShallowIsOnDomainBoundary ( const bool value );

   /// \brief Set whether or not the bottom-most face of the element is on the domain-boundary.
   void setDeepIsOnDomainBoundary ( const bool value );

   //@}

   /// \brief Return whether or not the subdomain-element is active.
   bool isActive () const;


   /// \brief Assign the associated LayerElement.
   ///
   /// Each GeneralElement and SubdomainElement has an associated LayerElement which 
   /// is used to access the property arrays that are contained wihtin each layer.
   void setLayerElement ( const LayerElement& le );

   /// \brief Get the layer element that is associated with this base-element.
   const LayerElement& getLayerElement () const;


   /// \name Neighbour information.
   //@{

   /// \brief Remove connections with neighbour elements.
   void clearNeighbours ();

   /// \brief Return the neighbouring element.
   ///
   /// May return null if element does not exist.
   const BaseElement* getNeighbour ( const VolumeData::BoundaryId id ) const;

   /// \brief Return the neighbouring element.
   ///
   /// May return null if element does not exist.
   /// If both vertical segments that border the face are zero-thickness then a 
   /// null element will be returned. For vertical faces (i.e. shallow and deep) 
   /// then some iteration may be required to find an active element, if none is 
   /// found then null will be returned.
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

   /// \brief The k-indices for each of the 8 corner nodes.
   int                 m_nodeKValues [ 8 ];

   /// \brief indicate whether or not on the top subdomain-boundary.
   bool m_onShallowBoundary;

   /// \brief indicate whether or not on the bottom subdomain-boundary.
   bool m_onDeepBoundary;


}; 

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
void BaseElement<ParentElement>::setNodeK ( const int node,
                             const int k ) {
   m_nodeKValues [ node ] = k;
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

   if ( id == VolumeData::ShallowFace ) {
      return m_onShallowBoundary;
   } else if ( id == VolumeData::DeepFace ) {
      return m_onDeepBoundary;
   } else {
      return getLayerElement ().isOnDomainBoundary ( id );
   }

}

//------------------------------------------------------------//

template<class ParentElement>
BaseElement<ParentElement>::BaseElement () {

   m_layerElement = 0;
   m_k = 0;

   for ( int i = 0; i < NumberOfBoundaries; ++i ) {
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

   for ( int i = 0; i < 6; ++i ) {
      m_neighbours [ i ] = 0;
   }

}

//------------------------------------------------------------//

template<class ParentElement>
const BaseElement<ParentElement>* BaseElement<ParentElement>::getActiveNeighbour ( const VolumeData::BoundaryId id ) const {

   const BaseElement* neighbour;

   if ( id == VolumeData::ShallowFace ) {
      neighbour = getShallowerActiveNeighbour ();
   } else if ( id == VolumeData::DeepFace ) {
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


#endif // FASTCAULDRON__BASE_ELEMENT__H
