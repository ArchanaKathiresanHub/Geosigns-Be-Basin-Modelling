#include "SourceRocksTemporalInterpolator.h"


SourceRocksTemporalInterpolator::SourceRocksTemporalInterpolator ( Subdomain& subdomain ) 
{
   int i, j, k;
  
   Subdomain::ActiveSourceRockLayerIterator iter;
   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {

      FormationSubdomainElementGrid & formationGrid = * iter;
      
      SourceTermTemporalInterpolator * stermInterpolator = new SourceTermTemporalInterpolator ( * iter );

      const ElementVolumeGrid& concentrationGrid = iter->getVolumeGrid ( NumberOfPVTComponents );
      
      LayerProps& srLayer = formationGrid.getFormation ();
      bool depthRetrieved = srLayer.Current_Properties.propertyIsActivated ( Basin_Modelling::Depth );
      bool genexRetrieved = srLayer.genexDataIsRetrieved ();

      if ( not depthRetrieved ) {
         srLayer.Current_Properties.Activate_Property ( Basin_Modelling::Depth, INSERT_VALUES, true );
      }
      
      if ( not genexRetrieved ) {
         srLayer.retrieveGenexData ();
      }
      
      for ( i = concentrationGrid.firstI (); i <= concentrationGrid.lastI (); ++i ) {
         
         for ( j = concentrationGrid.firstJ (); j <= concentrationGrid.lastJ (); ++j ) {
            
            if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {
               
               for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
                  stermInterpolator->initialize( formationGrid.getElement ( i, j, k ).getLayerElement() );
               }
            }
         }
      }
      if ( not genexRetrieved ) {
         // Restore back to original state.
         srLayer.restoreGenexData ();
      }
      
      if ( not depthRetrieved ) {
         // Restore back to original state.
         srLayer.Current_Properties.Restore_Property ( Basin_Modelling::Depth );
      }
      
      // */
      m_sourceTermInterpolators[ &formationGrid ] = stermInterpolator;
      
      ++ iter;
   }
}

SourceRocksTemporalInterpolator::~SourceRocksTemporalInterpolator () 
{
   std::map<const FormationSubdomainElementGrid *, SourceTermTemporalInterpolator*>::iterator it;

   for ( it = m_sourceTermInterpolators.begin (); it != m_sourceTermInterpolators.end (); ++it ) {
      delete it->second;
   }
}

const SourceTermTemporalInterpolator * SourceRocksTemporalInterpolator::accessSourceRock ( const FormationSubdomainElementGrid * formationGrid ) const
{ 
   std::map<const FormationSubdomainElementGrid *, SourceTermTemporalInterpolator*>::const_iterator iter = m_sourceTermInterpolators.find ( formationGrid );
   
   if ( iter == m_sourceTermInterpolators.end ()) {
      return 0;
   } else {
      return iter->second;
   }
   return 0;
}

