#ifndef _FASTCAULDRON_PRIMARY_OUTPUT_PROPERTY_MAP_H_
#define _FASTCAULDRON_PRIMARY_OUTPUT_PROPERTY_MAP_H_

#include "property_manager.h"
#include "layer.h"
#include "Property.h"
#include "OutputPropertyMap.h"
#include "PropertyValue.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"


class PrimaryOutputPropertyMap : public OutputPropertyMap {

public :

   PrimaryOutputPropertyMap ( const PropertyList         propertyName,
                                    LayerProps*          formation,
                              const Interface::Surface*  surface,
                              const Interface::Snapshot* snapshot );

   bool isCalculated () const;

   bool calculate ();

   bool initialise ();

   void finalise ();

   bool matches ( const LayerProps*          formation,
                  const Interface::Surface*  surface,
                  const Interface::Snapshot* snapshot,
                  const std::string&         propertyName ) const;


private :

   int m_kIndex;
   bool m_isCalculated;

};

OutputPropertyMap* allocatePrimaryPropertyCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );


#endif // _FASTCAULDRON_PRIMARY_OUTPUT_PROPERTY_MAP_H_
