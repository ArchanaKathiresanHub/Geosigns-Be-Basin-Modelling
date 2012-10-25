#include "CauldronDomain.h"

#include "Interface/Interface.h"
#include "NumericFunctions.h"

#include "PropertyInterpolator2D.h"


DataAccess::Mining::CauldronDomain::CauldronDomain ( Interface::ProjectHandle* handle ) : m_projectHandle ( handle ) {

   m_snapshot = 0;
   m_depthProperty = m_projectHandle->findProperty ( "Depth" );

   m_activityGrid = m_projectHandle->getLowResolutionOutputGrid ();
}

//------------------------------------------------------------//

DataAccess::Mining::CauldronDomain::~CauldronDomain () {

#if 0
   PropertyValueList::iterator gridIter;

   for ( gridIter = m_domainDepths.begin (); gridIter != m_domainDepths.end (); ++gridIter ) {
      const Interface::GridMap* grid = *gridIter;
      grid->release ();
   }

   m_domainDepths.clear ();
#endif

   clear ();
}

//------------------------------------------------------------//

void DataAccess::Mining::CauldronDomain::setSnapshot ( const Interface::Snapshot* snapshot ) {

   if (m_snapshot == snapshot) return;
   clear ();

   m_snapshot = snapshot;

   // Get all 3d depth grids.
   Interface::PropertyValueList* domainDepths = m_projectHandle->getPropertyValues ( FORMATION, m_depthProperty, m_snapshot, 0, 0, 0, VOLUME );
   Interface::PropertyValueList::const_iterator depthIter;

   for ( depthIter = domainDepths->begin (); depthIter != domainDepths->end (); ++depthIter ) {

      const Interface::PropertyValue* propertyValue = *depthIter;

      const Interface::GridMap* grid = propertyValue->getGridMap ();

      const Interface::Formation* formation = propertyValue->getFormation ();

      // Add depth grid to list.
      m_domainDepths.push_back ( propertyValue );

      // Create mapping from formation->depth-grid.
      m_formationToMap [ formation ] = propertyValue;

      // Create mapping from depth-grid->formation (the inverse of above).
      m_mapToFormation [ propertyValue ] = formation;

#if 0
      cout << " depth map : "  << propertyValue->getSnapshot ()->getTime () << "  "
           << propertyValue->getFormation ()->getName () << "  "
           << grid->getValue ( (unsigned int)(0), 0, 0 ) << "  " 
           << grid->getValue ( (unsigned int)(0), 0, grid->getDepth () - 1 ) << "  "
           << grid->getDepth ()
           << endl;
#endif

   }

   delete domainDepths;
}

//------------------------------------------------------------//

void DataAccess::Mining::CauldronDomain::setPlaneElement ( ElementPosition& element,
                                                           const double     x,
                                                           const double     y ) const {

   const double originX = m_activityGrid->minI ();
   const double originY = m_activityGrid->minJ ();

   const double endPointX = m_activityGrid->maxI ();
   const double endPointY = m_activityGrid->maxJ ();

   const double deltaX = m_activityGrid->deltaI ();
   const double deltaY = m_activityGrid->deltaJ ();

   if ( not NumericFunctions::inRange ( x, originX, endPointX ) or 
        not NumericFunctions::inRange ( y, originY, endPointY )) {

      element.clear ();

   } else {

      unsigned int i = (unsigned int)(std::floor (( x - originX ) / deltaX )) + m_activityGrid->firstI ();
      unsigned int j = (unsigned int)(std::floor (( y - originY ) / deltaY )) + m_activityGrid->firstJ ();

      double elementOriginX;
      double elementOriginY;

      if ( not m_activityGrid->getPosition ((unsigned int)(i), j, elementOriginX, elementOriginY )) {
         element.clear ();
      } else {

         double xi  = 2.0 * ( x - elementOriginX ) / deltaX - 1.0;
         double eta = 2.0 * ( y - elementOriginY ) / deltaY - 1.0;

         element.setMapPosition ( i, j );
         element.getReferencePoint ()( 0 ) = xi;
         element.getReferencePoint ()( 1 ) = eta;

         element.getActualPoint ()( 0 ) = x;
         element.getActualPoint ()( 1 ) = y;
      }

   }

}

