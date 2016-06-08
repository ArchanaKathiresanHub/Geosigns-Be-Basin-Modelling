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

// STL/C lib
#include <cassert>
#include <sstream>

namespace casa
{

// Create observable for the given trap property for specified areal position
ObsTrapProp::ObsTrapProp( double x
                        , double y
                        , const char * resName
                        , const char * propName
                        , double simTime
                        , const std::string & name
                        )
                        : m_x( x )
                        , m_y( y )
                        , m_resName( resName )
                        , m_propName( propName )
                        , m_simTime( simTime )
                        , m_posDataMiningTbl( -1 )
                        , m_saWeight( 1.0 )
                        , m_uaWeight( 1.0 )
                        , m_logTransf( false )

{
   // check input values
   if ( m_propName.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "No property name specified for trap target";

   // construct observable name
   if ( name.empty() )
   {
      std::ostringstream oss;
      oss << "Trap_" << m_propName << "(" << m_x << "," << m_y << "," << m_resName << "," << m_simTime << ")";
      m_name.push_back( oss.str() );
   }
   else { m_name.push_back( name ); }

   if ( m_propName.find( "Mass" ) != std::string::npos || m_propName.find( "Volume" ) != std::string::npos )
   {
      m_logTransf = true;
   }
}

// Destructor
ObsTrapProp::~ObsTrapProp() {;}

// Get name of the observable
std::vector<std::string> ObsTrapProp::name() const { return m_name; }
        
// Get standard deviations for the reference value
void ObsTrapProp::setReferenceValue( ObsValue * obsVal, ObsValue * devVal )
{
   assert( obsVal != NULL );
   assert( dynamic_cast<ObsValueDoubleScalar*>( obsVal ) != NULL );

   assert( devVal != NULL );
   assert( dynamic_cast<ObsValueDoubleScalar*>( devVal ) != NULL );

   m_refValue.reset( obsVal );
   m_devValue.reset( devVal );
}
 

ObsValue * ObsTrapProp::transform( const ObsValue * val ) const
{
   double ret = val->asDoubleArray()[0];

   if ( m_logTransf ) { ret = ret <= -5 ? 0.0 : pow( 10, ret ); } // set threshold for 1.e-5

   return ObsValueDoubleScalar::createNewInstance( this, ret );
}


// Update Model to be sure that requested property will be saved at the requested time
ErrorHandler::ReturnCode ObsTrapProp::requestObservableInModel( mbapi::Model & caldModel )
{
   if ( ErrorHandler::NoError != caldModel.snapshotManager().requestMajorSnapshot( m_simTime ) ) return caldModel.errorCode();

   m_posDataMiningTbl = caldModel.tableSize( Observable::s_dataMinerTable ); 
   
   if ( ErrorHandler::NoError != caldModel.addRowToTable( Observable::s_dataMinerTable ) ||
        ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "Time",          m_simTime            ) ||
        ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "XCoord",        m_x                  ) ||
        ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "YCoord",        m_y                  ) ||
        ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "ZCoord",        UndefinedDoubleValue ) ||
        ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "ReservoirName", m_resName            ) ||
        ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "PropertyName",  m_propName           ) ||
        ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "Value",         UndefinedDoubleValue ) 
      ) return caldModel.errorCode();

   return ErrorHandler::NoError;
}

  
// Get this observable value from Cauldron model
ObsValue * ObsTrapProp::getFromModel( mbapi::Model & caldModel )
{
   double val = UndefinedDoubleValue;
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
                  if ( caldModel.errorCode() == ErrorHandler::NoError && NumericFunctions::isEqual( zCrd, UndefinedDoubleValue, eps ) )
                  {
                     const std::string & resName = caldModel.tableValueAsString( Observable::s_dataMinerTable, i, "ReservoirName" );
                     if ( caldModel.errorCode() == ErrorHandler::NoError && m_resName == resName )
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

   // Data digger can't find specified trap property. It could be due to absence of the trap at given place
   // here we will try to avoid undefined values for some trap properties
   if ( NumericFunctions::isEqual( UndefinedDoubleValue, val, 1e-6 ) )
   {
      if ( m_propName.substr( 0, 6 ) == "Volume" || m_propName.substr( 0, 4 ) == "Mass" ) { val = 0.0; } // no trap at this place - no HC
   }

   if ( m_logTransf && UndefinedDoubleValue != val )
   {
       if ( val < 0.0 )
       {
          throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << 
             "TrapProp observable value for " << m_propName << " is negative: " << val << ", can not make logarithmic transformation";
       }  
      return ObsValueTransformable::createNewInstance( this, std::vector<double>( 1, log10( val < eps ? eps : val ) ) ); // set threshold for 1.e-5
   }

   return ObsValueDoubleScalar::createNewInstance(  this, val );
}


// Check well against project coordinates
std::string ObsTrapProp::checkObservableForProject( mbapi::Model & caldModel ) const
{
   std::ostringstream oss;

   double x0, y0;
   caldModel.origin( x0, y0 );
   
   double dimX, dimY;
   caldModel.arealSize( dimX, dimY );

   if ( m_x < x0 || m_x > x0 + dimX ||
        m_y < y0 || m_y > y0 + dimY )
   {
      oss << "Trap is outside of the project boundaries"; 
   }

   return oss.str();
}


// Create this observable value from double array (converting data from SUMlib for response surface evaluation
ObsValue * ObsTrapProp::createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const
{
   double dval = *val++;
   
   if ( m_logTransf ) return  ObsValueTransformable::createNewInstance( this, std::vector<double>( 1, dval ) );

   return ObsValueDoubleScalar::createNewInstance(  this, dval );
}

bool ObsTrapProp::save( CasaSerializer & sz, unsigned int /* version */) const
{
   // register observable with serializer to allow ObsValue objects keep reference after deserializtion
   CasaSerializer::ObjRefID obID = sz.ptr2id( this ); 

   bool ok = sz.save( obID, "ID" );
   ok = ok ? sz.save( m_x, "X" ) : ok;
   ok = ok ? sz.save( m_y, "Y" ) : ok;

   ok = ok ? sz.save( m_resName,  "reservoirName" ) : ok;
   ok = ok ? sz.save( m_propName, "propName"      ) : ok;
   ok = ok ? sz.save( m_simTime,  "simTime"       ) : ok;

   ok = ok ? sz.save( m_name, "name" ) : ok;

   ok = ok ? sz.save( m_posDataMiningTbl, "posDataMiningTbl" ) : ok;

   bool hasRefVal = m_refValue.get() ? true : false;
   ok = ok ? sz.save( hasRefVal, "HasRefValue" ) : ok;
   if ( hasRefVal ) { ok = ok ? sz.save( *(m_refValue.get()), "refValue" ) : ok; }
   
   bool hasDevVal = m_devValue.get() ? true : false;
   ok = ok ? sz.save( hasDevVal, "HasDevVal" ) : ok;
   if ( hasDevVal ) { ok = ok ? sz.save( *(m_devValue.get()), "devValue" ) : ok; }

   ok = ok ? sz.save( m_saWeight, "saWeight" ) : ok;
   ok = ok ? sz.save( m_uaWeight, "uaWeight" ) : ok;

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

   ok = ok ? dz.load( m_x, "X" ) : ok;
   ok = ok ? dz.load( m_y, "Y" ) : ok;

   ok = ok ? dz.load( m_resName,  "reservoirName" ) : ok;
   ok = ok ? dz.load( m_propName, "propName"      ) : ok;
   ok = ok ? dz.load( m_simTime,  "simTime"       ) : ok;
   ok = ok ? dz.load( m_name,      "name"         ) : ok;
   ok = ok ? dz.load( m_posDataMiningTbl, "posDataMiningTbl" ) : ok;

   bool hasVal;
   ok = ok ? dz.load( hasVal, "HasRefValue" ) : ok;

   if ( hasVal ) { m_refValue.reset( ObsValue::load( dz, "refValue" ) ); }

   if ( objVer == 0 )
   {
      double val;
      ok = ok ? dz.load( val, "devValue" ) : ok;
      if ( ok ) { m_devValue.reset( new  ObsValueDoubleScalar( this, val ) ); }
   }
   else
   {
      ok = ok ? dz.load( hasVal, "HasDevVal" ) : ok;
      if ( hasVal ) { m_devValue.reset( ObsValue::load( dz, "devValue" ) ); }
   }

   ok = ok ? dz.load( m_saWeight, "saWeight" ) : ok;
   ok = ok ? dz.load( m_uaWeight, "uaWeight" ) : ok;
   
   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "ObsTrapProp deserialization unknown error";
   }

   if ( m_propName.find( "Mass" ) != std::string::npos || m_propName.find( "Volume" ) != std::string::npos )
   {
      m_logTransf = true;
   }
   else { m_logTransf = false; }
}

}

