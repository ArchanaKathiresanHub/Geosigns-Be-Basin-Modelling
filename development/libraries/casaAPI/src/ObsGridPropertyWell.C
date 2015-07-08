//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsGridPropertyWell.C

// CASA API
#include "ObsValueDoubleArray.h"
#include "ObsGridPropertyWell.h"

// CMB API
#include "cmbAPI.h"
#include "NumericFunctions.h"

// STL/C lib
#include <cassert>
#include <sstream>

namespace casa
{

// Create observable for the given grid property for specified grid position
ObsGridPropertyWell::ObsGridPropertyWell( const std::vector<double> & x, const std::vector<double> & y, const std::vector<double> & z,
                                          const char * propName, double simTime )
   : m_x( x.begin(), x.end() )
   , m_y( y.begin(), y.end() )
   , m_z( z.begin(), z.end() )
   , m_posDataMiningTbl( x.size(), -1 )
   , m_propName( propName )
   , m_simTime( simTime )
   , m_devValue( 0.0 )
   , m_saWeight( 1.0 )
   , m_uaWeight( 1.0 )
{
   assert( !m_propName.empty() );
   assert( m_x.size() == m_y.size() && m_x.size() == m_z.size() );

   // construct observable name for each trajectory point
   for ( size_t i = 0; i < m_x.size(); ++i )
   {
      std::ostringstream oss;
      oss << m_propName << "(" << m_x[i] << "," << m_y[i] << "," << m_z[i] << "," << m_simTime << ")";
      m_name.push_back( oss.str() );
   }
}

// Destructor
ObsGridPropertyWell::~ObsGridPropertyWell() {;}

// Get name of the observable
std::vector< std::string > ObsGridPropertyWell::name() const
{
   return m_name;
}
        
// Get standard deviations for the reference value
void ObsGridPropertyWell::setReferenceValue( ObsValue * obsVal, double devVal )
{
   assert( obsVal != NULL );
   assert( dynamic_cast<ObsValueDoubleArray*>( obsVal ) != NULL );
   assert( devVal >= 0.0 );

   m_refValue.reset( obsVal );
   m_devValue = devVal;
}
 
// Update Model to be sure that requested property will be saved at the requested time
ErrorHandler::ReturnCode ObsGridPropertyWell::requestObservableInModel( mbapi::Model & caldModel )
{
   if ( ErrorHandler::NoError != caldModel.snapshotManager().requestMajorSnapshot(       m_simTime  ) ||
        ErrorHandler::NoError != caldModel.propertyManager().requestPropertyInSnapshots( m_propName )
      ) return caldModel.errorCode();

   size_t tblSize = caldModel.tableSize( Observable::s_dataMinerTable ); 

   for ( size_t i = 0; i < m_x.size(); ++i )
   {
      if ( ErrorHandler::NoError != caldModel.addRowToTable( Observable::s_dataMinerTable ) ) return caldModel.errorCode();

      m_posDataMiningTbl[i] = tblSize + i;

      if ( ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl[i], "Time",         m_simTime            ) || 
           ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl[i], "XCoord",       m_x[i]               ) ||
           ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl[i], "YCoord",       m_y[i]               ) ||
           ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl[i], "ZCoord",       m_z[i]               ) ||
           ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl[i], "PropertyName", m_propName           ) ||
           ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl[i], "Value",        UndefinedDoubleValue ) 
         ) return caldModel.errorCode();
   }
   return ErrorHandler::NoError;
}

  
// Get this observable value from Cauldron model
ObsValue * ObsGridPropertyWell::getFromModel( mbapi::Model & caldModel )
{
   std::vector<double> vals( m_posDataMiningTbl.size(), UndefinedDoubleValue );
   const double eps = 1.e-5;

   for ( size_t i = 0; i < m_posDataMiningTbl.size(); ++i )
   {
      if ( m_posDataMiningTbl[i] < 0 )
      {
         size_t tblSize = caldModel.tableSize( Observable::s_dataMinerTable );
         bool found = false;
         for ( size_t j = 0; j < tblSize && !found; ++j )
         {
            double obTime = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, j, "Time" );
            if ( caldModel.errorCode() == ErrorHandler::NoError && NumericFunctions::isEqual( obTime, m_simTime, eps ) )
            {
               double xCrd = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, j, "XCoord" );
               if ( caldModel.errorCode() == ErrorHandler::NoError && NumericFunctions::isEqual( xCrd, m_x[i], eps ) )
               {
                  double yCrd = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, j, "YCoord" );
                  if ( caldModel.errorCode() == ErrorHandler::NoError && NumericFunctions::isEqual( yCrd, m_y[i], eps ) )
                  {
                     double zCrd = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, j, "ZCoord" );
                     if ( caldModel.errorCode() == ErrorHandler::NoError && NumericFunctions::isEqual( zCrd, m_z[i], eps ) )
                     {
                        const std::string & propName = caldModel.tableValueAsString( Observable::s_dataMinerTable, j, "PropertyName" );
                        if ( caldModel.errorCode() == ErrorHandler::NoError && m_propName == propName )
                        {
                           found = true;
                           vals[i] = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, j, "Value" );
                           
                           m_posDataMiningTbl[i] = j; // fill the rest of the table as well data must be continuous
                           for ( size_t k = i+1; k < m_posDataMiningTbl.size(); ++k )
                           {
                              m_posDataMiningTbl[k] = m_posDataMiningTbl[k-1]+1;
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
         vals[i] = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, m_posDataMiningTbl[i], "Value" );
      }
      if ( caldModel.errorCode() != ErrorHandler::NoError ) return NULL;
   }

   return new ObsValueDoubleArray( this, vals );
}

// Create this observable value from double array (converting data from SUMlib for response surface evaluation
ObsValue * ObsGridPropertyWell::createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const
{
   std::vector<double> obsVal( m_x.size(), UndefinedDoubleValue );

   for ( size_t i = 0; i < m_x.size(); ++i )
   {
      obsVal[i] = *val++;
   }

   return new ObsValueDoubleArray( this, obsVal );
}

bool ObsGridPropertyWell::save( CasaSerializer & sz, unsigned int version ) const
{
   // register observable with serializer to allow ObsValue objects keep reference after deserializtion
   CasaSerializer::ObjRefID obID = sz.ptr2id( this );

   bool ok = sz.save( obID, "ID" );
   ok = ok ? sz.save( m_x, "X" ) : ok;
   ok = ok ? sz.save( m_y, "Y" ) : ok;
   ok = ok ? sz.save( m_z, "Z" ) : ok;

   ok = ok ? sz.save( m_propName, "propName" ) : ok;
   ok = ok ? sz.save( m_simTime, "simTime" ) : ok;

   ok = ok ? sz.save( m_name, "name" ) : ok;

   ok = ok ? sz.save( m_posDataMiningTbl, "posDataMiningTbl" ) : ok;

   bool hasRefVal = m_refValue.get() ? true : false;
   ok = ok ? sz.save( hasRefVal, "HasRefValue" ) : ok;
   if ( hasRefVal ) { ok = ok ? sz.save( *(m_refValue.get()), "refValue" ) : ok; }

   ok = ok ? sz.save( m_devValue, "devValue" ) : ok;

   ok = ok ? sz.save( m_saWeight, "saWeight" ) : ok;
   ok = ok ? sz.save( m_uaWeight, "uaWeight" ) : ok;

   return ok;
}

ObsGridPropertyWell::ObsGridPropertyWell( CasaDeserializer & dz, unsigned int objVer )
{
   if ( version() < objVer )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) <<
         "Version of ObsGridPropertyWell in file is newer. No forward compatibility!";
   }

   CasaDeserializer::ObjRefID obID;

   // load data necessary to create an object
   bool ok = dz.load( obID, "ID" );

   // register observable with deserializer under read ID to allow ObsValue objects keep reference after deserializtion
   dz.registerObjPtrUnderID( this, obID );

   ok = ok ? dz.load( m_x, "X" ) : ok;
   ok = ok ? dz.load( m_y, "Y" ) : ok;
   ok = ok ? dz.load( m_z, "Z" ) : ok;

   ok = ok ? dz.load( m_propName, "propName" ) : ok;
   ok = ok ? dz.load( m_simTime,  "simTime"  ) : ok;


   // load the rest of the object data
   ok = ok ? dz.load( m_name,     "name"     ) : ok;
   
   if ( objVer == 0 )
   {
      std::vector<size_t> pos;
      ok = ok ? dz.load( pos, "posDataMiningTbl" ) : ok;

      m_posDataMiningTbl.clear();
      for ( size_t i = 0; i < pos.size(); ++i ) m_posDataMiningTbl.push_back( static_cast<int>( pos[i] ) );
   }
   else if ( objVer >= 1 )
   {
      ok = ok ? dz.load( m_posDataMiningTbl, "posDataMiningTbl" ) : ok;
   }

   bool hasRefVal;
   ok = ok ? dz.load( hasRefVal, "HasRefValue" ) : ok;

   if ( hasRefVal ) { m_refValue.reset( ObsValue::load( dz, "refValue" ) ); }

   ok = ok ? dz.load( m_devValue, "devValue" ) : ok;
   ok = ok ? dz.load( m_saWeight, "saWeight" ) : ok;
   ok = ok ? dz.load( m_uaWeight, "uaWeight" ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "ObsGridPropertyWell deserialization unknown error";
   }
}

}

