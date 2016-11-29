//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Interface/OceanicCrustThicknessHistoryData.h"

//std library
#include <exception>
#include <string>

// TableIO library
#include "cauldronschemafuncs.h"
using namespace database;

// DataAccess library
using namespace DataAccess;
using namespace Interface;


const std::vector<std::string> OceanicCrustThicknessHistoryData::s_MapAttributeNames = {
   "Thickness"
};

OceanicCrustThicknessHistoryData::OceanicCrustThicknessHistoryData( ProjectHandle * projectHandle, Record * record ) : DAObject( projectHandle, record )
{
   // Call parent constructor (comment removes SonarQube issue)
}


OceanicCrustThicknessHistoryData::~OceanicCrustThicknessHistoryData()
{
   // Empty destructor (comment removes SonarQube issue)
}

GridMap const * OceanicCrustThicknessHistoryData::getMap( const OceanicCrustThicknessMaps attributeId ) const{
   return DAObject::getMap<const OceanicCrustThicknessMaps>( attributeId, s_MapAttributeNames );
}

double OceanicCrustThicknessHistoryData::getAge () const{
   return database::getAge( m_record );
}

