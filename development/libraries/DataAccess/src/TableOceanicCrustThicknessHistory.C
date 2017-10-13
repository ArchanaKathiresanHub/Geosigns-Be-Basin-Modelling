//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Interface/TableOceanicCrustThicknessHistory.h"

// DataAccess library
#include "Interface/TableLoader.h"
#include "Interface/TableFunctorCompound.h"
#include "Interface/ObjectFactory.h"

using namespace DataAccess::Interface;

TableOceanicCrustThicknessHistory::TableOceanicCrustThicknessHistory( ProjectHandle& projectHandle ) : m_name( "OceaCrustalThicknessIoTbl" )
{
   TableLoader::load( projectHandle, m_name, TableFunctorCompound<OceanicCrustThicknessHistoryData>( projectHandle, &ObjectFactory::produceOceanicCrustThicknessHistoryData, m_data ) );
}