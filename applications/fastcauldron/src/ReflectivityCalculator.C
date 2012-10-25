#include "ReflectivityCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/Formation.h"


OutputPropertyMap* allocateReflectivityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<ReflectivityCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateReflectivityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<ReflectivityVolumeCalculator>( property, formation, snapshot );
}

//---ReflectivityCalculator--------------------------------------------------------------------------------------------------------


ReflectivityCalculator::ReflectivityCalculator ( LayerProps* layer, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_layer ( layer ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_velocity = 0;
   m_bulkDensity = 0;
   m_isCalculated = false;
   m_computeReflectivity = false;
   m_prescribedValue = 0.0;

}

bool ReflectivityCalculator::findPropertiesAbove ( std::vector<OutputPropertyMap*>& allThicknesses,
                                                   std::vector<OutputPropertyMap*>& allBulkDensities,
                                                   std::vector<OutputPropertyMap*>& allVelocities ) {

   const Interface::Surface* surfaceAbove = m_layer->getTopSurface ();
   const Interface::Formation* dalFormationAbove = surfaceAbove->getTopFormation ();
   LayerProps* fcFormationAbove = ( dalFormationAbove == 0 ? 0 : FastcauldronSimulator::getInstance ().findLayer ( dalFormationAbove->getName ()));

   OutputPropertyMap* velocity;
   OutputPropertyMap* bulkDensity;
   OutputPropertyMap* thickness;

   bool formationAboveFound = fcFormationAbove != 0 and fcFormationAbove->depositionStartAge > m_snapshot->getTime ();

   while ( formationAboveFound ) {

      thickness =   PropertyManager::getInstance().findOutputPropertyMap ( "Thickness",   fcFormationAbove, 0, m_snapshot );
      bulkDensity = PropertyManager::getInstance().findOutputPropertyMap ( "BulkDensity", fcFormationAbove, dalFormationAbove->getBottomSurface(), m_snapshot );
      velocity =    PropertyManager::getInstance().findOutputPropertyMap ( "Velocity",    fcFormationAbove, dalFormationAbove->getBottomSurface(), m_snapshot );

      if ( not thickness->isCalculated ()) {

         if ( not thickness->calculate ()) {
            return false;
         }

      }

      if ( not bulkDensity->isCalculated ()) {

         if ( not bulkDensity->calculate ()) {
            return false;
         }

      }

      if ( not velocity->isCalculated ()) {

         if ( not velocity->calculate ()) {
            return false;
         }

      }

      allThicknesses.push_back ( thickness );
      allBulkDensities.push_back ( bulkDensity );
      allVelocities.push_back ( velocity );

      // Find next formation above.

      surfaceAbove = fcFormationAbove->getTopSurface ();
      dalFormationAbove = surfaceAbove->getTopFormation ();
      fcFormationAbove = ( dalFormationAbove == 0 ? 0 : FastcauldronSimulator::getInstance ().findLayer ( dalFormationAbove->getName ()));

      formationAboveFound = fcFormationAbove != 0 and fcFormationAbove->depositionStartAge > m_snapshot->getTime ();

   }

   return true;
}


bool ReflectivityCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated or not m_computeReflectivity ) 
   {
      return true;
   }

   unsigned int i;
   unsigned int j;

   if ( not m_thickness->isCalculated ()) 
   {
      if ( not m_thickness->calculate ()) 
      {
         return false;
      } 
   }

   if ( not m_velocity->isCalculated ()) 
   {
      if ( not m_velocity->calculate ()) 
      {
         return false;
      } 
   }

   if ( not m_bulkDensity->isCalculated ()) 
   {
      if ( not m_bulkDensity->calculate ()) 
      {
         return false;
      } 
   }

   std::vector<OutputPropertyMap*> allThicknesses;
   std::vector<OutputPropertyMap*> allBulkDensities;
   std::vector<OutputPropertyMap*> allVelocities;
   unsigned int propertyIndex;

   if ( not findPropertiesAbove ( allThicknesses, allBulkDensities, allVelocities )) {
      allThicknesses.clear ();
      allBulkDensities.clear ();
      allVelocities.clear ();
      return false;
   }

   Interface::GridMap* reflectivityMap;
   reflectivityMap = propertyValues [ 0 ]->getGridMap ();
   reflectivityMap->retrieveData ();

   //loop over all gridpoints
   for ( i = reflectivityMap->firstI (); i <= reflectivityMap->lastI (); ++i ) 
   {
      for ( j = reflectivityMap->firstJ (); j <= reflectivityMap->lastJ (); ++j ) 
      {
         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j ) && (*m_thickness)( i, j ) > 0.0 )//defined and has thickness? 
         {
            //determine first formation above which has a thickness>0.0 and determine 
            //OutputPropertyMaps (velocityAbove, bulkDensityAbove)

            propertyIndex = 0;

            while ( propertyIndex < allThicknesses.size ()) {

               if ((*allThicknesses [ propertyIndex ])( i, j ) > 0.0 ) {
                  break;
               }

               ++propertyIndex;
            }

            double reflectivityIJ;

            if ( allThicknesses.size () == 0 or propertyIndex == allThicknesses.size ())
            {
               reflectivityIJ = m_prescribedValue;
               reflectivityMap->setValue ( i, j, reflectivityIJ );
               continue;
            }

            OutputPropertyMap* velocityAbove     = allVelocities [ propertyIndex ];
            OutputPropertyMap* bulkDensityAbove  = allBulkDensities [ propertyIndex ];

            //compute reflectivity

            double bulkDensityIJ      = (*m_bulkDensity)( i, j );
            double bulkDensityAboveIJ = (*bulkDensityAbove)( i, j );

            double velocityIJ      = (*m_velocity)( i, j );
            double velocityAboveIJ = (*velocityAbove)( i, j );
            
            reflectivityIJ  = ( bulkDensityAboveIJ * velocityAboveIJ - bulkDensityIJ * velocityIJ ) /
                              ( bulkDensityAboveIJ * velocityAboveIJ + bulkDensityIJ * velocityIJ );

            reflectivityMap->setValue ( i, j, reflectivityIJ );
         } 
      }
   }

   reflectivityMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void ReflectivityCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "ReflectivityVec2", 
                                                                                                         m_snapshot, 0, 
                                                                                                         m_layer,
                                                                                                         m_surface )));

}

