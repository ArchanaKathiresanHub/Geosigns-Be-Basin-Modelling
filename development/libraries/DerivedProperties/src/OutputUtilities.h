//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__OUTPUT_UTILITIES_HH
#define DERIVED_PROPERTIES__OUTPUT_UTILITIES_HH

#include "GeoPhysicsProjectHandle.h"
#include "OutputPropertyValue.h"
#include "FormationOutputPropertyValue.h"
#include "FormationMapOutputPropertyValue.h"
#include "FormationSurfaceOutputPropertyValue.h"
#include "SurfaceOutputPropertyValue.h"
#include "ReservoirOutputPropertyValue.h"
#include "Snapshot.h"
#include "PropertyValue.h"
#include "Property.h"
#include "Surface.h"
#include "Formation.h"
#include "Reservoir.h"

using namespace std;
using namespace DataAccess;
using namespace Interface;
using namespace DerivedProperties;

namespace DerivedProperties {

   typedef vector < string > StringVector;

   typedef vector < const Interface::Formation * > FormationVector;

   typedef map < const Interface::Property *, OutputPropertyValuePtr > OutputPropertyValueMap;

   typedef pair < const Interface::Formation *, const Surface * > FormationSurface;
   typedef vector < FormationSurface > FormationSurfaceVector;
   typedef map < const FormationSurface, OutputPropertyValueMap > FormationSurfaceOutputPropertyValueMap;
   typedef map < const Snapshot *, FormationSurfaceOutputPropertyValueMap> SnapshotFormationSurfaceOutputPropertyValueMap;

   bool isEqualPropertyName( const string s1, const string s2 );

   // bool acquireFormations( GeoPhysics::ProjectHandle& projectHandle, FormationSurfaceVector & formationSurfacePairs,
   //                         StringVector & formationNames, const bool includeBasement = true ) ;

   /// \brief Create a vector of formation-surface pairs for the list of formation names
   ///
   /// \param [in]  formationNames         List of formation names
   /// \param [in]  projectHandle          Pointer to the project handle object
   /// \param [in]  useTop                 True for the top surfaces, false for the bottoms
   /// \param [out] formationSurfacePairs  On exit will contain formation-surface pairs

   bool acquireFormationSurfaces( GeoPhysics::ProjectHandle& projectHandle, FormationSurfaceVector & formationSurfacePairs,
                                  StringVector & formationNames, bool useTop, const bool includeBasement = true );

   /// \brief Create a vector of formation-surface pairs for the list of formation names. ( Surface is always empty )
   ///
   /// \param [in]  formationNames         List of formation names
   /// \param [in]  projectHandle          Pointer to the project handle object
   /// \param [out] formationSurfacePairs  On exit will contain formation-surface pairs

   bool acquireFormations( GeoPhysics::ProjectHandle& projectHandle, FormationSurfaceVector & formationSurfacePairs,
                           StringVector & formationNames, const bool includeBasement = true );

   /// \brief Create a vector of properties for the list of the property names. Determine whether or not a property can be computed.
   ///
   /// \param [in]  propertyNames   List of the property names
   /// \param [in]  projectHandle   Pointer to the project handle object
   /// \param [in]  propertyManager The property manager object
   /// \param [out] properties      On exit will contain a list of the properties

   bool acquireProperties( GeoPhysics::ProjectHandle& projectHandle,
                           const AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                           DataAccess::Interface::PropertyList & properties, StringVector & propertyNames );

   /// \brief Allocate OutputProperty for the formation-surface pair, the snapshot and the property if the property is computable.
   ///
   /// \param [in]  formationSurfaceItem   The formation and the surface for which the property to be allocated
   /// \param [in]  property               The property to be allocated
   /// \param [in]  snapshot               The snapshot for which the property to be allocated
   /// \param [in]  propertyManager        The property manager object

   OutputPropertyValuePtr allocateOutputProperty ( AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                                   const DataModel::AbstractProperty* property,
                                                   const DataModel::AbstractSnapshot* snapshot,
                                                   const FormationSurface& formationSurfaceItem,
                                                   const bool basementIncluded = false );

   /// \brief Create the property values in the project handle for the list of properties at the snapshot age.
   ///
   /// \param [in]  formationSurfaceItem    The formation and the surface for which the properties value to be created
   /// \param [in]  properties              The list of properties to be created
   /// \param [in]  snapshot                The snapshot for which the properties to be created
   /// \param [in]  projectHandle           The pointer to the project handle
   /// \param [in]  allOutputPropertyValues The set of allocated output property values (calculators)

   void outputSnapshotFormationData( GeoPhysics::ProjectHandle& projectHandle, const Snapshot * snapshot,
                                     const FormationSurface                         & formationSurfaceItem,
                                     DataAccess::Interface::PropertyList            & properties,
                                     SnapshotFormationSurfaceOutputPropertyValueMap & allOutputPropertyValues );


   /// \brief Create the property value in the project handle for the formation and the surface at the snapshot age.
   ///
   /// \param [in]  formation       The formation for which the property value to be created
   /// \param [in]  surface         The surface for which the property value to be created
   /// \param [in]  propertyValue   The output property value which contains the propery calculator
   /// \param [in]  snapshot        The snapshot for which the property value to be created
   /// \param [in]  projectHandle   The pointer to the project handle object

   bool createSnapshotResultPropertyValue ( GeoPhysics::ProjectHandle& projectHandle,
                                            OutputPropertyValuePtr propertyValue,
                                            const Snapshot * snapshot,
                                            const Interface::Formation * formation,
                                            const Interface::Surface * surface );

   void printDebugMsg ( const string outputMsg,
                        const DataModel::AbstractProperty  * property,
                        const DataModel::AbstractFormation * formation,
                        const DataModel::AbstractSurface   * surface,
                        const DataModel::AbstractSnapshot  * snapshot );

   void removeProperties (  const Snapshot * snapshot,
                            SnapshotFormationSurfaceOutputPropertyValueMap & allOutputPropertyValues );
   }
#endif // DERIVED_PROPERTIES__OUTPUT_UTILITIES_HH
