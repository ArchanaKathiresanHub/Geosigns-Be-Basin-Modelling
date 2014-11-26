#include "AbstractPropertyManager.h"

#include <algorithm>
#include <iostream>
using namespace std;

#include "FormationPropertyErasePredicate.h"

DerivedProperties::AbstractPropertyManager::AbstractPropertyManager () {}

DerivedProperties::AbstractPropertyManager::~AbstractPropertyManager () {
}


void DerivedProperties::AbstractPropertyManager::addSurfacePropertyCalculator ( const SurfacePropertyCalculatorPtr& calculator,
                                                                                const DataModel::AbstractSnapshot*  snapshot ) {

   const std::vector<std::string>& propertyNames = calculator->getPropertyNames ();

   assert ( propertyNames.size () > 0 );

   for ( size_t i = 0; i < propertyNames.size (); ++i ) {
      const DataModel::AbstractProperty* computedProperty = getProperty ( propertyNames [ i ]);

      if ( computedProperty != 0 ) {
         m_surfacePropertyCalculators.insert ( computedProperty, snapshot, calculator );
      } else {
         // Error
      }

   }

}

void DerivedProperties::AbstractPropertyManager::addFormationMapPropertyCalculator ( const FormationMapPropertyCalculatorPtr& calculator,
                                                                                     const DataModel::AbstractSnapshot*       snapshot ) {

   const std::vector<std::string>& propertyNames = calculator->getPropertyNames ();

   assert ( propertyNames.size () > 0 );

   for ( size_t i = 0; i < propertyNames.size (); ++i ) {
      const DataModel::AbstractProperty* computedProperty = getProperty ( propertyNames [ i ]);

      if ( computedProperty != 0 ) {
         m_formationMapPropertyCalculators.insert ( computedProperty, snapshot, calculator );
      } else {
         // Error
      }

   }

}

void DerivedProperties::AbstractPropertyManager::addFormationPropertyCalculator ( const FormationPropertyCalculatorPtr& calculator,
                                                                                  const DataModel::AbstractSnapshot*    snapshot ) {

   const std::vector<std::string>& propertyNames = calculator->getPropertyNames ();

   assert ( propertyNames.size () > 0 );

   for ( size_t i = 0; i < propertyNames.size (); ++i ) {
      const DataModel::AbstractProperty* computedProperty = getProperty ( propertyNames [ i ]);

      if ( computedProperty != 0 ) {
         m_formationPropertyCalculators.insert ( computedProperty, snapshot, calculator );
      } else {
         // Error
      }

   }

}

void DerivedProperties::AbstractPropertyManager::addFormationSurfacePropertyCalculator ( const FormationSurfacePropertyCalculatorPtr& calculator,
                                                                                         const DataModel::AbstractSnapshot*           snapshot ) {

   const std::vector<std::string>& propertyNames = calculator->getPropertyNames ();

   assert ( propertyNames.size () > 0 );

   for ( size_t i = 0; i < propertyNames.size (); ++i ) {
      const DataModel::AbstractProperty* computedProperty = getProperty ( propertyNames [ i ]);

      if ( computedProperty != 0 ) {
         m_formationSurfacePropertyCalculators.insert ( computedProperty, snapshot, calculator );
      } else {
         // Error
      }

   }

}

DerivedProperties::SurfacePropertyCalculatorPtr DerivedProperties::AbstractPropertyManager::getSurfaceCalculator ( const DataModel::AbstractProperty* property,
                                                                                                                   const DataModel::AbstractSnapshot* snapshot ) const {
   return m_surfacePropertyCalculators.get ( property, snapshot );
}

DerivedProperties::FormationMapPropertyCalculatorPtr DerivedProperties::AbstractPropertyManager::getFormationMapCalculator ( const DataModel::AbstractProperty* property,
                                                                                                                             const DataModel::AbstractSnapshot* snapshot ) const {
   return m_formationMapPropertyCalculators.get ( property, snapshot );
}

DerivedProperties::FormationPropertyCalculatorPtr DerivedProperties::AbstractPropertyManager::getFormationCalculator ( const DataModel::AbstractProperty* property,
                                                                                                                       const DataModel::AbstractSnapshot* snapshot ) const {
   return m_formationPropertyCalculators.get ( property, snapshot );
}

DerivedProperties::FormationSurfacePropertyCalculatorPtr DerivedProperties::AbstractPropertyManager::getFormationSurfaceCalculator ( const DataModel::AbstractProperty* property,
                                                                                                                                     const DataModel::AbstractSnapshot* snapshot ) const {

   return m_formationSurfacePropertyCalculators.get ( property, snapshot );
}

void DerivedProperties::AbstractPropertyManager::addSurfaceProperty ( const SurfacePropertyPtr& surfaceProperty ) {
   m_surfaceProperties.push_back ( surfaceProperty );
}

void DerivedProperties::AbstractPropertyManager::addFormationMapProperty ( const FormationMapPropertyPtr& formationMapProperty ) {
   m_formationMapProperties.push_back ( formationMapProperty );
}

