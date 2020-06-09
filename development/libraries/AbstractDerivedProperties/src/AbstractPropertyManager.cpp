//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "AbstractPropertyManager.h"

#include <algorithm>
#include <cassert>
#include <iostream>
using namespace std;

#include "PropertyErasePredicate.h"

// Surface property calcualtors with offset.
#include "FormationSurfacePropertyOffsetCalculator.h"
#include "SurfacePropertyOffsetCalculator.h"

// utility library
#include "LogHandler.h"

void AbstractDerivedProperties::AbstractPropertyManager::addSurfacePropertyCalculator ( const SurfacePropertyCalculatorPtr& calculator ) {

   const std::vector<std::string>& propertyNames = calculator->getPropertyNames ();

   assert ( propertyNames.size () > 0 );
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Adding surface derived property calculator for:";
   for ( size_t i = 0; i < propertyNames.size (); ++i ) {
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #" << propertyNames[i];
      const DataModel::AbstractProperty* computedProperty = getProperty ( propertyNames [ i ]);

      if ( computedProperty != 0 ) {
         m_surfacePropertyCalculators.insert ( computedProperty, 0, calculator );
      } else {
         throw AbstractPropertyException() << "Could not find the surface property '" << propertyNames[i] << "' to add to the surface property calculator.";
      }

   }

}

void AbstractDerivedProperties::AbstractPropertyManager::addFormationMapPropertyCalculator ( const FormationMapPropertyCalculatorPtr& calculator ) {

   const std::vector<std::string>& propertyNames = calculator->getPropertyNames ();

   assert ( propertyNames.size () > 0 );
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Adding formation map derived property calculator for:";
   for ( size_t i = 0; i < propertyNames.size (); ++i ) {
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #" << propertyNames[i];
      const DataModel::AbstractProperty* computedProperty = getProperty ( propertyNames [ i ]);

      if ( computedProperty != 0 ) {
         m_formationMapPropertyCalculators.insert ( computedProperty, 0, calculator );
      } else {
         throw AbstractPropertyException() << "Could not find the formation map property '" << propertyNames[i] << "' to add to the formation map property calculator.";
      }

   }

}

void AbstractDerivedProperties::AbstractPropertyManager::addFormationPropertyCalculator ( const FormationPropertyCalculatorPtr& calculator,
                                                                                          const bool                            debug ) {

   const std::vector<std::string>& propertyNames = calculator->getPropertyNames ();

   assert ( propertyNames.size () > 0 );

   if ( debug ) {
      LogHandler( LogHandler::INFO_SEVERITY ) << " Adding formation derived property calculator for: ";
   }

   for ( size_t i = 0; i < propertyNames.size (); ++i ) {
      const DataModel::AbstractProperty* computedProperty = getProperty ( propertyNames [ i ]);

      if ( computedProperty != 0 ) {
         // Add calculator to the property->calculator mapping.
         m_formationPropertyCalculators.insert ( computedProperty, 0, calculator );
         std::string pptyLogString;
         if ( debug ) {

            for ( size_t j = 0; j < propertyNames.size (); ++j ) {
               pptyLogString = "   #" + propertyNames[j];
            }

         }

         // Add the necessary surface/formation-surface offset property calculator for the 3d property.
         if ( computedProperty->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY ) {
            SurfacePropertyCalculatorPtr surfaceCalculator;
            surfaceCalculator = AbstractDerivedProperties::SurfacePropertyCalculatorPtr ( new SurfacePropertyOffsetCalculator ( computedProperty,
                                                                                                                        calculator->getDependentPropertyNames ()));

            if ( not surfacePropertyIsComputable ( computedProperty )) {

               if ( debug ) {
                  pptyLogString+= " (+surface)";
               }

               addSurfacePropertyCalculator ( surfaceCalculator );
            }

         } else if ( computedProperty->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) {
            FormationSurfacePropertyCalculatorPtr surfaceCalculator;
            surfaceCalculator = AbstractDerivedProperties::FormationSurfacePropertyCalculatorPtr ( new FormationSurfacePropertyOffsetCalculator ( computedProperty,
                                                                                                                                          calculator->getDependentPropertyNames ()));

            if ( not formationSurfacePropertyIsComputable ( computedProperty )) {

               if ( debug ) {
                  pptyLogString += " (+formation-surface)";
               }

               addFormationSurfacePropertyCalculator ( surfaceCalculator );
            }
         }
         if (debug) {
            LogHandler( LogHandler::INFO_SEVERITY ) << pptyLogString;
         }
      } else {
         throw AbstractPropertyException() << "Could not find the formation property '" << propertyNames[i] << "' to add to the formation property calculator.";
      }

   }

}

