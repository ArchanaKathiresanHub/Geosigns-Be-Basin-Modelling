//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _DATAACCESS__DATA_MINING__CAULDRON_DOMAIN_H_
#define _DATAACCESS__DATA_MINING__CAULDRON_DOMAIN_H_

#include "Interface/Formation.h"
#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Property.h"
#include "Interface/Grid.h"
#include "Interface/PropertyValue.h"

#include "DerivedPropertyManager.h"

#include "Point.h"
#include "ElementPosition.h"
#include "CauldronWell.h"

#include <vector>
#include <map>

namespace DataAccess
{

   namespace Mining
   {
      /// Objects of this type contain the 3d depths for the basin,
      /// at the specified snapshot time.
      class CauldronDomain {

      public :

         CauldronDomain( Interface::ProjectHandle * handle );

         ~CauldronDomain();

         /// \brief Set the snapshot time.
         ///
         /// All previously read depth data is removed and the new
         /// depth data (for the specified snapshot time) is read.
         void setSnapshot( const Interface::Snapshot*                 snapshot,
                           DerivedProperties::DerivedPropertyManager& propertyManager );

         /// \brief Find the element/formation in which the the point (x,y,z) lies.
         ///
         /// Also sets the reference co-ordinates.
         bool findLocation( double x,
                            double y,
                            double z,
                            ElementPosition & element ) const;

         /// \brief Find the element/formation in which the the point p lies.
         ///
         /// Also sets the reference co-ordinates.
         bool findLocation( const Numerics::Point & p,
                            ElementPosition       & element ) const;

         /// \brief Find the element/surface/formation in which the the point (x,y)-surface lies.
         ///
         /// Also sets the reference co-ordinates.
         bool findLocation( double x,
                            double y,
                            const Interface::Surface * surface,
                            ElementPosition          & element ) const;

         /// \brief Find the element/formation in which the the point (x,y)-lies.
         ///
         /// Also sets the reference co-ordinates.
         bool findLocation( double x,
                            double y,
                            const Interface::Formation * surface,
                            ElementPosition            & element ) const;

         /// \brief Return the top surface of the cauldron model at the (x,y)-point.
         void getTopSurface( double x,
                             double y,
                             ElementPosition & element,
                             bool              includeBasement = false ) const;

         /// \brief Return the bottom surface of the cauldron model at the (x,y)-point.
         void getBottomSurface( double x,
                                double y,
                                ElementPosition & element,
                                bool              includeBasement = false ) const;

         /// \brief Find elements along a well path.
         ///
         /// This can be a vertical or deviated well.
         /// Do we also need the inter-formation or even inter-element boundaries?
         void findWellPath( const CauldronWell      & well,
                            ElementPositionSequence & elements,
                            bool                      captureInterFormationBoundary = true,
                            bool                      captureInterPlanarElementBoundary = true,
                            bool                      captureInterVerticalElementBoundary = false ) const;

      protected :

         /// \brief Remove all data from the object.
         void clear();

         /// \brief Set the actual (x,y)-coordinates, the (xi,eta)-reference coordinates and the (i,j)-global position of the element.
         /// 
         /// If the (x,y) is invalid then element will be cleared on exit.
         void setPlaneElement( ElementPosition& element, double x, double y ) const;

         /// \brief Add elements that lie in the open interval ( startS, endS ).
         ///
         /// The additions are subject to the conditions: 
         ///      -# Add inter-formation boundary elements.
         ///      -# Add inter-planar boundary elements, i.e. If the well path passes through the side faces of an 
         ///         element then the elements either side of the face will be added.
         ///      -# Add inter-vertical boundary elements, i.e. If the well path passes through 
         ///         the upper or lower face of an element then the elements either side of this face will be added.
         void addIntermediateElements( const CauldronWell      & well,
                                       double                    startS,
                                       const ElementPosition   & startElement,
                                       double                    endS,
                                       const ElementPosition   & endElement,
                                       ElementPositionSequence & elements,
                                       bool                      captureInterFormationBoundary,
                                       bool                      captureInterPlanarElementBoundary,
                                       bool                      captureInterVerticalElementBoundary ) const;


         /// \brief Determine if to elements are equal subject to the conditions.
         ///
         /// The conditions: 
         ///      -# Do elements share the same formation.
         ///      -# Do the elements share the same lateral position (i.e. i and j position).
         ///      -# Do the elements share the same vertical position (i.e. k position).
         bool isEqual( const ElementPosition & startElement,
                       const ElementPosition & endElement,
                       bool                    captureInterFormationBoundary,
                       bool                    captureInterPlanarElementBoundary,
                       bool                    captureInterVerticalElementBoundary ) const;

         Interface::ProjectHandle* m_projectHandle;

         ///  The snapshot the at which to retrieve the data.
         const Interface::Snapshot*    m_snapshot;

         /// The depth of the nodes, ...
         const Interface::Property*    m_depthProperty;

         /// The depth properties for the layers.
         AbstractDerivedProperties::FormationPropertyList m_domainDerivedDepths;


      };
   }
}

#endif // _DATAACCESS__DATA_MINING__CAULDRON_DOMAIN_H_
