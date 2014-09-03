#ifndef DERIVED_PROPERTIES__DERIVED_PROPERTY_MANAGER_H
#define DERIVED_PROPERTIES__DERIVED_PROPERTY_MANAGER_H

#include <string>

#include "AbstractProperty.h"
#include "AbstractGrid.h"

#include "Interface/GridMap.h"

#include "AbstractPropertyManager.h"

#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   class DerivedPropertyManager : public AbstractPropertyManager {

   public :

      DerivedPropertyManager ( GeoPhysics::ProjectHandle* projectHandle );

      /// \brief Get the property given the property-name.
      ///
      /// If the name is not found then a null pointer will be returned.
      virtual const DataModel::AbstractProperty* getProperty ( const std::string& name ) const;

      /// \brief Get the grid for the map.
      virtual const DataModel::AbstractGrid* getMapGrid () const;

   private :

      /// \brief Load primary properties that are associated with surface only.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadSurfaceProperties ();

      /// \brief Load primary properties that are associated with formation and surface.
      ///
      /// These are map properties loaded from the property-values in the project-handle.
      void loadFormationSurfaceProperties ();

      /// \brief Load primary properties that are associated with formation only.
      ///
      /// These are volume properties loaded from the property-values in the project-handle.
      void loadFormationProperties ();

      GeoPhysics::ProjectHandle* m_projectHandle;

   }; 

} // namespace DerivedProperties

#endif // DERIVED_PROPERTIES__DERIVED_PROPERTY_MANAGER_H
