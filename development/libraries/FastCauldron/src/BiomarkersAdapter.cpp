#include "BiomarkersAdapter.h"
#include "timefilter.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

OutputPropertyMap* allocateBiomarkersAdapter ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {

   return new DerivedOutputPropertyMap<BiomarkersAdapter>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateBiomarkersVolumeAdapter ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Snapshot* snapshot ) {

   return new DerivedOutputPropertyMap<BiomarkersVolumeAdapter>( property, formation, snapshot );
}

BiomarkersAdapter::BiomarkersAdapter ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   if ( m_formation->getTopSurface () == surface ) {
      // Number-of-nodes = number-of-elements + 1.
      // But C array indexing starts a 0, so 1 must be subtracted.
      m_kIndex = m_formation->getMaximumNumberOfElements ();
   } else if ( m_formation->getBottomSurface () == surface ) {
      m_kIndex = 0;
   } else {
      assert ( false );
      // Error
   }

   m_isCalculated = false;

}

bool BiomarkersAdapter::operator ()( const OutputPropertyMap::OutputPropertyList& ,
                                                        OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;

   Interface::GridMap* hopaneIsomerisationMap;
   Interface::GridMap* steraneIsomerisationMap;
   Interface::GridMap* steraneAromatisationMap;

   double ***hopaneIsomerisationVector;
   double ***steraneIsomerisationVector;
   double ***steraneAromatisationVector;

   double undefinedValue;

   const bool isSediment = m_formation->isSediment();

   if ( isSediment ) {
      DMDAVecGetArray ( m_formation->layerDA,
                        m_formation->m_HopaneIsomerisation,
                        &hopaneIsomerisationVector );

      DMDAVecGetArray ( m_formation->layerDA,
                        m_formation->m_SteraneIsomerisation,
                        &steraneIsomerisationVector );

      DMDAVecGetArray ( m_formation->layerDA,
                        m_formation->m_SteraneAromatisation,
                        &steraneAromatisationVector );
   }

   hopaneIsomerisationMap = propertyValues [ 0 ]->getGridMap ();
   hopaneIsomerisationMap->retrieveData ();
   undefinedValue = hopaneIsomerisationMap->getUndefinedValue ();

   steraneIsomerisationMap = propertyValues [ 1 ]->getGridMap ();
   steraneIsomerisationMap->retrieveData ();

   steraneAromatisationMap = propertyValues [ 2 ]->getGridMap ();
   steraneAromatisationMap->retrieveData ();

   for ( i = hopaneIsomerisationMap->firstI (); i <= hopaneIsomerisationMap->lastI (); ++i ) {

      for ( j = hopaneIsomerisationMap->firstJ (); j <= hopaneIsomerisationMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j ) && isSediment ) {
            hopaneIsomerisationMap->setValue( i, j, hopaneIsomerisationVector [ m_kIndex ][ j ][ i ] );
            steraneIsomerisationMap->setValue( i, j, steraneIsomerisationVector [ m_kIndex ][ j ][ i ] );
            steraneAromatisationMap->setValue( i, j, steraneAromatisationVector [ m_kIndex ][ j ][ i ] );

         } else {
            hopaneIsomerisationMap->setValue(  i, j, undefinedValue );
            steraneIsomerisationMap->setValue( i, j, undefinedValue );
            steraneAromatisationMap->setValue( i, j, undefinedValue );
         }

      }

   }

   hopaneIsomerisationMap->restoreData ();
   steraneIsomerisationMap->restoreData ();
   steraneAromatisationMap->restoreData ();

   if ( isSediment )
   {
      DMDAVecRestoreArray ( m_formation->layerDA,
                            m_formation->m_HopaneIsomerisation,
                            & hopaneIsomerisationVector);
      DMDAVecRestoreArray ( m_formation->layerDA,
                            m_formation->m_SteraneIsomerisation,
                            & steraneIsomerisationVector);
      DMDAVecRestoreArray ( m_formation->layerDA,
                            m_formation->m_SteraneAromatisation,
                            & steraneAromatisationVector);
   }

   m_isCalculated = true;
   return true;
}

