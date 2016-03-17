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

      DerivedPropertyManager ( GeoPhysics::ProjectHandle* projectHandle,
                               const bool                 debug = false );

      /// \brief Get the property given the property-name.
      ///
      /// If the name is not found then a null pointer will be returned.
      /// \param [in] name The name of the required property.
      virtual const DataAccess::Interface::Property* getProperty ( const std::string& name ) const;

      /// \brief Get the grid for the map.
      virtual const DataAccess::Interface::Grid* getMapGrid () const;

      /// \brief Determine whether or not the node is valid.
      virtual bool getNodeIsValid ( const unsigned int i, const unsigned int j ) const;


      /// \brief Get the formation property values for all formations that are active at the snapshot age.
      ///
      /// If the formation property values have not been computed and there is an associated calculator
      /// then the values will be calculated as required. Additional properties may also be calculated.
      /// \param [in] property The property whose values are requested.
      /// \param [in] snapshot The snapshot time at which the values were calculated.
      /// \param [in] includeBasement Indicate whether or not the basement formations should be included.
      /// \pre property is not be null and points to a valid property.
      /// \pre snapshot is not null and points to a valid snapshot age.
      /// \pre A calculator or calculators for this property exists.
      /// \post The result contains a list of values of the required property at all the active formations for the required snapshot age.
      virtual FormationPropertyList getFormationProperties ( const DataModel::AbstractProperty* property,
                                                             const DataModel::AbstractSnapshot* snapshot,
                                                             const bool                         includeBasement = false );

      /// \brief Get the formation map property values for all formations that are active at the snapshot age.
      ///
      /// If the formation map property values have not been computed and there is an associated calculator
      /// then the values will be calculated as required. Additional properties may also be calculated.
      /// \param [in] property The property whose values are requested.
      /// \param [in] snapshot The snapshot time at which the values were calculated.
      /// \param [in] includeBasement Indicate whether or not the basement formations should be included.
      /// \pre property is not be null and points to a valid property.
      /// \pre snapshot is not null and points to a valid snapshot age.
      /// \pre A calculator or calculators for this property exists.
      /// \post The result contains a list of values of the required property at all the active formations for the required snapshot age.
      virtual FormationMapPropertyList getFormationMapProperties ( const DataModel::AbstractProperty* property,
                                                                   const DataModel::AbstractSnapshot* snapshot,
                                                                   const bool                         includeBasement = false );

      /// \brief Get the surface property values for all surfaces that are active at the snapshot age.
      ///
      /// If the surface property values have not been computed and there is an associated calculator
      /// then the values will be calculated as required. Additional properties may also be calculated.
      /// \param [in] property The property whose values are requested.
      /// \param [in] snapshot The snapshot time at which the values were calculated.
      /// \param [in] includeBasement Indicate whether or not the basement surfaces should be included.
      /// \pre property is not be null and is a valid property.
      /// \pre snapshot is not null and is a valid snapshot age.
      /// \pre A calculator or calculators for this property exists.
      /// \post The result contains a list of values of the required property at all the active surfaces for the required snapshot age.
      virtual SurfacePropertyList getSurfaceProperties ( const DataModel::AbstractProperty* property,
                                                         const DataModel::AbstractSnapshot* snapshot,
                                                         const bool                         includeBasement = false );


   protected :

      /// \brief Get the geophysics project handle
      const GeoPhysics::ProjectHandle* getProjectHandle () const;

   private :

      /// \brief Load primary property calculators that are associated with surface only.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadPrimarySurfacePropertyCalculators ( const bool debug );

      /// \brief Load primary property calculators that are associated with formation and surface.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadPrimaryFormationSurfacePropertyCalculators ( const bool debug );

      /// \brief Load primary property calculators that are associated with formation only.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadPrimaryFormationMapPropertyCalculators ( const bool debug );

      /// \brief Load primary property calculators that are associated with reservoirs.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadPrimaryReservoirPropertyCalculators ( const bool debug );

      /// \brief Load primary property calculators that are associated with formation only.
      ///
      /// These are volume properties loaded from the property-values in the project-handle.
      void loadPrimaryFormationPropertyCalculators ( const bool debug );

      /// \brief Load  derived formation property calculators.
      ///
      /// Here all the calculators for derived properties on formation are added to the set of calculators.
      void loadDerivedFormationPropertyCalculator ( const bool debug );

      /// \brief Load derived formation-map property calculators.
      ///
      /// Here all the calculators for derived properties for formation-map are added to the set of calculators.
      void loadDerivedFormationMapPropertyCalculator ( const bool debug );

      /// \brief Load derived surface property calculators.
      ///
      /// Here all the calculators for derived properties for surfaces are added to the set of calculators.
      void loadDerivedSurfacePropertyCalculator ( const bool debug );

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

      /// \brief Determine if the derived property calculator should be added to the property-manager.
      ///
      /// This does not look at the properties required for the property to be calculated, only
      /// if a calculator for the property is already in the list.
      bool canAddDerivedSurfacePropertyCalculator ( const SurfacePropertyCalculatorPtr& surfacePropertyCalculator ) const;

      /// \brief The geophysics project handle
      GeoPhysics::ProjectHandle* m_projectHandle;

      /// \brief Flag to determine which properties to load
      bool m_loadAllProperties;

   }; 

} // namespace DerivedProperties

//
// Inline functions.
//

inline bool DerivedProperties::DerivedPropertyManager::getNodeIsValid ( const unsigned int i, const unsigned int j ) const {
   return m_projectHandle->getNodeIsValid ( i, j );
}


#endif // DERIVED_PROPERTIES__DERIVED_PROPERTY_MANAGER_H
