//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Interface/CrustalThicknessData.h"

// TableIO library
#include "cauldronschemafuncs.h"
using namespace database;

using namespace DataAccess;
using namespace Interface;

const std::vector<std::string> CrustalThicknessData::s_MapAttributeNames =
{
   "HCuIni", "HLMuIni"
};

CrustalThicknessData::CrustalThicknessData (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{
}


CrustalThicknessData::~CrustalThicknessData ()
{
}

double CrustalThicknessData::getHCuIni() const
{
   return database::getHCuIni (m_record);
}

double CrustalThicknessData::getHLMuIni() const
{
   return database::getHLMuIni (m_record);
}

int CrustalThicknessData::getFilterHalfWidth() const
{
   return database::getFilterHalfWidth (m_record);
}

double CrustalThicknessData::getUpperLowerContinentalCrustRatio() const
{
   return database::getUpperLowerContinentalCrustRatio( m_record );
}

double CrustalThicknessData::getUpperLowerOceanicCrustRatio() const
{
   return database::getUpperLowerOceanicCrustRatio( m_record );
}

GridMap const * CrustalThicknessData::getMap( const DataAccess::Interface::CTCMapAttributeId attributeId ) const{
   return DAObject::getMap<const DataAccess::Interface::CTCMapAttributeId>( attributeId , s_MapAttributeNames );
}
