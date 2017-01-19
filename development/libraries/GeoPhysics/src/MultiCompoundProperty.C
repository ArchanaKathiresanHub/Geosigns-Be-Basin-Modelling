//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "MultiCompoundProperty.h"
#include "AlignedMemoryAllocator.h"

GeoPhysics::MultiCompoundProperty::MultiCompoundProperty () :
   m_numberOfLithologies ( 0 ),
   m_dataCount ( 0 ),
   m_allValues ( nullptr )
{
}

GeoPhysics::MultiCompoundProperty::MultiCompoundProperty ( const unsigned int lithoCount,
                                                           const unsigned int dataCount ) :
   m_numberOfLithologies ( lithoCount ),
   m_dataCount ( dataCount ),
   m_allValues ( nullptr )
{
   allocate ( m_numberOfLithologies, m_dataCount );
}

GeoPhysics::MultiCompoundProperty::~MultiCompoundProperty () {
   deallocate ();
}

void GeoPhysics::MultiCompoundProperty::resize ( const unsigned int lithoCount,
                                                 const unsigned int dataCount ) {

   if ( lithoCount == m_numberOfLithologies and dataCount == m_dataCount ) {
      // Nothing to do
      return;
   }

   if ( dataCount != m_dataCount ) {
      // Only need to reallocate if the nuber of data points changes.
      allocate ( lithoCount, dataCount );
   }

   m_numberOfLithologies = lithoCount;
   m_dataCount = dataCount;
}


void GeoPhysics::MultiCompoundProperty::allocate ( const unsigned int lithoCount,
                                                   const unsigned int dataCount ) {

   unsigned int arraySizeRequired = sizeof ( double ) * dataCount;
   unsigned int remainder = arraySizeRequired % ARRAY_ALIGNMENT;
   unsigned int totalSizeRequired;

   if ( remainder != 0 ) {
      arraySizeRequired += ARRAY_ALIGNMENT - remainder;
   }

   m_alignedArraySize = arraySizeRequired / sizeof ( double );

   // Always allocate the enough space for the maximum number of lithologies.
   totalSizeRequired = ( MaximumNumberOfLithologies + 1 ) * arraySizeRequired;

   if ( m_allValues != nullptr ) {
      deallocate ();
   }

   m_allValues = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( totalSizeRequired );
   ArrayDefs::Real_ptr buffer = m_allValues;

   for ( unsigned int i = 0; i < MaximumNumberOfLithologies; ++i, buffer += m_alignedArraySize ) {
      m_simpleValues [ i ] = buffer;
   }

   m_mixedValues = buffer;
}

void GeoPhysics::MultiCompoundProperty::deallocate () {
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( m_allValues );
   m_allValues = nullptr;

   for ( unsigned int i = 0; i < MaximumNumberOfLithologies; ++i ) {
      m_simpleValues [ i ] = nullptr;
   }

   m_mixedValues = nullptr;
}
