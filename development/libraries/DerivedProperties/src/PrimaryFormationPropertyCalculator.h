#ifndef DERIVED_PROPERTIES__PRIMARY_FORMATION_PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__PRIMARY_FORMATION_PROPERTY_CALCULATOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "GeoPhysicsProjectHandle.h"

#include "AbstractPropertyManager.h"
#include "FormationProperty.h"
#include "FormationPropertyCalculator.h"

namespace DerivedProperties {

   // Forward declaration of PropertyManager
   class AbstractPropertyManager;

   /// \brief Loads a primary property.
   ///
   /// A primary property is a property that has been computed by one of 
   /// the simulators and saved to a file.
   class PrimaryFormationPropertyCalculator : public FormationPropertyCalculator {

   public :

      /// \brief Construct with the project-handle and the property whose values this calculator will load.
      PrimaryFormationPropertyCalculator ( const GeoPhysics::ProjectHandle*   projectHandle,
                                           const DataModel::AbstractProperty* property );

      ~PrimaryFormationPropertyCalculator ();

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

      /// \brief Get the snapshots for which the property is available.
      const DataModel::AbstractSnapshotSet& getSnapshots () const;

   private :

      /// \brief The property.
      const DataModel::AbstractProperty*       m_property;

      /// \brief The list of all the property-values for the property.
      DataAccess::Interface::PropertyValueList m_formationPropertyValues;

      /// \brief Contains a set of snapshots for which there are property-values available for this property.
      DataModel::AbstractSnapshotSet           m_snapshots;

   };

   typedef boost::shared_ptr<PrimaryFormationPropertyCalculator> PrimaryFormationPropertyCalculatorPtr;


} // namespace DerivedProperties

#endif // DERIVED_PROPERTIES__PRIMARY_FORMATION_PROPERTY_CALCULATOR_H
