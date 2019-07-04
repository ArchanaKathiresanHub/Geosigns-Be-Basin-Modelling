//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef ABSTRACTDERIVED_PROPERTIES__RESERVOIR_PROPERTY_H
#define ABSTRACTDERIVED_PROPERTIES__RESERVOIR_PROPERTY_H

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractReservoir.h"
#include "AbstractGrid.h"

#include "AbstractPropertyValues.h"
#include "FormationMapProperty.h"

#include <memory>
#include <vector>

namespace AbstractDerivedProperties {

   /// \brief Stores the values of the designated property for the reservoir.
   ///
   /// The indices will use global index numbering.
   class ReservoirProperty : public FormationMapProperty {

   public :

      ReservoirProperty ( const DataModel::AbstractProperty*  property,
                          const DataModel::AbstractSnapshot*  snapshot,
                          const DataModel::AbstractReservoir* reservoir,
                          const DataModel::AbstractGrid*      grid );

      /// \brief Get the grid on which the property values are defined.
      const DataModel::AbstractReservoir* getReservoir () const;

   private :

      const DataModel::AbstractReservoir*  m_reservoir;

   };

   typedef std::shared_ptr<const ReservoirProperty> ReservoirPropertyPtr;

   typedef std::vector<ReservoirPropertyPtr> ReservoirPropertyList;

} // namespace AbstractDerivedProperties

//--------------------------------
//  Inlined functions
//--------------------------------

inline const DataModel::AbstractReservoir* AbstractDerivedProperties::ReservoirProperty::getReservoir () const {
   return m_reservoir;
}

#endif // ABSTRACTDERIVED_PROPERTIES__RESERVOIR_PROPERTY_H
