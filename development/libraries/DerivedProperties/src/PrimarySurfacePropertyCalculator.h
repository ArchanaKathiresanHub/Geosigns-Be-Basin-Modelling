#ifndef DERIVED_PROPERTIES__PRIMARY_SURFACE_PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__PRIMARY_SURFACE_PROPERTY_CALCULATOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "GeoPhysicsProjectHandle.h"

#include "AbstractPropertyManager.h"
#include "SurfaceProperty.h"
#include "SurfacePropertyCalculator.h"

namespace DerivedProperties {

   // Forward declaration of PropertyManager
   class AbstractPropertyManager;

   /// \brief Loads a primary property.
   ///
   /// A primary property is a property that has been computed by one of 
   /// the simulators and saved to a file.
   class PrimarySurfacePropertyCalculator : public SurfacePropertyCalculator {

   public :

      /// \brief Construct with the project-handle and the property whose values this calculator will load.
      PrimarySurfacePropertyCalculator ( const GeoPhysics::ProjectHandle*   projectHandle,
                                         const DataModel::AbstractProperty* property );

      ~PrimarySurfacePropertyCalculator ();

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&           propManager,
                               const DataModel::AbstractSnapshot* snapshot,
                               const DataModel::AbstractSurface*  surface,
                                     SurfacePropertyList&         derivedProperties ) const;

      /// \brief Determine if the property is computable for the specific combination of surface and snapshot.
      virtual bool isComputable ( const AbstractPropertyManager&     propManager,
                                  const DataModel::AbstractSnapshot* snapshot,
                                  const DataModel::AbstractSurface*  surface ) const;

      /// \brief Get the snapshots for which the property is available.
      const DataModel::AbstractSnapshotSet& getSnapshots () const;

   private :

      /// \brief The property.
      const DataModel::AbstractProperty*       m_property;

      /// \brief The list of all the property-values for the property.
      DataAccess::Interface::PropertyValueList m_surfacePropertyValues;

      /// \brief Contains a set of snapshots for which there are property-values available for this property.
      DataModel::AbstractSnapshotSet           m_snapshots;

   };

   typedef boost::shared_ptr<PrimarySurfacePropertyCalculator> PrimarySurfacePropertyCalculatorPtr;


} // namespace DerivedProperties

#endif // DERIVED_PROPERTIES__PRIMARY_SURFACE_PROPERTY_CALCULATOR_H
