//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Interface/TableCTCRiftingHistory.h"

// DataAccess library
#include "Interface/TableLoader.h"
#include "Interface/TableFunctorCompound.h"
#include "Interface/ObjectFactory.h"

using namespace DataAccess::Interface;

TableCTCRiftingHistory::TableCTCRiftingHistory( ProjectHandle& projectHandle ) : m_name( "CTCRiftingHistoryIoTbl" )
{
   TableLoader::load( projectHandle, m_name, TableFunctorCompound<CrustalThicknessRiftingHistoryData>( projectHandle, &ObjectFactory::produceCrustalThicknessRiftingHistoryData, m_data ) );
}