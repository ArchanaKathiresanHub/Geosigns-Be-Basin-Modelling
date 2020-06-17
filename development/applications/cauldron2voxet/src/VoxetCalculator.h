//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _VOXET_CALCULATOR_H_
#define _VOXET_CALCULATOR_H_

#include <iostream>
#include <istream>
#include <sstream>

// Data model library.
#include "AbstractSnapshot.h"

// Data Access and GeoPhysics libraries.
#include "Interface.h"
#include "GeoPhysicsProjectHandle.h"

// Derived property library
#include "AbstractPropertyManager.h"
#include "DerivedPropertyManager.h"
#include "FormationProperty.h"
#include "SurfaceProperty.h"

#include "DepthInterpolator.h"
#include "VoxetDomainInterpolator.h"
#include "VoxetPropertyGrid.h"
#include "GridDescription.h"
#include "CauldronProperty.h"

namespace DataAccess { namespace Interface {
   class Property;
   class Snapshot;
   class PropertyValue;
   class ProjectHandle;
   class Grid;
   class GridMap;
} }

class VoxetCalculator {


   class PropertyInterpolator {

   public :

      PropertyInterpolator ( const unsigned int nodesX,
                             const unsigned int nodesY,
                             const DataAccess::Interface::Property* property );

      ~PropertyInterpolator ();

      void setSnapshot ( const GeoPhysics::ProjectHandle& projectHandle,
                         DerivedProperties::DerivedPropertyManager& propertyManager,
                         const DataAccess::Interface::Snapshot* snapshot,
                         const bool useBasement );

      const DataAccess::Interface::Property* getProperty () const;

      const DataAccess::Interface::Snapshot* getSnapshot () const;

      AbstractDerivedProperties::FormationPropertyPtr getDerivedProperty ( const unsigned int position ) const;

      VoxetDomainInterpolator& getInterpolator ();

   private :

      const DataAccess::Interface::Property* m_property;
      const DataAccess::Interface::Snapshot* m_snapshot;
      VoxetDomainInterpolator m_interpolators;

      AbstractDerivedProperties::FormationPropertyList m_derivedPropertyValues;

   };


   typedef std::map<const DataAccess::Interface::Property*, PropertyInterpolator* > PropertyInterpolatorMap;


public :

   VoxetCalculator ( const GeoPhysics::ProjectHandle& projectHandle,
                     DerivedProperties::DerivedPropertyManager& propertyManager,
                     const GridDescription& gridDescription,
                     const std::map<std::string, double >& propertyNullValueReplaceLookup = std::map<std::string, double >() );

   ~VoxetCalculator ();


   void setDepthProperty ( const DataAccess::Interface::Property* depth );

   void addProperty ( const DataAccess::Interface::Property* property );

   void deleteProperty ( const DataAccess::Interface::Property* property );

   int computeInterpolators ( const DataAccess::Interface::Snapshot * snapshot,
                              const bool verbose = false );

   void computeProperty ( const CauldronProperty* property,
                          VoxetPropertyGrid& propertyValues,
                          const bool verbose = false );

   // If the property is null then the default map null value will be returned.
   float getNullValue ( const DataAccess::Interface::Property* property) const;

   float getNullValue ( );

   const GridDescription& getGridDescription () const;

   bool & useBasement ();
   bool useBasement () const;

   void print ( std::ostream& o ) const;


   VoxetDomainInterpolator& getVoxetDomainInterpolator ( const DataAccess::Interface::Property* property );
   VoxetDomainInterpolator& getAnyVoxetDomainInterpolator ();

private :

   static const float DefaultNullValue;

   int getMaximumNumberOfLayerNodes ( const DataAccess::Interface::PropertyValueList* depthPropertyValueList ) const;

   void initialiseInterpolators ( const DataAccess::Interface::PropertyValueList* depthPropertyValueList,
                                  const DataAccess::Interface::Snapshot* snapshot,
                                  const bool verbose );

   void calculatorInterpolatorValues ( const DataAccess::Interface::PropertyValueList* depthPropertyValueList,
                                       const bool verbose );

   int getMaximumNumberOfLayerNodes ( const AbstractDerivedProperties::FormationPropertyList& depthPropertyValueList ) const;

   void calculatorInterpolatorValues ( const AbstractDerivedProperties::FormationPropertyList& depthPropertyValueList,
                                       const bool verbose );

   void initialiseInterpolators ( const AbstractDerivedProperties::FormationPropertyList& depthPropertyValueList,
                                  const DataAccess::Interface::Snapshot* snapshot,
                                  const bool verbose );

   void setDefinedNodes ( const AbstractDerivedProperties::FormationPropertyList& depthPropertyValueList,
                          const bool verbose);

   /// Return whether the element {(i,j),(i+1,j),(i+1,j+1),(i,j+1)}, from the cauldron map, has all valid nodes.
   ///
   /// So:
   ///     0 <= i < number-I-Nodes - 1; and
   ///     0 <= j < number-J-Nodes - 1.
   bool validCauldronElement ( const int i, const int j ) const;

   const GeoPhysics::ProjectHandle& m_projectHandle;
   DerivedProperties::DerivedPropertyManager& m_propertyManager;
   const GridDescription m_gridDescription;

   bool** m_nodeIsDefined;
   bool** m_interpolatorIsDefined;
   const DataAccess::Interface::Property* m_depthProperty;
   PropertyInterpolatorMap m_propertyInterpolators;
   bool m_useBasement;

   std::map<std::string, double > m_propertyNullValueReplaceLookup;

};


#endif // _VOXET_CALCULATOR_H_
