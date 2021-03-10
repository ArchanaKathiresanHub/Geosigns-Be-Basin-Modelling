#ifndef _FASTCAULDRON_BASEMENT_PROPERTY_CALCULATOR_H_
#define _FASTCAULDRON_BASEMENT_PROPERTY_CALCULATOR_H_

#include "layer.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Snapshot.h"
#include "GridMap.h"

class BasementPropertyCalculator : public OutputPropertyMap  {

public :

   BasementPropertyCalculator (  const PropertyIdentifier         propertyName, 
                                 LayerProps               * formation, 
                                 const Interface::Surface * surface, 
                                 const Interface::Snapshot* snapshot );

   bool isCalculated () const;

   bool calculate ();
   bool calculateProperty ();

   bool initialise ();

   void finalise ();

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;
   // string m_propertyName;

   int m_kIndex;
   bool m_isCalculated;

   /// Other dependencies.
   AppCtx*     m_BasinModel;
};


OutputPropertyMap* allocateBasementPropertyCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

#endif // _FASTCAULDRON_BASEMENT_PROPERTY_CALCULATOR_H_
