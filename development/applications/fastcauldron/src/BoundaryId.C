#include "BoundaryId.h"

VolumeData::BoundaryId VolumeData::opposite ( const BoundaryId id ) {

   static const BoundaryId oppositeBoundaries [ NumberOfBoundaries ] = { GAMMA_6, GAMMA_4, GAMMA_5, GAMMA_2, GAMMA_3, GAMMA_1 };

   return oppositeBoundaries [ id ];
}

const std::string& VolumeData::boundaryIdImage ( const BoundaryId id ) {

   static const std::string names [ NumberOfBoundaries ] = { "GAMMA_1", "GAMMA_2", "GAMMA_3", "GAMMA_4", "GAMMA_5", "GAMMA_6" };

   return names [ id ];
}

const std::string& VolumeData::boundaryIdAliasImage ( const BoundaryId id ) {

   static const std::string aliases [ NumberOfBoundaries ] = { "ShallowFace", "FrontFace", "RightFace", "BackFace", "LeftFace", "DeepFace" };

   return aliases [ id ];
}
