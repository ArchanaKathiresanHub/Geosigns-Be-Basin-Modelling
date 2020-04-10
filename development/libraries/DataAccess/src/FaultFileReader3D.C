//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FaultFileReader3D.h"

using namespace DataAccess;
using namespace Interface;

FaultFileReader3D::FaultDataSetIterator FaultFileReader3D::begin () const {
  return m_faultData.begin();
}

FaultFileReader3D::FaultDataSetIterator FaultFileReader3D::end () const {
  return m_faultData.end();
}

void FaultFileReader3D::addFault ( const std::string&   newFaultName,
                                   const std::vector<PointSequence>& newfaultStick )
{
  int countFault = 0;

  for ( PointSequence pointSeq : newfaultStick )
  {
    FaultDataItem newItem;
    newItem.faultName = newFaultName + "_" + std::to_string( countFault++ );
    newItem.fault.push_back( pointSeq );
    m_faultData.push_back( newItem );
  }
}


