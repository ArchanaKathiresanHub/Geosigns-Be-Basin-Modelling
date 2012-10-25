#include "mpi.h"
#include "LayerElement.h"

#include <sstream>

#include "layer.h"
#include "Lithology.h"

using namespace VolumeData;

//------------------------------------------------------------//

double LayerElement::faceNormalDirectionScaling ( const BoundaryId id ) {

   static const double directionScaling [ NumberOfBoundaries ] = { -1.0, -1.0, 1.0, 1.0, -1.0, 1.0 };

   return directionScaling [ id ];
}

//------------------------------------------------------------//

LayerElement::LayerElement () {

   int i;

   for ( i = 0; i < NumberOfBoundaries; ++i ) {
      m_onDomainBoundary [ i ] = false;
      m_onProcessorBoundary [ i ] = false;
      m_neighbours [ i ] = 0;
      m_activeBoundary [ i ] = false;
   }

   m_isActive = false;
   m_onProcessor = false;

   m_formation = 0;
   m_lithology = 0;

}

//------------------------------------------------------------//

LayerElement::LayerElement ( const LayerElement& element ) {
   *this = element;
}

//------------------------------------------------------------//

LayerElement& LayerElement::operator= ( const LayerElement& element ) {

   int i;

   for ( i = 0; i < NumberOfBoundaries; ++i ) {
      m_onDomainBoundary [ i ] = element.m_onDomainBoundary [ i ];
      m_onProcessorBoundary [ i ] = element.m_onProcessorBoundary [ i ];
      m_neighbours [ i ] = element.m_neighbours [ i ];
      m_activeBoundary [ i ] = element.m_activeBoundary [ i ];
   }

   m_isActive = element.m_isActive;
   m_onProcessor = element.m_onProcessor;

   m_formation = element.m_formation;
   m_lithology = element.m_lithology;

   for ( i = m_nodePositions.first (); i <= m_nodePositions.last (); ++i ) {
      m_nodePositions ( i ) = element.m_nodePositions ( i );
   }

   m_arrayPosition = element.m_arrayPosition;

   return *this;
}

//------------------------------------------------------------//

void LayerElement::setIsActive ( const bool value ) {
   m_isActive = value;
}

//------------------------------------------------------------//

void LayerElement::setPosition ( const int i,
                                  const int j,
                                  const int localK ) {

   m_arrayPosition.set ( i, j, localK );
}

//------------------------------------------------------------//

void LayerElement::setPosition ( const int globalK ) {
   m_arrayPosition.set ( globalK );
}

//------------------------------------------------------------//

void LayerElement::setNodePosition ( const int node,
                                      const int i,
                                      const int j,
                                      const int localK ) {


   m_nodePositions ( node ).set ( i, j, localK );
}

//------------------------------------------------------------//

void LayerElement::setNodePosition ( const int node,
                                      const int globalK ) {

   m_nodePositions ( node ).set ( globalK );
}

//------------------------------------------------------------//

void LayerElement::setIsOnDomainBoundary ( const BoundaryId id,
                                           const bool       value ) {
   m_onDomainBoundary [ id ] = value;
}

//------------------------------------------------------------//

void LayerElement::setIsOnProcessorBoundary ( const BoundaryId id,
                                              const bool       value ) {
   m_onProcessorBoundary [ id ] = value;
}

//------------------------------------------------------------//

void LayerElement::setIsActiveBoundary ( const BoundaryId id,
                                         const bool       value ) {
   m_activeBoundary [ id ] = value;
}

//------------------------------------------------------------//

void LayerElement::setIsOnProcessor ( const bool value ) {
   m_onProcessor = value;
}

//------------------------------------------------------------//

void LayerElement::setNeighbour ( const BoundaryId     id,
                                  const LayerElement* neighbour ) {
   m_neighbours [ id ] = neighbour;
}

//------------------------------------------------------------//