bool ReflectivityCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   //get thickness, bulk density, velocity

   Interface::GridMap* reflectivityMap;
   reflectivityMap = propertyValues [ 0 ]->getGridMap ();
   
   if ( m_layer == 0 
        or m_layer->depoage <= m_snapshot->getTime () 
        or m_layer->kind() != Interface::SEDIMENT_FORMATION )
   {
      m_computeReflectivity = false;
      m_prescribedValue = reflectivityMap->getUndefinedValue();

      return true;
   }

   const Interface::Formation* formation = m_layer;
        
   if ( formation -> getBottomSurface () == m_surface ) 
   {
      m_computeReflectivity = false;
      m_prescribedValue = 0.0;
      return true;
   } 

   m_thickness = PropertyManager::getInstance().findOutputPropertyMap ( "Thickness", m_layer, 0, m_snapshot );
   
   m_bulkDensity = PropertyManager::getInstance().findOutputPropertyMap ( "BulkDensity", m_layer, m_surface, m_snapshot );

   m_velocity = PropertyManager::getInstance().findOutputPropertyMap ( "Velocity", m_layer, m_surface, m_snapshot );

   m_computeReflectivity = true;

   return m_bulkDensity != 0 and m_velocity != 0 and m_thickness != 0;
}



//---ReflectivityVolumeCalculator--------------------------------------------------------------------------------------------------------



ReflectivityVolumeCalculator::ReflectivityVolumeCalculator ( LayerProps* layer, const Interface::Snapshot* snapshot ) :
   m_layer ( layer ), m_snapshot ( snapshot ) {

   m_velocity = 0;
   m_bulkDensity = 0;
   m_isCalculated = false;
   m_computeReflectivity = false;
   m_prescribedValue = 0.0;

}

bool ReflectivityVolumeCalculator::findPropertiesAbove ( std::vector<OutputPropertyMap*>& allThicknesses,
                                                         std::vector<OutputPropertyMap*>& allBulkDensities,
                                                         std::vector<OutputPropertyMap*>& allVelocities ) {


   const Interface::Surface* surfaceAbove = m_layer->getTopSurface ();
   const Interface::Formation* dalFormationAbove = surfaceAbove->getTopFormation ();
   LayerProps* fcFormationAbove = ( dalFormationAbove == 0 ? 0 : FastcauldronSimulator::getInstance ().findLayer ( dalFormationAbove->getName ()));

   OutputPropertyMap* velocity;
   OutputPropertyMap* bulkDensity;
   OutputPropertyMap* thickness;

   bool formationAboveFound = fcFormationAbove != 0 and fcFormationAbove->depositionStartAge > m_snapshot->getTime ();

   while ( formationAboveFound ) {

      thickness =   PropertyManager::getInstance().findOutputPropertyMap ( "Thickness",   fcFormationAbove, 0, m_snapshot );
      bulkDensity = PropertyManager::getInstance().findOutputPropertyMap ( "BulkDensity", fcFormationAbove, dalFormationAbove->getBottomSurface(), m_snapshot );
      velocity =    PropertyManager::getInstance().findOutputPropertyMap ( "Velocity",    fcFormationAbove, dalFormationAbove->getBottomSurface(), m_snapshot );

      if ( not thickness->isCalculated ()) {

         if ( not thickness->calculate ()) {
            return false;
         }

      }

      if ( not bulkDensity->isCalculated ()) {

         if ( not bulkDensity->calculate ()) {
            return false;
         }

      }

      if ( not velocity->isCalculated ()) {

         if ( not velocity->calculate ()) {
            return false;
         }

      }

      allThicknesses.push_back ( thickness );
      allBulkDensities.push_back ( bulkDensity );
      allVelocities.push_back ( velocity );

      // Find next formation above.

      surfaceAbove = fcFormationAbove->getTopSurface ();
      dalFormationAbove = surfaceAbove->getTopFormation ();
      fcFormationAbove = ( dalFormationAbove == 0 ? 0 : FastcauldronSimulator::getInstance ().findLayer ( dalFormationAbove->getName ()));

      formationAboveFound = fcFormationAbove != 0 and fcFormationAbove->depositionStartAge > m_snapshot->getTime ();
   }

   return true;
}