//------------------------------------------------------------//

bool DataAccess::Mining::CauldronDomain::findLocation ( const double x,
                                                        const double y,
                                                        const double z,
                                                        ElementPosition& element ) const {

   bool elementFound = false;

   setPlaneElement ( element, x, y );

   if ( not element.isValidPlaneElement ()) {
      elementFound = false;
      element.clear ();
   } else {

      int count;
      unsigned int l;

      PropertyInterpolator2D interpolate2D;

      if ( element.isValidPlaneElement ()) {
         element.getActualPoint ()( 2 ) = z;

         PropertyValueList::const_iterator depthIter;

         double topDepth;
         double bottomDepth;

         for ( depthIter = m_domainDepths.begin (), count = 0; depthIter != m_domainDepths.end (); ++depthIter ) {
            const Interface::PropertyValue* grid = *depthIter;

            if ( count == 0 ) {
               topDepth = interpolate2D ( element, grid->getGridMap (), 0 );
            }

            bottomDepth = interpolate2D ( element, grid->getGridMap (), grid->getGridMap ()->getDepth () - 1 );

            if ( topDepth == Interface::DefaultUndefinedMapValue or bottomDepth == Interface::DefaultUndefinedMapValue ) {
               element.clear ();
               break;
            }

            if ( NumericFunctions::inRange ( z, topDepth, bottomDepth )) {

               topDepth = interpolate2D ( element, grid->getGridMap (), 0 );

               for ( l = 0; l < grid->getGridMap ()->getDepth () - 1; ++l ) {
                  bottomDepth = interpolate2D ( element, grid->getGridMap (), l + 1 );

                  if ( topDepth < bottomDepth and NumericFunctions::inRange ( z, topDepth, bottomDepth )) {
                     elementFound = true;

                     element.getReferencePoint ()( 2 ) = 2.0 * ( z - topDepth ) / ( bottomDepth - topDepth ) - 1.0;
                     element.setDepthPosition ( count + l, l );

                     // The grid must be in the mapping.
                     element.setFormation ( m_mapToFormation.find ( grid )->second );

                     break;
                  }

                  topDepth = bottomDepth;
               } 

               break;
            } else {
               // Minus 1 because we do not want to include the end (bottom) point twice.
               count += grid->getGridMap ()->getDepth () - 1;
            }

            topDepth = bottomDepth;
         }

         if ( not elementFound ) {
            element.clear ();
         }

      }

   }

   return elementFound;
}


bool DataAccess::Mining::CauldronDomain::findLocation ( const Numerics::Point&     p,
                                                        ElementPosition& element ) const {
   return findLocation ( p ( 0 ), p ( 1 ), p ( 2 ), element );
}

//------------------------------------------------------------//

bool DataAccess::Mining::CauldronDomain::findLocation ( const double x,
                                                        const double y,
                                                        const Interface::Surface* surface,
                                                        ElementPosition& element ) const {

   bool elementFound = false;

   setPlaneElement ( element, x, y );

   if ( surface == 0 or not element.isValidPlaneElement ()) {
      elementFound = false;
      element.clear ();
   } else {

      int count;
      double surfaceDepth;
      PropertyInterpolator2D interpolate2D;

      if ( element.getI () == (unsigned int)(Interface::DefaultUndefinedMapValue)) {
         element.clear ();
      } else {
         PropertyValueList::const_iterator depthIter;

         for ( depthIter = m_domainDepths.begin (), count = 0; depthIter != m_domainDepths.end (); ++depthIter ) {

            const Interface::PropertyValue* grid = *depthIter;

            // The formation must be in the domain of the map, since the this grid was added at the same time!
            const Interface::Formation* formation = m_mapToFormation.find ( grid )->second;

            if ( formation->getTopSurface () == surface ) {

               // Interpolate the depth at the (x,y) point on the surface.
               surfaceDepth = interpolate2D ( element, grid->getGridMap (), 0 );

               if ( surfaceDepth == Interface::DefaultUndefinedMapValue ) {
                  element.clear ();
               } else {

                  elementFound = true;

                  element.getReferencePoint ()( 2 ) = -1.0;
                  element.setDepthPosition ( count, 0 );

                  // The grid must be in the mapping.
                  element.setFormation ( formation );
                  element.setSurface ( surface );
                  element.getActualPoint ()( 2 ) = surfaceDepth;
               }

               break;
            } else {
               count += grid->getGridMap ()->getDepth () - 1;
            }

         }

         if ( not elementFound ) {
            element.clear ();
         }
      }
   }
   return elementFound;
}

