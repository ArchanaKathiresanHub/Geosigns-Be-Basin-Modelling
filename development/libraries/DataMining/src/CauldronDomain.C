#include "CauldronDomain.h"

#include "Interface/Interface.h"
#include "NumericFunctions.h"

#include "PropertyInterpolator2D.h"


//------------------------------------------------------------//
DataAccess::Mining::CauldronDomain::CauldronDomain( Interface::ProjectHandle * handle ) 
   : m_projectHandle( handle )
{
   m_snapshot = 0;
   m_depthProperty = m_projectHandle->findProperty( "Depth" );
}

//------------------------------------------------------------//
DataAccess::Mining::CauldronDomain::~CauldronDomain()
{
   clear();
}

//------------------------------------------------------------//
void DataAccess::Mining::CauldronDomain::setSnapshot( const Interface::Snapshot *                snapshot,
                                                      DerivedProperties::DerivedPropertyManager& propertyManager )
{
   if ( m_snapshot == snapshot ) return;

   clear ();

   m_snapshot = snapshot;

   // Get all 3d depth grids.
   m_domainDerivedDepths = propertyManager.getFormationProperties ( m_depthProperty, m_snapshot, true );
}

//------------------------------------------------------------//

void DataAccess::Mining::CauldronDomain::setPlaneElement( ElementPosition & element, double x, double y ) const
{
   const Interface::Grid * activityGrid = m_projectHandle->getActivityOutputGrid();
   if ( !activityGrid ) activityGrid = m_projectHandle->getLowResolutionOutputGrid();

   const double originX   = activityGrid->minI();
   const double originY   = activityGrid->minJ();

   const double endPointX = activityGrid->maxI();
   const double endPointY = activityGrid->maxJ();

   const double deltaX    = activityGrid->deltaI();
   const double deltaY    = activityGrid->deltaJ();

   bool foundElement = false;

   if ( NumericFunctions::inRange( x, originX, endPointX ) and NumericFunctions::inRange( y, originY, endPointY ) )
   {
      unsigned int i = (unsigned int)(std::floor( ( x - originX ) / deltaX ) ) + activityGrid->firstI();
      unsigned int j = (unsigned int)(std::floor( ( y - originY ) / deltaY ) ) + activityGrid->firstJ();

      double elementOriginX;
      double elementOriginY;

      if ( activityGrid->getPosition( i, j, elementOriginX, elementOriginY ) )
      {
         foundElement = true;
         double xi  = 2.0 * ( x - elementOriginX ) / deltaX - 1.0;
         double eta = 2.0 * ( y - elementOriginY ) / deltaY - 1.0;

         element.setMapPosition( i, j );
         element.getReferencePoint()( 0 ) = xi;
         element.getReferencePoint()( 1 ) = eta;

         element.getActualPoint()( 0 ) = x;
         element.getActualPoint()( 1 ) = y;
      }
   }

   if ( !foundElement ) element.clear();
}

//------------------------------------------------------------//
bool DataAccess::Mining::CauldronDomain::findLocation( double x, double y, double z, ElementPosition & element ) const
{
   bool elementFound = false;

   setPlaneElement( element, x, y );

   if ( element.isValidPlaneElement() )
   {
      PropertyInterpolator2D interpolate2D;

      element.getActualPoint()( 2 ) = z;

      double topDepth    = Interface::DefaultUndefinedMapValue;
      double bottomDepth = Interface::DefaultUndefinedMapValue;

      int count = 0;


      for ( DerivedProperties::FormationPropertyList::const_iterator depthIter = m_domainDerivedDepths.begin(); depthIter != m_domainDerivedDepths.end(); ++depthIter )
      {

         DerivedProperties::FormationPropertyPtr grid = *depthIter;

         if ( count == 0 ) { 
            topDepth = interpolate2D ( element, grid, 0 );
         }

         bottomDepth = interpolate2D( element, grid, grid->lastK ());

         if ( topDepth == Interface::DefaultUndefinedMapValue or bottomDepth == Interface::DefaultUndefinedMapValue )
         {
            break;
         }

         if ( NumericFunctions::inRange( z, topDepth, bottomDepth ) )
         {
            topDepth = interpolate2D( element, grid, 0 );

            for ( unsigned int l = 0; l < grid->lengthK () - 1; ++l )
            {
               bottomDepth = interpolate2D( element, grid, l + 1 );

               if ( topDepth < bottomDepth and NumericFunctions::inRange( z, topDepth, bottomDepth ) )
               {
                  elementFound = true;

                  element.getReferencePoint()( 2 ) = 2.0 * ( z - topDepth ) / ( bottomDepth - topDepth ) - 1.0;
                  element.setDepthPosition( count + l, l );

                  // The grid must be in the mapping.
                  element.setFormation( dynamic_cast<const Interface::Formation*>( grid->getFormation ()));
                  // element.setFormation( m_mapToFormation.find( grid )->second );
                  break;
               }
               topDepth = bottomDepth;
            } 
            break;
         }
         else
         {
            // Minus 1 because we do not want to include the end (bottom) point twice.
            count += grid->lengthK () - 1;
         }

         topDepth = bottomDepth;
      }

   }

   if ( not elementFound ) {
      element.clear ();
   }

   return elementFound;
}


