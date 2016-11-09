#ifndef DERIVED_PROPERTIES__FORMATION_PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__FORMATION_PROPERTY_CALCULATOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "FormationProperty.h"

#include "PropertyCalculator.h"

namespace DerivedProperties {

   // Forward declaration of PropertyManager
   class AbstractPropertyManager;

   class Elt2dIndices
   {
   public:
      
      bool exists;
      int i[4];
      int j[4];
      
      bool nodeDefined[4];
   };

   typedef std::vector < Elt2dIndices > ElementList;

   /// \brief Calculates a derived property or set of properties.
   class FormationPropertyCalculator : public PropertyCalculator {

   public :

      virtual ~FormationPropertyCalculator () {}

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const = 0;

      /// \brief Determine if the property is computable for the specific combination of formation and snapshot.
      virtual bool isComputable ( const AbstractPropertyManager&      propManager,
                                  const DataModel::AbstractSnapshot*  snapshot,
                                  const DataModel::AbstractFormation* formation ) const;

      virtual void setUp2dEltMapping( AbstractPropertyManager& propManager, const FormationPropertyPtr aProperty, ElementList & mapElementList ) const;

    };


   typedef boost::shared_ptr<const FormationPropertyCalculator> FormationPropertyCalculatorPtr;


} // namespace DerivedProperties


#endif // DERIVED_PROPERTIES__FORMATION_PROPERTY_CALCULATOR_H
