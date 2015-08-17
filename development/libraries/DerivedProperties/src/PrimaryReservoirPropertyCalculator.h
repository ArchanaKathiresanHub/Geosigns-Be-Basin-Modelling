#ifndef DERIVED_PROPERTIES__PRIMARY_RESERVOIR_PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__PRIMARY_RESERVOIR_PROPERTY_CALCULATOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractReservoir.h"

#include "GeoPhysicsProjectHandle.h"

#include "AbstractPropertyManager.h"
#include "ReservoirProperty.h"
#include "ReservoirPropertyCalculator.h"

namespace DerivedProperties {

   // Forward declaration of PropertyManager
   class AbstractPropertyManager;

   /// \brief Loads a primary property.
   ///
   /// A primary property is a property that has been computed by one of 
   /// the simulators and saved to a file.
   class PrimaryReservoirPropertyCalculator : public ReservoirPropertyCalculator {

   public :

      /// \brief Construct with the project-handle and the property whose values this calculator will load.
      PrimaryReservoirPropertyCalculator ( const GeoPhysics::ProjectHandle*   projectHandle,
                                           const DataModel::AbstractProperty* property );

      ~PrimaryReservoirPropertyCalculator ();

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractReservoir* reservoir,
                                     ReservoirPropertyList&        derivedProperties ) const;

      /// \brief Determine if the property is computable for the specific combination of reservoir and snapshot.
      virtual bool isComputable ( const AbstractPropertyManager&      propManager,
                                  const DataModel::AbstractSnapshot*  snapshot,
                                  const DataModel::AbstractReservoir* reservoir ) const;

      /// \brief Get the snapshots for which the property is available.
      const DataModel::AbstractSnapshotSet& getSnapshots () const;

   private :

      /// \brief The property.
      const DataModel::AbstractProperty*       m_property;

      /// \brief The list of all the property-values for the property.
      DataAccess::Interface::PropertyValueList m_reservoirPropertyValues;

      /// \brief Contains a set of snapshots for which there are property-values available for this property.
      DataModel::AbstractSnapshotSet           m_snapshots;

   };

   typedef boost::shared_ptr<PrimaryReservoirPropertyCalculator> PrimaryReservoirPropertyCalculatorPtr;


} // namespace DerivedProperties

#endif // DERIVED_PROPERTIES__PRIMARY_RESERVOIR_PROPERTY_CALCULATOR_H
