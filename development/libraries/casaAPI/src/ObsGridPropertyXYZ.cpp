//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file ObsGridPropertyXYZ.cpp

#include "ObsGridPropertyXYZ.h"
#include "ObsValueDoubleScalar.h"

// CMB API
#include "cmbAPI.h"

// utilities
#include "NumericFunctions.h"
#include "LogHandler.h"

// C & STL
#include <cassert>
#include <sstream>

using namespace Utilities::Numerical;

namespace casa
{

// Create observable for the given grid property for specified grid position
ObsGridPropertyXYZ::ObsGridPropertyXYZ( const double        x
                                      , const double        y
                                      , const double        z
                                      , const std::string & propName
                                      , const double        mTime
                                      , const std::string & myName
                                      )
                                      : ObservableSinglePoint( myName, propName, mTime, x, y, z )
{
  // Note: size of PosDataMiningTbl.size = 1 here!
  setPosDataMiningTbl( { -1 } );
}

// Destructor
ObsGridPropertyXYZ::~ObsGridPropertyXYZ() {}

// Update Model to be sure that requested property will be saved at the requested time
ErrorHandler::ReturnCode ObsGridPropertyXYZ::requestObservableInModel( mbapi::Model & caldModel )
{
   if ( ErrorHandler::NoError != caldModel.snapshotManager().requestMajorSnapshot(       simulationTime() ) ) return caldModel.errorCode();
   if ( ErrorHandler::NoError != caldModel.propertyManager().requestPropertyInSnapshots( propertyName() ) ) return caldModel.errorCode();

   setPosDataMiningTbl( { caldModel.tableSize( s_dataMinerTable ) } );

   if ( ErrorHandler::NoError != caldModel.addRowToTable( s_dataMinerTable ) ) return caldModel.errorCode();

   if ( !setCommonTableValues( caldModel, posDataMiningTbl()[0], xCoord(), yCoord(), zCoord() ) )
   { return caldModel.errorCode(); }

   return ErrorHandler::NoError;
}


// Get this observable value from Cauldron model
ObsValue * ObsGridPropertyXYZ::getFromModel( mbapi::Model & caldModel )
{
   double val = IbsNoDataValue;

   if ( !checkObservableForProject( caldModel ) )
   {
     return new ObsValueDoubleScalar( this, val );
   }

   const double eps = 1.e-5;

   if ( posDataMiningTbl()[0] < 0 ) // do search in table for this Observable
   {
      size_t tblSize = caldModel.tableSize( s_dataMinerTable );
      for ( size_t i = 0; i < tblSize; ++i )
      {
         if ( !checkObsMatchesModel( caldModel, i, xCoord(), yCoord(), zCoord(), eps ) ) { continue; }

         val = caldModel.tableValueAsDouble( s_dataMinerTable, i, "Value" );
         setPosDataMiningTbl( { static_cast<int>( i ) } );
         break;
      }
   }
   else { val = caldModel.tableValueAsDouble( s_dataMinerTable, posDataMiningTbl()[0], "Value" ); }

   return caldModel.errorCode() == ErrorHandler::NoError ? new ObsValueDoubleScalar( this, val ) : nullptr;
}

// Create this observable value from double array (converting data from SUMlib for response surface evaluation
ObsValue * ObsGridPropertyXYZ::createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const
{
   return new ObsValueDoubleScalar( this, *val++ );
}

bool ObsGridPropertyXYZ::save( CasaSerializer & sz ) const
{
   bool ok = false;
   saveCommon( this, sz, ok );
   return ok;
}

ObsGridPropertyXYZ::ObsGridPropertyXYZ( CasaDeserializer & dz, unsigned int objVer )
{
   if ( version() < objVer )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) <<
         "Version of ObsGridPropertyXYZ in file is newer. No forward compatibility!";
   }

   CasaDeserializer::ObjRefID obID;

   // load data necessary to create an object
   bool ok = dz.load( obID, "ID" );

   // register observable with deserializer under read ID to allow ObsValue objects keep reference after deserializtion
   dz.registerObjPtrUnderID( this, obID );

   double x = 0.0;
   double y = 0.0;
   double z = 0.0;
   ok = ok ? dz.load( x, "X" ) : ok;
   ok = ok ? dz.load( y, "Y" ) : ok;
   ok = ok ? dz.load( z, "Z" ) : ok;
   setCoords(x, y, z);

   std::string propName;
   double simTime;
   std::vector<std::string> myName;
   ok = ok ? dz.load( propName, "propName" ) : ok;
   ok = ok ? dz.load( simTime,  "simTime"  ) : ok;
   ok = ok ? dz.load( myName,   "name"     ) : ok;
   setPropertyName( propName );
   setSimTime( simTime );
   setName( myName );

   if (objVer <= 0)
   {
     int posData = 0;
     ok = ok ? dz.load( posData, "posDataMiningTbl" ) : ok;
     setPosDataMiningTbl( { posData } );
   }
   else
   {
     std::vector<int> posDataVec;
     ok = ok ? dz.load( posDataVec, "posDataMiningTbl" ) : ok;
     setPosDataMiningTbl( posDataVec );
   }

   bool hasRefVal;
   ok = ok ? dz.load( hasRefVal, "HasRefValue" ) : ok;

   if ( hasRefVal ) { setRefValue( ObsValue::load( dz, "refValue" ) ); }

   if ( objVer == 0 )
   {
      double val;
      ok = ok ? dz.load( val, "devValue" ) : ok;
      if ( ok ) { setDevValue( new ObsValueDoubleScalar( this, val ) ); }
   }
   else
   {
      bool hasDevVal;
      ok = ok ? dz.load( hasDevVal, "HasDevVal" ) : ok;
      if ( hasDevVal ) { setDevValue( ObsValue::load( dz, "devValue" ) ); }
   }

   double mySAWeight;
   double myUAWeight;
   ok = ok ? dz.load( mySAWeight, "saWeight" ) : ok;
   ok = ok ? dz.load( myUAWeight, "uaWeight" ) : ok;
   setSAWeight( mySAWeight );
   setUAWeight( myUAWeight );

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "ObsGridPropertyXYZ deserialization unknown error";
   }
}

} // namespace casa

