#ifndef DERIVED_PROPERTIES__ABSTRACT_PROPERTY_MANAGER_H
#define DERIVED_PROPERTIES__ABSTRACT_PROPERTY_MANAGER_H

#include <vector>
#include <map>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"
#include "AbstractFormation.h"

#include "SurfacePropertyCalculator.h"
#include "FormationMapPropertyCalculator.h"
#include "FormationPropertyCalculator.h"
#include "FormationSurfacePropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Handles derived properties and their calculation.
   class AbstractPropertyManager {

   public :

      /// \brief Constructor.
      AbstractPropertyManager ();

      /// \brief Destructor.
      virtual ~AbstractPropertyManager () {}


      /// \brief Get the property given the property-name.
      ///
      /// If the name is not found then a null pointer will be returned.
      /// \param [in] name The name of the required property.
      virtual const DataModel::AbstractProperty* getProperty ( const std::string& name ) const = 0;

      /// \brief Get the surface property values.
      ///
      /// If the surface property values have not been computed and there is an associated calculator
      /// then the values will be calculated as required. Additional properties may also be calculated.
      /// \param [in] property The property whose values are requested.
      /// \param [in] snapshot The snapshot time at which the values were calculated.
      /// \param [in] surface  The surface to which the values are associated.
      SurfacePropertyPtr getSurfaceProperty ( const DataModel::AbstractProperty* property,
                                              const DataModel::AbstractSnapshot* snapshot,
                                              const DataModel::AbstractSurface*  surface );

      /// \brief Get the formation property values.
      ///
      /// If the formation property values have not been computed and there is an associated calculator
      /// then the values will be calculated as required. Additional properties may also be calculated.
      /// \param [in] property  The property whose values are requested.
      /// \param [in] snapshot  The snapshot time at which the values were calculated.
      /// \param [in] formation The formation to which the values are associated.
      FormationMapPropertyPtr getFormationMapProperty ( const DataModel::AbstractProperty*  property,
                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                        const DataModel::AbstractFormation* formation );

      /// \brief Get the formation property values.
      ///
      /// If the formation property values have not been computed and there is an associated calculator
      /// then the values will be calculated as required. Additional properties may also be calculated.
      /// \param [in] property  The property whose values are requested.
      /// \param [in] snapshot  The snapshot time at which the values were calculated.
      /// \param [in] formation The formation to which the values are associated.
      FormationPropertyPtr getFormationProperty ( const DataModel::AbstractProperty*  property,
                                                  const DataModel::AbstractSnapshot*  snapshot,
                                                  const DataModel::AbstractFormation* formation );

      /// \brief Get the surface and formation property values.
      ///
      /// If the property values have not been computed and there is an associated calculator
      /// then the values will be calculated as required. Additional properties may also be calculated.
      /// \param [in] property  The property whose values are requested.
      /// \param [in] snapshot  The snapshot time at which the values were calculated.
      /// \param [in] formation The formation to which the values are associated.
      /// \param [in] surface   The surface to which the values are associated.
      FormationSurfacePropertyPtr getFormationSurfaceProperty ( const DataModel::AbstractProperty*  property,
                                                                const DataModel::AbstractSnapshot*  snapshot,
                                                                const DataModel::AbstractFormation* formation,
                                                                const DataModel::AbstractSurface*   surface );

      /// \brief Get the grid for the map.
      virtual const DataModel::AbstractGrid* getMapGrid () const = 0;

   protected :

      /// \brief Add a calculator for a property or set of properties defined on a surface.
      ///
      /// \param [in] calculator  A calculator of surface properties.
      void addSurfacePropertyCalculator ( const SurfacePropertyCalculatorPtr calculator );

      /// \brief Add a calculator for a property or set of map properties defined on a formation.
      ///
      /// \param [in] calculator  A calculator of formation map properties.
      void addFormationMapPropertyCalculator ( const FormationMapPropertyCalculatorPtr calculator );

      /// \brief Add a calculator for a property or set of properties defined on a formation.
      ///
      /// \param [in] calculator  A calculator of formation properties.
      void addFormationPropertyCalculator ( const FormationPropertyCalculatorPtr calculator );
 
      /// \brief Add a calculator for a property or set of properties defined on a surface and formation.
      ///
      /// \param [in] calculator  A calculator of formation-surface properties.
      void addFormationSurfacePropertyCalculator ( const FormationSurfacePropertyCalculatorPtr calculator );


      /// \brief Add a set of property values to the availble property values.
      ///
      /// \param [in] surfaceProperty  A set of property values associated with a surface.
      void addSurfaceProperty ( const SurfacePropertyPtr surfaceProperty );
      
      /// \brief Add a set of property values to the available property values.
      ///
      /// \param [in] formationMapProperty  A set of map property values associated with a formation.
      void addFormationMapProperty ( const FormationMapPropertyPtr formationMapProperty );
      
      /// \brief Add a set of property values to the availble property values.
      ///
      /// \param [in] formationProperty  A set of property values associated with a formation.
      void addFormationProperty ( const FormationPropertyPtr formationProperty );

      /// \brief Add a set of property values to the availble property values.
      ///
      /// \param [in] formationSurfaceProperty  A set of property values associated with a formation and surface.
      void addFormationSurfaceProperty ( const FormationSurfacePropertyPtr formationSurfaceProperty );


      /// \brief Get the calculator for the property.
      ///
      /// If no calculator has been added then a null will be returned.
      /// \param [in] property The property whose calulator is requested.
      SurfacePropertyCalculatorPtr getSurfaceCalculator ( const DataModel::AbstractProperty* property ) const;

      /// \brief Get the calculator for the property.
      ///
      /// If no calculator has been added then a null will be returned.
      /// \param [in] property The property whose calulator is requested.
      FormationMapPropertyCalculatorPtr getFormationMapCalculator ( const DataModel::AbstractProperty* property ) const;

      /// \brief Get the calculator for the property.
      ///
      /// If no calculator has been added then a null will be returned.
      /// \param [in] property The property whose calulator is requested.
      FormationPropertyCalculatorPtr getFormationCalculator ( const DataModel::AbstractProperty* property ) const;

      /// \brief Get the calculator for the property.
      ///
      /// If no calculator has been added then a null will be returned.
      /// \param [in] property The property whose calulator is requested.
      FormationSurfacePropertyCalculatorPtr getFormationSurfaceCalculator ( const DataModel::AbstractProperty* property ) const;


      /// \brief Search the list of available property values for a specific set of values.
      ///
      /// If the values are not found then a null will be returned.
      /// \param [in] property The property whose values are requested.
      /// \param [in] snapshot The snapshot time at which the values were calculated.
      /// \param [in] surface  The surface to which the values are associated.
      SurfacePropertyPtr findSurfacePropertyValues ( const DataModel::AbstractProperty* property,
                                                     const DataModel::AbstractSnapshot* snapshot,
                                                     const DataModel::AbstractSurface*  surface ) const;

      /// \brief Search the list of available map property values for a specific set of values.
      ///
      /// If the values are not found then a null will be returned.
      /// \param [in] property  The property whose values are requested.
      /// \param [in] snapshot  The snapshot time at which the values were calculated.
      /// \param [in] formation The formation to which the values are associated.
      FormationMapPropertyPtr findFormationMapPropertyValues ( const DataModel::AbstractProperty*  property,
                                                               const DataModel::AbstractSnapshot*  snapshot,
                                                               const DataModel::AbstractFormation* formation ) const;

      /// \brief Search the list of available property values for a specific set of values.
      ///
      /// If the values are not found then a null will be returned.
      /// \param [in] property  The property whose values are requested.
      /// \param [in] snapshot  The snapshot time at which the values were calculated.
      /// \param [in] formation The formation to which the values are associated.
      FormationPropertyPtr findFormationPropertyValues ( const DataModel::AbstractProperty*  property,
                                                         const DataModel::AbstractSnapshot*  snapshot,
                                                         const DataModel::AbstractFormation* formation ) const;

      /// \brief Search the list of available property values for a specific set of values.
      ///
      /// If the values are not found then a null will be returned.
      /// \param [in] property  The property whose values are requested.
      /// \param [in] snapshot  The snapshot time at which the values were calculated.
      /// \param [in] formation The formation to which the values are associated.
      /// \param [in] surface   The surface to which the values are associated.
      FormationSurfacePropertyPtr findFormationSurfacePropertyValues ( const DataModel::AbstractProperty*  property,
                                                                       const DataModel::AbstractSnapshot*  snapshot,
                                                                       const DataModel::AbstractFormation* formation,
                                                                       const DataModel::AbstractSurface*   surface ) const;      

   private :

      /// \brief Mapping from property to is associated surface property calculator.
      typedef std::map<const DataModel::AbstractProperty*,SurfacePropertyCalculatorPtr> SurfacePropertyCalculatorMap;

      /// \brief Mapping from property to is associated formation map property calculator.
      typedef std::map<const DataModel::AbstractProperty*, FormationMapPropertyCalculatorPtr> FormationMapPropertyCalculatorMap;

      /// \brief Mapping from property to is associated formation property calculator.
      typedef std::map<const DataModel::AbstractProperty*, FormationPropertyCalculatorPtr> FormationPropertyCalculatorMap;

      /// \brief Mapping from property to is associated formation and surface property calculator.
      typedef std::map<const DataModel::AbstractProperty*, FormationSurfacePropertyCalculatorPtr> FormationSurfacePropertyCalculatorMap;

      /// \brief Map of property to surface-property calculator.
      SurfacePropertyCalculatorMap m_surfacePropertyCalculators;

      /// \brief List of all surface-properties that have been stored.
      SurfacePropertyList          m_surfaceProperties;

      /// \brief Map of property to formation-property calculator.
      FormationMapPropertyCalculatorMap m_formationMapPropertyCalculators;

      /// \brief List of all formation-properties that have been stored.
      FormationMapPropertyList       m_formationMapProperties;

      /// \brief Map of property to formation and surface property calculator.
      FormationSurfacePropertyCalculatorMap m_formationSurfacePropertyCalculators;

      /// \brief List of all formation and surface properties that have been stored.
      FormationSurfacePropertyList  m_formationSurfaceProperties;

      /// \brief Map of property to formation-property calculator.
      FormationPropertyCalculatorMap m_formationPropertyCalculators;

      /// \brief List of all formation-properties that have been stored.
      FormationPropertyList          m_formationProperties;

   };

} // namespace DerivedProperties


#endif // DERIVED_PROPERTIES__ABSTRACT_PROPERTY_MANAGER_H
