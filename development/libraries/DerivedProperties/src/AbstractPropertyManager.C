#include "AbstractPropertyManager.h"

DerivedProperties::AbstractPropertyManager::AbstractPropertyManager () {}

void DerivedProperties::AbstractPropertyManager::addCalculator ( const SurfacePropertyCalculatorPtr calculator ) {

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

void DerivedProperties::AbstractPropertyManager::addCalculator ( const FormationPropertyCalculatorPtr calculator ) {

   const std::vector<std::string>& propertyNames = calculator->getPropertyNames ();

   assert ( propertyNames.size () > 0 );

   for ( size_t i = 0; i < propertyNames.size (); ++i ) {
      const DataModel::AbstractProperty* computedProperty = getProperty ( propertyNames [ i ]);

      if ( computedProperty != 0 ) {

         if ( m_formationPropertyCalculators.find ( computedProperty ) == m_formationPropertyCalculators.end ()) {
            m_formationPropertyCalculators [ computedProperty ] = calculator;
         } else {
            // What to do?
         }

      } else {
         // Error
      }

   }

}

void DerivedProperties::AbstractPropertyManager::addCalculator ( const FormationSurfacePropertyCalculatorPtr calculator ) {

   const std::vector<std::string>& propertyNames = calculator->getPropertyNames ();

   assert ( propertyNames.size () > 0 );

   for ( size_t i = 0; i < propertyNames.size (); ++i ) {
      const DataModel::AbstractProperty* computedProperty = getProperty ( propertyNames [ i ]);

      if ( computedProperty != 0 ) {

         if ( m_formationSurfacePropertyCalculators.find ( computedProperty ) == m_formationSurfacePropertyCalculators.end ()) {
            m_formationSurfacePropertyCalculators [ computedProperty ] = calculator;
         } else {
            // What to do?
         }

      } else {
         // Error
      }

   }

}

DerivedProperties::SurfacePropertyCalculatorPtr DerivedProperties::AbstractPropertyManager::getCalculator ( const DataModel::AbstractProperty* property ) const {

   SurfacePropertyCalculatorMap::const_iterator surfaceMapiter = m_surfacePropertyCalculators.find ( property );

   if ( surfaceMapiter != m_surfacePropertyCalculators.end ()) {
      return surfaceMapiter->second;
   } else {
      return SurfacePropertyCalculatorPtr ();
   }

}

DerivedProperties::FormationPropertyCalculatorPtr DerivedProperties::AbstractPropertyManager::getFormationCalculator ( const DataModel::AbstractProperty* property ) const {

   FormationPropertyCalculatorMap::const_iterator formationMapiter = m_formationPropertyCalculators.find ( property );

   if ( formationMapiter != m_formationPropertyCalculators.end ()) {
      return formationMapiter->second;
   } else {
      return FormationPropertyCalculatorPtr ();
   }

}

DerivedProperties::FormationSurfacePropertyCalculatorPtr DerivedProperties::AbstractPropertyManager::getFormationSurfaceCalculator ( const DataModel::AbstractProperty* property ) const {

   FormationSurfacePropertyCalculatorMap::const_iterator formationSurfaceMapiter = m_formationSurfacePropertyCalculators.find ( property );

   if ( formationSurfaceMapiter != m_formationSurfacePropertyCalculators.end ()) {
      return formationSurfaceMapiter->second;
   } else {
      return FormationSurfacePropertyCalculatorPtr ();
   }

}

void DerivedProperties::AbstractPropertyManager::addSurfaceProperty ( const SurfacePropertyPtr surfaceProperty ) {
   m_surfaceProperties.push_back ( surfaceProperty );
}

void DerivedProperties::AbstractPropertyManager::addFormationProperty ( const FormationPropertyPtr formationProperty ) {
   m_formationProperties.push_back ( formationProperty );
}

void DerivedProperties::AbstractPropertyManager::addFormationSurfaceProperty ( const FormationSurfacePropertyPtr formationSurfaceProperty ) {
   m_formationSurfaceProperties.push_back ( formationSurfaceProperty );
}


DerivedProperties::SurfacePropertyPtr DerivedProperties::AbstractPropertyManager::findPropertyValues ( const DataModel::AbstractProperty* property,
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

DerivedProperties::FormationPropertyPtr DerivedProperties::AbstractPropertyManager::findPropertyValues ( const DataModel::AbstractProperty*  property,
                                                                                                         const DataModel::AbstractSnapshot*  snapshot,
                                                                                                         const DataModel::AbstractFormation* formation ) const {

   SurfacePropertyPtr result;

   for ( size_t i = 0; i < m_formationProperties.size (); ++i ) {

      if ( m_formationProperties [ i ]->getProperty () == property and
           m_formationProperties [ i ]->getSnapshot () == snapshot and
           m_formationProperties [ i ]->getFormation() == formation ) {
         return m_formationProperties [ i ];
      }

   }

   return FormationPropertyPtr ();
}


DerivedProperties::FormationSurfacePropertyPtr DerivedProperties::AbstractPropertyManager::findPropertyValues ( const DataModel::AbstractProperty*  property,
                                                                                                                const DataModel::AbstractSnapshot*  snapshot,
                                                                                                                const DataModel::AbstractFormation* formation,
                                                                                                                const DataModel::AbstractSurface*  surface ) const {

   SurfacePropertyPtr result;

   for ( size_t i = 0; i < m_formationSurfaceProperties.size (); ++i ) {

      if ( m_formationSurfaceProperties [ i ]->getProperty () == property and
           m_formationSurfaceProperties [ i ]->getSnapshot () == snapshot and
           m_formationSurfaceProperties [ i ]->getFormation() == formation and
           m_formationSurfaceProperties [ i ]->getSurface()   == surface ) {
         return m_formationSurfaceProperties [ i ];
      }

   }

   return FormationSurfacePropertyPtr ();
}


DerivedProperties::SurfacePropertyPtr DerivedProperties::AbstractPropertyManager::getSurfaceProperty ( const DataModel::AbstractProperty* property,
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

DerivedProperties::FormationPropertyPtr DerivedProperties::AbstractPropertyManager::getFormationProperty ( const DataModel::AbstractProperty*  property,
                                                                                                           const DataModel::AbstractSnapshot*  snapshot,
                                                                                                           const DataModel::AbstractFormation* formation ) {
   
   FormationPropertyPtr result;

   result = findPropertyValues ( property, snapshot, formation );

   if ( result == 0 ) {
      const FormationPropertyCalculatorPtr calculator = getFormationCalculator ( property );
      FormationPropertyList  calculatedProperties;
 
      if ( calculator != 0 ) {
         calculator->calculate ( *this, snapshot, formation, calculatedProperties );

         for ( size_t i = 0; i < calculatedProperties.size (); ++i ) {
            addFormationProperty ( calculatedProperties [ i ]);

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

DerivedProperties::FormationSurfacePropertyPtr DerivedProperties::AbstractPropertyManager::getFormationSurfaceProperty ( const DataModel::AbstractProperty*  property,
                                                                                                                         const DataModel::AbstractSnapshot*  snapshot,
                                                                                                                         const DataModel::AbstractFormation* formation,
                                                                                                                         const DataModel::AbstractSurface*   surface ) {
   
   FormationSurfacePropertyPtr result;

   result = findPropertyValues ( property, snapshot, formation, surface );

   if ( result == 0 ) {
      const FormationSurfacePropertyCalculatorPtr calculator = getFormationSurfaceCalculator ( property );
      FormationSurfacePropertyList  calculatedProperties;
 
      if ( calculator != 0 ) {
         calculator->calculate ( *this, snapshot, formation, surface, calculatedProperties );

         for ( size_t i = 0; i < calculatedProperties.size (); ++i ) {
            addFormationSurfaceProperty ( calculatedProperties [ i ]);

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
