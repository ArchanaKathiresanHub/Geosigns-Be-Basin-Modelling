// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "CollectKrigingData.h"
#include "Exception.h"

namespace SUMlib {

CollectKrigingData::CollectKrigingData()
{
   // empty
}

CollectKrigingData::CollectKrigingData(
         ParameterSet const&        parSet
         )
{
   initialise( parSet, 0 );
}

CollectKrigingData::CollectKrigingData(
         ParameterSet const&        parSet,
         unsigned int               numCategorical
         )
{
   initialise( parSet, numCategorical );
}


CollectKrigingData::~CollectKrigingData()
{
   // destroy the KrigingData objects owned by this instance
   for ( KrigingDataMap::iterator it = m_KrigingDataMap.begin(); it != m_KrigingDataMap.end(); ++it )
   {
      delete (*it).second;
   }
}

void CollectKrigingData::initialise(
         ParameterSet const&        parSet,
         unsigned int               num
         )
{
   if ( parSet.empty() )
   {
      THROW2( DimensionOutOfBounds, "Parameter set cannot be empty" );
   }

   if ( num > 0 )
   {
      //@todo separate categorical part of each parameter and use it as a key in a map

      THROW2( InvalidValue, "Categorical parameters not yet implemented" );
      // For all keys of m_KrigingDataMap
      // * select the parSet for each key
      // * store a ptr to the corresponding Kriging data model
   }
   else
   {
      CategoricalCombination key;
      m_KrigingDataMap[key] = new KrigingData( parSet, parSet.front().size() );
   }
}

} // namespace SUMlib