void LayerElement::setLithology ( const Lithology* lithology ) {
   m_lithology = lithology;
}

//------------------------------------------------------------//

void LayerElement::setFormation ( const LayerProps* formation ) {
   m_formation = formation;
}

//------------------------------------------------------------//

std::string LayerElement::positionImage () const {
   return m_arrayPosition.image ();
}

//------------------------------------------------------------//

std::string LayerElement::image () const {

   std::stringstream buffer;
   int i;

   buffer << " LayerElement  : " << ( m_formation == 0 ? " NO FORMATION " : m_formation->layername ) << "  " 
          << ( m_lithology == 0 ? " NO LITHOLOGY " : m_lithology->getName ()) 
          << endl;

   buffer << " Is active      : " << ( m_isActive ? " TRUE " : " FALSE " ) << endl;

   buffer << " Element position : " << m_arrayPosition.image () << endl;

   if ( m_isActive ) {
      buffer << " On processor   : " << ( m_onProcessor ? " TRUE " : " FALSE " ) << endl;
      buffer << " Node numbers   : {";

      for ( i = m_nodePositions.first (); i <= m_nodePositions.last (); ++i ) {
         buffer << m_nodePositions ( i ).image ();
         // buffer << "{ " << m_nodePositions [ i ].i << ", "  << m_nodePositions [ i ].j << ", "  << m_nodePositions [ i ].localK << ", " << m_nodePositions [ i ].globalK << " } ";

         if ( i < m_nodePositions.last ()) {
            buffer << ", ";
         }

      }

      buffer << "}" << endl;

      buffer << " On domain boundary: {";

      for ( i = 0; i < NumberOfBoundaries; ++i ) {
         buffer << ( m_onDomainBoundary [ i ] ? " TRUE" : "FALSE" ) << "  ";
      }

      buffer << "}" << endl;

      buffer << " On processor boundary: {";

      for ( i = 0; i < NumberOfBoundaries; ++i ) {
         buffer << ( m_onProcessorBoundary [ i ] ? " TRUE" : "FALSE" ) << "  ";
      }

      buffer << "}" << endl;

      buffer << " Active boundary: {";

      for ( i = 0; i < NumberOfBoundaries; ++i ) {
         buffer << ( m_activeBoundary [ i ] ? " TRUE" : "FALSE" ) << "  ";
      }

      buffer << "}" << endl;
   }


   return buffer.str ();
}

//------------------------------------------------------------//

const LayerElement* LayerElement::getActiveNeighbour ( const BoundaryId id ) const {

   const LayerElement* neighbour;

   if ( id == GAMMA_1 ) {
      neighbour = getShallowerActiveNeighbour ();
   } else if ( id == GAMMA_6 ) {
      neighbour = getDeeperActiveNeighbour ();
   } else {

      if ( isActiveBoundary ( id )) {
         neighbour = getNeighbour ( id );
      } else {
         neighbour = 0;
      }

   }

   return neighbour;
}

//------------------------------------------------------------//

const LayerElement* LayerElement::getShallowerActiveNeighbour () const {

   const LayerElement* neighbour = getNeighbour ( ShallowFace );

   while ( neighbour != 0 and not neighbour->isActive ()) {
      neighbour = neighbour->getNeighbour ( ShallowFace );
   } 

   // neighbour is either null-pointer or active-element.
   return neighbour;
}

//------------------------------------------------------------//

const LayerElement* LayerElement::getDeeperActiveNeighbour () const {

   const LayerElement* neighbour = getNeighbour ( DeepFace );

   while ( neighbour != 0 and not neighbour->isActive ()) {
      neighbour = neighbour->getNeighbour ( DeepFace );
   } 

   // neighbour is either null-pointer or active-element.
   return neighbour;
}

//------------------------------------------------------------//

const GeoPhysics::FluidType* LayerElement::getFluid () const {
   return m_formation->fluid;
}

//------------------------------------------------------------//
