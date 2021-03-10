#ifndef _FASTCAULDRON_DERIVED_OUTPUT_PROPERTY_MAP_FACTORY_H_
#define _FASTCAULDRON_DERIVED_OUTPUT_PROPERTY_MAP_FACTORY_H_

#include <map>

#include "OutputPropertyMap.h"
#include "DerivedOutputPropertyMap.h"
#include "timefilter.h"

#include "Surface.h"
#include "Snapshot.h"
#include "GridMap.h"

/// A function-pointer type for allocating property maps.
typedef OutputPropertyMap* (*DerivedOutputPropertyMapAllocator)( const PropertyIdentifier&        property,
                                                                 LayerProps*                formation,
                                                                 const Interface::Surface*  surface,
                                                                 const Interface::Snapshot* snapshot );

/// A function-pointer type for allocating property volumes.
typedef OutputPropertyMap* (*DerivedOutputPropertyVolumeAllocator)( const PropertyIdentifier&        property,
                                                                    LayerProps*                formation,
                                                                    const Interface::Snapshot* snapshot );


enum OutputPropertyMapAssociation {
   SURFACE_ASSOCIATION,            //!< Property is associated with a surface only, this is true for continuous properties such as temperature and pressure.
   FORMATION_ASSOCIATION,          //!< Property is associated with a formation only, this is true for properties such as thickness.
   SURFACE_FORMATION_ASSOCIATION,  //!< Property is associated with both a surface and formation, this is true for properties that have a dependancy on the formation-lithology, such as porosity.
   UNKNOWN_ASSOCIATION };          //!< Association is not known.

/// \var NumberOfAssociations
/// The number of property-associations, including the unknown-association.
const int NumberOfAssociations = 4;

// Change name, because its not only about derived property maps now.
class DerivedOutputPropertyMapFactory {

   struct MapPropertyTraits {
      DerivedOutputPropertyMapAllocator m_propertyAllocator;
      OutputPropertyMapAssociation      m_outputAssociation;
      bool                              m_isPrimaryProperty;
   };

   struct VolumePropertyTraits {
      DerivedOutputPropertyVolumeAllocator m_propertyAllocator;
      bool                                 m_isPrimaryProperty;
   };

   /// \typedef MapPropertyTraitsMap
   /// Mapping from a property-list (enum) to the map-property-traits struct.
   typedef std::map<PropertyIdentifier, MapPropertyTraits> MapPropertyTraitsMap;

   /// \typedef VolumePropertyTraitsMap
   /// Mapping from a property-list (enum) to the volume-property-traits struct.
   typedef std::map<PropertyIdentifier, VolumePropertyTraits> VolumePropertyTraitsMap;

public :

   DerivedOutputPropertyMapFactory ();

   /// Allocates a derived property.
   ///
   /// If property is not found then a null pointer will be returned.
   OutputPropertyMap* allocateMap ( const PropertyIdentifier&        derivedProperty,
                                          LayerProps*          formation,
                                    const Interface::Surface*  surface,
                                    const Interface::Snapshot* snapshot ) const;

   /// Allocates a derived property volume.
   ///
   /// If property is not found then a null pointer will be returned.
   OutputPropertyMap* allocateVolume ( const PropertyIdentifier&        derivedProperty,
                                             LayerProps*          formation,
                                       const Interface::Snapshot* snapshot ) const;

   /// Return whether or not the property is on the list of map properties.
   bool isMapDefined ( const PropertyIdentifier& derivedProperty ) const;

   /// Return whether or not the property is on the list of volume properties.
   bool isVolumeDefined ( const PropertyIdentifier& derivedProperty ) const;


   /// Get the output-range for the property.
   ///
   /// If the property is not defined then unknown-output will be returned.
   OutputPropertyMapAssociation getMapAssociation ( const PropertyIdentifier& derivedProperty ) const;

private :

   MapPropertyTraitsMap m_mapPropertyTraitsMap;
   VolumePropertyTraitsMap m_volumePropertyTraitsMap;

};


#endif // _FASTCAULDRON_DERIVED_OUTPUT_PROPERTY_MAP_FACTORY_H_
