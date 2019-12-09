//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__PRIMARY_RESERVOIR_PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__PRIMARY_RESERVOIR_PROPERTY_CALCULATOR_H

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractReservoir.h"

#include "GeoPhysicsProjectHandle.h"

#include "AbstractPropertyManager.h"
#include "ReservoirProperty.h"
#include "ReservoirPropertyCalculator.h"

#include <memory>
#include <string>
#include <vector>

namespace AbstractDerivedProperties {
   // Forward declaration of PropertyManager
   class AbstractPropertyManager;
}

namespace DerivedProperties {

   /// \brief Loads a primary property.
   ///
   /// A primary property is a property that has been computed by one of
   /// the simulators and saved to a file.
   class PrimaryReservoirPropertyCalculator : public AbstractDerivedProperties::ReservoirPropertyCalculator {

   public :

      /// \brief Construct with the project-handle and the property whose values this calculator will load.
      PrimaryReservoirPropertyCalculator ( const DataModel::AbstractProperty* property,
                                           const DataAccess::Interface::PropertyValueList& propertyValues );

      ~PrimaryReservoirPropertyCalculator ();

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractReservoir*                       reservoir,
                                     AbstractDerivedProperties::ReservoirPropertyList&   derivedProperties ) const;

      /// \brief Determine if the property is computable for the specific combination of reservoir and snapshot.
      virtual bool isComputable ( const AbstractDerivedProperties::AbstractPropertyManager& propManager,
                                  const DataModel::AbstractSnapshot*                        snapshot,
                                  const DataModel::AbstractReservoir*                       reservoir ) const;

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

   typedef std::shared_ptr<PrimaryReservoirPropertyCalculator> PrimaryReservoirPropertyCalculatorPtr;


} // namespace DerivedProperties

#endif // DERIVED_PROPERTIES__PRIMARY_RESERVOIR_PROPERTY_CALCULATOR_H
