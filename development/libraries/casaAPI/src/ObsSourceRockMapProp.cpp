//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file ObsSourceRockMapProp.C

#include "ObsValueDoubleScalar.h"
#include "ObsSourceRockMapProp.h"

#include "cmbAPI.h"
#include "UndefinedValues.h"

#include "NumericFunctions.h"
#include "LogHandler.h"

#include <cassert>
#include <sstream>

namespace casa
{

// Create observable for the given Genex map property for specified areal position
ObsSourceRockMapProp::ObsSourceRockMapProp( double              x
                                          , double              y
                                          , const char        * layerName
                                          , const char        * propName
                                          , double              simTime
                                          , const std::string & name
                                          )
                                          : m_x( x )
                                          , m_y( y )
                                          , m_layerName( layerName )
                                          , m_propName( propName )
                                          , m_simTime( simTime )
                                          , m_posDataMiningTbl( -1 )
                                          , m_saWeight( 1.0 )
                                          , m_uaWeight( 1.0 )



{
   // check input values
   if ( m_propName.empty() )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << 
         "No property name specified for source rock map property target";
   }

   // construct observable name
   if ( name.empty() )
   {
      std::ostringstream oss;
      oss << m_propName << "(" << m_x << "," << m_y << "," << m_layerName << "," << m_simTime << ")";
      m_name.push_back( oss.str() );
   }
   else { m_name.push_back( name ); }
}

// Destructor
ObsSourceRockMapProp::~ObsSourceRockMapProp() {;}

// Get name of the observable
std::vector<std::string> ObsSourceRockMapProp::name() const { return m_name; }

// Get standard deviations for the reference value
void ObsSourceRockMapProp::setReferenceValue( ObsValue * obsVal, ObsValue * devVal )
{
   assert( obsVal != nullptr );
   ObsValueDoubleScalar * val = dynamic_cast<ObsValueDoubleScalar*>( obsVal );
   assert( val != nullptr );

   m_refValue.reset( obsVal );

   assert( devVal != nullptr );
   ObsValueDoubleScalar * dev = dynamic_cast<ObsValueDoubleScalar*>( devVal ); 
   assert( dev != nullptr );

   // check dev for negative/zero value
   if ( dev->value() <= 0.0 )
   {
      double newDev = std::abs( val->value() ) * 0.1;
      if ( newDev == 0.0 ) { newDev = 0.1; }

      LogHandler( LogHandler::WARNING_SEVERITY ) << "Invalid the standard deviation value: " << dev->value()
                                                 << " for the target " << m_name[0] << ", possible error in scenario setup. "
                                                 << "Replacing it with the default value (0.1*refVal): " << newDev;
      delete devVal;
      devVal = ObsValueDoubleScalar::createNewInstance( this, newDev );
   }

   m_devValue.reset( devVal );
}


// Update Model to be sure that requested property will be saved at the requested time
ErrorHandler::ReturnCode ObsSourceRockMapProp::requestObservableInModel( mbapi::Model & cldModel )
{
   if ( ErrorHandler::NoError != cldModel.snapshotManager().requestMajorSnapshot(       m_simTime ) ||
        ErrorHandler::NoError != cldModel.propertyManager().requestPropertyInSnapshots( m_propName, "SourceRockOnly" )
      ) return cldModel.errorCode();

   m_posDataMiningTbl = cldModel.tableSize( Observable::s_dataMinerTable );

   if ( ErrorHandler::NoError != cldModel.addRowToTable( Observable::s_dataMinerTable ) ) return cldModel.errorCode();

   if ( ErrorHandler::NoError != cldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "Time",          m_simTime            ) ||
        ErrorHandler::NoError != cldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "XCoord",        m_x                  ) ||
        ErrorHandler::NoError != cldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "YCoord",        m_y                  ) ||
        ErrorHandler::NoError != cldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "ZCoord",
                                                                                                            Utilities::Numerical::IbsNoDataValue ) ||
        ErrorHandler::NoError != cldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "FormationName", m_layerName          ) ||
        ErrorHandler::NoError != cldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "PropertyName",  m_propName           ) ||
        ErrorHandler::NoError != cldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "Value",
                                                                                                            Utilities::Numerical::IbsNoDataValue ) 
      ) return cldModel.errorCode();

   return ErrorHandler::NoError;
}


// Get this observable value from Cauldron model
ObsValue * ObsSourceRockMapProp::getFromModel( mbapi::Model & caldModel )
{
   double val = Utilities::Numerical::IbsNoDataValue;
   double eps = 1.e-5;

   const std::string & msg = checkObservableForProject( caldModel );
   if ( !msg.empty() ) { return new ObsValueDoubleScalar( this, val ); }

   if ( m_posDataMiningTbl < 0 )
   {
      size_t tblSize = caldModel.tableSize( Observable::s_dataMinerTable );
      bool found = false;
      for ( size_t i = 0; i < tblSize && !found; ++i )
      {
         double obTime = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, i, "Time" );
         if ( caldModel.errorCode() == ErrorHandler::NoError && NumericFunctions::isEqual( obTime, m_simTime, eps ) )
         {
            double xCrd = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, i, "XCoord" );
            if ( caldModel.errorCode() == ErrorHandler::NoError && NumericFunctions::isEqual( xCrd, m_x, eps ) )
            {
               double yCrd = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, i, "YCoord" );
               if ( caldModel.errorCode() == ErrorHandler::NoError && NumericFunctions::isEqual( yCrd, m_y, eps ) )
               {
                  double zCrd = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, i, "ZCoord" );
                  if ( caldModel.errorCode() == ErrorHandler::NoError && IsValueUndefined( zCrd ) )
                  {
                     const std::string & formName = caldModel.tableValueAsString( Observable::s_dataMinerTable, i, "FormationName" );
                     if ( caldModel.errorCode() == ErrorHandler::NoError && m_layerName == formName )
                     {
                        const std::string & propName = caldModel.tableValueAsString( Observable::s_dataMinerTable, i, "PropertyName" );
                        if ( caldModel.errorCode() == ErrorHandler::NoError && m_propName == propName )
                        {
                           found = true;
                           val = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, i, "Value" );
                           m_posDataMiningTbl = static_cast<int>( i );
                        }
                     }
                  }
               }
            }
         }
      }
   }
   else
   {
      val = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, m_posDataMiningTbl, "Value" );
   }

   if ( caldModel.errorCode() != ErrorHandler::NoError ) return NULL;

   return new ObsValueDoubleScalar( this, val );
}

