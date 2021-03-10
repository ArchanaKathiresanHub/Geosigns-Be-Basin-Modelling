#include <sstream>
#include "MapElement.h"


inline MapElement::BoundaryId MapElement::opposite ( const BoundaryId id ) {

   static const BoundaryId oppositeBoundaries [ NumberOfBoundaries ] = { GAMMA_3, GAMMA_4, GAMMA_1, GAMMA_2 };

   return oppositeBoundaries [ id ];
}

const std::string& MapElement::boundaryIdImage ( const BoundaryId id ) {

   static const std::string names [ NumberOfBoundaries ] = { "EDGE_1", "EDGE_2", "EDGE_3", "EDGE_4" };

#if 0
   static const std::string names [ NumberOfBoundaries ] = { "GAMMA_1", "GAMMA_2", "GAMMA_3", "GAMMA_4" };
#endif

   return names [ id ];
}

const std::string& MapElement::boundaryIdAliasImage ( const BoundaryId id ) {

   static const std::string aliases [ NumberOfBoundaries ] = { "FrontEdge", "RightEdge", "BackEdge", "LeftEdge" };

   return aliases [ id ];
}



MapElement::MapElement () {

   int i;

   m_isValid = false;
   m_onProcessor = false;

   m_mapPosition.i = -1;
   m_mapPosition.j = -1;

   for ( i = 0; i < NumberOfNodes; ++i ) {
      m_nodePositions [ i ].i = -1;
      m_nodePositions [ i ].j = -1;
      // m_nodeExists [ i ] = false;
   }

   for ( i = 0; i < NumberOfBoundaries; ++i ) {
      m_onDomainBoundary [ i ] = false;
      m_onProcessorBoundary [ i ] = false;
   }

}

void MapElement::setIsValid ( const bool value ) {
   m_isValid = value;
}

void MapElement::setPosition ( const int i,
                               const int j ) {
   m_mapPosition.i = i;
   m_mapPosition.j = j;
}

void MapElement::setNodePosition ( const int node,
                                   const int i,
                                   const int j ) {
   m_nodePositions [ node ].i = i;
   m_nodePositions [ node ].j = j;
}

void MapElement::setIsOnDomainBoundary ( const BoundaryId boundaryId,
                                         const bool value ) {
   m_onDomainBoundary [ boundaryId ] = value;
}

void MapElement::setIsOnProcessorBoundary ( const BoundaryId boundaryId,
                                            const bool value ) {
   m_onProcessorBoundary [ boundaryId ] = value;
}

void MapElement::setIsOnProcessor ( const bool value ) {
   m_onProcessor = value;
}

std::string MapElement::image () const {

   std::stringstream buffer;

   if ( isValid ()) {
      buffer << "map-element is valid" << std::endl;

      buffer << " on partition         : " << isOnProcessor () << std::endl;
      buffer << "at map position       : (" << getIPosition () << ", " << getJPosition () << ")" << std::endl;

      if ( isOnProcessor ())  {
         buffer << "node positions        : {(" 
                << getNodeIPosition ( 0 ) << ", " << getNodeJPosition ( 0 ) << "), (" 
                << getNodeIPosition ( 1 ) << ", " << getNodeJPosition ( 1 ) << "), (" 
                << getNodeIPosition ( 2 ) << ", " << getNodeJPosition ( 2 ) << "), (" 
                << getNodeIPosition ( 3 ) << ", " << getNodeJPosition ( 3 ) << ")}"  << std::endl;

         buffer << "on domain boundary    : " 
                << isOnDomainBoundary ( EDGE_1 ) << ", " 
                << isOnDomainBoundary ( EDGE_2 ) << ", " 
                << isOnDomainBoundary ( EDGE_3 ) << ", " 
                << isOnDomainBoundary ( EDGE_4 ) << std::endl;

         buffer << "on partition boundary : " 
                << isOnProcessorBoundary ( EDGE_1 ) << ", " 
                << isOnProcessorBoundary ( EDGE_2 ) << ", " 
                << isOnProcessorBoundary ( EDGE_3 ) << ", " 
                << isOnProcessorBoundary ( EDGE_4 ) << std::endl;
      }

   } else {
      buffer << "map-element is not valid" << std::endl;
   }

   return buffer.str ();
}
