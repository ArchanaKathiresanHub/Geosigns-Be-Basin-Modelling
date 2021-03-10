#ifndef _FASTCAULDRON__ELEMENT_FACE_VALUES__H_
#define _FASTCAULDRON__ELEMENT_FACE_VALUES__H_

#include <string>

#include "BoundaryId.h"
#include "PetscBlockVector.h"
#include "PVTCalculator.h"

class ElementFaceValues {

public :

   static const int NumberOfFaces = VolumeData::NumberOfBoundaries;

   static const int BlockSize = NumberOfFaces;
   // static const int BlockSize = NumberOfPVTPhases * NumberOfFaces;

   typedef VolumeData::BoundaryId IndexType;


   /// Return the value for the face.
   double  operator ()( const IndexType face ) const;

   /// Return the value for the face.
   double&  operator ()( const IndexType face );


   /// \brief Multiply the face values by a scalar.
   ElementFaceValues& operator*=( const double scalar );

   /// \brief Sum of all values greater than 0.
   double sumGt0() const;

   /// \brief Set all entries to zero.
   void zero ();

private :

   double m_values [ NumberOfFaces ];

};


typedef PetscBlockVector<ElementFaceValues> ElementFaceValueVector;

//------------------------------------------------------------//
//   Inline functions.
//------------------------------------------------------------//

inline double ElementFaceValues::operator ()( const IndexType face ) const {
   return m_values [ face ];
}

inline double& ElementFaceValues::operator ()( const IndexType face ) {
   return m_values [ face ];
}

#endif // _FASTCAULDRON__ELEMENT_FACE_VALUES__H_
