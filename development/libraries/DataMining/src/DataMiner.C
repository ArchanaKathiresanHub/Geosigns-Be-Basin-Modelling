#include "DataMiner.h"

#include "DataMiningProjectHandle.h"

DataAccess::Mining::DataMiner::DataMiner ( ProjectHandle& projectHandle,
                                           DerivedProperties::DerivedPropertyManager& propertyManager ) : m_propertyManager ( propertyManager ) {
   m_collection = projectHandle.getDomainPropertyCollection ();
}

DataAccess::Mining::DataMiner::~DataMiner () {
}

void DataAccess::Mining::DataMiner::setProperties ( const PropertySet& properties ) {


   PropertySet::const_iterator propIter;

   for ( propIter = properties.begin (); propIter != properties.end (); ++propIter ) {

      if ( (*propIter) != 0 ) {
         m_collection->getDomainProperty ( *propIter, m_propertyManager );
      }

   }

   m_collection->initialiseProperties ();
}

void DataAccess::Mining::DataMiner::compute ( const ElementPosition&            position,
                                              const Interface::Property*        property,
                                                    InterpolatedPropertyValues& result ) {

   DomainProperty* domainProperty;

   domainProperty = m_collection->getDomainProperty ( property, m_propertyManager );

   if ( domainProperty != 0 ) {
      result.setValue ( domainProperty->getProperty (), domainProperty->compute ( position ));
      result.setElement ( position );
   }

}

void DataAccess::Mining::DataMiner::compute ( const ElementPosition&            position,
                                              const PropertySet&                properties,
                                                    InterpolatedPropertyValues& result ) {


   PropertySet::const_iterator propIter;

   for ( propIter = properties.begin (); propIter != properties.end (); ++propIter ) {
      compute ( position, *propIter, result );
   }

}


void DataAccess::Mining::DataMiner::compute ( const ElementPositionSequence& positions,
                                              const PropertySet&        properties,
                                                    ResultValues&       results ) {

   ElementPositionSequence::const_iterator positionIter;
   InterpolatedPropertyValues positionResult;

   results.clear ();

   for ( positionIter = positions.begin (); positionIter != positions.end (); ++positionIter ) {
      positionResult.clear ();
      compute ( *positionIter, properties, positionResult );
      results.push_back ( positionResult );
   }

}


void DataAccess::Mining::DataMiner::compute ( const ElementPositionSequence& positions,
                                              const Interface::Property*     property,
                                                    ResultValues&            results ) {

   ElementPositionSequence::const_iterator positionIter;
   InterpolatedPropertyValues positionResult;

   results.clear ();

   for ( positionIter = positions.begin (); positionIter != positions.end (); ++positionIter ) {
      positionResult.clear ();
      compute ( *positionIter, property, positionResult );
      results.push_back ( positionResult );
   }

}


void DataAccess::Mining::DataMiner::compute ( const ElementPositionSequence& ,
                                                    ResultValues&             ) {

   // ElementPositionSequence::const_iterator positionIter;
   // InterpolatedPropertyValues positionResult;

   // results.clear ();

   // for ( positionIter = positions.begin (); positionIter != positions.end (); ++positionIter ) {
   //    positionResult.clear ();
   //    compute ( *positionIter, property, positionResult );
   //    results.push_back ( positionResult );
   // }

}
