#ifndef DERIVED_PROPERTIES__FORMATION_MAP_OUTPUT_PROPERTY_VALUE__H
#define DERIVED_PROPERTIES__FORMATION_MAP_OUTPUT_PROPERTY_VALUE__H

#include "OutputPropertyValue.h"

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"

#include "AbstractPropertyManager.h"
#include "FormationMapProperty.h"

namespace DerivedProperties {

   /// \brief Stores the FormationMap property to be calculated and saved to disk
   class FormationMapOutputPropertyValue : public OutputPropertyValue {

   public :

      /// \brief Allocate the FormationMap property 
      FormationMapOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                        const DataModel::AbstractProperty*          property,
                                        const DataModel::AbstractSnapshot*          snapshot,
                                        const DataModel::AbstractFormation*         formation );
      
      FormationMapOutputPropertyValue ( DerivedProperties::FormationMapPropertyPtr& formationMapProperty );
      
      /// \brief Get the value of the property at the position i,j,k
      virtual double getValue ( const double i, const double j, const double k ) const;
      
      /// \brief Get the number of nodes in the z direction
      unsigned int getDepth () const;
      
      /// \brief Get the name of the property
      const string & getName() const;
      
      /// \brief Determine whether or not the property has a grid map allocated
      bool hasMap() const;
      
      /// \brief Determine whether or not the property is a primary property
      bool isPrimary() const;
      
      /// \brief Retreive the grid map
      void retrieveData () const;
     
      /// \brief Restore the grid map
      void restoreData () const;
      
   private :
 
      /// \brief The FormationMap property
      DerivedProperties::FormationMapPropertyPtr m_formationMapProperty;
      
   };
   
}
#endif // DERIVED_PROPERTIES__FORMATION_MAP_OUTPUT_PROPERTY_VALUE__H