#ifndef DERIVED_PROPERTIES__FORMATION_SURFACE_OUTPUT_PROPERTY_VALUE__H
#define DERIVED_PROPERTIES__FORMATION_SURFACE_OUTPUT_PROPERTY_VALUE__H

#include "OutputPropertyValue.h"

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"

#include "AbstractPropertyManager.h"
#include "FormationProperty.h"

namespace DerivedProperties {

   /// \brief Stores the Formation-Surface property to be calculated and saved to disk

   class FormationSurfaceOutputPropertyValue : public OutputPropertyValue {
      
   public :
      
      /// \brief Allocate the FormationSurface property 
      FormationSurfaceOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                            const DataModel::AbstractProperty*          property,
                                            const DataModel::AbstractSnapshot*          snapshot,
                                            const DataModel::AbstractFormation*         formation,
                                            const DataModel::AbstractSurface*           surface );
      
      FormationSurfaceOutputPropertyValue ( DerivedProperties::FormationSurfacePropertyPtr& formationSurfaceProperty );
      
      /// \brief Get the value of the property at the position i,j,k
      virtual double getValue ( const double i, const double j, const double k ) const;

      /// \brief Get the number of nodes in the z direction
      unsigned int getDepth () const;
      
      /// \brief Determine whether or not the property is a primary property
      bool isPrimary() const;
      
      /// \brief Retreive the grid map
      void retrieveData() const;

      /// \brief Restore the grid map
      void restoreData() const;
      
      /// \brief Set the name for the property to be saved in a project file
      void setMapName( const string& mapName );

      /// \brief Get the name of the property to be saved in a project file
      const string & getMapName() const;

      /// \brief Determine whether or not the property has a grid map allocated
      bool hasMap() const;

      /// \brief Get the name of the property
      const string & getName() const;

      /// \brief Get the surface for which the property is allocated
      const DataModel::AbstractSurface* getSurface() const;
      
   private :
      
      /// \brief The Formation-Surface property
      DerivedProperties::FormationSurfacePropertyPtr m_formationSurfaceProperty;

      /// \brief The name which is used in the project file
      string m_mapName;
      
   };
   
}

#endif // DERIVED_PROPERTIES__FORMATION_OUTPUT_PROPERTY_VALUE__H
