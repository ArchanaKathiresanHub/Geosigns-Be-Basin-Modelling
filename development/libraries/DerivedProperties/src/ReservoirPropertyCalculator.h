#ifndef DERIVED_PROPERTIES__RESERVOIR_PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__RESERVOIR_PROPERTY_CALCULATOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractReservoir.h"

#include "ReservoirProperty.h"
#include "PropertyCalculator.h"

namespace DerivedProperties {

   // Forward declaration of PropertyManager
   class AbstractPropertyManager;

   /// \brief Calculates a derived property or set of properties.
   class ReservoirPropertyCalculator : public PropertyCalculator {

   public :

      virtual ~ReservoirPropertyCalculator () {}

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractReservoir* reservoir,
                                     ReservoirPropertyList&        derivedProperties ) const = 0;

      /// \brief Determine if the property is computable for the specific combination of reservoir and snapshot.
      virtual bool isComputable ( const AbstractPropertyManager&      propManager,
                                  const DataModel::AbstractSnapshot*  snapshot,
                                  const DataModel::AbstractReservoir* reservoir ) const;

   };


   typedef boost::shared_ptr<const ReservoirPropertyCalculator> ReservoirPropertyCalculatorPtr;


} // namespace DerivedProperties


#endif // DERIVED_PROPERTIES__RESERVOIR_PROPERTY_CALCULATOR_H
