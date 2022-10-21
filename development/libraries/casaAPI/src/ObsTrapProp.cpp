//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file ObsTrapProp.cpp

// CASA API
#include "ObsValueDoubleScalar.h"
#include "ObsValueTransformable.h"
#include "ObsTrapProp.h"

// CMB API
#include "cmbAPI.h"
#include "UndefinedValues.h"

// Utilities lib
#include "NumericFunctions.h"
#include "LogHandler.h"
#include "Utilities.h"

// STL/C lib
#include <cassert>
#include <sstream>

namespace casa
{

// Create observable for the given trap property for specified areal position
ObsTrapProp::ObsTrapProp( double              x
                        , double              y
                        , const char        * resName
                        , const char        * propName
                        , double              simTime
                        , bool                logTrans
                        , const std::string & myName
                        )
                        : ObservableSinglePoint( myName, propName, simTime, x, y )
                        , m_resName( resName )
                        , m_logTransf( logTrans )
{
   // Note: size of PosDataMiningTbl.size = 1 here!
   setPosDataMiningTbl( { -1 } );
}

// Destructor
ObsTrapProp::~ObsTrapProp() {}

ObsValue * ObsTrapProp::transform( const ObsValue * val ) const
{
   double ret = val->asDoubleArray()[0];

   if ( m_logTransf ) { ret = ret <= -5 ? 0.0 : pow( 10, ret ); } // set threshold for 1.e-5

   return ObsValueDoubleScalar::createNewInstance( this, ret );
}


// Update Model to be sure that requested property will be saved at the requested time
ErrorHandler::ReturnCode ObsTrapProp::requestObservableInModel( mbapi::Model & caldModel )
{
   if ( ErrorHandler::NoError != caldModel.snapshotManager().requestMajorSnapshot( simulationTime() ) ) return caldModel.errorCode();

   setPosDataMiningTbl( { caldModel.tableSize( Observable::s_dataMinerTable ) } );

   if ( !setCommonTableValues( caldModel, posDataMiningTbl()[0], xCoord(), yCoord(), zCoord(), "ReservoirName", m_resName ) )
   { return caldModel.errorCode(); }

   return ErrorHandler::NoError;
}


// Get this observable value from Cauldron model
ObsValue * ObsTrapProp::getFromModel( mbapi::Model & caldModel )
{
   double val = Utilities::Numerical::IbsNoDataValue;
   double eps = 1.e-5;

   if ( !checkObservableForProject( caldModel ) )
   {
     return new ObsValueDoubleScalar( this, val );
   }

   if ( posDataMiningTbl()[0] < 0 )
   {  // search in table for correct position
      size_t tblSize = caldModel.tableSize( Observable::s_dataMinerTable );
      bool found = false;
      for ( size_t i = 0; i < tblSize; ++i )
      {
        if ( !checkObsMatchesModel( caldModel, i, xCoord(), yCoord(), zCoord(), eps, "ReservoirName", m_resName ) ) { continue; }

         val = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, i, "Value" );
         setPosDataMiningTbl( { static_cast<int>( i ) } );
         break;
      }
   }
   else
   {
      val = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, posDataMiningTbl()[0], "Value" );
   }

   if ( caldModel.errorCode() != ErrorHandler::NoError ) return NULL;

   // Data digger can't find specified trap property. It could be due to absence of the trap at given place
   // here we will try to avoid undefined values for some trap properties
   if ( Utilities::isValueUndefined( val ) )
   {
      if ( propertyName().substr( 0, 6 ) == "Volume" || propertyName().substr( 0, 4 ) == "Mass" ) { val = 0.0; } // no trap at this place - no HC
   }

   if ( m_logTransf && !Utilities::isValueUndefined( val ) )
   {
      if ( val < 0.0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) <<
            "TrapProp observable value for " << propertyName() << " is negative: " << val << ", can not make logarithmic transformation";
      }
      return ObsValueTransformable::createNewInstance( this, std::vector<double>( 1, log10( val < eps ? eps : val ) ) ); // set threshold for 1.e-5
   }

   return ObsValueDoubleScalar::createNewInstance(  this, val );
}

// Create this observable value from double array (converting data from SUMlib for response surface evaluation
ObsValue * ObsTrapProp::createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const
{
   double dval = *val++;

   if ( m_logTransf ) return  ObsValueTransformable::createNewInstance( this, std::vector<double>( 1, dval ) );

   return ObsValueDoubleScalar::createNewInstance(  this, dval );
}

bool ObsTrapProp::save( CasaSerializer & sz ) const
{
   bool ok = false;
   saveCommon( this, sz, ok, "reservoirName", m_resName );

   ok = ok ? sz.save( m_logTransf, "logTransf" ) : ok;

   return ok;
}

ObsTrapProp::ObsTrapProp( CasaDeserializer & dz, unsigned int objVer )
{
   if ( version() < objVer )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) <<
         "Version of ObsTrapProp in file is newer. No forward compatibility!";
   }

   CasaDeserializer::ObjRefID obID;

   // load data necessary to create an object
   bool ok = dz.load( obID, "ID" );

   // register observable with deserializer under read ID to allow ObsValue objects keep reference after deserializtion
   dz.registerObjPtrUnderID( this, obID );

   double x = 0.0;
   double y = 0.0;
   ok = ok ? dz.load( x, "X" ) : ok;
   ok = ok ? dz.load( y, "Y" ) : ok;
   setCoords(x, y);

   ok = ok ? dz.load( m_resName,  "reservoirName" ) : ok;

   std::string propName;
   double simTime;
   std::vector<std::string> myName;
   ok = ok ? dz.load( propName, "propName" ) : ok;
   ok = ok ? dz.load( simTime,  "simTime"  ) : ok;
   ok = ok ? dz.load( myName,   "name"     ) : ok;
   setPropertyName( propName );
   setSimTime( simTime );
   setName( myName );

   std::vector<int> posDataVec;
   ok = ok ? dz.load( posDataVec, "posDataMiningTbl" ) : ok;
   setPosDataMiningTbl( posDataVec );

   bool hasVal;
   ok = ok ? dz.load( hasVal, "HasRefValue" ) : ok;

   if ( hasVal ) { setRefValue( ObsValue::load( dz, "refValue" ) ); }

   if ( objVer == 0 )
   {
      double val;
      ok = ok ? dz.load( val, "devValue" ) : ok;
      if ( ok ) { setDevValue( new  ObsValueDoubleScalar( this, val ) ); }
   }
   else
   {
      ok = ok ? dz.load( hasVal, "HasDevVal" ) : ok;
      if ( hasVal ) { setDevValue( ObsValue::load( dz, "devValue" ) ); }
   }

   double mySAWeight;
   double myUAWeight;
   ok = ok ? dz.load( mySAWeight, "saWeight" ) : ok;
   ok = ok ? dz.load( myUAWeight, "uaWeight" ) : ok;
   setSAWeight( mySAWeight );
   setUAWeight( myUAWeight );

   if ( objVer > 1 )
   {
      ok = ok ? dz.load( m_logTransf, "logTransf" ) : ok;
   }
   else
   {
      if ( propertyName().find( "Mass" ) != std::string::npos || propertyName().find( "Volume" ) != std::string::npos )
      {
         m_logTransf = true;
      }
      else { m_logTransf = false; }
   }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "ObsTrapProp deserialization unknown error";
   }
}

} // namespace casa
