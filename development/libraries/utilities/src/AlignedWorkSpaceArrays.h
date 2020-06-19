//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef UTILTIES__ALIGNED_WORK_SPACE_ARRAYS__H
#define UTILTIES__ALIGNED_WORK_SPACE_ARRAYS__H

#include "AlignedMemoryAllocator.h"
#include "ArrayDefinitions.h"
#include "FormattingException.h"


/// \brief Contains a set of arrays that can be used as work-space vectors in some calculations.
///
/// For example, the mixing of the permeability will require the storage of the permeability for
/// each of the individual lithologies prior to mixing.
/// All values will be contiguous.
template<const unsigned int GenericMaximumSize>
class AlignedWorkSpaceArrays {

public :

   /// \brief The maximum number of workspace vectors.
   enum { MaximumSize = GenericMaximumSize };

   explicit AlignedWorkSpaceArrays ( const unsigned int valueCount );
   AlignedWorkSpaceArrays ( const AlignedWorkSpaceArrays& alignedWorkSpaceArrays) = delete;
   AlignedWorkSpaceArrays ( AlignedWorkSpaceArrays&& alignedWorkSpaceArrays) = delete;

   AlignedWorkSpaceArrays& operator =(const AlignedWorkSpaceArrays& alignedWorkSpaceArrays) = delete;
   AlignedWorkSpaceArrays& operator =(AlignedWorkSpaceArrays&& alignedWorkSpaceArrays) = delete;

   ~AlignedWorkSpaceArrays ();

   /// \brief Resize the workspace object.
   ///
   /// If the valueCount matches the previous value then no reallocation will occur.
   /// \param [in] valueCount  The number of values.
   /// \pre vectorCount must be the same as the number of vectors used in the
   void resize ( const unsigned int valueCount );

   /// \brief Get the number of data values assigned to this object.
   unsigned int getNumberOfValues () const;

   /// \brief Get the size of the allocated array.
   ///
   /// This may differ from the number of data values due to
   /// memory alignment requirements.
   /// This value is calculated:
   ///
   ///      if getNumberOfValues () mod ARRAY_ALIGMNENT == 0
   ///         n = getNumberOfValues ()
   ///      else
   ///         n = getNumberOfValues () + ARRAY_ALIGMNENT - getNumberOfValues () mod ARRAY_ALIGMNENT
   ///      end if
   ///
   unsigned int getLeadingDimension () const;

   /// \brief Get the data value array for the work-space.
   ///
   /// \param [in] vec Which work-space vector to return.
   /// \pre 0 <= vec < MaximumSize
   ArrayDefs::ConstReal_ptr getData ( const unsigned int vec ) const;

   /// \brief Get the data value array for the work-space
   ///
   /// \param [in] vec Which work-space vector to return.
   /// \pre 0 <= vec < MaximumSize
   ArrayDefs::Real_ptr getData ( const unsigned int vec );

private :

   /// \brief Allocate the data arrays.
   void allocate ( const unsigned int valueCount );

   /// \brief Deallocates the data arrays and sets them to null.
   void deallocate ();

   /// \brief The number of values for each array.
   unsigned int m_valueCount;

   /// \brief The number of values that have been allocated for each array.
   ///
   /// This may differ so that the start of each array aligns on the correct memory address.
   unsigned int m_alignedArraySize;

   /// \brief Array containing all the data values
   ArrayDefs::Real_ptr m_allValues = nullptr;

   /// \brief Array of arrays containing the property values for each work-space vector.
   ArrayDefs::Real_ptr m_workSpaceValues [ MaximumSize ];

};

//------------------------------------------------------------//

template<const unsigned int GenericMaximumSize>
AlignedWorkSpaceArrays<GenericMaximumSize>::AlignedWorkSpaceArrays ( const unsigned int valueCount ) :
   m_valueCount ( valueCount )
{
   allocate ( m_valueCount );
}

template<const unsigned int GenericMaximumSize>
AlignedWorkSpaceArrays<GenericMaximumSize>::~AlignedWorkSpaceArrays () {
   deallocate ();
}

template<const unsigned int GenericMaximumSize>
void AlignedWorkSpaceArrays<GenericMaximumSize>::resize ( const unsigned int valueCount ) {

   if ( valueCount == m_valueCount ) {
      // Nothing to do
      return;
   }

   if ( valueCount != m_valueCount ) {
      // Only need to reallocate if the nuber of data points changes.
      allocate ( valueCount );
   }

   m_valueCount = valueCount;
}


template<const unsigned int GenericMaximumSize>
void AlignedWorkSpaceArrays<GenericMaximumSize>::allocate ( const unsigned int valueCount ) {

   unsigned int arraySizeRequiredBytes = sizeof ( double ) * valueCount;
   unsigned int remainder = arraySizeRequiredBytes % ARRAY_ALIGNMENT;
   unsigned int totalSizeRequired;

   if ( remainder != 0 ) {
      arraySizeRequiredBytes += ARRAY_ALIGNMENT - remainder;
   }

   m_alignedArraySize = arraySizeRequiredBytes / sizeof ( double );

   // Always allocate the enough space for the maximum number of vectors.
   totalSizeRequired = MaximumSize * arraySizeRequiredBytes;

   if ( m_allValues != nullptr ) {
      deallocate ();
   }

   m_allValues = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( totalSizeRequired );
   ArrayDefs::Real_ptr buffer = m_allValues;

   for ( unsigned int i = 0; i < MaximumSize; ++i, buffer += m_alignedArraySize ) {
      m_workSpaceValues [ i ] = buffer;
   }

}

template<const unsigned int GenericMaximumSize>
void AlignedWorkSpaceArrays<GenericMaximumSize>::deallocate () {
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( m_allValues );
   m_allValues = nullptr;

   for ( unsigned int i = 0; i < MaximumSize; ++i ) {
      m_workSpaceValues [ i ] = nullptr;
   }

}


template<const unsigned int GenericMaximumSize>
inline unsigned int AlignedWorkSpaceArrays<GenericMaximumSize>::getNumberOfValues () const {
   return m_valueCount;
}

template<const unsigned int GenericMaximumSize>
inline unsigned int AlignedWorkSpaceArrays<GenericMaximumSize>::getLeadingDimension () const {
   return m_alignedArraySize;
}

template<const unsigned int GenericMaximumSize>
inline ArrayDefs::Real_ptr AlignedWorkSpaceArrays<GenericMaximumSize>::getData ( const unsigned int vec ) {

   if ( vec >= MaximumSize ) {
      throw formattingexception::GeneralException ();
   }

   return m_workSpaceValues [ vec ];
}

template<const unsigned int GenericMaximumSize>
inline ArrayDefs::ConstReal_ptr AlignedWorkSpaceArrays<GenericMaximumSize>::getData ( const unsigned int vec ) const {

   if ( vec >= MaximumSize ) {
      throw formattingexception::GeneralException ();
   }

   return m_workSpaceValues [ vec ];
}

#endif // UTILTIES__ALIGNED_WORK_SPACE_ARRAYS__H
