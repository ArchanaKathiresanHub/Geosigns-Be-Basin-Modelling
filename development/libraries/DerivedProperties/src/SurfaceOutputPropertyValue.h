#ifndef DERIVED_PROPERTIES__SURFACE_OUTPUT_PROPERTY_VALUE__H
#define DERIVED_PROPERTIES__SURFACE_OUTPUT_PROPERTY_VALUE__H

#include "OutputPropertyValue.h"

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "AbstractPropertyManager.h"
#include "SurfaceProperty.h"

namespace DerivedProperties {

   /// \brief Stores the Surface property to be calculated and saved to disk
   class SurfaceOutputPropertyValue : public OutputPropertyValue {
      
   public :
      
      SurfaceOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                   const DataModel::AbstractProperty*        property,
                                   const DataModel::AbstractSnapshot*        snapshot,
                                   const DataModel::AbstractSurface*         surface );
      
      SurfaceOutputPropertyValue ( DerivedProperties::SurfacePropertyPtr& surfaceProperty );
      
      /// \brief Get the value of the property at the position i,j,k
      virtual double getValue ( const double i, const double j, const double k ) const;
      
      /// \brief Get the number of nodes in the z direction
      unsigned int getDepth () const;
      
      /// \brief Get the name of the property
      const string & getName() const;
      
      /// \brief Get the Surface for which the property is allocated
      const DataModel::AbstractSurface* getSurface() const;
      
      /// \brief Determine whether or not the property has a grid map allocated
      bool hasMap() const;
            
      /// \brief Get the grid on which the property values are defined.
      const DataModel::AbstractGrid* getGrid () const;

      /// \brief Retreive the grid map
      void retrieveData() const;
 
      /// \brief Restore the grid map
      void restoreData() const;
      
      /// \brief Determine whether or not the property is a primary property
      bool isPrimary() const;
      
      /// \brief Set the map name to be used in a project file
      void setMapName( const string& mapName );

      /// \brief Get the map name to be used in a project file
      const string & getMapName() const;
   private :
      
      /// \brief The Surface property
      DerivedProperties::SurfacePropertyPtr m_surfaceProperty;

      /// \brief The name which is used in a project file
      string m_mapName;
      
   };
   
}
#endif // DERIVED_PROPERTIES__SURFACE_OUTPUT_PROPERTY_VALUE__H
