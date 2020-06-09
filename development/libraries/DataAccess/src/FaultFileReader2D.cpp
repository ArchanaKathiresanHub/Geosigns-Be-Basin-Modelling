//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FaultFileReader2D.h"

#include "auxiliaryfaulttypes.h"
#include "ObjectFactory.h"
#include "ProjectHandle.h"
#include "FaultCollection.h"

using namespace DataAccess;
using namespace Interface;

//------------------------------------------------------------//

FaultFileReader2D::FaultFileReader2D () {
}

//------------------------------------------------------------//

FaultFileReader2D::~FaultFileReader2D () {
}


MutableFaultCollectionList FaultFileReader2D::parseFaults(ProjectHandle* projectHandle, const std::string& mapName ) const
{
  FaultCollection* faultCollection = projectHandle->getFactory()->produceFaultCollection( *projectHandle, mapName );

  FaultDataSetIterator faultIter;
  for ( faultIter = m_faultData.begin(); faultIter != m_faultData.end(); ++faultIter )
  {
    faultCollection->addFault( faultIter->faultName, faultIter->fault );
  }

  return {faultCollection};
}

//------------------------------------------------------------//

void FaultFileReader2D::addFault ( const std::string&   newFaultName,
                                   const PointSequence& newFault ) {

  FaultDataItem newItem;
  newItem.faultName = newFaultName;
  newItem.fault = newFault;

  m_faultData.push_back( newItem );
}

//------------------------------------------------------------//
