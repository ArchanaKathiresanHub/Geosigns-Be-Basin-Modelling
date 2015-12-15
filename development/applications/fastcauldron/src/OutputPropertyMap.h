#ifndef _FASTCAULDRON_OUTPUT_PROPERTY_MAP_H_
#define _FASTCAULDRON_OUTPUT_PROPERTY_MAP_H_

#include <string>
#include <vector>

#include "layer.h"
#include "Property.h"
#include "PropertyValue.h"
#include "timefilter.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"

/// 
/// The expected order of function call is initialise; calculate; operator (); finalise.
// Change name of this class, since it now also is a base class for the volume data.
class OutputPropertyMap {

public :

   typedef std::vector<const Property*> OutputPropertyList;

   typedef std::vector<PropertyValue*>  PropertyValueList;

   typedef std::vector<Interface::GridMap*> GridMapList;


   OutputPropertyMap ( const PropertyList         propertyName,
                             LayerProps*          formation,
                       const Interface::Surface*  surface,
                       const Interface::Snapshot* snapshot );

   virtual ~OutputPropertyMap () {}

   virtual bool isCalculated () const = 0;

   virtual bool calculate () = 0;

   virtual bool initialise () = 0;

   virtual void finalise () = 0;

   virtual bool isRequired ( const unsigned int position ) const;

   virtual bool anyIsRequired () const;

   /// Return the value of the property at the position.
   ///
   /// Some properties, e.g. permeability, are compute for both the vertical and the horizontal.
   /// The third parameter here determines which to return.
   virtual double operator ()( const unsigned int i, const unsigned int j, const unsigned int position = 0 ) const;

   virtual double getMapValue ( const unsigned int i, const unsigned int j, const unsigned int position = 0 ) const;

   virtual double getVolumeValue ( const unsigned int i, const unsigned int j, const unsigned int k, const unsigned int position = 0 ) const;


   unsigned int numberOfProperties () const;

   /// 
   virtual bool matches ( const std::string&         propertyName,
                          const LayerProps*          formation,
                          const Interface::Surface*  surface,
                          const Interface::Snapshot* snapshot ) const;

   PropertyList getPropertyName () const;

   /// Return the string representation of the object.
   ///
   /// Consists of the formation and surface names, the snapshot age and the property names.
   virtual std::string image () const;

protected :

   const PropertyList m_propertyName;

   LayerProps*                m_formation;
   const Interface::Surface*  m_surface;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyList m_properties;
   PropertyValueList  m_values;
   GridMapList        m_propertyMaps;

}; 

#endif // _FASTCAULDRON_OUTPUT_PROPERTY_MAP_H_