// Check well against project coordinates
std::string ObsSourceRockMapProp::checkObservableForProject( mbapi::Model & caldModel ) const
{
   std::ostringstream oss;

   double x0, y0;
   caldModel.origin( x0, y0 );
   
   double dimX, dimY;
   caldModel.arealSize( dimX, dimY );

   if ( m_x < x0 || m_x > x0 + dimX ||
        m_y < y0 || m_y > y0 + dimY )
   {
      oss << "Observable for source rock map property " << m_name.front() << " is outside of the project boundaries: "; 
   }

   return oss.str();
}


// Create this observable value from double array (converting data from SUMlib for response surface evaluation
ObsValue * ObsSourceRockMapProp::createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const
{
   return new ObsValueDoubleScalar( this, *val++ );
}

bool ObsSourceRockMapProp::save( CasaSerializer & sz, unsigned int /* version */ ) const
{
   // register observable with serializer to allow ObsValue objects keep reference after deserializtion
   CasaSerializer::ObjRefID obID = sz.ptr2id( this );

   bool ok = sz.save( obID, "ID" );
   ok = ok ? sz.save( m_x, "X" ) : ok;
   ok = ok ? sz.save( m_y, "Y" ) : ok;

   ok = ok ? sz.save( m_layerName, "layerName" ) : ok;
   ok = ok ? sz.save( m_propName,  "propName"  ) : ok;
   ok = ok ? sz.save( m_simTime,   "simTime"   ) : ok;

   ok = ok ? sz.save( m_name, "name" ) : ok;

   ok = ok ? sz.save( m_posDataMiningTbl, "posDataMiningTbl" ) : ok;

   bool hasRefVal = m_refValue.get() ? true : false;
   ok = ok ? sz.save( hasRefVal, "HasRefValue" ) : ok;
   if ( hasRefVal ) { ok = ok ? sz.save( *(m_refValue.get()), "refValue" ) : ok; }

   bool hasDevVal = m_devValue.get( ) ? true : false;
   ok = ok ? sz.save( hasDevVal, "HasDevVal" ) : ok;
   if ( hasDevVal ) { ok = ok ? sz.save( *( m_devValue.get( ) ), "devValue" ) : ok; }
   
   ok = ok ? sz.save( m_saWeight, "saWeight" ) : ok;
   ok = ok ? sz.save( m_uaWeight, "uaWeight" ) : ok;

   return ok;
}

ObsSourceRockMapProp::ObsSourceRockMapProp( CasaDeserializer & dz, unsigned int objVer )
{
   if ( version() < objVer )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) <<
         "Version of ObsSourceRockMapProp in file is newer. No forward compatibility!";
   }

   CasaDeserializer::ObjRefID obID;

   // load data necessary to create an object
   bool ok = dz.load( obID, "ID" );

   // register observable with deserializer under read ID to allow ObsValue objects keep reference after deserializtion
   dz.registerObjPtrUnderID( this, obID );

   ok = ok ? dz.load( m_x, "X" ) : ok;
   ok = ok ? dz.load( m_y, "Y" ) : ok;

   ok = ok ? dz.load( m_layerName,        "layerName"        ) : ok;
   ok = ok ? dz.load( m_propName,         "propName"         ) : ok;
   ok = ok ? dz.load( m_simTime,          "simTime"          ) : ok;
   ok = ok ? dz.load( m_name,             "name"             ) : ok;
   ok = ok ? dz.load( m_posDataMiningTbl, "posDataMiningTbl" ) : ok;


   bool hasRefVal;
   ok = ok ? dz.load( hasRefVal,          "HasRefValue" ) : ok;

   if ( hasRefVal ) { m_refValue.reset( ObsValue::load( dz, "refValue" ) ); }

   if ( objVer == 0 )
   {
      double val;
      ok = ok ? dz.load( val, "devValue" ) : ok;
      if ( ok ) { m_devValue.reset( new  ObsValueDoubleScalar( this, val ) ); }  
   }
   else
   {
      bool hasDevVal;
      ok = ok ? dz.load( hasDevVal, "HasDevVal" ) : ok;
      if ( hasDevVal ) { m_devValue.reset( ObsValue::load( dz, "devValue" ) ); }
   }

   ok = ok ? dz.load( m_saWeight, "saWeight" ) : ok;
   ok = ok ? dz.load( m_uaWeight, "uaWeight" ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "ObsSourceRockMapProp deserialization unknown error";
   }
}

}
