//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file ObsValueTransformable.cpp

#include "Observable.h"
#include "ObsValueTransformable.h"

#include <cassert>

namespace casa
{

   ObsValueTransformable::ObsValueTransformable( const Observable * parent, const std::vector<double> & val )
                                               : m_value( val.begin(), val.end() )
                                               , m_parent( parent )
   {
      ObsValueDoubleArray inpVal( parent, val );
      // transform observable value
      ObsValue * trObsVal = parent->transform( &inpVal );
      m_transfVals = trObsVal->asDoubleArray();
      delete trObsVal;
   }

   double ObsValueTransformable::MSE() const
   {
      double rmse = 0.0;

      if ( m_parent && m_parent->hasReferenceValue() )
      {
         const ObsValue * refVal = m_parent->referenceValue();

         if ( refVal )
         {

            const std::vector<double> & rv = refVal->asDoubleArray();
            const std::vector<double> * value = 0;

            if ( rv.size() == m_transfVals.size() ) { value = &m_transfVals; }
            else if ( rv.size() == m_value.size() ) { value = &m_value;      }

            const ObsValue * stDevVal = m_parent->stdDeviationForRefValue();
            assert( stDevVal );
            const std::vector<double> & stdev = stDevVal->asDoubleArray();
            assert( stdev.size() == rv.size() );

            if ( value )
            {
               for ( size_t i = 0; i < rv.size(); ++i )
               {
                  double sigma = ( stdev[i] > 0.0 ? stdev[i] : (0.1 * rv[i]) );

                  double v = ( rv[i] - (*value)[i] ) / sigma;
                  rmse += v * v;
               }
               rmse /= rv.size();
            }
         }
      }
      return rmse;
   }

   bool ObsValueTransformable::save( CasaSerializer & sz ) const
   {
      CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );

      bool ok = sz.save( parentID,     "ObservableID"      );
      ok = ok ? sz.save( m_value,      "arrayOfVals"       ) : ok;
      ok = ok ? sz.save( m_transfVals, "arrayOfTransfVals" ) : ok;
      return ok;
   }

   // Create a new ObsValueDoubleScalar instance and deserialize it from the given stream
   ObsValueTransformable::ObsValueTransformable( CasaDeserializer & dz, unsigned int /* objVer */ )
   {
      CasaDeserializer::ObjRefID parentID;

      bool ok = dz.load( parentID, "ObservableID" );
      m_parent = ok ? dz.id2ptr<Observable>( parentID ) : 0;

      ok = ok ? dz.load( m_value,      "arrayOfVals"       ) : ok;
      ok = ok ? dz.load( m_transfVals, "arrayOfTransfVals" ) : ok;

      if ( !ok || !m_parent )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "ObsValueTransformable deserialization unknown error";
      }
   }
}

