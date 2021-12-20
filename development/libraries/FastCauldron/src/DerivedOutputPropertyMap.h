#ifndef _FASTCAULDRON_DERIVED_OUTPUT_PROPERTY_MAP_H_
#define _FASTCAULDRON_DERIVED_OUTPUT_PROPERTY_MAP_H_

#include "layer.h"
#include "CompoundLithologyArray.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"

#include "Surface.h"
#include "Snapshot.h"
#include "GridMap.h"
#include "Formation.h"
#include "Interface.h"

//#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

/// Expects an interface:
///
///    For map data:
///    "property-calculator constructor" ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ); or
///
///    For volume data:
///    "property-calculator constructor" ( LayerProps* formation, const Interface::Snapshot* snapshot );
///
///
///    Allocates all the property-values that are required for output/computation of the property.
///    void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );
///
///
///    Compute the values of the property/properties.
///    bool operator ()( const OutputPropertyMap::PropertyList&      properties,
///                            OutputPropertyMap::PropertyValueList& propertyValues );
///
///
///    Initialise the property-calculator, get dependant properties, lithologies, fluids, ...
///    bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );
///
template<class PropertyCalculator>
class DerivedOutputPropertyMap : public OutputPropertyMap {

public :

   // Constructor for map data.
   DerivedOutputPropertyMap ( const PropertyIdentifier         propertyName,
                                    LayerProps*          formation,
                              const Interface::Surface*  surface,
                              const Interface::Snapshot* snapshot );

   // Constructor for volume data.
   DerivedOutputPropertyMap ( const PropertyIdentifier         propertyName,
                                    LayerProps*          formation,
                              const Interface::Snapshot* snapshot );

   /// Determine whether or not the property has been calculated.
   bool isCalculated () const;

   /// Calculate the property and retrieve the maps ready for use.
   bool calculate ();

   /// Initialise the property-calculator.
   bool initialise ();

   /// Finalise the property-calculator.
   ///
   /// Restore maps, delete any allocated memory, ...
   void finalise ();


private :

   /// The object that will calculate the derived property.
   PropertyCalculator m_propertyCalculator;

   /// Indicates whether or not the property has been calculated.
   bool               m_isCalculated;

   bool m_containsVolumeData;

};


template<class PropertyCalculator>
DerivedOutputPropertyMap<PropertyCalculator>::DerivedOutputPropertyMap ( const PropertyIdentifier         propertyName,
                                                                               LayerProps*          formation,
                                                                         const Interface::Surface*  surface,
                                                                         const Interface::Snapshot* snapshot) :
   OutputPropertyMap ( propertyName, formation, surface, snapshot ),
   m_propertyCalculator ( formation, surface, snapshot ) {

   unsigned int i;
   m_isCalculated = false;
   m_propertyCalculator.allocatePropertyValues ( m_values );

   for ( i = 0; i < m_values.size (); ++i ) {
      m_properties.push_back ((const Property*)m_values [ i ]->getProperty ());
   }

   m_containsVolumeData = false;
}

template<class PropertyCalculator>
DerivedOutputPropertyMap<PropertyCalculator>::DerivedOutputPropertyMap ( const PropertyIdentifier         propertyName,
                                                                               LayerProps*          formation,
                                                                         const Interface::Snapshot* snapshot) :
   OutputPropertyMap ( propertyName, formation, 0, snapshot ),
   m_propertyCalculator ( formation, snapshot ) {

   unsigned int i;
   m_isCalculated = false;
   m_propertyCalculator.allocatePropertyValues ( m_values );

   for ( i = 0; i < m_values.size (); ++i ) {
      m_properties.push_back ((const Property*)m_values [ i ]->getProperty ());
   }

   m_containsVolumeData = true;
}

template<class PropertyCalculator>
bool DerivedOutputPropertyMap<PropertyCalculator>::isCalculated () const {
   return m_isCalculated;
}

namespace property_manager
{
  OutputPropertyMap* FindOutputPropertyMap ( const std::string&         propertyName,
                                              const LayerProps*          formation,
                                              const Interface::Surface*  surface,
                                             const Interface::Snapshot* snapshot ) ;
}

template<class PropertyCalculator>
bool DerivedOutputPropertyMap<PropertyCalculator>::calculate ()
{
   unsigned int i;

   if ( not isCalculated () )
   {
     m_isCalculated = m_propertyCalculator ( m_properties, m_values );

     for ( i = 0; i < m_values.size (); ++i )
     {
       m_propertyMaps.push_back ( const_cast<Interface::GridMap*>(m_values [ i ]->getGridMap ()));
     }
   }

   for ( i = 0; i < m_propertyMaps.size (); ++i )
   {

      if ( not m_propertyMaps [ i ]->retrieved ())
      {
        m_propertyMaps [ i ]->retrieveGhostedData ();
      }
   }

   return m_isCalculated;
}

template<class PropertyCalculator>
bool DerivedOutputPropertyMap<PropertyCalculator>::initialise () {
   return m_propertyCalculator.initialise ( m_values );
}

template<class PropertyCalculator>
void DerivedOutputPropertyMap<PropertyCalculator>::finalise () {

   unsigned int i;

   for ( i = 0; i < m_propertyMaps.size (); ++i ) {
      m_propertyMaps [ i ]->restoreData ( true, true );
   }

}

#endif // _FASTCAULDRON_DERIVED_OUTPUT_PROPERTY_MAP_H_