//------------------------------------------------------------//

void DataAccess::Mining::CauldronDomain::clear () {
   m_domainDepths.clear ();
   m_formationToMap.clear ();
   m_mapToFormation.clear ();
}

//------------------------------------------------------------//

void DataAccess::Mining::CauldronDomain::getTopSurface ( const double x,
                                                         const double y,
                                                         ElementPosition& element,
                                                         bool includeBasement ) const {


   setPlaneElement ( element, x, y );

   if ( not element.isValidPlaneElement () or m_domainDepths.size () == 0 ) {
      element.clear ();
   } else {

      const Interface::PropertyValue* grid = m_domainDepths [ 0 ];
      const Interface::Formation* formation = m_mapToFormation.find ( grid )->second;
      const Interface::Surface* topSurface = formation->getTopSurface ();

      if ( not includeBasement and formation->kind () == Interface::BASEMENT_FORMATION ) {
         element.clear ();
      } else {
         double surfaceDepth;
         PropertyInterpolator2D interpolate2D;

         // Interpolate the depth at the (x,y) point on the surface.
         surfaceDepth = interpolate2D ( element, grid->getGridMap (), 0 );

         if ( surfaceDepth == Interface::DefaultUndefinedMapValue ) {
            element.clear ();
         } else {

            element.getReferencePoint ()( 2 ) = -1.0;
            element.setDepthPosition ( 0, 0 );

            // The grid must be in the mapping.
            element.setFormation ( formation );
            element.setSurface ( topSurface );
            element.getActualPoint ()( 2 ) = surfaceDepth;
         }

      }

   }   

}

//------------------------------------------------------------//

void DataAccess::Mining::CauldronDomain::getBottomSurface ( const double x,
                                                            const double y,
                                                            ElementPosition& element,
                                                            bool includeBasement ) const {

   setPlaneElement ( element, x, y );

   if ( not element.isValidPlaneElement () or m_domainDepths.size () == 0 ) {
      element.clear ();
   } else if ( not includeBasement and m_domainDepths [ 0 ]->getFormation ()->kind () == Interface::BASEMENT_FORMATION ) {
      element.clear ();
   } else {

      unsigned int bottomSedimentGridIndex;
      unsigned int i;
      int count = 0;

      if ( m_domainDepths [ m_domainDepths.size () - 1 ]->getFormation ()->kind () == Interface::BASEMENT_FORMATION ) {
         // If the bottom formation is a basement-formation then so is the one above.
         // The bottom is the crust and above this is the crust.
         bottomSedimentGridIndex = m_domainDepths.size () - 1 - 2;
      } else {
         bottomSedimentGridIndex = m_domainDepths.size () - 1;
      }

      // How many nodes are above us?
      for ( i = 0; i <= bottomSedimentGridIndex; ++i ) {
         // Minus one because we do not want to count the end points twice.
         count += m_domainDepths [ i ]->getGridMap ()->getDepth () - 1;
      }

      const Interface::PropertyValue* grid = m_domainDepths [ bottomSedimentGridIndex ];
      const Interface::Formation* formation = m_mapToFormation.find ( grid )->second;
      const Interface::Surface* topSurface = formation->getTopSurface ();

      double surfaceDepth;
      PropertyInterpolator2D interpolate2D;

      // Set the k value before interpolating
      element.setDepthPosition ( count, m_domainDepths [ bottomSedimentGridIndex ]->getGridMap ()->getDepth () - 1 );

      // Interpolate the depth at the (x,y) point on the surface.
      surfaceDepth = interpolate2D ( element, grid->getGridMap ());

      if ( surfaceDepth == Interface::DefaultUndefinedMapValue ) {
         element.clear ();
      } else {

         element.getReferencePoint ()( 2 ) = 1.0;

         element.setFormation ( formation );
         element.setSurface ( topSurface );
         element.getActualPoint ()( 2 ) = surfaceDepth;
      }

   }   


}

