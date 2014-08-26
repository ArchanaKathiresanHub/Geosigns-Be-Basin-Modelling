#ifndef DERIVED_PROPERTIES__PROPERTY_MANAGER_H
#define DERIVED_PROPERTIES__PROPERTY_MANAGER_H

#include <vector>
#include <map>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"
#include "AbstractFormation.h"

#include "SurfacePropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Handles derived properties and their calculation.
   class DerivedPropertyManager {

   public :

      DerivedPropertyManager ();

      virtual ~DerivedPropertyManager () {}


      /// \brief Get the property given the property-name.
      ///
      /// If the name is not found then a null pointer will be returned.
      const DataModel::AbstractProperty* getProperty ( const std::string& name ) const;

      /// \brief Get the surface property values.
      ///
      /// If the surface property values have not been computed and there is an associated calculator
      /// then the values will be calculated as required.
      SurfacePropertyPtr getSurfaceProperty ( const DataModel::AbstractProperty* property,
                                              const DataModel::AbstractSnapshot* snapshot,
                                              const DataModel::AbstractSurface*  surface );

      /// \brief Get the grid for the map.
      virtual const DataModel::AbstractGrid* getMapGrid () const = 0;

      // virtual const DataModel::AbstractGrid* getFormationGrid ( const DataModel::AbstractFormation* formation ) const = 0;

   protected :

      /// \brief Add a calculator for the property defined on a surface.
      void addCalculator ( const DataModel::AbstractProperty* property,
                           const SurfacePropertyCalculatorPtr calculator );

      /// \brief Add an abstract property to the list of available properties.
      ///
      /// If a property has been added already then it will not be added a second time.
      void addProperty ( const DataModel::AbstractProperty* property );

      /// \brief Add a set of property values to the availble property values.
      void addSurfaceProperty ( const SurfacePropertyPtr surfaceProperty );

      /// \brief Get the property 
      ///
      /// E.g. For the HeatFlow encompasses three properties: HeatFlowX, HeatFlowY and HeatFlowZ.
      /// Most properties this will be an identity operation.
      const DataModel::AbstractProperty* getEncompassingProperty ( const DataModel::AbstractProperty* property ) const;

      /// \brief Set up an encompassing property relationship.
      ///
      /// E.g. set up the relationship between HeatFlowX and HeatFlow.
      void addEncompassingProperty ( const DataModel::AbstractProperty* encompassedProperty,
                                     const DataModel::AbstractProperty* encompassingProperty );

      /// \brief Get the calculator for the property.
      ///
      /// If no calculator has been added then a null will be returned.
      SurfacePropertyCalculatorPtr getCalculator ( const DataModel::AbstractProperty* property ) const;

      /// \brief Search the list of available property values for a specific set of values.
      ///
      /// If the values are not found then a null will be returned.
      SurfacePropertyPtr findPropertyValues ( const DataModel::AbstractProperty* property,
                                              const DataModel::AbstractSnapshot* snapshot,
                                              const DataModel::AbstractSurface*  surface ) const;


   private :

      /// \brief List of all available properties.
      typedef std::vector <const DataModel::AbstractProperty*> PropertyList;

      /// \brief Maps the property to its encompassing property.
      typedef std::map<const DataModel::AbstractProperty*, const DataModel::AbstractProperty*> EncompassingPropertyMap;

      /// \brief Mapping from property to is associated surface property calculator.
      typedef std::map<const DataModel::AbstractProperty*,SurfacePropertyCalculatorPtr> SurfacePropertyCalculatorMap;

      PropertyList                 m_properties;
      EncompassingPropertyMap      m_encompassingProperties;
      SurfacePropertyCalculatorMap m_surfacePropertyCalculators;
      SurfacePropertyList          m_surfaceProperties;

   };

} // namespace DerivedProperties


#endif // DERIVED_PROPERTIES__PROPERTY_MANAGER_H