void BiomarkersAdapter::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "HopaneIsomerisation",
                                                                                                         m_snapshot, 0,
                                                                                                         0,
                                                                                                         m_surface )));

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "SteraneIsomerisation",
                                                                                                         m_snapshot, 0,
                                                                                                         0,
                                                                                                         m_surface )));

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "SteraneAromatisation",
                                                                                                         m_snapshot, 0,
                                                                                                         0,
                                                                                                         m_surface )));

}

bool BiomarkersAdapter::initialise ( OutputPropertyMap::PropertyValueList&  ) {
   return true;
}


BiomarkersVolumeAdapter::BiomarkersVolumeAdapter ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

bool BiomarkersVolumeAdapter::operator ()( const OutputPropertyMap::OutputPropertyList& ,
                                                              OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;

   Interface::GridMap* hopaneIsomerisationMap;
   Interface::GridMap* steraneIsomerisationMap;
   Interface::GridMap* steraneAromatisationMap;

   double ***hopaneIsomerisationVector;
   double ***steraneIsomerisationVector;
   double ***steraneAromatisationVector;

   double undefinedValue;
   const bool isSediment = m_formation->isSediment();

   if ( isSediment ) {
      DMDAVecGetArray ( m_formation->layerDA,
                        m_formation->m_HopaneIsomerisation,
                        &hopaneIsomerisationVector );

      DMDAVecGetArray ( m_formation->layerDA,
                        m_formation->m_SteraneIsomerisation,
                        &steraneIsomerisationVector );

      DMDAVecGetArray ( m_formation->layerDA,
                        m_formation->m_SteraneAromatisation,
                        &steraneAromatisationVector );
   }

   hopaneIsomerisationMap = propertyValues [ 0 ]->getGridMap ();
   hopaneIsomerisationMap->retrieveData ();
   undefinedValue = hopaneIsomerisationMap->getUndefinedValue ();

   steraneIsomerisationMap = propertyValues [ 1 ]->getGridMap ();
   steraneIsomerisationMap->retrieveData ();

   steraneAromatisationMap = propertyValues [ 2 ]->getGridMap ();
   steraneAromatisationMap->retrieveData ();

   for ( i = hopaneIsomerisationMap->firstI (); i <= hopaneIsomerisationMap->lastI (); ++i ) {

      for ( j = hopaneIsomerisationMap->firstJ (); j <= hopaneIsomerisationMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j ) && isSediment ) {

            for ( k = hopaneIsomerisationMap->firstK (); k <= hopaneIsomerisationMap->lastK (); ++k ) {
               hopaneIsomerisationMap->setValue( i, j, hopaneIsomerisationVector [ k ][ j ][ i ] );
               steraneIsomerisationMap->setValue( i, j, steraneIsomerisationVector [ k ][ j ][ i ] );
               steraneAromatisationMap->setValue( i, j, steraneAromatisationVector [ k ][ j ][ i ] );
            }

         } else {

            for ( k = hopaneIsomerisationMap->firstK (); k <= hopaneIsomerisationMap->lastK (); ++k ) {
               hopaneIsomerisationMap->setValue(  i, j, k, undefinedValue );
               steraneIsomerisationMap->setValue( i, j, k, undefinedValue );
               steraneAromatisationMap->setValue( i, j, k, undefinedValue );
            }

         }

      }

   }

   hopaneIsomerisationMap->restoreData ();
   steraneIsomerisationMap->restoreData ();
   steraneAromatisationMap->restoreData ();

   if ( isSediment ) {
      DMDAVecRestoreArray ( m_formation->layerDA,
                            m_formation->m_HopaneIsomerisation,
                            & hopaneIsomerisationVector);
      DMDAVecRestoreArray ( m_formation->layerDA,
                            m_formation->m_SteraneIsomerisation,
                            & steraneIsomerisationVector);
      DMDAVecRestoreArray ( m_formation->layerDA,
                            m_formation->m_SteraneAromatisation,
                            & steraneAromatisationVector);
   }

   m_isCalculated = true;
   return true;
}

void BiomarkersVolumeAdapter::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HopaneIsomerisation",
                                                                                                            m_snapshot, 0,
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "SteraneIsomerisation",
                                                                                                            m_snapshot, 0,
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "SteraneAromatisation",
                                                                                                            m_snapshot, 0,
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));

}

bool BiomarkersVolumeAdapter::initialise ( OutputPropertyMap::PropertyValueList&  ) {
   return true;
}
