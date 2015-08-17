#ifndef DERIVED_PROPERTIES__RESERVOIR_PROPERTY_H
#define DERIVED_PROPERTIES__RESERVOIR_PROPERTY_H

#include <vector>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractReservoir.h"
#include "AbstractGrid.h"

#include "AbstractPropertyValues.h"
#include "FormationMapProperty.h"

namespace DerivedProperties {

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

   typedef boost::shared_ptr<const ReservoirProperty> ReservoirPropertyPtr;

   typedef std::vector<ReservoirPropertyPtr> ReservoirPropertyList;

} // namespace DerivedProperties

//--------------------------------
//  Inlined functions
//--------------------------------

inline const DataModel::AbstractReservoir* DerivedProperties::ReservoirProperty::getReservoir () const {
   return m_reservoir;
}

#endif // DERIVED_PROPERTIES__RESERVOIR_PROPERTY_H