bool DataAccess::Mining::CauldronDomain::findLocation( const Numerics::Point & p, ElementPosition & element ) const
{
   return findLocation( p( 0 ), p( 1 ), p( 2 ), element );
}

//------------------------------------------------------------//

bool DataAccess::Mining::CauldronDomain::findLocation( double x, double y, const Interface::Surface * surface, ElementPosition & element ) const
{
   bool elementFound = false;

   setPlaneElement( element, x, y );

   if ( surface != 0 and element.isValidPlaneElement() )
   {
      double surfaceDepth;
      PropertyInterpolator2D interpolate2D;

      if ( element.getI() != (unsigned int)(Interface::DefaultUndefinedMapValue ) )
      {
         int  count = 0;


         for ( DerivedProperties::FormationPropertyList::const_iterator depthIter = m_domainDerivedDepths.begin(); depthIter != m_domainDerivedDepths.end(); ++depthIter ) {

            DerivedProperties::FormationPropertyPtr grid = *depthIter;

            // The formation must be in the domain of the map, since the this grid was added at the same time!
            const Interface::Formation* formation = dynamic_cast<const Interface::Formation*>( grid->getFormation ());//m_mapToFormation.find( grid )->second;

            if ( formation->getTopSurface() == surface )
            {
               // Interpolate the depth at the (x,y) point on the surface.
               surfaceDepth = interpolate2D ( element, grid, 0 );

               if ( surfaceDepth != Interface::DefaultUndefinedMapValue )
               {
                  elementFound = true;

                  element.getReferencePoint()( 2 ) = -1.0;
                  element.setDepthPosition( count, 0 );

                  // The grid must be in the mapping.
                  element.setFormation( formation );
                  element.setSurface( surface );
                  element.getActualPoint()( 2 ) = surfaceDepth;
               }
               break;
            }
            else
            {
               count += grid->lengthK () - 1;
            }
         }

      }

   }
   
   if ( not elementFound ) { element.clear (); }
   
   return elementFound;
}


bool DataAccess::Mining::CauldronDomain::findLocation( double x, double y, const Interface::Formation * formation, ElementPosition & element ) const
{
   bool elementFound = false;

   setPlaneElement( element, x, y );

   if ( formation != 0 and element.isValidPlaneElement() )
   {
      double surfaceDepth;
      PropertyInterpolator2D interpolate2D;

      if ( element.getI() != (unsigned int)(Interface::DefaultUndefinedMapValue ) )
      {
         int  count = 0;

         for ( DerivedProperties::FormationPropertyList::const_iterator depthIter = m_domainDerivedDepths.begin(); depthIter != m_domainDerivedDepths.end(); ++depthIter ) {

            DerivedProperties::FormationPropertyPtr grid = *depthIter;

            // The formation must be in the domain of the map, since the this grid was added at the same time!
            if ( grid->getFormation () == formation )
            { 
               // Interpolate the depth at the (x,y) point on the surface.
               surfaceDepth = interpolate2D( element, grid, 0 );

               if ( surfaceDepth != Interface::DefaultUndefinedMapValue )
               {
                  elementFound = true;

                  element.getReferencePoint()( 2 ) = -1.0;
                  element.setDepthPosition( count, 0 );

                  // The grid must be in the mapping.
                  element.setFormation( formation );
                  element.getActualPoint()( 2 ) = surfaceDepth;
               }
               break;
            }
            else
            {
               count += grid->lengthK () - 1;
            }
         }
      }
   }
   
   if ( not elementFound ) { element.clear (); }
   
   return elementFound;
}


//------------------------------------------------------------//

void DataAccess::Mining::CauldronDomain::clear () {
   m_domainDerivedDepths.clear ();
}

//------------------------------------------------------------//

