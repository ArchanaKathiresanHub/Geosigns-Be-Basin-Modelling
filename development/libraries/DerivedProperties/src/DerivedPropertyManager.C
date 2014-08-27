#include "DerivedPropertyManager.h"

DerivedProperties::DerivedPropertyManager::DerivedPropertyManager () {}

void DerivedProperties::DerivedPropertyManager::addCalculator ( const SurfacePropertyCalculatorPtr calculator ) {

   const std::vector<std::string>& propertyNames = calculator->getPropertyNames ();

   assert ( propertyNames.size () > 0 );

   for ( size_t i = 0; i < propertyNames.size (); ++i ) {
      const DataModel::AbstractProperty* computedProperty = getProperty ( propertyNames [ i ]);

      if ( computedProperty != 0 ) {

         if ( m_surfacePropertyCalculators.find ( computedProperty ) == m_surfacePropertyCalculators.end ()) {
            m_surfacePropertyCalculators [ computedProperty ] = calculator;
         } else {
            // What to do?
         }

      } else {
         // Error
      }

   }

}

void DerivedProperties::DerivedPropertyManager::addProperty ( const DataModel::AbstractProperty* property ) {
   
   if ( std::find ( m_properties.begin (), m_properties.end (), property ) == m_properties.end ()) {
      m_properties.push_back ( property );
   }

}


const DataModel::AbstractProperty* DerivedProperties::DerivedPropertyManager::getProperty ( const std::string& name ) const {

   for ( size_t i = 0; i < m_properties.size (); ++i ) {

      if ( m_properties [ i ]->getName () == name ) {
         return m_properties [ i ];
      }

   }

   return 0;
}

DerivedProperties::SurfacePropertyCalculatorPtr DerivedProperties::DerivedPropertyManager::getCalculator ( const DataModel::AbstractProperty* property ) const {

   SurfacePropertyCalculatorMap::const_iterator surfaceMapiter = m_surfacePropertyCalculators.find ( property );

   if ( surfaceMapiter != m_surfacePropertyCalculators.end ()) {
      return surfaceMapiter->second;
   } else {
      return SurfacePropertyCalculatorPtr ();
   }

}

void DerivedProperties::DerivedPropertyManager::addSurfaceProperty ( const SurfacePropertyPtr surfaceProperty ) {
   m_surfaceProperties.push_back ( surfaceProperty );
}


DerivedProperties::SurfacePropertyPtr DerivedProperties::DerivedPropertyManager::findPropertyValues ( const DataModel::AbstractProperty* property,
                                                                                                      const DataModel::AbstractSnapshot* snapshot,
                                                                                                      const DataModel::AbstractSurface*  surface ) const {

   SurfacePropertyPtr result;

   for ( size_t i = 0; i < m_surfaceProperties.size (); ++i ) {

      if ( m_surfaceProperties [ i ]->getProperty () == property and
           m_surfaceProperties [ i ]->getSnapshot () == snapshot and
           m_surfaceProperties [ i ]->getSurface  () == surface ) {
         return m_surfaceProperties [ i ];
      }

   }

   return SurfacePropertyPtr ();
}


DerivedProperties::SurfacePropertyPtr DerivedProperties::DerivedPropertyManager::getSurfaceProperty ( const DataModel::AbstractProperty* property,
                                                                                                      const DataModel::AbstractSnapshot* snapshot,
                                                                                                      const DataModel::AbstractSurface*  surface ) {

   SurfacePropertyPtr result;

   result = findPropertyValues ( property, snapshot, surface );

   if ( result == 0 ) {
      const SurfacePropertyCalculatorPtr calculator = getCalculator ( property );
      SurfacePropertyList  calculatedProperties;

      if ( calculator != 0 ) {
         calculator->calculate ( *this, snapshot, surface, calculatedProperties );

         for ( size_t i = 0; i < calculatedProperties.size (); ++i ) {
            addSurfaceProperty ( calculatedProperties [ i ]);

            if ( calculatedProperties [ i ]->getProperty () == property ) {
               result = calculatedProperties [ i ];
            }

         }

         if ( result == 0 ) {
            // Error.
         }

      } else {
         // Error.
      }

   }

   return result;
}
