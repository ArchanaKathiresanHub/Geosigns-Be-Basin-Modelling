//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PropertyValue.h"

Ctc::PropertyValue::PropertyValue( Interface::ProjectHandle * projectHandle,
   database::Record * record,
   const string & name,
   const Interface::Property * property,
   const Interface::Snapshot * snapshot,
   const Interface::Reservoir * reservoir,
   const Interface::Formation * formation,
   const Interface::Surface * surface,
   const Interface::PropertyStorage storage ) :
   Interface::PropertyValue( projectHandle, record, name, property, snapshot, reservoir, formation, surface, storage ) {
   m_allowOutput = true;
}

bool Ctc::PropertyValue::toBeSaved() const {
   return m_allowOutput;
}


void Ctc::PropertyValue::allowOutput( const bool output ) {
   m_allowOutput = output;
}
