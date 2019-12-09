#ifndef _FASTCAULDRON_PROPERTY_MANAGER_H_
#define _FASTCAULDRON_PROPERTY_MANAGER_H_

#include <vector>
#include <string>

#include "OutputPropertyMap.h"
#include "propinterface.h"
#include "layer.h"
#include "layer_iterators.h"
#include "DerivedOutputPropertyMapFactory.h"
#include "timefilter.h"

#include "Interface.h"

#include "Surface.h"
#include "Snapshot.h"
#include "GridMap.h"

#include "DerivedPropertyManager.h"

class PropertyManager {

  /// \typedef PropertyNameMap
  /// A mapping from the output-property name to the name that appears in the time-io table in the project file.
  typedef std::map <std::string, std::string> PropertyNameMap;

  /// \typedef OutputPropertyMapSet
  /// A list of output-property map pointers.
  typedef std::vector<OutputPropertyMap*> OutputPropertyMapSet;

  /// A two-dimensional array containing the output-property map-lists.
  ///
  /// Separating the single array to a two-d array of arrays removes some of the
  /// searching required when searching for specific output-property-maps.

  typedef std::map<OutputPropertyMapAssociation, std::map<PropertyIdentifier, OutputPropertyMapSet>> OutputPropertyMapSetArray;

public :

  static PropertyManager& getInstance ();

  /// Delete the property manager instance
  static void deleteInstance( );

  /// Maps the output-property name of the property-name.
  const std::string& findOutputPropertyName ( const std::string& name ) const;

  /// Maps the property name of the output-property-name.
  const std::string& findPropertyName ( const std::string& name ) const;

  /// Computes all map-properties on required-property list.
  ///
  // Appctx is required for the layers.
  void computeMapProperties ( AppCtx*                               cauldron,
                              const PropListVec&                    mapProperties,
                              const Interface::Snapshot*            snapshot,
                              const Interface::PropertyOutputOption maximumOutputOption );

  /// Computes map-properties for source rock layers.
  ///
  // Appctx is required for the layers.
  void computeSourceRockPropertyMaps ( AppCtx*                    cauldron,
                                       const Interface::Snapshot* snapshot,
                                       const PropListVec&         genexProperties,
                                       const PropListVec&         shaleGasProperties );

  /// Computes volume-properties for source rock layers.
  ///
  // Appctx is required for the layers.
  void computeSourceRockPropertyVolumes ( AppCtx*                    cauldron,
                                          const Interface::Snapshot* snapshot,
                                          const PropListVec&         genexProperties,
                                          const PropListVec&         shaleGasProperties );

  /// Computes all volume-properties on required-property list.
  ///
  // Appctx is required for the layers.
  void computeVolumeProperties ( AppCtx*                               cauldron,
                                 const PropListVec&                    volumeProperties,
                                 const Interface::Snapshot*            snapshot,
                                 const Interface::PropertyOutputOption maximumOutputOption );

  /// Computes all map- and volume-properties on required-property lists.
  ///
  // Appctx is required for the layers.
  void computeProperties ( AppCtx*                               cauldron,
                           const PropListVec&                    requiredPropertyMaps,
                           const PropListVec&                    requiredPropertyVolumes,
                           const Interface::Snapshot*            snapshot,
                           const Interface::PropertyOutputOption maximumOutputOption );

  /// Remove all output-property-maps that have been allocated.
  ///
  /// Note: neither the property-value nor the grid-map are deleted here.
  // Change name to be more descriptive.
  void clear ();


  /// Retrieve the output-property map for the given property-name, formation, surface and snapshot.
  OutputPropertyMap* findOutputPropertyMap ( const std::string&         propertyName,
                                             const LayerProps*          formation,
                                             const Interface::Surface*  surface,
                                             const Interface::Snapshot* snapshot ) const;

  /// Retrieve the output-property volume for the given property-name, formation, surface and snapshot.
  OutputPropertyMap* findOutputPropertyVolume ( const std::string&         propertyName,
                                                const LayerProps*          formation,
                                                const Interface::Snapshot* snapshot ) const;

  /// \brief Determine whether or not the property should be output for a given formation and surface
  ///
  /// \param [in]  formation The formation for which the property is to be calculated.
  /// \param [in]  surface   The surface for which the property is to be calculated.
  /// \pre surface   is not null and is a valid surface.
  /// \pre formation is not null and is a valid formation.
  bool allowOutput ( const string& propertyName, const Interface::Formation* formation, const Interface::Surface* surface ) const;

private :

  Interface::PropertyList getProperties(const PropertyIdentifier& requiredProperty );


  void computePrimaryPropertyMaps ( AppCtx*                               cauldron,
                                    PropListVec&                    requiredProperties,
                                    const Interface::Snapshot*            snapshot,
                                    const Interface::PropertyOutputOption maximumOutputOption );

