#include "PropertyRetriever.h"

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
}
