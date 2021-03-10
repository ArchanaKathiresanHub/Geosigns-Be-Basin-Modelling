#ifndef _FASTCAULDRON__MAP_ELEMENT__H_
#define _FASTCAULDRON__MAP_ELEMENT__H_

#include <string>


/// Map element.
///
/// Structure containing information about activity and connectivity of elements in the domain map.
/// If element is not valid (active) then all other values should be ignored except the element's position.
///
///
///             Gamma_3
///           +----------+
///           |          |
///           |          |
///  Gamma_4  |          | Gamma_2
///           |          |
///           |          |
///           +----------+
///             Gamma_1
///
///
///       Node_3       Node_2
///           +----------+
///           |          |
///           |          |
///           |          |
///           |          |
///           |          |
///           +----------+
///        Node_0      node_1
///
class MapElement {

   /// A simple pair of integers.
   struct IntegerPair {
      int i;
      int j;
   };

public :

   /// The names given to each of the exges of the map-element, in standard order.
   enum BoundaryId { EDGE_1, EDGE_2, EDGE_3, EDGE_4 };

   /// Aliases of boundary-id's
   /// The face closest to the front.
   static const BoundaryId GAMMA_1 = EDGE_1;

   /// The face closest to the right.
   static const BoundaryId GAMMA_2 = EDGE_2;

   /// The face closest to the back.
   static const BoundaryId GAMMA_3 = EDGE_3;

   /// The face closest to the left.
   static const BoundaryId GAMMA_4 = EDGE_4;


   /// Aliases of boundary-id's
   /// The face closest to the front.
   static const BoundaryId Front = GAMMA_1;

   /// The face closest to the right.
   static const BoundaryId Right = GAMMA_2;

   /// The face closest to the back.
   static const BoundaryId Back  = GAMMA_3;

   /// The face closest to the left.
   static const BoundaryId Left  = GAMMA_4;


   /// Return the boundary opposite the one indicated.
   static BoundaryId opposite ( const BoundaryId id );

   /// Return the string representation of the boundary-id.
   static const std::string& boundaryIdImage ( const BoundaryId id );

   /// Return the string representation of the boundary-id alias.
   static const std::string& boundaryIdAliasImage ( const BoundaryId id );


   /// The number of nodes in a map element.
   static const int NumberOfNodes = 4;

   /// The number of boundaries in a map element.
   static const int NumberOfBoundaries = 4;


   /// Constructor.
   MapElement ();


   /// Return whether or not the element is part of a defined region.
   ///
   /// If element is not valid (active) then all other values should be ignored.
   bool isValid () const;

   /// Set whether or not the element is part of a defined region.
   void setIsValid ( const bool value );


   /// Get the i-index of the map.
   int getIPosition () const;

   /// Get the j-index of the map.
   int getJPosition () const;

   /// Set the map indicies.
   void setPosition ( const int i,
                      const int j );


   /// Get the i-index of the node.
   ///
   /// node in range [ 0 .. 3 ]
   int getNodeIPosition ( const int node ) const;

   /// Get the j-index of the node.
   ///
   /// node in range [ 0 .. 3 ]
   int getNodeJPosition ( const int node ) const;

   /// Set the node indices.
   ///
   /// node in range [ 0 .. 3 ]
   void setNodePosition ( const int node,
                          const int i,
                          const int j );


   /// Get whether or not the node is part of the valid domain.
   ///
   /// node in range [ 0 .. 3 ]
   bool getNodeExists ( const int node ) const;

   /// Set whether or not the node is part of the valid domain.
   ///
   /// node in range [ 0 .. 3 ]
   void setNodeExists ( const int  node,
                        const bool exists );


   /// Return whether or not the boundary of the element is on the defined domain boundary.
   ///
   /// Which element-boundary is indicated by boundary-id.
   /// boundaryId in range [ 0 .. 3 ]
   // bool isOnDomainBoundary ( const int boundaryId ) const;
   bool isOnDomainBoundary ( const BoundaryId boundaryId ) const;

   /// Set whether or not the boundary of the element is on the defined domain boundary.
   ///
   /// Which element-boundary is indicated by boundary-id.
   /// boundaryId in range [ 0 .. 3 ]
   void setIsOnDomainBoundary ( const BoundaryId boundaryId,
                                const bool value );

   /// Return whether or not the boundary of the element is on the processor boundary.
   ///
   /// The processor boundary is a sub-set of the domain-boundary, i.e. if an element
   /// is not valid then even though it may lie on a processsor boundary this will return false.
   /// Which element-boundary is indicated by boundary-id.
   /// boundaryId in range [ 0 .. 3 ]
   bool isOnProcessorBoundary ( const BoundaryId boundaryId ) const;

   /// Set whether or not the boundary of the element is on the processor boundary.
   ///
   /// Which element-boundary is indicated by boundary-id.
   /// boundaryId in range [ 0 .. 3 ]
   void setIsOnProcessorBoundary ( const BoundaryId boundaryId,
                                   const bool value );

   /// Get whether or not the element lies on the current processor.
   ///
   /// If element does not lie on this processor then node and boundary data should be ignored.
   bool isOnProcessor () const;

   /// Set whether or not the element lies on the current processor.
   void setIsOnProcessor ( const bool value );


   /// Return a string representation of the map-element.
   std::string image () const;

private :

   /// Array of node positions.
   IntegerPair m_nodePositions [ NumberOfNodes ];

   /// Element position
   IntegerPair m_mapPosition;

   /// Array indicating if element boundary lies on domain boundary.
   bool        m_onDomainBoundary [ NumberOfBoundaries ];

   /// Array indicating if element boundary lies on processor-partition boundary.
   bool        m_onProcessorBoundary [ NumberOfBoundaries ];

   /// Is the element valid/active or not.
   bool        m_isValid;

   /// Does the element lie in this processor partition or is it a ghost element.
   bool        m_onProcessor;

}; 


//------------------------------------------------------------//
// Inline functions
//------------------------------------------------------------//


inline bool MapElement::isValid () const {
   return m_isValid;
}

inline int MapElement::getIPosition () const {
   return m_mapPosition.i;
}

inline int MapElement::getJPosition () const {
   return m_mapPosition.j;
}

inline int MapElement::getNodeIPosition ( const int node ) const {
   return m_nodePositions [ node ].i;
}

inline int MapElement::getNodeJPosition ( const int node ) const {
   return m_nodePositions [ node ].j;
}

inline bool MapElement::isOnDomainBoundary ( const BoundaryId boundaryId ) const {
   return m_onDomainBoundary [ boundaryId ];
}

inline bool MapElement::isOnProcessorBoundary ( const BoundaryId boundaryId ) const {
   return m_onProcessorBoundary [ boundaryId ];
}

inline bool MapElement::isOnProcessor () const {
   return m_onProcessor;
}


#endif // _FASTCAULDRON__MAP_ELEMENT__H_
