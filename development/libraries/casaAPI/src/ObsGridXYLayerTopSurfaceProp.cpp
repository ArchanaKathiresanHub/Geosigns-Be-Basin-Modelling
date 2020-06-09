//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file ObsGridXYLayerTopSurfaceProp.cpp

#include "ObsValueDoubleScalar.h"
#include "ObsGridXYLayerTopSurfaceProp.h"

#include "cmbAPI.h"

#include "LogHandler.h"

#include <cassert>
#include <sstream>

namespace casa
{

// Create observable for the given property for specified areal position in a layer (based on given layer name)
ObsGridXYLayerTopSurfaceProp::ObsGridXYLayerTopSurfaceProp( const double        x
                                                          , const double        y
                                                          , const std::string & layerName
                                                          , const std::string & propName
                                                          , const double        simTime
                                                          , const std::string & myName
                                                          )
                                                          : ObservableSinglePoint( myName, propName, simTime, x, y )
                                                          , m_layerName( layerName )
{
   // Note: size of PosDataMiningTbl.size = 1 here!
   setPosDataMiningTbl( { -1 } );
}

// Destructor
ObsGridXYLayerTopSurfaceProp::~ObsGridXYLayerTopSurfaceProp() {}

// Update Model to be sure that requested property will be saved at the requested time
ErrorHandler::ReturnCode ObsGridXYLayerTopSurfaceProp::requestObservableInModel( mbapi::Model & cldModel )
{
   if ( ErrorHandler::NoError != cldModel.snapshotManager().requestMajorSnapshot( simulationTime() ) ||
        ErrorHandler::NoError != cldModel.propertyManager().requestPropertyInSnapshots( propertyName() )
      ) return cldModel.errorCode();

   setPosDataMiningTbl( { cldModel.tableSize( Observable::s_dataMinerTable ) } );

   if ( ErrorHandler::NoError != cldModel.addRowToTable( Observable::s_dataMinerTable ) ) return cldModel.errorCode();

   if ( !setCommonTableValues( cldModel, posDataMiningTbl()[0], xCoord(), yCoord(), zCoord(), "FormationName", m_layerName ) )
   { return cldModel.errorCode(); }

   return ErrorHandler::NoError;
}

// Get this observable value from Cauldron model
ObsValue * ObsGridXYLayerTopSurfaceProp::getFromModel( mbapi::Model & caldModel )
{
   double val = Utilities::Numerical::IbsNoDataValue;
   double eps = 1.e-5;

   if ( !checkObservableForProject( caldModel ) )
   {
     return new ObsValueDoubleScalar( this, val );
   }

   if ( posDataMiningTbl()[0] < 0 )
   {
      size_t tblSize = caldModel.tableSize( Observable::s_dataMinerTable );
      for ( size_t i = 0; i < tblSize; ++i )
      {
        if ( !checkObsMatchesModel( caldModel, i, xCoord(), yCoord(), zCoord(), eps, "FormationName", m_layerName ) ) { continue; }

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

   return new ObsValueDoubleScalar( this, val );
}

// Create this observable value from double array (converting data from SUMlib for response surface evaluation
ObsValue * ObsGridXYLayerTopSurfaceProp::createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const
{
   return new ObsValueDoubleScalar( this, *val++ );
}

bool ObsGridXYLayerTopSurfaceProp::save( CasaSerializer & sz ) const
{
   bool ok = false;
   saveCommon( this, sz, ok, "layerName", m_layerName );
   return ok;
}

ObsGridXYLayerTopSurfaceProp::ObsGridXYLayerTopSurfaceProp( CasaDeserializer & dz, unsigned int objVer )
{
   if ( version() < objVer )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) <<
         "Version of ObsGridXYLayerTopSurfaceProp in file is newer. No forward compatibility!";
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

   ok = ok ? dz.load( m_layerName, "layerName" ) : ok;

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

   bool hasRefVal;
   ok = ok ? dz.load( hasRefVal, "HasRefValue" ) : ok;

   if ( hasRefVal ) { setRefValue( ObsValue::load( dz, "refValue" ) ); }

   bool hasDevVal;
   ok = ok ? dz.load( hasDevVal, "HasDevVal" ) : ok;
   if ( hasDevVal ) { setDevValue( ObsValue::load( dz, "devValue" ) ); }

   double mySAWeight;
   double myUAWeight;
   ok = ok ? dz.load( mySAWeight, "saWeight" ) : ok;
   ok = ok ? dz.load( myUAWeight, "uaWeight" ) : ok;
   setSAWeight( mySAWeight );
   setUAWeight( myUAWeight );

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "ObsGridXYLayerTopSurfaceProp deserialization unknown error";
   }
}

} //namespace casa
