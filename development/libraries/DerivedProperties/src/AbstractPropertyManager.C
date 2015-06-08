#include "AbstractPropertyManager.h"

#include <algorithm>
#include <iostream>
using namespace std;

#include "PropertyErasePredicate.h"
#include "SurfacePropertyOffsetCalculator.h"
#include "FormationSurfacePropertyOffsetCalculator.h"

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
                                                                                                                   const DataModel::AbstractSnapshot* snapshot ) {

   DerivedProperties::SurfacePropertyCalculatorPtr surfaceCalculator = m_surfacePropertyCalculators.get ( property, snapshot );

   if ( surfaceCalculator == 0 ) {
      // If there is no surface calculator for this property then allocate one that can extract the data from the formation property.
      //
      // Check to see if there is a formation calculator, from which the formation data can be or has been calculated.
      if ( containsFormationCalculator ( property, 0 )) {
         // First check for a general formation calculator, i.e. one which is not associated with any snapshot.
         surfaceCalculator = DerivedProperties::SurfacePropertyCalculatorPtr ( new SurfacePropertyOffsetCalculator ( property ));

         // Do not associate this surface calculator with any snapshot time, since it will be the same for all snapshot times.
         addSurfacePropertyCalculator ( surfaceCalculator, 0 );
      } else if ( containsFormationCalculator ( property, snapshot )) {
         // Then check for a formation calculator that is for a specific snapshot.
         surfaceCalculator = DerivedProperties::SurfacePropertyCalculatorPtr ( new SurfacePropertyOffsetCalculator ( property ));
         addSurfacePropertyCalculator ( surfaceCalculator, snapshot );
      }

   }

   return surfaceCalculator;
}

DerivedProperties::FormationSurfacePropertyCalculatorPtr DerivedProperties::AbstractPropertyManager::getFormationSurfaceCalculator ( const DataModel::AbstractProperty* property,
                                                                                                                                     const DataModel::AbstractSnapshot* snapshot ) {

   DerivedProperties::FormationSurfacePropertyCalculatorPtr formationSurfaceCalculator = m_formationSurfacePropertyCalculators.get ( property, snapshot );

   if ( formationSurfaceCalculator == 0 ) {
      // If there is no formation surface calculator for this property then allocate one that can extract the data from the formation property.
      //
      // Check to see if there is a formation calculator, from which the formation data can be or has been calculated.
      if ( containsFormationCalculator ( property, 0 )) {
         // First check for a general formation calculator, i.e. one which is not associated with any snapshot.
         formationSurfaceCalculator = DerivedProperties::FormationSurfacePropertyCalculatorPtr ( new FormationSurfacePropertyOffsetCalculator ( property ));

         // Do not associate this formation surface calculator with any snapshot time, since it will be the same for all snapshot times.
         addFormationSurfacePropertyCalculator ( formationSurfaceCalculator, 0 );
      } else if ( containsFormationCalculator ( property, snapshot )) {
         // Then check for a formation calculator that is for a specific snapshot.
         formationSurfaceCalculator = DerivedProperties::FormationSurfacePropertyCalculatorPtr ( new FormationSurfacePropertyOffsetCalculator ( property ));
         addFormationSurfacePropertyCalculator ( formationSurfaceCalculator, snapshot );
      }

   }

   return formationSurfaceCalculator;
}

DerivedProperties::FormationMapPropertyCalculatorPtr DerivedProperties::AbstractPropertyManager::getFormationMapCalculator ( const DataModel::AbstractProperty* property,
                                                                                                                             const DataModel::AbstractSnapshot* snapshot ) const {
   return m_formationMapPropertyCalculators.get ( property, snapshot );
}

DerivedProperties::FormationPropertyCalculatorPtr DerivedProperties::AbstractPropertyManager::getFormationCalculator ( const DataModel::AbstractProperty* property,
                                                                                                                       const DataModel::AbstractSnapshot* snapshot ) const {
   return m_formationPropertyCalculators.get ( property, snapshot );
}

bool DerivedProperties::AbstractPropertyManager::containsFormationCalculator ( const DataModel::AbstractProperty* property,
                                                                               const DataModel::AbstractSnapshot* snapshot ) const {
   return m_formationPropertyCalculators.contains ( property, snapshot );
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

   if ( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
        property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) {

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

   }

   return result;
}

DerivedProperties::FormationMapPropertyPtr DerivedProperties::AbstractPropertyManager::getFormationMapProperty ( const DataModel::AbstractProperty*  property,
                                                                                                                 const DataModel::AbstractSnapshot*  snapshot,
                                                                                                                 const DataModel::AbstractFormation* formation ) {
   
   FormationMapPropertyPtr result;

   if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY ) {

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

   }

   return result;
}

DerivedProperties::FormationPropertyPtr DerivedProperties::AbstractPropertyManager::getFormationProperty ( const DataModel::AbstractProperty*  property,
                                                                                                           const DataModel::AbstractSnapshot*  snapshot,
                                                                                                           const DataModel::AbstractFormation* formation ) {
   
   FormationPropertyPtr result;

   if ( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
        property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) {

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

   }

   return result;
}

DerivedProperties::FormationSurfacePropertyPtr DerivedProperties::AbstractPropertyManager::getFormationSurfaceProperty ( const DataModel::AbstractProperty*  property,
                                                                                                                         const DataModel::AbstractSnapshot*  snapshot,
                                                                                                                         const DataModel::AbstractFormation* formation,
                                                                                                                         const DataModel::AbstractSurface*   surface ) {

   FormationSurfacePropertyPtr result;

   if ( property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) {

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

   }

   return result;
}

void DerivedProperties::AbstractPropertyManager::removeProperties ( const DataModel::AbstractSnapshot* snapshot ) {

   // Remove formation properties at snapshot time.
   FormationPropertyList::iterator formationsToRemove = std::remove_if ( m_formationProperties.begin (), m_formationProperties.end (),
                                                                         PropertyErasePredicate<FormationPropertyPtr> ( snapshot ));
   m_formationProperties.erase ( formationsToRemove, m_formationProperties.end ());

   // Remove formation-surface properties at snapshot time.
   FormationSurfacePropertyList::iterator formationSurfacesToRemove = std::remove_if ( m_formationSurfaceProperties.begin (), m_formationSurfaceProperties.end (),
                                                                                       PropertyErasePredicate<FormationSurfacePropertyPtr> ( snapshot ));
   m_formationSurfaceProperties.erase ( formationSurfacesToRemove, m_formationSurfaceProperties.end ());

   // Remove formation-map properties at snapshot time.
   FormationMapPropertyList::iterator formationMapsToRemove = std::remove_if ( m_formationMapProperties.begin (), m_formationMapProperties.end (),
                                                                               PropertyErasePredicate<FormationMapPropertyPtr> ( snapshot ));
   m_formationMapProperties.erase ( formationMapsToRemove, m_formationMapProperties.end ());

   // Remove surface properties at snapshot time.
   SurfacePropertyList::iterator surfacesToRemove = std::remove_if ( m_surfaceProperties.begin (), m_surfaceProperties.end (),
                                                                     PropertyErasePredicate<SurfacePropertyPtr> ( snapshot ));
   m_surfaceProperties.erase ( surfacesToRemove, m_surfaceProperties.end ());


}
