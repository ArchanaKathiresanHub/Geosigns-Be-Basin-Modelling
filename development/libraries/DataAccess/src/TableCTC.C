//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Interface/TableCTC.h"

// DataAccess library
#include "Interface/TableLoader.h"
#include "Interface/TableFunctorSimple.h"
#include "Interface/ObjectFactory.h"

using namespace DataAccess::Interface;

TableCTC::TableCTC( ProjectHandle& projectHandle ) : m_name( "CTCIoTbl" )
{
   TableLoader::load( projectHandle, m_name, TableFunctorSimple<CrustalThicknessData>( projectHandle, &ObjectFactory::produceCrustalThicknessData, m_data ) );
}