#ifndef DERIVED_PROPERTIES__DERIVED_PROPERTY_MANAGER_H
#define DERIVED_PROPERTIES__DERIVED_PROPERTY_MANAGER_H

#include <string>

#include "AbstractProperty.h"
#include "AbstractGrid.h"

#include "Interface/GridMap.h"

#include "AbstractPropertyManager.h"

#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   /// \brief Handles derived properties and their calculation.
   ///
   /// First all properties are loaded from disk.
   class DerivedPropertyManager : public AbstractPropertyManager {

   public :

      DerivedPropertyManager ( GeoPhysics::ProjectHandle* projectHandle );

      /// \brief Get the property given the property-name.
      ///
      /// If the name is not found then a null pointer will be returned.
      /// \param [in] name The name of the required property.
      virtual const DataModel::AbstractProperty* getProperty ( const std::string& name ) const;

      /// \brief Get the grid for the map.
      virtual const DataModel::AbstractGrid* getMapGrid () const;

   private :

      /// \brief Load primary property calculators that are associated with surface only.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadSurfacePropertyCalculators ();

      /// \brief Load primary property calculators that are associated with formation and surface.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadFormationSurfacePropertyCalculators ();

      /// \brief Load primary property calculators that are associated with formation only.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadFormationMapPropertyCalculators ();

      /// \brief Load primary property calculators that are associated with formation only.
      ///
      /// These are volume properties loaded from the property-values in the project-handle.
      void loadFormationPropertyCalculators ();

      /// \brief The geophysics project handle
      GeoPhysics::ProjectHandle* m_projectHandle;

   }; 

} // namespace DerivedProperties

#endif // DERIVED_PROPERTIES__DERIVED_PROPERTY_MANAGER_H
