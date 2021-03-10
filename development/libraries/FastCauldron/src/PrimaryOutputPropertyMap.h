#ifndef _FASTCAULDRON_PRIMARY_OUTPUT_PROPERTY_MAP_H_
#define _FASTCAULDRON_PRIMARY_OUTPUT_PROPERTY_MAP_H_

#include "property_manager.h"
#include "layer.h"
#include "Property.h"
#include "OutputPropertyMap.h"
#include "PropertyValue.h"

#include "Surface.h"
#include "Snapshot.h"
#include "GridMap.h"


class PrimaryOutputPropertyMap : public OutputPropertyMap {

public :

   PrimaryOutputPropertyMap ( const PropertyIdentifier         propertyName,
                                    LayerProps*          formation,
                              const Interface::Surface*  surface,
                              const Interface::Snapshot* snapshot );

   bool isCalculated () const;

   bool calculate ();

   bool initialise ();

   void finalise ();

private :

   int m_kIndex;
   bool m_isCalculated;

};

OutputPropertyMap* allocatePrimaryPropertyCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );


#endif // _FASTCAULDRON_PRIMARY_OUTPUT_PROPERTY_MAP_H_