//------------------------------------------------------------//

bool DataAccess::Mining::CauldronDomain::isEqual ( const ElementPosition& startElement,
                                                   const ElementPosition& endElement,
                                                   const bool             captureInterFormationBoundary,
                                                   const bool             captureInterPlanarElementBoundary,
                                                   const bool             captureInterVerticalElementBoundary ) const {

   if ( not captureInterFormationBoundary and
        not captureInterPlanarElementBoundary and
        not captureInterVerticalElementBoundary )
   {
      return true;
   }
   else if ( captureInterFormationBoundary and
             not captureInterPlanarElementBoundary and
             not captureInterVerticalElementBoundary )
   {
      // Need only check for the same formation.
      return startElement.getFormation () == endElement.getFormation ();
   }
   else if ( not captureInterFormationBoundary and
             captureInterPlanarElementBoundary and
             not captureInterVerticalElementBoundary )
   {
      // Need to check for the same formation and the same element in the plane.
      return startElement.getI () == endElement.getI () and
             startElement.getJ () == endElement.getJ ();
   }
   else if ( captureInterFormationBoundary and
             captureInterPlanarElementBoundary and
             not captureInterVerticalElementBoundary )
   {
      // Need to check for the same formation and the same element in the plane.
      return startElement.getFormation () == endElement.getFormation () and
             startElement.getI () == endElement.getI () and
             startElement.getJ () == endElement.getJ ();
   }
   else if ( not captureInterFormationBoundary and
             not captureInterPlanarElementBoundary and
             captureInterVerticalElementBoundary )
   {
      return startElement.getLocalK () == endElement.getLocalK () and
             startElement.getGlobalK () == endElement.getGlobalK ();
   }
   else if ( captureInterFormationBoundary and
             not captureInterPlanarElementBoundary and
             captureInterVerticalElementBoundary )
   {
      // Check for the same element in the plane.
      // Is the formation check necessary here?
      return startElement.getFormation () == endElement.getFormation () and 
             startElement.getLocalK () == endElement.getLocalK ();
   }
   else if ( not captureInterFormationBoundary and
             captureInterPlanarElementBoundary and
             captureInterVerticalElementBoundary )
   {
      // Check for the same element in the plane.
      // Is the formation check necessary here?
      return startElement.getI () == endElement.getI () and
             startElement.getJ () == endElement.getJ () and
             startElement.getLocalK () == endElement.getLocalK () and
             startElement.getGlobalK () == endElement.getGlobalK ();
   }
   else if ( captureInterFormationBoundary and
             captureInterPlanarElementBoundary and
             captureInterVerticalElementBoundary )
   {
      // Check for the same element in the plane.
      // Is the formation check necessary here?
      return startElement.getFormation () == endElement.getFormation () and 
             startElement.getI () == endElement.getI () and
             startElement.getJ () == endElement.getJ () and
             startElement.getLocalK () == endElement.getLocalK ();
   }

#if 0
   bool formationMatches = true;
   bool planeMatches     = true;
   bool vecticalMatches  = true;

   if ( captureInterFormationBoundary ) {
      formationMatches = startElement.getFormation () == endElement.getFormation ();
   }

   if ( captureInterPlanarElementBoundary ) {
      planeMatches = startElement.getI () == endElement.getI () and startElement.getJ () == endElement.getJ ()
   }

   if ( captureInterVerticalElementBoundary ) {
      verticalMatches = startElement.getLocalK () == endElement.getLocalK () and startElement.getGlobalK () == endElement.getGlobalK ();
   }

   return formationMatches and planeMatches and verticalMatches;
#endif

   return true;
}

