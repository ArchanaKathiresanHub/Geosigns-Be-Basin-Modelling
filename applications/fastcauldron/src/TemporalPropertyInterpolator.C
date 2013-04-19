#include "TemporalPropertyInterpolator.h"

#include "ElementContributions.h"
#include "FastcauldronSimulator.h"
#include "FormationSubdomainElementGrid.h"
#include "SubdomainElement.h"
#include "LayerElement.h"

//------------------------------------------------------------//

void ElementTemporalPropertyInterpolator::set ( const double startValue,
                                                const double endValue ) {

   m_propertyStart = startValue;
   m_propertyEnd   = endValue;
}

//------------------------------------------------------------//

TemporalPropertyInterpolator::TemporalPropertyInterpolator ( Subdomain& sub ) : m_subdomain ( sub ), m_elementGrid ( sub.getVolumeGrid ( 2 )) {
   DACreateGlobalVector ( m_elementGrid.getDa (), &m_propertyVector );
   VecZeroEntries ( m_propertyVector );
   m_vectorRetrieved = false;
}

//------------------------------------------------------------//

TemporalPropertyInterpolator::TemporalPropertyInterpolator ( Subdomain& sub,
                                                             const Basin_Modelling::Fundamental_Property property ) : m_subdomain ( sub ), m_elementGrid ( sub.getVolumeGrid ( 2 )) {

   DACreateGlobalVector ( m_elementGrid.getDa (), &m_propertyVector );
   VecZeroEntries ( m_propertyVector );
   m_vectorRetrieved = false;
   setProperty ( property );
}

//------------------------------------------------------------//

TemporalPropertyInterpolator::~TemporalPropertyInterpolator () {

   if ( m_vectorRetrieved ) {
      m_propertyArray.restoreVector ( NO_UPDATE );
   }

   VecDestroy ( m_propertyVector );
}

//------------------------------------------------------------//

void TemporalPropertyInterpolator::computeTemporalProperty ( FormationSubdomainElementGrid&              formationGrid,
                                                             const Basin_Modelling::Fundamental_Property property,
                                                             InterpolatorArray&                          interpolator ) {

   int i;
   int j;
   int k;
   double startProperty;
   double endProperty;

   for ( i = formationGrid.firstI (); i <= formationGrid.lastI (); ++i ) {

      for ( j = formationGrid.firstJ (); j <= formationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.lastK (); k >= formationGrid.firstK (); --k ) {
               SubdomainElement& element = formationGrid ( i, j, k );
               const LayerElement& layerElement = element.getLayerElement ();

               ElementTemporalPropertyInterpolator& elementInterpolator = interpolator ( element.getK (), element.getJ (), element.getI ());

               startProperty = computePreviousProperty ( layerElement, property );
               endProperty = computeProperty ( layerElement, property );

               elementInterpolator.set ( startProperty, endProperty );
            }

         }

      }

   }

}

//------------------------------------------------------------//

void TemporalPropertyInterpolator::setProperty ( const Basin_Modelling::Fundamental_Property property ) {

   Subdomain::ActiveLayerIterator iter;

   m_subdomain.initialiseLayerIterator ( iter );

   if ( m_vectorRetrieved ) {
      m_propertyArray.restoreVector ( NO_UPDATE );
      m_vectorRetrieved = false;
   }

   m_propertyArray.setVector ( m_elementGrid, m_propertyVector, INSERT_VALUES );

   while ( not iter.isDone ()) {
      computeTemporalProperty ( *iter, property, m_propertyArray );
      ++iter;
   }

   m_propertyArray.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
   m_propertyArray.setVector ( m_elementGrid, m_propertyVector, INSERT_VALUES, true );
   m_vectorRetrieved = true;
}

//------------------------------------------------------------//
