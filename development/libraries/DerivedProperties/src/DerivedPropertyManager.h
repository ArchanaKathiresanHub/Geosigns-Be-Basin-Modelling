//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__DERIVED_PROPERTY_MANAGER_H
#define DERIVED_PROPERTIES__DERIVED_PROPERTY_MANAGER_H

#include <string>

#include "AbstractProperty.h"
#include "AbstractGrid.h"

#include "Grid.h"
#include "GridMap.h"
#include "Property.h"

#include "AbstractPropertyManager.h"

#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   /// \brief Handles derived properties and their calculation.
   ///
   /// First all properties are loaded from disk.
   class DerivedPropertyManager : public AbstractDerivedProperties::AbstractPropertyManager {

   public :

      DerivedPropertyManager ( GeoPhysics::ProjectHandle& projectHandle,
                               const bool                 loadAllProperties = true,
                               const bool                 debug = false );

      /// \brief Get the property given the property-name.
      ///
      /// If the name is not found then a null pointer will be returned.
      /// \param [in] name The name of the required property.
      virtual const DataModel::AbstractProperty* getProperty ( const std::string& name ) const;

      /// \brief Get the grid for the map.
      virtual const DataModel::AbstractGrid* getMapGrid () const;

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
      virtual AbstractDerivedProperties::FormationPropertyList getFormationProperties ( const DataModel::AbstractProperty* property,
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
      virtual AbstractDerivedProperties::FormationMapPropertyList getFormationMapProperties ( const DataModel::AbstractProperty* property,
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
      virtual AbstractDerivedProperties::SurfacePropertyList getSurfaceProperties ( const DataModel::AbstractProperty* property,
                                                                                    const DataModel::AbstractSnapshot* snapshot,
                                                                                    const bool                         includeBasement = false );


   protected :

      /// \brief Get the geophysics project handle
      const GeoPhysics::ProjectHandle& getProjectHandle() const;

      /// \brief Load one formation property calculator.
      ///
      /// Here one calculator for derived properties on formation is added to the set of calculators.
      void  loadDerivedFormationPropertyCalculator(AbstractDerivedProperties::FormationPropertyCalculatorPtr formationPropertyCalculator, const bool debug);

   private :

      /// \brief Load primary property calculators that are associated with surface only.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadPrimarySurfacePropertyCalculators ( const bool loadAllProperties, const bool debug );

      /// \brief Load primary property calculators that are associated with formation and surface.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadPrimaryFormationSurfacePropertyCalculators ( const bool loadAllProperties, const bool debug );

      /// \brief Load primary property calculators that are associated with formation only.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadPrimaryFormationMapPropertyCalculators ( const bool loadAllProperties, const bool debug );

      /// \brief Load primary property calculators that are associated with reservoirs.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadPrimaryReservoirPropertyCalculators ( const bool debug );

      /// \brief Load primary property calculators that are associated with formation only.
      ///
      /// These are volume properties loaded from the property-values in the project-handle.
      void loadPrimaryFormationPropertyCalculators (const bool loadAllProperties, const bool debug );

      /// \brief Load derived formation property calculators.
      ///
      /// Here all the calculators for derived properties on formation are added to the set of calculators.
      void loadDerivedFormationPropertyCalculators ( const bool debug );

      /// \brief Load derived formation-map property calculators.
      ///
      /// Here all the calculators for derived properties for formation-map are added to the set of calculators.
      void loadDerivedFormationMapPropertyCalculators (const bool debug);

      /// \brief Load one derived formation-map property calculator.
      ///
      /// Here one calculator for derived properties for formation-map is added to the set of calculators.
      void loadDerivedFormationMapPropertyCalculator(AbstractDerivedProperties::FormationMapPropertyCalculatorPtr formationMapPropertyCalculator);

      /// \brief Load derived surface property calculators.
      ///
      /// Here all the calculators for derived properties for surfaces are added to the set of calculators.
      void loadDerivedSurfacePropertyCalculators ( const bool debug );

      /// \brief Load one derived surface property calculator.
      ///
      /// Here one calculator for derived properties for surfaces is added to the set of calculators.
      void loadDerivedSurfacePropertyCalculator(AbstractDerivedProperties::SurfacePropertyCalculatorPtr formationSurfacePropertyCalculator);

      /// \brief Load one derived surface property calculator.
      ///
      /// Here one calculator for derived properties for formation surfaces is added to the set of calculators.
      void loadDerivedFormationSurfacePropertyCalculator(AbstractDerivedProperties::FormationSurfacePropertyCalculatorPtr formationSurfacePropertyCalculator);

      /// \brief Determine if the derived property calculator should be added to the property-manager.
      ///
      /// This does not look at the properties required for the property to be calculated, only
      /// if a calculator for the property is already in the list.
      bool canAddDerivedFormationPropertyCalculator ( const AbstractDerivedProperties::FormationPropertyCalculatorPtr& formationPropertyCalculator) const;

      /// \brief Determine if the derived property calculator should be added to the property-manager.
      ///
      /// This does not look at the properties required for the property to be calculated, only
      /// if a calculator for the property is already in the list.
      bool canAddDerivedFormationMapPropertyCalculator ( const AbstractDerivedProperties::FormationMapPropertyCalculatorPtr& formationMapPropertyCalculator) const;

      /// \brief Determine if the derived property calculator should be added to the property-manager.
      ///
      /// This does not look at the properties required for the property to be calculated, only
      /// if a calculator for the property is already in the list.
      bool canAddDerivedSurfacePropertyCalculator ( const AbstractDerivedProperties::SurfacePropertyCalculatorPtr& surfacePropertyCalculator ) const;

      /// \brief Determine if the derived property calculator should be added to the property-manager.
      ///
      /// This does not look at the properties required for the property to be calculated, only
      /// if a calculator for the property is already in the list.
      bool canAddDerivedFormationSurfacePropertyCalculator ( const AbstractDerivedProperties::FormationSurfacePropertyCalculatorPtr& formationSurfacePropertyCalculator ) const;

      /// \brief The geophysics project handle
      GeoPhysics::ProjectHandle& m_projectHandle;

   };

} // namespace DerivedProperties

//
// Inline functions.
//

inline bool DerivedProperties::DerivedPropertyManager::getNodeIsValid ( const unsigned int i, const unsigned int j ) const {
   return m_projectHandle.getNodeIsValid ( i, j );
}


#endif // DERIVED_PROPERTIES__DERIVED_PROPERTY_MANAGER_H
