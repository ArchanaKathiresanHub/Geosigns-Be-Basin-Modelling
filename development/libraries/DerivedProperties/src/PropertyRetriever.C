#include "PropertyRetriever.h"

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
