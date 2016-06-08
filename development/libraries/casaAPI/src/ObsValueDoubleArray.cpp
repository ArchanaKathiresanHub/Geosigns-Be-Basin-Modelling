//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsValueDoubleArray.C

#include "Observable.h"
#include "ObsValueDoubleArray.h"

#include <cassert>

namespace casa
{
   double ObsValueDoubleArray::MSE() const
   {
      double rmse = 0.0;

      if ( m_value.size() && m_parent && m_parent->hasReferenceValue() )
      {
         const ObsValueDoubleArray * refVal = dynamic_cast<const ObsValueDoubleArray*>( m_parent->referenceValue() );
   
         if ( refVal )
         {
            const std::vector<double> & rv = refVal->asDoubleArray( true );
            assert( rv.size() == m_value.size() );

            for ( size_t i = 0; i < rv.size(); ++i )
            {
               double sigma = ( m_parent->stdDeviationForRefValue( )->asDoubleArray( )[i] > 0.0 ? m_parent->stdDeviationForRefValue( )->asDoubleArray( )[i] : ( 0.1 * rv[i] ) );

               double v = ( rv[i] - m_value[i] ) / sigma;
               rmse += v * v;
            }
            rmse /= rv.size();
         }
      }
      return rmse;
   }

   bool ObsValueDoubleArray::save( CasaSerializer & sz, unsigned int /* version */ ) const
   {
      CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );

      bool ok = sz.save( parentID, "ObservableID" );
      ok = ok ? sz.save( m_value, "arrayOfVals" ) : ok;
      return ok;
   }

   // Create a new ObsValueDoubleScalar instance and deserialize it from the given stream
   ObsValueDoubleArray::ObsValueDoubleArray( CasaDeserializer & dz, unsigned int /* objVer */ )
   {
      CasaDeserializer::ObjRefID parentID;

      bool ok = dz.load( parentID, "ObservableID" );
      m_parent = ok ? dz.id2ptr<Observable>( parentID ) : 0;
      
      ok = ok ? dz.load( m_value, "arrayOfVals" ) : ok;

      if ( !ok || !m_parent )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "ObsValueDoubleArray deserialization unknown error";
      }
   }

}

