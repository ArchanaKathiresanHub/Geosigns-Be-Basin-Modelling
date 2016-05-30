//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsValueDoubleScalar.C

#include "Observable.h"
#include "ObsValueDoubleScalar.h"

#include <cassert>

namespace casa
{
   double ObsValueDoubleScalar::MSE() const
   {
      double rmse = 0.0;

      if ( m_parent && m_parent->hasReferenceValue() )
      {
         const ObsValueDoubleScalar * refVal = dynamic_cast<const ObsValueDoubleScalar*>(m_parent->referenceValue());

         if ( refVal )
         {
            const std::vector<double> & rv = refVal->asDoubleArray();
            rmse = ( rv[0] - value( ) ) / ( m_parent->stdDeviationForRefValue( )->asDoubleArray( )[0]  > 0.0 ? m_parent->stdDeviationForRefValue( )->asDoubleArray( )[0] : ( 0.1 * rv[0] ) );
         }
      }
      return rmse * rmse;
   }

   bool ObsValueDoubleScalar::save( CasaSerializer & sz, unsigned int /* version */ ) const
   {
      CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );
      
      bool ok = sz.save( parentID, "ObservableID" );
      ok = ok ? sz.save( m_value, "scalarVal" ) : ok;
      return ok;
   }

   // Create a new ObsValueDoubleScalar instance and deserialize it from the given stream
   ObsValueDoubleScalar::ObsValueDoubleScalar( CasaDeserializer & dz, unsigned int /* objVer */ )
   {
      CasaDeserializer::ObjRefID parentID;

      bool ok = dz.load( parentID, "ObservableID" );
      m_parent = ok ? dz.id2ptr<Observable>( parentID ) : 0;

      ok = ok ? dz.load( m_value,  "scalarVal" ) : ok;

      if ( !ok || !m_parent )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "ObsValueDoubleScalar deserialization unknown error";
      }
   }
}
