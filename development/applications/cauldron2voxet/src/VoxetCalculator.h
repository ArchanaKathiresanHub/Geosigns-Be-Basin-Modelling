#ifndef _VOXET_CALCULATOR_H_
#define _VOXET_CALCULATOR_H_

#include <iostream>
#include <istream>
#include <sstream>

// Data model library.
#include "AbstractSnapshot.h"

// Data Access and GeoPhysics libraries.
#include "Interface/Interface.h"
#include "GeoPhysicsProjectHandle.h"

// Derived property library
#include "AbstractPropertyManager.h"
#include "DerivedPropertyManager.h"
#include "FormationProperty.h"
#include "SurfaceProperty.h"

namespace DataAccess { namespace Interface {
   class Property;
   class Snapshot;
   class PropertyValue;
   class ProjectHandle;
   class Grid;
   class GridMap;
} }


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

      void setSnapshot ( const GeoPhysics::ProjectHandle*           projectHandle,
                         DerivedProperties::DerivedPropertyManager& propertyManager,
                         const Snapshot*                            snapshot,
                         const bool                                 useBasement );

      const Property* getProperty () const;

      const Snapshot* getSnapshot () const;

      DerivedProperties::FormationPropertyPtr getDerivedProperty ( const unsigned int position ) const;

      VoxetDomainInterpolator& getInterpolator ();

   private :

      const Property*         m_property;
      const Snapshot*         m_snapshot;
      VoxetDomainInterpolator m_interpolators;

      DerivedProperties::FormationPropertyList m_derivedPropertyValues;

   };


   typedef std::map<const Property*, PropertyInterpolator* > PropertyInterpolatorMap;


public :

   VoxetCalculator ( const GeoPhysics::ProjectHandle*           projectHandle,
                     DerivedProperties::DerivedPropertyManager& propertyManager,
                     const GridDescription&                     gridDescription );

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

   bool & useBasement ();
   bool useBasement () const;

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

   int getMaximumNumberOfLayerNodes ( const DerivedProperties::FormationPropertyList& depthPropertyValueList ) const;

   void calculatorInterpolatorValues ( const DerivedProperties::FormationPropertyList& depthPropertyValueList,
                                       const bool                                      verbose );

   void initialiseInterpolators ( const DerivedProperties::FormationPropertyList& depthPropertyValueList,
                                  const Snapshot*                                 snapshot,
                                  const bool                                      verbose );

   void setDefinedNodes ( const DerivedProperties::FormationPropertyList& depthPropertyValueList);

   /// Return whether the element {(i,j),(i+1,j),(i+1,j+1),(i,j+1)}, from the cauldron map, has all valid nodes.
   ///
   /// So:
   ///     0 <= i < number-I-Nodes - 1; and
   ///     0 <= j < number-J-Nodes - 1.
   bool validCauldronElement ( const int i, const int j ) const;

   const GeoPhysics::ProjectHandle*           m_projectHandle;
   DerivedProperties::DerivedPropertyManager& m_propertyManager;
   const GridDescription                      m_gridDescription;
   
   bool**                    m_nodeIsDefined;
   bool**                    m_interpolatorIsDefined;
   const Property*           m_depthProperty;
   PropertyInterpolatorMap   m_propertyInterpolators;
   bool                      m_useBasement;


};


#endif // _VOXET_CALCULATOR_H_

