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

   /// \brief Set all entries to zero.
   void zero ();

   /// Return the string representation of the element-face-values.
   std::string image () const;

   /// \brief Sum all values.
   double sum () const;

   /// \brief Sum abs of all values.
   double sumAbs () const;

   /// \brief Sum of all values greater than 0.
   double sumGt0 () const;

   /// \brief Sum of all values less than 0.
   double sumLt0 () const;

private :

   double m_values [ NumberOfFaces ];

};

ElementFaceValues operator+( const ElementFaceValues& left, const ElementFaceValues& right );

ElementFaceValues operator-( const ElementFaceValues& left, const ElementFaceValues& right );


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