//------------------------------------------------------------//

void DataAccess::Mining::CauldronDomain::addIntermediateElements ( const CauldronWell&            well,
                                                                   const double                   startS,
                                                                   const ElementPosition&         startElement,
                                                                   const double                   endS,
                                                                   const ElementPosition&         endElement,
                                                                         ElementPositionSequence& elements,
                                                                   const bool                     captureInterFormationBoundary,
                                                                   const bool                     captureInterPlanarElementBoundary,
                                                                   const bool                     captureInterVerticalElementBoundary ) const {

   const double epsilon = 1.0e-4;

   if ( not isEqual ( startElement, endElement, captureInterFormationBoundary, captureInterPlanarElementBoundary, captureInterVerticalElementBoundary )) {
      double topS;
      double bottomS;
      double rangeStartS = startS;
      double midS;
      ElementPosition e;
      ElementPosition rangeStartElement = startElement;
      bool converged;
      bool endOfSection = false;

      while ( not endOfSection ) {
         topS = rangeStartS;
         bottomS = endS;

         converged = false;

         // Find interface.
         while ( not converged ) {
            midS = 0.5 * ( topS + bottomS );
            findLocation ( well.getLocation ( midS ), e );
            converged = std::abs ( topS - bottomS ) < epsilon;

            if ( isEqual ( rangeStartElement, e, captureInterFormationBoundary, captureInterPlanarElementBoundary, captureInterVerticalElementBoundary )) {
               topS = midS;
            } else {
               bottomS = midS;
            }

         }

         // Now that the interface has been found add the elements to the element sequence, if necessary.

         // The tolerance here must be greater than the tolerance used in the bisection
         // algorithm. The bottomS will be within 1 epsilon but the top can be an additional epsilon.
         if ( std::abs ( topS - endS ) > 2.1 * epsilon ) {
            findLocation ( well.getLocation ( topS ), e );
            e.setDepthAlongHole ( topS );

            if ( e.getFormation () != 0 ) {
               elements.push_back ( e );
            }

         }

         if ( std::abs ( bottomS - endS ) > epsilon ) {
            findLocation ( well.getLocation ( bottomS ), e );
            e.setDepthAlongHole ( bottomS );

            if ( e.getFormation () != 0 ) {
               elements.push_back ( e );
            }

            rangeStartS = bottomS;
            rangeStartElement = e;
         } else {
            endOfSection = true;
         }


      }


   }

}

//------------------------------------------------------------//

void DataAccess::Mining::CauldronDomain::findWellPath ( const CauldronWell&            well,
                                                              ElementPositionSequence& elements,
                                                        const bool                     captureInterFormationBoundary,
                                                        const bool                     captureInterPlanarElementBoundary,
                                                        const bool                     captureInterVerticalElementBoundary ) const {

   const double tolerance = 1.0e-4;

   double deltaS = well.getSamplingResolution ();
   double startS;
   double endS = 0.0;
   double usedEndS;
   double length = well.getLength ();

   ElementPosition startElement;
   ElementPosition endElement;

   do  {
      startS = endS;
      endS += deltaS;

      usedEndS = NumericFunctions::Minimum ( endS, length ) - tolerance;

      findLocation ( well.getLocation ( startS ), startElement );
      startElement.setDepthAlongHole ( startS );
      findLocation ( well.getLocation ( usedEndS ), endElement );
      endElement.setDepthAlongHole ( usedEndS );

      if ( startElement.getFormation () != 0 ) {
         elements.push_back ( startElement );
      }

      addIntermediateElements ( well, startS, startElement, usedEndS, endElement, elements,
                                captureInterFormationBoundary,
                                captureInterPlanarElementBoundary,
                                captureInterVerticalElementBoundary );

   } while ( endS < length );

   if ( endElement.getFormation () != 0 ) {
      elements.push_back ( endElement );
   }

}

//------------------------------------------------------------//
