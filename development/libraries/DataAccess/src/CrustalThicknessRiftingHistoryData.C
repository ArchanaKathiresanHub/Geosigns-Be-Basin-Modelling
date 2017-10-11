//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Interface/CrustalThicknessRiftingHistoryData.h"

//std library
#include <exception>
#include <string>

// TableIO library
#include "cauldronschemafuncs.h"
#include "database.h"
using namespace database;

// DataAccess library
#include "Interface/GridMap.h"
#include "Interface/ProjectHandle.h"
using namespace DataAccess;
using namespace Interface;


const std::vector<std::string> CrustalThicknessRiftingHistoryData::s_MapAttributeNames = {
   "HBu", "DeltaSL"
};

CrustalThicknessRiftingHistoryData::CrustalThicknessRiftingHistoryData( ProjectHandle * projectHandle, Record * record ) : DAObject( projectHandle, record )
{
}


CrustalThicknessRiftingHistoryData::~CrustalThicknessRiftingHistoryData()
{
}

TectonicFlag CrustalThicknessRiftingHistoryData::getTectonicFlag() const{
   std::string flag = database::getTectonicFlag( m_record );
   if (flag == "Flexural Basin"){
      return FLEXURAL_BASIN;
   }
   else if (flag == "Active Rifting"){
      return ACTIVE_RIFTING;
   }
   else if (flag == "Passive Margin"){
      return PASSIVE_MARGIN;
   }
   else{
      throw std::invalid_argument( "Unknown tectonic flag " + flag + " in table [" + m_record->getTable()->name() + "]" );
   }
}

const std::string& CrustalThicknessRiftingHistoryData::getTectonicFlagName() const{
   return database::getTectonicFlag( m_record );
}

GridMap const * CrustalThicknessRiftingHistoryData::getMap( const Interface::CTCRiftingHistoryMapAttributeId attributeId ) const{
   return DAObject::getMap<const DataAccess::Interface::CTCRiftingHistoryMapAttributeId>( attributeId, s_MapAttributeNames );
}

