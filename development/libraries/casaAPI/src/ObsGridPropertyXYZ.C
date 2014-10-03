//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsGridPropertyXYZ.C

#include "ObsValueDoubleScalar.h"
#include "ObsGridPropertyXYZ.h"
#include "cmbAPI.h"

#include <cassert>
#include <sstream>

namespace casa
{
   
   static const char * s_basementTblName      = "BasementIoTbl";
   static const char * s_bottomBoundaryModel  = "BottomBoundaryModel";
   static const char * s_topCrushHeatProd     = "TopCrustHeatProd";
   static const char * s_topCrushHeatProdGrid = "TopCrustHeatProdGrid";


// Create observable for the given grid property for specified grid position
ObsGridPropertyXYZ::ObsGridPropertyXYZ( double x, double y, double z, const char * propName, double simTime )
{
   assert( propName  != NULL );
   
   m_posDataMiningTbl = 0;

   m_x = x;
   m_y = y;
   m_z = z;

   m_propName  = propName;
   m_simTime  = simTime;
   m_devValue = 0.0;

   m_saWeight = 1.0;
   m_uaWeight = 1.0;


   // construct observable name
   std::ostringstream oss;
   oss << "GridPropertyXYZ(" << m_propName << ", (" << m_x << "," << m_y << "," << m_z << ")," << m_simTime << ")";
   m_name = oss.str();
}

// Destructor
ObsGridPropertyXYZ::~ObsGridPropertyXYZ() {;}

// Get name of the observable
const char * ObsGridPropertyXYZ::name() const { return m_name.c_str(); }
        
// Get standard deviations for the reference value
void ObsGridPropertyXYZ::setReferenceValue( ObsValue * obsVal, double devVal )
{
   assert( obsVal != NULL );
   assert( dynamic_cast<ObsValueDoubleScalar*>( obsVal ) != NULL );
   assert( devVal >= 0.0 );

   m_refValue.reset( obsVal );
   m_devValue = devVal;
}
 
// Update Model to be sure that requested property will be saved at the requested time
ErrorHandler::ReturnCode ObsGridPropertyXYZ::requestObservableInModel( mbapi::Model & caldModel )
{
   if ( ErrorHandler::NoError != caldModel.snapshotManager().requestMajorSnapshot(       m_simTime  ) ) return caldModel.errorCode();
   if ( ErrorHandler::NoError != caldModel.propertyManager().requestPropertyInSnapshots( m_propName ) ) return caldModel.errorCode();

   m_posDataMiningTbl = caldModel.tableSize( Observable::s_dataMinerTable ); 
   
   if ( ErrorHandler::NoError != caldModel.addRowToTable( Observable::s_dataMinerTable ) ) return caldModel.errorCode();

   if ( ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "Time",         m_simTime  ) ) return caldModel.errorCode();
   if ( ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "XCoord",       m_x        ) ) return caldModel.errorCode();
   if ( ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "YCoord",       m_y        ) ) return caldModel.errorCode();
   if ( ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "ZCoord",       m_z        ) ) return caldModel.errorCode();
   if ( ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "PropertyName", m_propName ) ) return caldModel.errorCode();
   if ( ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, m_posDataMiningTbl, "Value",        -9999.0    ) ) return caldModel.errorCode();

   return ErrorHandler::NoError;
}

  
// Get this observable value from Cauldron model
ObsValue * ObsGridPropertyXYZ::getFromModel( mbapi::Model & caldModel )
{

   double val = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, m_posDataMiningTbl, "Value" );
   if ( caldModel.errorCode() != ErrorHandler::NoError ) return NULL;

   return new ObsValueDoubleScalar( this, val );
}

// Create this observable value from double array (converting data from SUMlib for response surface evaluation
ObsValue * ObsGridPropertyXYZ::creatNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const
{
   return new ObsValueDoubleScalar( this, *val++ );
}

}

