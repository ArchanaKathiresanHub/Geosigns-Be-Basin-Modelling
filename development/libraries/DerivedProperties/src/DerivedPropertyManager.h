#ifndef DERIVED_PROPERTIES__DERIVED_PROPERTY_MANAGER_H
#define DERIVED_PROPERTIES__DERIVED_PROPERTY_MANAGER_H

#include <string>

#include "AbstractProperty.h"
#include "AbstractGrid.h"

#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/Property.h"

#include "AbstractPropertyManager.h"

#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   /// \brief Handles derived properties and their calculation.
   ///
   /// First all properties are loaded from disk.
   class DerivedPropertyManager : public AbstractPropertyManager {

   public :

      DerivedPropertyManager ( GeoPhysics::ProjectHandle* projectHandle );

      /// \brief Get the property given the property-name.
      ///
      /// If the name is not found then a null pointer will be returned.
      /// \param [in] name The name of the required property.
      virtual const DataAccess::Interface::Property* getProperty ( const std::string& name ) const;

      /// \brief Get the grid for the map.
      virtual const DataAccess::Interface::Grid* getMapGrid () const;

      /// \brief Determine whether or not the node is valid.
      virtual bool getNodeIsValid ( const unsigned int i, const unsigned int j ) const;


   protected :

      /// \brief Get the geophysics project handle
      const GeoPhysics::ProjectHandle* getProjectHandle () const;

   private :

      /// \brief Load primary property calculators that are associated with surface only.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadPrimarySurfacePropertyCalculators ();

      /// \brief Load primary property calculators that are associated with formation and surface.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadPrimaryFormationSurfacePropertyCalculators ();

      /// \brief Load primary property calculators that are associated with formation only.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadPrimaryFormationMapPropertyCalculators ();

      /// \brief Load primary property calculators that are associated with formation only.
      ///
      /// These are volume properties loaded from the property-values in the project-handle.
      void loadPrimaryFormationPropertyCalculators ();

      /// \brief Load  derived formation property calculators.
      ///
      /// Here all the calculators for derived properties on formation are added to the set of calculators.
      void loadDerivedFormationPropertyCalculator ();

      /// \brief Load derived formation-map property calculators.
      ///
      /// Here all the calculators for derived properties for formation-map are added to the set of calculators.
      void loadDerivedFormationMapPropertyCalculator ();

      /// \brief Determine if the derived property calculator should be added to the property-manager.
      ///
      /// This does not look at the properties required for the property to be calculated, only
      /// if a calculator for the property is already in the list.
      bool canAddDerivedFormationPropertyCalculator ( const FormationPropertyCalculatorPtr& formationPropertyCalculator) const;

      /// \brief Determine if the derived property calculator should be added to the property-manager.
      ///
      /// This does not look at the properties required for the property to be calculated, only
      /// if a calculator for the property is already in the list.
      bool canAddDerivedFormationMapPropertyCalculator ( const FormationMapPropertyCalculatorPtr& formationMapPropertyCalculator) const;


      /// \brief The geophysics project handle
      GeoPhysics::ProjectHandle* m_projectHandle;

   }; 

} // namespace DerivedProperties

//
// Inline functions.
//

inline bool DerivedProperties::DerivedPropertyManager::getNodeIsValid ( const unsigned int i, const unsigned int j ) const {
   return m_projectHandle->getNodeIsValid ( i, j );
}


#endif // DERIVED_PROPERTIES__DERIVED_PROPERTY_MANAGER_H
