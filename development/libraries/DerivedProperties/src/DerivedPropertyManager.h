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

      void loadSurfaceProperties ();

      // void loadFormationSurfaceProperties ();

      // void loadFormationProperties ();

      GeoPhysics::ProjectHandle* m_projectHandle;

   }; 

} // namespace DerivedProperties

#endif // DERIVED_PROPERTIES__DERIVED_PROPERTY_MANAGER_H
