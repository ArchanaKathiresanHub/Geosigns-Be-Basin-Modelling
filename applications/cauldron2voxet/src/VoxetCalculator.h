#ifndef _VOXET_CALCULATOR_H_
#define _VOXET_CALCULATOR_H_

#include <iostream>
#include <istream>
#include <sstream>

#include "Interface.h"
#include "GridMap.h"
#include "Grid.h"
#include "../Interface/ProjectHandle.h"
#include "Snapshot.h"
#include "Property.h"
#include "PropertyValue.h"

using namespace DataAccess;
using namespace Interface;

#include "DepthInterpolator.h"
#include "VoxetDomainInterpolator.h"
#include "VoxetPropertyGrid.h"
#include "GridDescription.h"
#include "CauldronProperty.h"

class VoxetCalculator {


   class PropertyInterpolator {

   public :

      PropertyInterpolator ( const unsigned int nodesX,
                             const unsigned int nodesY,
                             const Property*    property );

      ~PropertyInterpolator ();

      void setSnapshot ( const ProjectHandle* projectHandle,
                         const Snapshot*      snapshot );

      const Property* getProperty () const;

      const Snapshot* getSnapshot () const;

      const PropertyValueList& getPropertyValues () const;

      const PropertyValue* getPropertyValue ( const unsigned int position ) const;

      VoxetDomainInterpolator& getInterpolator ();

   private :

      const Property*         m_property;
      const Snapshot*         m_snapshot;
      VoxetDomainInterpolator m_interpolators;
      PropertyValueList*      m_propertyValues;

   };


   typedef std::map<const Property*, PropertyInterpolator* > PropertyInterpolatorMap;


public :

   VoxetCalculator ( const ProjectHandle*   projectHandle,
                     const GridDescription& gridDescription );

   ~VoxetCalculator ();


   void setDepthProperty ( const Property* depth );

   void addProperty ( const Property* property );

   void deleteProperty ( const Property* property );

   int computeInterpolators ( const Snapshot * snapshot,
                              const bool   verbose = false );

   void computeProperty ( const CauldronProperty*  property,
                                VoxetPropertyGrid& propertyValues,
                          const bool               verbose = false );

   // If the property is null then the default map null value will be returned.
   float getNullValue ( const Property* property) const;

   float getNullValue ( );

   const GridDescription& getGridDescription () const;

   void print ( std::ostream& o ) const;


   VoxetDomainInterpolator& getVoxetDomainInterpolator ( const Property* property );
   VoxetDomainInterpolator& getAnyVoxetDomainInterpolator ();

private :

   static const float DefaultNullValue;

   int getMaximumNumberOfLayerNodes ( const PropertyValueList* depthPropertyValueList ) const;

   void initialiseInterpolators ( const PropertyValueList* depthPropertyValueList,
                                  const Snapshot*          snapshot,
                                  const bool               verbose );

   void calculatorInterpolatorValues ( const PropertyValueList* depthPropertyValueList,
                                       const bool               verbose );

   void setDefinedNodes ( const PropertyValueList* depthPropertyValueList);

   float interpolatedProperty ( const Grid*    grid,
                                const GridMap* property,
                                const unsigned int computedI,
                                const unsigned int computedJ,
                                const unsigned int k,
                                const float    x,
                                const float    y  ) const;

   /// Return whether the element {(i,j),(i+1,j),(i+1,j+1),(i,j+1)}, from the cauldron map, has all valid nodes.
   ///
   /// So:
   ///     0 <= i < number-I-Nodes - 1; and
   ///     0 <= j < number-J-Nodes - 1.
   bool validCauldronElement ( const int i, const int j ) const;

   const ProjectHandle*      m_projectHandle;

   const GridDescription     m_gridDescription;
   
   bool**                    m_nodeIsDefined;
   bool**                    m_interpolatorIsDefined;
   const Property*           m_depthProperty;
   PropertyInterpolatorMap   m_propertyInterpolators;

};


#endif // _VOXET_CALCULATOR_H_