  void computeDerivedPropertyMaps ( AppCtx*                               cauldron,
                                    const PropListVec&                    requiredProperties,
                                    const Interface::Snapshot*            snapshot,
                                    const Interface::PropertyOutputOption maximumOutputOption );

  void computePrimaryPropertyVolumes ( AppCtx*                               cauldron,
                                       PropListVec&                          requiredProperties,
                                       const Interface::Snapshot*            snapshot,
                                       const Interface::PropertyOutputOption maximumOutputOption );

  void computeDerivedPropertyVolumes ( AppCtx*                               cauldron,
                                       const PropListVec&                    requiredProperties,
                                       const Interface::Snapshot*            snapshot,
                                       const Interface::PropertyOutputOption maximumOutputOption );

  /// Computes the property specified.
  ///
  // Appctx is required for the layers.
  void computePropertyMaps ( AppCtx*                               cauldron,
                             const PropertyIdentifier& requiredProperty,
                             const Interface::Snapshot*            snapshot,
                             const Interface::PropertyOutputOption maximumOutputOption,
                             const bool isPrimary = true );

  /// Computes all properties on required-property list.
  ///
  // Appctx is required for the layers.
  void computePropertyVolumes ( AppCtx*                               cauldron,
                                const PropertyIdentifier&                   requiredProperty,
                                const Interface::Snapshot*            snapshot,
                                const Interface::PropertyOutputOption maximumOutputOption,
                                const bool isPrimary = true );


  /// Creates a surface property and adds it the to the m_mapProperties list.
  ///
  /// More than one output-property may be created here, if
  /// the formation is at the bottom of the sediments.
  /// createForTopSurfaceOnly applies only to sediment formations
  void createSurfacePropertyMaps ( const PropertyIdentifier&        requiredProperty,
                                   const Interface::Snapshot* snapshot,
                                   const LayerProps_Ptr       formation,
                                   const LayerProps_Ptr       formationBelow,
                                   const LayerProps_Ptr formationAbove,
                                   const bool                 createForTopSurfaceOnly,
                                   const bool isPrimary = true );

  /// Creates a formation property and adds it the to the m_mapProperties list.
  void createFormationMapPropertyMaps ( const PropertyIdentifier&        requiredProperty,
                                        const Interface::Snapshot* snapshot,
                                        const LayerProps_Ptr       formation,
                                        const bool isPrimary = true );

  /// Creates a surface-formation property and adds it the to the m_mapProperties list.
  ///
  /// createForTopSurfaceOnly applies only to sediment formations
  void createSurfaceFormationPropertyMaps ( const PropertyIdentifier&         requiredProperty,
                                            const Interface::Snapshot* snapshot,
                                            const LayerProps_Ptr       formation,
                                            const bool                 createForTopSurfaceOnly,
                                            const bool isPrimary = true );

  /// Initialises all of the 2d output-properties.
  // Better to have a more generic function using member-function pointers.
  void initialisePropertyMaps ();

  /// Calculates all of the 2d output-properties.
  void calculatePropertyMaps ();

  /// Finalises all of the 2d output-properties.
  void finalisePropertyMaps ();

  /// Initialises all of the 3d output-properties.
  void initialisePropertyVolumes ();

  /// Calculates all of the 3d output-properties.
  void calculatePropertyVolumes ();

  /// Finalises all of the 3d output-properties.
  void finalisePropertyVolumes ();

  void addMapPropertyMap ( const OutputPropertyMapAssociation association,
                           const PropertyIdentifier&                property,
                           OutputPropertyMap*           propertyMap );

  static PropertyManager* s_propertyManager;

  static std::string s_nullString;

  PropertyManager ();

  OutputPropertyMapSet m_mapProperties;
  OutputPropertyMapSet m_volumeProperties;
  PropertyNameMap m_propertyName2OutputName;
  DerivedOutputPropertyMapFactory m_derivedProperties;

  OutputPropertyMapSetArray m_allMapProperties;

  std::unique_ptr<DerivedProperties::DerivedPropertyManager> m_derivedPropertyManager;

  void createDerivedProperty(const PropertyIdentifier& requiredProperty, const Interface::Snapshot* snapshot, const LayerProps_Ptr formation, const Interface::Surface* surface);
};

// Some inline functions.

inline PropertyManager& PropertyManager::getInstance () {

  if ( s_propertyManager == 0 ) {
    s_propertyManager = new PropertyManager;
  }

  return *s_propertyManager;
}

inline void PropertyManager::deleteInstance( )
{
  if ( s_propertyManager ) delete s_propertyManager;
  s_propertyManager = 0;
}

namespace property_manager
{
//global access function
OutputPropertyMap* FindOutputPropertyMap ( const std::string&         propertyName,
                                           const LayerProps*          formation,
                                           const Interface::Surface*  surface,
                                           const Interface::Snapshot* snapshot ) ;

}



#endif // _FASTCAULDRON_PROPERTY_MANAGER_H_
