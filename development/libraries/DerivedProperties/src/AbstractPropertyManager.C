#include "AbstractPropertyManager.h"

#include <algorithm>
#include <iostream>
using namespace std;

#include "PropertyErasePredicate.h"

// Surface property calcualtors with offset.
#include "FormationSurfacePropertyOffsetCalculator.h"
#include "SurfacePropertyOffsetCalculator.h"

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
         // Add calculator to the property->calculator mapping.
         m_formationPropertyCalculators.insert ( computedProperty, snapshot, calculator );

         // Add the necessary surface/formation-surface offset property calculator for the 3d property.
         if ( computedProperty->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY ) {
            SurfacePropertyCalculatorPtr surfaceCalculator;
            surfaceCalculator = DerivedProperties::SurfacePropertyCalculatorPtr ( new SurfacePropertyOffsetCalculator ( computedProperty,
                                                                                                                        calculator->getDependentPropertyNames ()));

            if ( not surfacePropertyIsComputable ( computedProperty )) {
               addSurfacePropertyCalculator ( surfaceCalculator, snapshot );
            }

         } else if ( computedProperty->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) {
            FormationSurfacePropertyCalculatorPtr surfaceCalculator;
            surfaceCalculator = DerivedProperties::FormationSurfacePropertyCalculatorPtr ( new FormationSurfacePropertyOffsetCalculator ( computedProperty,
                                                                                                                                          calculator->getDependentPropertyNames ()));

            if ( not formationSurfacePropertyIsComputable ( computedProperty )) {
               addFormationSurfacePropertyCalculator ( surfaceCalculator, snapshot );
            }
         }

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

DerivedProperties::FormationSurfacePropertyCalculatorPtr DerivedProperties::AbstractPropertyManager::getFormationSurfaceCalculator ( const DataModel::AbstractProperty* property,
                                                                                                                                     const DataModel::AbstractSnapshot* snapshot ) const {
   return m_formationSurfacePropertyCalculators.get ( property, snapshot );
}

DerivedProperties::FormationMapPropertyCalculatorPtr DerivedProperties::AbstractPropertyManager::getFormationMapCalculator ( const DataModel::AbstractProperty* property,
                                                                                                                             const DataModel::AbstractSnapshot* snapshot ) const {
   return m_formationMapPropertyCalculators.get ( property, snapshot );
}

DerivedProperties::FormationPropertyCalculatorPtr DerivedProperties::AbstractPropertyManager::getFormationCalculator ( const DataModel::AbstractProperty* property,
                                                                                                                       const DataModel::AbstractSnapshot* snapshot ) const {
   return m_formationPropertyCalculators.get ( property, snapshot );
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

bool DerivedProperties::AbstractPropertyManager::formationPropertyIsComputable ( const DataModel::AbstractProperty*  property,
                                                                                 const DataModel::AbstractSnapshot*  snapshot,
                                                                                 const DataModel::AbstractFormation* formation ) const {

   FormationPropertyCalculatorPtr calculator = getFormationCalculator ( property, 0 );
   bool isComputable;

   if ( calculator != 0 ) {
      isComputable = calculator->isComputable ( *this, snapshot, formation );
   } else {
      isComputable = false;
   }

   return isComputable;
}

bool DerivedProperties::AbstractPropertyManager::formationSurfacePropertyIsComputable ( const DataModel::AbstractProperty*  property,
                                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                                        const DataModel::AbstractFormation* formation,
                                                                                        const DataModel::AbstractSurface*   surface ) const {

   FormationSurfacePropertyCalculatorPtr calculator = getFormationSurfaceCalculator ( property, 0 );
   bool isComputable;

   if ( calculator != 0 ) {
      isComputable = calculator->isComputable ( *this, snapshot, formation, surface );
   } else {
      isComputable = false;
   }

   return isComputable;
}

bool DerivedProperties::AbstractPropertyManager::surfacePropertyIsComputable ( const DataModel::AbstractProperty* property,
                                                                               const DataModel::AbstractSnapshot* snapshot,
                                                                               const DataModel::AbstractSurface*  surface ) const {

   SurfacePropertyCalculatorPtr calculator = getSurfaceCalculator ( property, 0 );
   bool isComputable;


   if ( calculator != 0 ) {
      isComputable = calculator->isComputable ( *this, snapshot, surface );
   } else {
      isComputable = false;
   }

   return isComputable;
}

bool DerivedProperties::AbstractPropertyManager::formationMapPropertyIsComputable ( const DataModel::AbstractProperty*  property,
                                                                                    const DataModel::AbstractSnapshot*  snapshot,
                                                                                    const DataModel::AbstractFormation* formation ) const {

   FormationMapPropertyCalculatorPtr calculator = getFormationMapCalculator ( property, 0 );
   bool isComputable;

   if ( calculator != 0 ) {
      isComputable = calculator->isComputable ( *this, snapshot, formation );
   } else {
      isComputable = false;
   }

   return isComputable;
}
