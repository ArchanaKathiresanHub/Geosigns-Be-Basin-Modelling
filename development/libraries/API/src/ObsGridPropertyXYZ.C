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
// Create observable for the given grid property for specified grid position
ObsGridPropertyXYZ::ObsGridPropertyXYZ( double x, double y, double z, const char * propName, double simTime )
{
   assert( propName  != NULL );

   m_x = x;
   m_y = y;
   m_z = z;

   m_propName  = propName;
   m_simTime  = simTime;
   m_devValue = 0.0;
}

// Destructor
ObsGridPropertyXYZ::~ObsGridPropertyXYZ() {;}

// Get name of the observable
const char * ObsGridPropertyXYZ::name()
{
   if ( m_name.empty() )
   {
      std::ostringstream oss;
      oss << "GridPropertyXYZ(" << m_propName << ", (" << m_x << "," << m_y << "," << m_z << ")," << m_simTime << ")";
      m_name = oss.str();
   }
   return m_name.c_str();
}
         
// Get standard deviations for the reference value
void ObsGridPropertyXYZ::setReferenceValue( ObsValue * obsVal, double devVal )
{
   assert( obsVal != NULL );
   assert( dynamic_cast<ObsValueDoubleScalar*>( obsVal ) != NULL );
   assert( devVal >= 0.0 );

   m_refValue.reset( obsVal );
   m_devValue = devVal;
}
   
// Get this observable value from Cauldron model
ObsValue * ObsGridPropertyXYZ::getFromModel( mbapi::Model & caldModel )
{
   double val = -9999;

   //if ( ErrorHandler::NoError !=
         //caldModel.gridProperty( m_layerName, m_i, m_j, m_k, m_propName, m_simTime, val ) )
   //{
      //return NULL;
   //}
   return new ObsValueDoubleScalar( this, val );
}

}