void AbstractDerivedProperties::AbstractPropertyManager::addFormationSurfacePropertyCalculator ( const FormationSurfacePropertyCalculatorPtr& calculator ) {

   const std::vector<std::string>& propertyNames = calculator->getPropertyNames ();

   assert ( propertyNames.size () > 0 );

   for ( size_t i = 0; i < propertyNames.size (); ++i ) {
      const DataModel::AbstractProperty* computedProperty = getProperty ( propertyNames [ i ]);

      if ( computedProperty != 0 ) {
         m_formationSurfacePropertyCalculators.insert ( computedProperty, 0, calculator );
      } else {
         throw AbstractPropertyException() << "Could not find the formation surface property '" << propertyNames[i] << "' to add to the formation surface property calculator.";
      }

   }

}

void AbstractDerivedProperties::AbstractPropertyManager::addReservoirPropertyCalculator ( const ReservoirPropertyCalculatorPtr& calculator ) {

   const std::vector<std::string>& propertyNames = calculator->getPropertyNames ();

   assert ( propertyNames.size () > 0 );

   for ( size_t i = 0; i < propertyNames.size (); ++i ) {
      const DataModel::AbstractProperty* computedProperty = getProperty ( propertyNames [ i ]);

      if ( computedProperty != 0 ) {
         m_reservoirPropertyCalculators.insert ( computedProperty, 0, calculator );
      } else {
         throw AbstractPropertyException() << "Could not find the reservoir property '" << propertyNames[i] << "' to add to the reservoir property calculator.";
      }

   }

}

AbstractDerivedProperties::SurfacePropertyCalculatorPtr AbstractDerivedProperties::AbstractPropertyManager::getSurfaceCalculator ( const DataModel::AbstractProperty* property ) const {
   return m_surfacePropertyCalculators.get ( property, 0 );
}

AbstractDerivedProperties::FormationSurfacePropertyCalculatorPtr AbstractDerivedProperties::AbstractPropertyManager::getFormationSurfaceCalculator ( const DataModel::AbstractProperty* property ) const {
   return m_formationSurfacePropertyCalculators.get ( property, 0 );
}

AbstractDerivedProperties::FormationMapPropertyCalculatorPtr AbstractDerivedProperties::AbstractPropertyManager::getFormationMapCalculator ( const DataModel::AbstractProperty* property ) const {

  return m_formationMapPropertyCalculators.get ( property, 0 );
}

AbstractDerivedProperties::FormationPropertyCalculatorPtr AbstractDerivedProperties::AbstractPropertyManager::getFormationCalculator ( const DataModel::AbstractProperty* property ) const {
   return m_formationPropertyCalculators.get ( property, 0 );
}


AbstractDerivedProperties::ReservoirPropertyCalculatorPtr AbstractDerivedProperties::AbstractPropertyManager::getReservoirCalculator ( const DataModel::AbstractProperty* property ) const {
   return m_reservoirPropertyCalculators.get ( property, 0 );
}


void AbstractDerivedProperties::AbstractPropertyManager::addSurfaceProperty ( const SurfacePropertyPtr& surfaceProperty ) {
   m_surfaceProperties.push_back ( surfaceProperty );
}

void AbstractDerivedProperties::AbstractPropertyManager::addFormationMapProperty ( const FormationMapPropertyPtr& formationMapProperty ) {
   m_formationMapProperties.push_back ( formationMapProperty );
}

void AbstractDerivedProperties::AbstractPropertyManager::addFormationProperty ( const FormationPropertyPtr& formationProperty ) {
   m_formationProperties.push_back ( formationProperty );
}

void AbstractDerivedProperties::AbstractPropertyManager::addFormationSurfaceProperty ( const FormationSurfacePropertyPtr& formationSurfaceProperty ) {
   m_formationSurfaceProperties.push_back ( formationSurfaceProperty );
}

