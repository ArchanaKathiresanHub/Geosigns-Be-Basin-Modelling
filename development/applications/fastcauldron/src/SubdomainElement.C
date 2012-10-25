#include "mpi.h"
#include "SubdomainElement.h"

//------------------------------------------------------------//

using namespace VolumeData;

//------------------------------------------------------------//

SubdomainElement::SubdomainElement () {

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

void SubdomainElement::setLayerElement ( const LayerElement& ve ) {
   m_layerElement = &ve;
}

//------------------------------------------------------------//

void SubdomainElement::setK ( const int k ) {
   m_k = k;
}

//------------------------------------------------------------//

void SubdomainElement::setShallowIsOnDomainBoundary ( const bool value ) {
   m_onShallowBoundary = value;
}

//------------------------------------------------------------//

void SubdomainElement::setDeepIsOnDomainBoundary ( const bool value ) {
   m_onDeepBoundary = value;
}

//------------------------------------------------------------//

void SubdomainElement::setNeighbour ( const VolumeData::BoundaryId id,
                                      const SubdomainElement*      neighbour ) {
   m_neighbours [ id ] = neighbour;
}

//------------------------------------------------------------//

void SubdomainElement::clearNeighbours () {
   m_neighbours [ 0 ] = 0;
   m_neighbours [ 1 ] = 0;
   m_neighbours [ 2 ] = 0;
   m_neighbours [ 3 ] = 0;
   m_neighbours [ 4 ] = 0;
   m_neighbours [ 5 ] = 0;
}

//------------------------------------------------------------//

const SubdomainElement* SubdomainElement::getActiveNeighbour ( const VolumeData::BoundaryId id ) const {

   const SubdomainElement* neighbour;

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

const SubdomainElement* SubdomainElement::getShallowerActiveNeighbour () const {

   const SubdomainElement* neighbour = getNeighbour ( VolumeData::ShallowFace );

   while ( neighbour != 0 and not neighbour->m_layerElement->isActive ()) {
      neighbour = neighbour->getNeighbour ( VolumeData::ShallowFace );
   } 

   // neighbour is either null-pointer or active-element.
   return neighbour;
}

//------------------------------------------------------------//

const SubdomainElement* SubdomainElement::getDeeperActiveNeighbour () const {

   const SubdomainElement* neighbour = getNeighbour ( VolumeData::DeepFace );

   while ( neighbour != 0 and not neighbour->m_layerElement->isActive ()) {
      neighbour = neighbour->getNeighbour ( VolumeData::DeepFace );
   } 

   // neighbour is either null-pointer or active-element.
   return neighbour;
}

//------------------------------------------------------------//

void SubdomainElement::setNodeK ( const int node,
                                  const int k ) {
   m_nodeKValues [ node ] = k;
}

//------------------------------------------------------------//