void DataAccess::Mining::CauldronDomain::getTopSurface( double x, double y, ElementPosition& element, bool includeBasement ) const
{
   setPlaneElement( element, x, y );

   bool foundSurface = false;

   if ( element.isValidPlaneElement() and m_domainDerivedDepths.size() != 0 ) 
   {

      DerivedProperties::FormationPropertyPtr grid = m_domainDerivedDepths [ 0 ];
      const Interface::Formation*             formation = dynamic_cast<const Interface::Formation*>( grid->getFormation ());
      const Interface::Surface*               topSurface = formation->getTopSurface ();

      if ( includeBasement or formation->kind() != Interface::BASEMENT_FORMATION )
      {
         double surfaceDepth;
         PropertyInterpolator2D interpolate2D;

         // Interpolate the depth at the (x,y) point on the surface.
         surfaceDepth = interpolate2D( element, grid, 0 );

         if ( surfaceDepth != Interface::DefaultUndefinedMapValue )
         {
            foundSurface = true;

            element.getReferencePoint ()( 2 ) = -1.0;
            element.setDepthPosition ( 0, 0 );

            // The grid must be in the mapping.
            element.setFormation ( formation );
            element.setSurface ( topSurface );
            element.getActualPoint ()( 2 ) = surfaceDepth;
         }

      }

   }   

   if ( not foundSurface ) {
      element.clear();
   }

}

//------------------------------------------------------------//
void DataAccess::Mining::CauldronDomain::getBottomSurface( double x, double y, ElementPosition & element, bool includeBasement ) const
{
   bool foundSurface = false;

   setPlaneElement ( element, x, y );

   if ( element.isValidPlaneElement() and m_domainDerivedDepths.size() != 0 and
        ( includeBasement or dynamic_cast<const Interface::Formation*>( m_domainDerivedDepths[ 0 ]->getFormation ())->kind () != Interface::BASEMENT_FORMATION )) {

      unsigned int bottomSedimentGridIndex = m_domainDerivedDepths.size() - 1;

      if ( dynamic_cast<const Interface::Formation*>( m_domainDerivedDepths[ m_domainDerivedDepths.size() - 1 ]->getFormation ())->kind () == Interface::BASEMENT_FORMATION )
      {
         // If the bottom formation is a basement-formation then so is the one above.
         // The bottom is the crust and above this is the crust.
         bottomSedimentGridIndex -= 2;
      }

      // How many nodes are above us?
      int count = 0;
      for ( unsigned int i = 0; i <= bottomSedimentGridIndex; ++i )
      {
         // Minus one because we do not want to count the end points twice.
         count += m_domainDerivedDepths [ i ]->lengthK () - 1;
      }

      DerivedProperties::FormationPropertyPtr grid = m_domainDerivedDepths[ bottomSedimentGridIndex ];
      const Interface::Formation*             formation = dynamic_cast<const Interface::Formation*>( grid->getFormation ());
      const Interface::Surface*               topSurface = formation->getTopSurface ();

      double surfaceDepth;
      PropertyInterpolator2D interpolate2D;

      // Set the k value before interpolating
      element.setDepthPosition( count, m_domainDerivedDepths [ bottomSedimentGridIndex ]->lengthK () - 1 );

      // Interpolate the depth at the (x,y) point on the surface.
      surfaceDepth = interpolate2D ( element, grid );

      if ( surfaceDepth != Interface::DefaultUndefinedMapValue )
      {
         foundSurface = true;
         element.getReferencePoint()( 2 ) = 1.0;

         element.setFormation( formation );
         element.setSurface( topSurface );
         element.getActualPoint()( 2 ) = surfaceDepth;
      }
   }   
   if ( not foundSurface ) { element.clear(); }
}

//------------------------------------------------------------//