bool ReflectivityVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated or not m_computeReflectivity ) 
   {
      return true;
   }

   unsigned int i;
   unsigned int j;

   if ( not m_thickness->isCalculated ()) 
   {
      if ( not m_thickness->calculate ()) 
      {
         return false;
      } 
   }

   if ( not m_velocity->isCalculated ()) 
   {
      if ( not m_velocity->calculate ()) 
      {
         return false;
      } 
   }

   if ( not m_bulkDensity->isCalculated ()) 
   {
      if ( not m_bulkDensity->calculate ()) 
      {
         return false;
      } 
   }

   Interface::GridMap* reflectivityMap;
   reflectivityMap = propertyValues [ 0 ]->getGridMap ();
   reflectivityMap->retrieveData ();

   std::vector<OutputPropertyMap*> allThicknesses;
   std::vector<OutputPropertyMap*> allBulkDensities;
   std::vector<OutputPropertyMap*> allVelocities;
   unsigned int propertyIndex;

   if ( not findPropertiesAbove ( allThicknesses, allBulkDensities, allVelocities )) {
      allThicknesses.clear ();
      allBulkDensities.clear ();
      allVelocities.clear ();
      return false;
   }

   //loop over all gridpoints
   for ( i = reflectivityMap->firstI (); i <= reflectivityMap->lastI (); ++i ) 
   {
      for ( j = reflectivityMap->firstJ (); j <= reflectivityMap->lastJ (); ++j ) 
      {
         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j ) && (*m_thickness)( i, j ) > 0.0 )//defined and has thickness? 
         {
            //determine first formation above which has a thickness>0.0 and determine 
            //OutputPropertyMaps (velocityAbove, bulkDensityAbove)
            
            propertyIndex = 0;

            while ( propertyIndex < allThicknesses.size ()) {

               if ((*allThicknesses [ propertyIndex ])( i, j ) > 0.0 ) {
                  break;
               }

               ++propertyIndex;
            }

            double reflectivityIJK;

            if ( allThicknesses.size () == 0 or propertyIndex == allThicknesses.size ())
            {
               reflectivityIJK = m_prescribedValue;
               continue;
            }

            OutputPropertyMap* velocityAbove     = allVelocities [ propertyIndex ];
            OutputPropertyMap* bulkDensityAbove  = allBulkDensities [ propertyIndex ];

            //compute reflectivity

            double bulkDensityIJ      = (*m_bulkDensity)( i, j );
            double bulkDensityAboveIJ = (*bulkDensityAbove)( i, j );

            double velocityIJ      = (*m_velocity)( i, j );
            double velocityAboveIJ = (*velocityAbove)( i, j );
            
            reflectivityIJK  = ( bulkDensityAboveIJ * velocityAboveIJ - bulkDensityIJ * velocityIJ ) /
                              ( bulkDensityAboveIJ * velocityAboveIJ + bulkDensityIJ * velocityIJ );

            reflectivityMap->setValue ( i, j, reflectivityMap->lastK (), reflectivityIJK );  //top of layer
            reflectivityMap->setValue ( i, j, reflectivityMap->firstK (), 0.0 );              //bottom of layer

         } 
      }
   }

   reflectivityMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void ReflectivityVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "Reflectivity", 
                                                                                                            m_snapshot, 0, 
                                                                                                            m_layer,
                                                                                                            m_layer->getMaximumNumberOfElements () + 1 )));

}

bool ReflectivityVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   //get thickness, bulk density, velocity

   Interface::GridMap* reflectivityMap;
   reflectivityMap = propertyValues [ 0 ]->getGridMap ();
   
   if ( m_layer == 0 
        or m_layer->depoage <= m_snapshot->getTime () 
        or m_layer->kind() != Interface::SEDIMENT_FORMATION )
   {
      m_computeReflectivity = false;
      m_prescribedValue = reflectivityMap->getUndefinedValue();

      return true;
   }

   m_thickness = PropertyManager::getInstance().findOutputPropertyMap ( "Thickness", m_layer, 0, m_snapshot );
   
   m_bulkDensity = PropertyManager::getInstance().findOutputPropertyMap ( "BulkDensity", m_layer, m_layer->getTopSurface(), m_snapshot );

   m_velocity = PropertyManager::getInstance().findOutputPropertyMap ( "Velocity", m_layer, m_layer->getTopSurface(), m_snapshot );

   m_computeReflectivity = true;

   return m_bulkDensity != 0 and m_velocity != 0 and m_thickness != 0;
}
