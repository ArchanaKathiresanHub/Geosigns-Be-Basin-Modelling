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

#include "ObsValueDoubleArray.h"
#include "ObsGridPropertyWell.h"
#include "cmbAPI.h"

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
   , m_posDataMiningTbl( x.size(), 0 )
   , m_propName( propName )
   , m_simTime( simTime )
   , m_devValue( 0.0 )
   , m_saWeight( 1.0 )
   , m_uaWeight( 1.0 )
{
   assert( !m_propName.empty() );
   assert( m_x.size() == m_y.size() && m_x.size() == m_z.size() );

   // construct observable name
   std::ostringstream oss;
   oss << "GridPropertyWell(" << m_propName << ", " << m_simTime << ", " << "TrajPt(" << x.size() << "))";
   m_name = oss.str();
}

// Destructor
ObsGridPropertyWell::~ObsGridPropertyWell() {;}

// Get name of the observable
const char * ObsGridPropertyWell::name() const { return m_name.c_str(); }
        
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
   if ( ErrorHandler::NoError != caldModel.snapshotManager().requestMajorSnapshot(       m_simTime  ) ) return caldModel.errorCode();
   if ( ErrorHandler::NoError != caldModel.propertyManager().requestPropertyInSnapshots( m_propName ) ) return caldModel.errorCode();

   size_t tblSize = caldModel.tableSize( Observable::s_dataMinerTable ); 

   for ( size_t i = 0; i < m_x.size(); ++i )
   {
      if ( ErrorHandler::NoError != caldModel.addRowToTable( Observable::s_dataMinerTable ) ) return caldModel.errorCode();

      m_posDataMiningTbl[i] = tblSize + i;

      if ( ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl[i], "Time",         m_simTime  ) ) return caldModel.errorCode();
      if ( ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl[i], "XCoord",       m_x[i]     ) ) return caldModel.errorCode();
      if ( ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl[i], "YCoord",       m_y[i]     ) ) return caldModel.errorCode();
      if ( ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl[i], "ZCoord",       m_z[i]     ) ) return caldModel.errorCode();
      if ( ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl[i], "PropertyName", m_propName ) ) return caldModel.errorCode();
      if ( ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl[i], "Value",        -9999.0    ) ) return caldModel.errorCode();

   }
   return ErrorHandler::NoError;
}

  
// Get this observable value from Cauldron model
ObsValue * ObsGridPropertyWell::getFromModel( mbapi::Model & caldModel )
{
   std::vector<double> vals( m_posDataMiningTbl.size(), -9999.0 );

   for ( size_t i = 0; i < m_posDataMiningTbl.size(); ++i )
   {
      vals[i] = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, m_posDataMiningTbl[i], "Value" );
      if ( caldModel.errorCode() != ErrorHandler::NoError ) return NULL;
   }

   return new ObsValueDoubleArray( this, vals );
}

// Create this observable value from double array (converting data from SUMlib for response surface evaluation
ObsValue * ObsGridPropertyWell::creatNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const
{
   std::vector<double> obsVal( m_x.size(), -9999.0 );

   for ( size_t i = 0; i < m_x.size(); ++i )
   {
      obsVal[i] = *val++;
   }

   return new ObsValueDoubleArray( this, obsVal );
}

}