void DerivedProperties::AbstractPropertyManager::addFormationProperty ( const FormationPropertyPtr& formationProperty ) {
   m_formationProperties.push_back ( formationProperty );
}

void DerivedProperties::AbstractPropertyManager::addFormationSurfaceProperty ( const FormationSurfacePropertyPtr& formationSurfaceProperty ) {
   m_formationSurfaceProperties.push_back ( formationSurfaceProperty );
}


DerivedProperties::SurfacePropertyPtr DerivedProperties::AbstractPropertyManager::findSurfacePropertyValues ( const DataModel::AbstractProperty* property,
                                                                                                              const DataModel::AbstractSnapshot* snapshot,
                                                                                                              const DataModel::AbstractSurface*  surface ) const {

   for ( size_t i = 0; i < m_surfaceProperties.size (); ++i ) {

      if ( m_surfaceProperties [ i ]->getProperty () == property and
           m_surfaceProperties [ i ]->getSnapshot () == snapshot and
           m_surfaceProperties [ i ]->getSurface  () == surface ) {
         return m_surfaceProperties [ i ];
      }

   }

   return SurfacePropertyPtr ();
}

DerivedProperties::FormationMapPropertyPtr DerivedProperties::AbstractPropertyManager::findFormationMapPropertyValues ( const DataModel::AbstractProperty*  property,
                                                                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                                                                        const DataModel::AbstractFormation* formation ) const {

   for ( size_t i = 0; i < m_formationMapProperties.size (); ++i ) {

      if ( m_formationMapProperties [ i ]->getProperty () == property and
           m_formationMapProperties [ i ]->getSnapshot () == snapshot and
           m_formationMapProperties [ i ]->getFormation() == formation ) {
         return m_formationMapProperties [ i ];
      }

   }

   return FormationMapPropertyPtr ();
}


DerivedProperties::FormationPropertyPtr DerivedProperties::AbstractPropertyManager::findFormationPropertyValues ( const DataModel::AbstractProperty*  property,
                                                                                                                  const DataModel::AbstractSnapshot*  snapshot,
                                                                                                                  const DataModel::AbstractFormation* formation ) const {

   for ( size_t i = 0; i < m_formationProperties.size (); ++i ) {

      if ( m_formationProperties [ i ]->getProperty () == property and
           m_formationProperties [ i ]->getSnapshot () == snapshot and
           m_formationProperties [ i ]->getFormation() == formation ) {
         return m_formationProperties [ i ];
      }

   }

   return FormationPropertyPtr ();
}


DerivedProperties::FormationSurfacePropertyPtr DerivedProperties::AbstractPropertyManager::findFormationSurfacePropertyValues ( const DataModel::AbstractProperty*  property,
                                                                                                                                const DataModel::AbstractSnapshot*  snapshot,
                                                                                                                                const DataModel::AbstractFormation* formation,
                                                                                                                                const DataModel::AbstractSurface*   surface ) const {

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

   result = findSurfacePropertyValues ( property, snapshot, surface );

   if ( result == 0 ) {
      const SurfacePropertyCalculatorPtr calculator = getSurfaceCalculator ( property, snapshot );
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

DerivedProperties::FormationMapPropertyPtr DerivedProperties::AbstractPropertyManager::getFormationMapProperty ( const DataModel::AbstractProperty*  property,
                                                                                                                 const DataModel::AbstractSnapshot*  snapshot,
                                                                                                                 const DataModel::AbstractFormation* formation ) {
   
   FormationMapPropertyPtr result;

   result = findFormationMapPropertyValues ( property, snapshot, formation );

   if ( result == 0 ) {
      const FormationMapPropertyCalculatorPtr calculator = getFormationMapCalculator ( property, snapshot );
      FormationMapPropertyList  calculatedProperties;
 
      if ( calculator != 0 ) {
         calculator->calculate ( *this, snapshot, formation, calculatedProperties );

         for ( size_t i = 0; i < calculatedProperties.size (); ++i ) {
            addFormationMapProperty ( calculatedProperties [ i ]);

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

   result = findFormationPropertyValues ( property, snapshot, formation );

   if ( result == 0 ) {
      const FormationPropertyCalculatorPtr calculator = getFormationCalculator ( property, snapshot );
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

   result = findFormationSurfacePropertyValues ( property, snapshot, formation, surface );

   if ( result == 0 ) {
      const FormationSurfacePropertyCalculatorPtr calculator = getFormationSurfaceCalculator ( property, snapshot );
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

void DerivedProperties::AbstractPropertyManager::removeProperties ( const DataModel::AbstractSnapshot* snapshot ) {
   FormationPropertyList::iterator toRemove = std::remove_if ( m_formationProperties.begin (), m_formationProperties.end (),
                                                               FormationPropertyErasePredicate ( snapshot ));
   m_formationProperties.erase ( toRemove, m_formationProperties.end ());
}
