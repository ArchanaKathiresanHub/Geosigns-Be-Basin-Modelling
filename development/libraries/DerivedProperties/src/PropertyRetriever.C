//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PropertyRetriever.h"

using namespace AbstractDerivedProperties;

DerivedProperties::PropertyRetriever::PropertyRetriever () : m_restoreOnExit ( false ) {}

DerivedProperties::PropertyRetriever::PropertyRetriever ( const AbstractPropertyValuesPtr& propertyValues ) :
   m_propertyValues ( propertyValues ),
   m_restoreOnExit ( propertyValues != 0 and not propertyValues->isRetrieved ())
{

   if ( m_restoreOnExit ) {
      m_propertyValues->retrieveData ();
   }

}

DerivedProperties::PropertyRetriever::~PropertyRetriever () {

   if ( m_restoreOnExit ) {
      m_propertyValues->restoreData ();
   }

}

void DerivedProperties::PropertyRetriever::reset ( const AbstractPropertyValuesPtr& propertyValues ) {

   if ( m_propertyValues != 0 and m_restoreOnExit ) {
      m_propertyValues->restoreData ();
   }

   m_propertyValues = propertyValues;
   m_restoreOnExit =  ( propertyValues != 0 and not propertyValues->isRetrieved ());

   if ( m_restoreOnExit ) {
      m_propertyValues->retrieveData ();
   }
}
