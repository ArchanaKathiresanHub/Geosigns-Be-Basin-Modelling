//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FaultFileReader2D.h"

using namespace DataAccess;
using namespace Interface;

//------------------------------------------------------------//

FaultFileReader2D::FaultFileReader2D () {
}

//------------------------------------------------------------//

FaultFileReader2D::~FaultFileReader2D () {
}

//------------------------------------------------------------//

FaultFileReader2D::FaultDataSetIterator FaultFileReader2D::begin () const {
  return m_faultData.begin();
}

//------------------------------------------------------------//

FaultFileReader2D::FaultDataSetIterator FaultFileReader2D::end () const {
  return m_faultData.end();
}

//------------------------------------------------------------//

void FaultFileReader2D::addFault ( const std::string&   newFaultName,
                                 const std::vector<PointSequence>& newfault ) {

  FaultDataItem newItem;
  newItem.faultName = newFaultName;
  newItem.fault.push_back( newfault[0] );

  m_faultData.push_back( newItem );
}

//------------------------------------------------------------//
