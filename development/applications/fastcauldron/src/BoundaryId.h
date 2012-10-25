#ifndef _FASTCAULDRON__BOUNDARY_ID__H_
#define _FASTCAULDRON__BOUNDARY_ID__H_

#include <string>

namespace VolumeData {

   /// The number of boundaries in an element.
   ///
   /// Since all elements are hexahedral there are 6 faces for all elements.
   static const int NumberOfBoundaries = 6;

   /// The names given to each of the faces of the element, in standard order.
   enum BoundaryId { GAMMA_1, GAMMA_2, GAMMA_3, GAMMA_4, GAMMA_5, GAMMA_6 };

   // Aliases of boundary-id's

   /// The face closest to the surface.
   static const BoundaryId ShallowFace = GAMMA_1;

   /// The face farthest from the surface.
   static const BoundaryId DeepFace = GAMMA_6;

   /// The face at the front of the element.
   static const BoundaryId Front = GAMMA_2;

   /// The face to the right of the element.
   static const BoundaryId Right = GAMMA_3;

   /// The face at the back of the element.
   static const BoundaryId Back  = GAMMA_4;

   /// The face to the left of the element.
   static const BoundaryId Left  = GAMMA_5;


   /// Return the boundary opposite the one indicated.
   BoundaryId opposite ( const BoundaryId id );

   /// Return the string representation of the boundary-id.
   const std::string& boundaryIdImage ( const BoundaryId id );

   /// Return the string representation of the boundary-id alias.
   const std::string& boundaryIdAliasImage ( const BoundaryId id );


}

#endif // _FASTCAULDRON__BOUNDARY_ID__H_
