#include "BrineProperties.h"
#include "CompoundLithology.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"
#include "element_contributions.h"
#include "FiniteElementTypes.h"

#include "LayerElement.h"
#include "PetscBlockVector.h"
#include "Saturation.h"
#include "ElementContributions.h"
#include "Lithology.h"

#include "Interface/RunParameters.h"
#include "Interface/Interface.h"
#include "consts.h"

#include "timefilter.h"

#define DEBUG

using namespace FiniteElementMethod;
using namespace pvtFlash;



OutputPropertyMap* allocateBrinePropertiesVolumeCalculator ( const ::PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<BrinePropertiesVolumeCalculator>( property, formation, snapshot );
}




//volume calculator 
//constructor
BrinePropertiesVolumeCalculator::BrinePropertiesVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {
   m_isCalculated = false;
}

BrinePropertiesVolumeCalculator::~BrinePropertiesVolumeCalculator() {
}

void BrinePropertiesVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {


 PropertyValue* phase;
 phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "BrineDensity", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 ));
 properties.push_back ( phase );

 
 phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "BrineViscosity", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 )); 
 properties.push_back ( phase );

 
}

bool BrinePropertiesVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues )
{
   m_temperature = PropertyManager::getInstance().findOutputPropertyVolume ( "Temperature", m_formation, m_snapshot );
   m_pressure = PropertyManager::getInstance().findOutputPropertyVolume ( "Pressure", m_formation, m_snapshot );
   m_lithologies = &m_formation->getCompoundLithologyArray ();
   m_fluid = m_formation->fluid;
   
   return true;
}

bool BrinePropertiesVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                     OutputPropertyMap::PropertyValueList&  propertyValues )
{
  
   using namespace CBMGenerics;

   if (m_isCalculated)
   {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   

   Interface::GridMap * brineDensityMap;
   Interface::GridMap * brineViscosityMap;

   const ElementVolumeGrid& grid = m_formation->getVolumeGrid ( Saturation::NumberOfPhases );

   double brineDensity = 0.0;
   double brineViscosity = 0.0;
   double temperature = 0.0;
   double pressure = 0.0;
   double undefinedValue;
  


   if ( not m_temperature->isCalculated ()) {

      if ( not m_temperature->calculate ()) {
         return false;
      } 

   }

   if ( not m_pressure->isCalculated ()) {

      if ( not m_pressure->calculate ()) {
         return false;
      } 

   }

   brineDensityMap = propertyValues[0]->getGridMap ();
   brineDensityMap->retrieveData ();
   brineViscosityMap = propertyValues[1]->getGridMap ();
   brineViscosityMap->retrieveData (); 

   undefinedValue =  brineDensityMap->getUndefinedValue ();

   for ( i = brineDensityMap->firstI (); i <= brineDensityMap->lastI (); ++i )
      {
         for ( j = brineDensityMap->firstJ (); j <= brineDensityMap->lastJ (); ++j )
            {
               for ( k = brineDensityMap->firstK (); k <= brineDensityMap->lastK (); ++k )
                  {
                     if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j ) and m_fluid != 0 )
                        {

                           // temperature in celcius
                           temperature = m_temperature->getVolumeValue (i, j, k);
                           // pressure in MPa
	                   pressure = m_pressure->getVolumeValue (i, j, k);
                           //Pressure in MPa, temp in C

                           brineDensity = m_fluid->density (temperature, pressure);
                           brineViscosity=m_fluid->viscosity(temperature, pressure);
                           
                           brineDensityMap->setValue(i,j,k,brineDensity);
                           brineViscosityMap->setValue(i,j,k,brineViscosity);
                        }
                     else
                        {
                           brineDensityMap->setValue(i,j,k,undefinedValue);
                           brineViscosityMap->setValue(i,j,k,undefinedValue);
                        }                           
                  }
            }
         
      }

   
   brineDensityMap->restoreData ();
   brineViscosityMap->restoreData ();
   m_isCalculated = true;

   return true;
}