bool DataAccess::Mining::CauldronDomain::isEqual( const ElementPosition & startElement
                                                , const ElementPosition & endElement
                                                , bool  captureInterFormationBoundary
                                                , bool  captureInterPlanarElementBoundary
                                                , bool  captureInterVerticalElementBoundary
                                                ) const
{

   if ( not captureInterFormationBoundary and
        not captureInterPlanarElementBoundary and
        not captureInterVerticalElementBoundary )
   {
      return true;
   }

   if (     captureInterFormationBoundary and
        not captureInterPlanarElementBoundary and
        not captureInterVerticalElementBoundary )
   {
      // Need only check for the same formation.
      return startElement.getFormation () == endElement.getFormation ();
   }

   if ( not captureInterFormationBoundary and
            captureInterPlanarElementBoundary and
        not captureInterVerticalElementBoundary )
   {
      // Need to check for the same formation and the same element in the plane.
      return startElement.getI () == endElement.getI () and
             startElement.getJ () == endElement.getJ ();
   }

   if (     captureInterFormationBoundary and
            captureInterPlanarElementBoundary and
        not captureInterVerticalElementBoundary )
   {
      // Need to check for the same formation and the same element in the plane.
      return startElement.getFormation () == endElement.getFormation () and
             startElement.getI () == endElement.getI () and
             startElement.getJ () == endElement.getJ ();
   }
   
   if ( not captureInterFormationBoundary and
        not captureInterPlanarElementBoundary and
            captureInterVerticalElementBoundary )
   {
      return startElement.getLocalK () == endElement.getLocalK () and
             startElement.getGlobalK () == endElement.getGlobalK ();
   }

   if (     captureInterFormationBoundary and
        not captureInterPlanarElementBoundary and
            captureInterVerticalElementBoundary )
   {
      // Check for the same element in the plane.
      // Is the formation check necessary here?
      return startElement.getFormation () == endElement.getFormation () and 
             startElement.getLocalK () == endElement.getLocalK ();
   }

   if ( not captureInterFormationBoundary and
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
   
   if ( captureInterFormationBoundary and
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
void DataAccess::Mining::CauldronDomain::addIntermediateElements( const CauldronWell      & well
                                                                , double                    startS
                                                                , const ElementPosition   & startElement
                                                                , double                    endS
                                                                , const ElementPosition   & endElement
                                                                , ElementPositionSequence & elements
                                                                , bool                      captureInterFormationBoundary
                                                                , bool                      captureInterPlanarElementBoundary
                                                                , bool                      captureInterVerticalElementBoundary
                                                                ) const
{
   const double epsilon = 1.0e-4;

   if ( not isEqual( startElement, endElement, captureInterFormationBoundary, captureInterPlanarElementBoundary, captureInterVerticalElementBoundary ) )
   {
      double topS;
      double bottomS;
      double rangeStartS = startS;
      double midS;

      ElementPosition e;
      ElementPosition rangeStartElement = startElement;

      bool converged;
      bool endOfSection = false;

      while ( not endOfSection )
      {
         topS = rangeStartS;
         bottomS = endS;

         converged = false;

         // Find interface.
         while ( not converged )
         {
            midS = 0.5 * ( topS + bottomS );
            findLocation ( well.getLocation ( midS ), e );
            converged = std::abs ( topS - bottomS ) < epsilon;

            if ( isEqual( rangeStartElement, e, captureInterFormationBoundary, captureInterPlanarElementBoundary, captureInterVerticalElementBoundary ) )
            {
               topS = midS;
            }
            else
            {
               bottomS = midS;
            }
         }

         // Now that the interface has been found add the elements to the element sequence, if necessary.

         // The tolerance here must be greater than the tolerance used in the bisection
         // algorithm. The bottomS will be within 1 epsilon but the top can be an additional epsilon.
         if ( std::abs( topS - endS ) > 2.1 * epsilon )
         {
            findLocation ( well.getLocation ( topS ), e );
            e.setDepthAlongHole ( topS );

            if ( e.getFormation () != 0 )
            {
               elements.push_back ( e );
            }
         }

         if ( std::abs( bottomS - endS ) > epsilon )
         {
            findLocation ( well.getLocation ( bottomS ), e );
            e.setDepthAlongHole ( bottomS );

            if ( e.getFormation () != 0 )
            {
               elements.push_back ( e );
            }

            rangeStartS = bottomS;
            rangeStartElement = e;
         }
         else
         {
            endOfSection = true;
         }
      }
   }
}

//------------------------------------------------------------//

void DataAccess::Mining::CauldronDomain::findWellPath( const CauldronWell&       well
                                                     , ElementPositionSequence & elements
                                                     , bool                      captureInterFormationBoundary
                                                     , bool                      captureInterPlanarElementBoundary
                                                     , bool                      captureInterVerticalElementBoundary
                                                     ) const
{

   const double tolerance = 1.0e-4;

   double deltaS = well.getSamplingResolution ();
   double startS;
   double endS = 0.0;
   double usedEndS;
   double length = well.getLength ();

   ElementPosition startElement;
   ElementPosition endElement;

   do
   {
      startS = endS;
      endS += deltaS;

      usedEndS = NumericFunctions::Minimum ( endS, length ) - tolerance;

      findLocation ( well.getLocation ( startS ), startElement );
      startElement.setDepthAlongHole ( startS );
      findLocation ( well.getLocation ( usedEndS ), endElement );
      endElement.setDepthAlongHole ( usedEndS );

      if ( startElement.getFormation () != 0 )
      {
         elements.push_back ( startElement );
      }

      addIntermediateElements ( well, startS, startElement, usedEndS, endElement, elements,
                                captureInterFormationBoundary,
                                captureInterPlanarElementBoundary,
                                captureInterVerticalElementBoundary );

   } while ( endS < length );

   if ( endElement.getFormation () != 0 )
   {
      elements.push_back ( endElement );
   }
}

//-----------------------------------//-------------------------//