void AbstractDerivedProperties::AbstractPropertyManager::addReservoirProperty ( const ReservoirPropertyPtr& reservoirProperty ) {
   m_reservoirProperties.push_back ( reservoirProperty );
}


AbstractDerivedProperties::SurfacePropertyPtr AbstractDerivedProperties::AbstractPropertyManager::findSurfacePropertyValues ( const DataModel::AbstractProperty* property,
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

AbstractDerivedProperties::FormationMapPropertyPtr AbstractDerivedProperties::AbstractPropertyManager::findFormationMapPropertyValues ( const DataModel::AbstractProperty*  property,
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


AbstractDerivedProperties::FormationPropertyPtr AbstractDerivedProperties::AbstractPropertyManager::findFormationPropertyValues ( const DataModel::AbstractProperty*  property,
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


AbstractDerivedProperties::FormationSurfacePropertyPtr AbstractDerivedProperties::AbstractPropertyManager::findFormationSurfacePropertyValues ( const DataModel::AbstractProperty*  property,
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

AbstractDerivedProperties::ReservoirPropertyPtr AbstractDerivedProperties::AbstractPropertyManager::findReservoirPropertyValues ( const DataModel::AbstractProperty*  property,
                                                                                                                  const DataModel::AbstractSnapshot*  snapshot,
                                                                                                                  const DataModel::AbstractReservoir* reservoir ) const {

   for ( size_t i = 0; i < m_reservoirProperties.size (); ++i ) {

      if ( m_reservoirProperties [ i ]->getProperty  () == property  and
           m_reservoirProperties [ i ]->getSnapshot  () == snapshot  and
           m_reservoirProperties [ i ]->getReservoir () == reservoir ) {
         return m_reservoirProperties [ i ];
      }

   }

   return ReservoirPropertyPtr ();
}


AbstractDerivedProperties::SurfacePropertyPtr AbstractDerivedProperties::AbstractPropertyManager::getSurfaceProperty ( const DataModel::AbstractProperty* property,
                                                                                                       const DataModel::AbstractSnapshot* snapshot,
                                                                                                       const DataModel::AbstractSurface*  surface ) {

   SurfacePropertyPtr result;

   if ( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
        property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) {

      result = findSurfacePropertyValues ( property, snapshot, surface );

      if ( result == 0 ) {
         const SurfacePropertyCalculatorPtr calculator = getSurfaceCalculator ( property );
         SurfacePropertyList  calculatedProperties;

         if ( calculator != 0 ) {
            calculator->calculate ( *this, snapshot, surface, calculatedProperties );

            for ( SurfacePropertyPtr calculatedProperty : calculatedProperties )
            {
               addSurfaceProperty ( calculatedProperty );

               if ( calculatedProperty and calculatedProperty->getProperty() == property ) {
                     result = calculatedProperty;
               }

            }

         } else {
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "Derived property " << property->getName() << " @ snapshot " << snapshot->getTime() << " already computed.";
         }

      }

   }
   else{
      throw AbstractPropertyException() << "Could not compute surface derived property " << property->getName() << " @ snapshot " << snapshot->getTime() << ":"
         << " this property is neither a 2D surface property or a 3D continuous property.";
   }

   return result;
}

AbstractDerivedProperties::FormationMapPropertyPtr AbstractDerivedProperties::AbstractPropertyManager::getFormationMapProperty ( const DataModel::AbstractProperty*  property,
                                                                                                                 const DataModel::AbstractSnapshot*  snapshot,
                                                                                                                 const DataModel::AbstractFormation* formation ) {

   FormationMapPropertyPtr result;

   if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY ) {

      result = findFormationMapPropertyValues ( property, snapshot, formation );

      if ( result == 0 ) {
         const FormationMapPropertyCalculatorPtr calculator = getFormationMapCalculator ( property );
         FormationMapPropertyList  calculatedProperties;

         if ( calculator != 0 ) {
            calculator->calculate ( *this, snapshot, formation, calculatedProperties );

            for ( FormationMapPropertyPtr calculatedProperty : calculatedProperties )
            {
               addFormationMapProperty ( calculatedProperty );

               if ( calculatedProperty->getProperty () == property ) {
                  result = calculatedProperty;
               }

            }

         } else {
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "Derived property " << property->getName() << " @ snapshot " << snapshot->getTime() << " already computed.";
         }

      }

   }

   else{
      throw AbstractPropertyException() << "Could not compute formation map derived property " << property->getName() << " @ snapshot " << snapshot->getTime() << ":"
         << " this property is not a 2D formation property.";
   }

   return result;
}

AbstractDerivedProperties::FormationPropertyPtr AbstractDerivedProperties::AbstractPropertyManager::getFormationProperty ( const DataModel::AbstractProperty*  property,
                                                                                                           const DataModel::AbstractSnapshot*  snapshot,
                                                                                                           const DataModel::AbstractFormation* formation ) {

   FormationPropertyPtr result;

   if ( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
        property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) {

      result = findFormationPropertyValues ( property, snapshot, formation );

      if ( result == 0 ) {
         const FormationPropertyCalculatorPtr calculator = getFormationCalculator ( property );
         FormationPropertyList  calculatedProperties;

         if ( calculator )
         {
            calculator->calculate ( *this, snapshot, formation, calculatedProperties );

            for ( FormationPropertyPtr calculatedProperty : calculatedProperties )
            {
               addFormationProperty ( calculatedProperty );

               if ( calculatedProperty->getProperty () == property ) {
                  result = calculatedProperty;
               }
            }

         } else {
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "Derived property " << property->getName()
               << " @ snapshot " << snapshot->getTime() << "Ma for formation " << formation->getName() << " already computed.";
         }

      }

   }

   else{
      // This error is commented because of a bug in cauldron2voxet
      // Should be reactivated after correction of BUG 56710
      //throw AbstractPropertyException() << "Could not compute formation derived property " << property->getName()
      //   << " @ snapshot " << snapshot->getTime() << "Ma for formation " << formation->getName() << ": this property is neither a 3D continuous property or a 3D discontinuous property.";
   }

   return result;
}

AbstractDerivedProperties::FormationSurfacePropertyPtr AbstractDerivedProperties::AbstractPropertyManager::getFormationSurfaceProperty ( const DataModel::AbstractProperty*  property,
                                                                                                                         const DataModel::AbstractSnapshot*  snapshot,
                                                                                                                         const DataModel::AbstractFormation* formation,
                                                                                                                         const DataModel::AbstractSurface*   surface ) {

   FormationSurfacePropertyPtr result;

   if ( property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) {

      result = findFormationSurfacePropertyValues ( property, snapshot, formation, surface );

      if ( result == 0 ) {
         const FormationSurfacePropertyCalculatorPtr calculator = getFormationSurfaceCalculator ( property );
         FormationSurfacePropertyList  calculatedProperties;

         if ( calculator != 0 ) {
            calculator->calculate ( *this, snapshot, formation, surface, calculatedProperties );

            for ( FormationSurfacePropertyPtr calculatedProperty : calculatedProperties )
            {
               addFormationSurfaceProperty ( calculatedProperty );

               if ( calculatedProperty->getProperty () == property ) {
                  result = calculatedProperty;
               }

            }

         } else {
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "Derived property " << property->getName() << " @ snapshot " << snapshot->getTime() << " already computed.";
         }

      }

   }

   else{
      throw AbstractPropertyException() << "Could not compute formation surface derived property " << property->getName() << " @ snapshot " << snapshot->getTime() << ":"
         << " this property is not a 3D discontinuous property.";
   }

   return result;
}

AbstractDerivedProperties::ReservoirPropertyPtr AbstractDerivedProperties::AbstractPropertyManager::getReservoirProperty ( const DataModel::AbstractProperty*  property,
                                                                                                           const DataModel::AbstractSnapshot*  snapshot,
                                                                                                           const DataModel::AbstractReservoir* reservoir ) {

   ReservoirPropertyPtr result;

   if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY ) {

      result = findReservoirPropertyValues ( property, snapshot, reservoir );

      if ( result == 0 ) {
         const ReservoirPropertyCalculatorPtr calculator = getReservoirCalculator ( property );
         ReservoirPropertyList  calculatedProperties;

         if ( calculator != 0 ) {
            calculator->calculate ( *this, snapshot, reservoir, calculatedProperties );

            for ( ReservoirPropertyPtr calculatedProperty : calculatedProperties )
            {
               addReservoirProperty ( calculatedProperty );

               if ( calculatedProperty->getProperty () == property ) {
                  result = calculatedProperty;
               }

            }

         } else {
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "Derived property " << property->getName() << " @ snapshot " << snapshot->getTime() << " already computed.";
         }

      }

   }

   else{
      throw AbstractPropertyException() << "Could not compute reservoir derived property " << property->getName() << " @ snapshot " << snapshot->getTime() << ":"
         << " this property is not a 2D formation property.";
   }

   return result;
}

void AbstractDerivedProperties::AbstractPropertyManager::removeProperties ( const DataModel::AbstractSnapshot* snapshot ) {

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

   // Remove reservoir properties at snapshot time.
   ReservoirPropertyList::iterator reservoirsToRemove = std::remove_if ( m_reservoirProperties.begin (), m_reservoirProperties.end (),
                                                                         PropertyErasePredicate<ReservoirPropertyPtr> ( snapshot ));
   m_reservoirProperties.erase ( reservoirsToRemove, m_reservoirProperties.end ());

}

bool AbstractDerivedProperties::AbstractPropertyManager::formationPropertyIsComputable ( const DataModel::AbstractProperty*  property,
                                                                                 const DataModel::AbstractSnapshot*  snapshot,
                                                                                 const DataModel::AbstractFormation* formation ) const {

   FormationPropertyCalculatorPtr calculator = getFormationCalculator ( property );
   bool isComputable;

   if ( calculator ) {
      isComputable = calculator->isComputable ( *this, snapshot, formation );
   } else {
      isComputable = false;
   }

   if (!isComputable) {
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "Derived formation property '" << property->getName() << "' is not computable.";
   }
   return isComputable;
}

bool AbstractDerivedProperties::AbstractPropertyManager::formationSurfacePropertyIsComputable ( const DataModel::AbstractProperty*  property,
                                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                                        const DataModel::AbstractFormation* formation,
                                                                                        const DataModel::AbstractSurface*   surface ) const {

   FormationSurfacePropertyCalculatorPtr calculator = getFormationSurfaceCalculator ( property );
   bool isComputable;

   if ( calculator ) {
      isComputable = calculator->isComputable ( *this, snapshot, formation, surface );
   } else {
      isComputable = false;
   }

   if (!isComputable) {
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "Derived formation surface property '" << property->getName() << "' is not computable.";
   }
   return isComputable;
}

bool AbstractDerivedProperties::AbstractPropertyManager::surfacePropertyIsComputable ( const DataModel::AbstractProperty* property,
                                                                               const DataModel::AbstractSnapshot* snapshot,
                                                                               const DataModel::AbstractSurface*  surface ) const {
   SurfacePropertyCalculatorPtr calculator = getSurfaceCalculator ( property );
   bool isComputable;

   if ( calculator ) {
      isComputable = calculator->isComputable ( *this, snapshot, surface );
   } else {
      isComputable = false;
   }

   if (!isComputable) {
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "Derived surface property '" << property->getName() << "' is not computable.";
   }
   return isComputable;
}

bool AbstractDerivedProperties::AbstractPropertyManager::formationMapPropertyIsComputable ( const DataModel::AbstractProperty*  property,
                                                                                    const DataModel::AbstractSnapshot*  snapshot,
                                                                                    const DataModel::AbstractFormation* formation ) const {

   FormationMapPropertyCalculatorPtr calculator = getFormationMapCalculator ( property );
   bool isComputable;

   if ( calculator ) {
      isComputable = calculator->isComputable ( *this, snapshot, formation );
   } else {
      isComputable = false;
   }

   if (!isComputable) {
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "Derived formation map property '" << property->getName() << "' is not computable.";
   }
   return isComputable;
}

bool AbstractDerivedProperties::AbstractPropertyManager::reservoirPropertyIsComputable ( const DataModel::AbstractProperty*  property,
                                                                                 const DataModel::AbstractSnapshot*  snapshot,
                                                                                 const DataModel::AbstractReservoir* reservoir ) const {

   ReservoirPropertyCalculatorPtr calculator = getReservoirCalculator ( property );
   bool isComputable;

   if ( calculator ) {
      isComputable = calculator->isComputable ( *this, snapshot, reservoir );
   } else {
      isComputable = false;
   }

   if (!isComputable) {
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "Derived reservoir property '" << property->getName() << "' is not computable.";
   }
   return isComputable;
}
