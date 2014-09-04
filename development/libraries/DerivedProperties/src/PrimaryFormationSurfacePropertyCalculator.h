#ifndef DERIVED_PROPERTIES__PRIMARY_FORMATION_SURFACE_PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__PRIMARY_FORMATION_SURFACE_PROPERTY_CALCULATOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "GeoPhysicsProjectHandle.h"

#include "AbstractPropertyManager.h"
#include "FormationSurfaceProperty.h"
#include "FormationSurfacePropertyCalculator.h"

namespace DerivedProperties {

   // Forward declaration of PropertyManager
   class AbstractPropertyManager;

   /// \brief Loads a primary property.
   ///
   /// A primary property is a property that has been computed by one of 
   /// the simulators and saved to a file.
   class PrimaryFormationSurfacePropertyCalculator : public FormationSurfacePropertyCalculator {

   public :

      /// \brief Construct with the project-handle and the property whose values this calculator will load.
      PrimaryFormationSurfacePropertyCalculator ( const GeoPhysics::ProjectHandle*   projectHandle,
                                           const DataModel::AbstractProperty* property );

      ~PrimaryFormationSurfacePropertyCalculator ();

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                               const DataModel::AbstractSurface*   surface,
                                     FormationSurfacePropertyList& derivedProperties ) const;

      /// \brief Get a list of the property names that will be calculated by the calculator.
      virtual const std::vector<std::string>& getPropertyNames () const;

   private :

      /// \brief The property.
      const DataModel::AbstractProperty*       m_property;

      /// \brief The list of all the property-values for the property.
      DataAccess::Interface::PropertyValueList m_formationSurfacePropertyValues;

      /// \brief Will contain the name of the property.
      std::vector<std::string>                 m_propertyNames;

   };

   typedef boost::shared_ptr<PrimaryFormationSurfacePropertyCalculator> PrimaryFormationSurfacePropertyCalculatorPtr;


} // namespace DerivedProperties

#endif // DERIVED_PROPERTIES__PRIMARY_FORMATION_SURFACE_PROPERTY_CALCULATOR_H
