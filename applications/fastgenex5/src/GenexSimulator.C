
#ifdef sgi
  #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
     using namespace std;
      #define USESTANDARD
  #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
  #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   using namespace std;
   #define USESTANDARD
#endif // sgi

#include <algorithm>
#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
using namespace database;


#include "GenexSimulator.h"
#include "SourceRock.h"
using namespace GenexSimulation;

#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"
#include "Interface/SourceRock.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
using namespace DataAccess;

#include "ComponentManager.h"
#include "GenexResultManager.h"
using namespace Genex5;

GenexSimulator::GenexSimulator (database::Database * database, const std::string & name, const std::string & accessMode)
      : Interface::ProjectHandle (database, name, accessMode)
{
  registerProperties();
  m_adsorptionProjectHandle = 0;
}

GenexSimulator::~GenexSimulator (void)
{
   m_registeredProperties.clear();
   m_requestedProperties.clear();
}

GenexSimulator *GenexSimulator::CreateFrom(const std::string & inputFileName)
{
   return (GenexSimulator *) Interface::OpenCauldronProject (inputFileName, "rw");
}

bool GenexSimulator::saveTo (const std::string & outputFileName)
{
   return saveToFile (outputFileName);
}

void GenexSimulator::setAdsorptionSimulation ( AdsorptionProjectHandle* adsorptionProjectHandle ) {

   std::vector<Interface::SourceRock*>::iterator sourceRockIter;

   m_adsorptionProjectHandle = adsorptionProjectHandle;

   for (sourceRockIter = m_sourceRocks.begin(); sourceRockIter != m_sourceRocks.end(); ++sourceRockIter) { 
      ((SourceRock*)(*sourceRockIter))->setAdsorptionProjectHandle ( m_adsorptionProjectHandle ); 
   }

}


bool GenexSimulator::run(void)
{
   bool started = startActivity ("Genex5", getLowResolutionOutputGrid ());
  
   if (!started) return false;

   setRequestedOutputProperties();

   std::vector<Interface::SourceRock*>::iterator sourceRockIter;

   PetscLogDouble run_Start_Time;
  
  
   PetscGetTime(&run_Start_Time);

   for (sourceRockIter = m_sourceRocks.begin(); sourceRockIter != m_sourceRocks.end(); ++sourceRockIter)
   { 
       if(started) 
       {
          started =((SourceRock*)(*sourceRockIter))->compute(); 
       }
       else
       {
          finishActivity ();
          return false;
       }
      //if (started) started =(*sourceRockIter)->computeGenerationExpulsion(); 
   }

   PetscLogDouble run_End_Time;
   PetscGetTime(&run_End_Time);

   if(started)
   {
      if(GetRank() == 0)
      {
         PetscLogDouble totalSimulationTime =  (run_End_Time - run_Start_Time);
         
         int hours   = (int)( totalSimulationTime / 3600.0 );
         int minutes = (int)( ( totalSimulationTime - (hours * 3600.0) ) / 60.0 );
         int seconds = (int) ( totalSimulationTime - hours * 3600.0 - minutes * 60.0 );
         cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ "<<endl;
	 cout<<"Simulation Time: "<< hours <<" hours "<< minutes <<" minutes "<< seconds <<" seconds "<<endl;
	 cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ "<<endl;	 
	 cout<<"Saving Genex5 results to disk. Please wait... "<<endl;
	 cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ "<<endl;
      }
   }

   finishActivity (); 

   if(started)
   {
      if(GetRank() == 0)
      {    
	 cout<<"Results saved sucessfully."<<endl;
	 cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ "<<endl;
      }
   }

   return started; 
}
void GenexSimulator::setRequestedSpeciesOutputProperties()
{
   using namespace CBMGenerics;
   ComponentManager & theManager = ComponentManager::getInstance();

   int i;
   for (i = 0; i < ComponentManager::NumberOfOutputSpecies; ++i)
   {
      m_requestedProperties.push_back ( theManager.GetSpeciesOutputPropertyName( i ) );
   }

}
void GenexSimulator::setRequestedOutputProperties()
{
   //cumullative expulsion mass for all species is required
   setRequestedSpeciesOutputProperties();

   using namespace CBMGenerics;
   GenexResultManager & theResultManager = GenexResultManager::getInstance();
  
   Interface::ModellingMode theMode = getModellingMode();
   string theModellingMode = "3d";

   if( Interface::MODE1D == theMode )
   {
      theModellingMode = "1d";
   }
   
   Table * timeIoTbl = getTable ("FilterTimeIoTbl");
   Table::iterator tblIter;
   for (tblIter = timeIoTbl->begin (); tblIter != timeIoTbl->end (); ++tblIter)
   {
      Record * filterTimeIoRecord = * tblIter;
      const string & outPutOption = database::getOutputOption(filterTimeIoRecord);
      const string & modellingMode = database::getModellingMode(filterTimeIoRecord);
      const string & propertyName = database::getPropertyName (filterTimeIoRecord);

      if(outPutOption != "None" && modellingMode == theModellingMode)
      {
         if( isPropertyRegistered(propertyName) )
         {

            m_requestedProperties.push_back(propertyName);
            theResultManager.SetResultToggleByName(propertyName, true);

         }
      }
      else if ( 
               ( propertyName == theResultManager.GetResultName(GenexResultManager::HcGasExpelledCum) )
               ||( propertyName == theResultManager.GetResultName(GenexResultManager::OilExpelledCum  ) )
               && modellingMode == theModellingMode
               
                )
      {
         if( isPropertyRegistered(propertyName) )
         {
            m_requestedProperties.push_back(propertyName);
            theResultManager.SetResultToggleByName(propertyName, true);
         }
         
      }
   }
   sort(m_requestedProperties.begin(), m_requestedProperties.end());
}
void GenexSimulator::registerProperties()
{
   using namespace CBMGenerics;
   ComponentManager & theManager = ComponentManager::getInstance();
   GenexResultManager & theResultManager = GenexResultManager::getInstance();
   
   int i;
   for (i = 0; i < ComponentManager::NumberOfOutputSpecies; ++i)
   {
      m_registeredProperties.push_back ( theManager.GetSpeciesOutputPropertyName( i ) );
   }

   for(i = 0; i < GenexResultManager::NumberOfResults; ++i)
   {
      m_registeredProperties.push_back ( theResultManager.GetResultName( i ) );                      
   }  

   for (i = 0; i < ComponentManager::NumberOfOutputSpecies; ++i)
   {
      m_shaleGasProperties.push_back ( theManager.GetSpeciesOutputPropertyName ( i, true ));
   }


   // Adding all possible species that can be used in the adsorption process.
   for (i = 0; i < ComponentManager::NumberOfOutputSpecies; ++i)
   {
      m_shaleGasProperties.push_back ( theManager.GetSpeciesName ( i ) + "Retained" );
      m_shaleGasProperties.push_back ( theManager.GetSpeciesName ( i ) + "Adsorped" );
      m_shaleGasProperties.push_back ( theManager.GetSpeciesName ( i ) + "AdsorpedExpelled" );
      m_shaleGasProperties.push_back ( theManager.GetSpeciesName ( i ) + "AdsorpedFree" );
   }

   m_shaleGasProperties.push_back ( "HcSaturation" );
   m_shaleGasProperties.push_back ( "ImmobileWaterSat" );
   m_shaleGasProperties.push_back ( "FractionOfAdsorptionCap" );
   m_shaleGasProperties.push_back ( "HcVapourSat" );
   m_shaleGasProperties.push_back ( "HcLiquidSat" );

   m_shaleGasProperties.push_back ( "AdsorptionCapacity" );

   m_shaleGasProperties.push_back ( "RetainedGasVolumeST" );
   m_shaleGasProperties.push_back ( "RetainedOilVolumeST" );
   m_shaleGasProperties.push_back ( "GasExpansionRatio_Bg" );

   m_shaleGasProperties.push_back ( "Oil2GasGeneratedCumulative" );
   m_shaleGasProperties.push_back ( "TotalGasGeneratedCumulative" );

   m_shaleGasProperties.push_back ( "RetainedOilApiSR" );
   m_shaleGasProperties.push_back ( "RetainedCondensateApiSR" );

   m_shaleGasProperties.push_back ( "RetainedGorSR" );
   m_shaleGasProperties.push_back ( "RetainedCgrSR" );

   for ( i = 0; i < m_shaleGasProperties.size (); ++i ) {
      m_registeredProperties.push_back ( m_shaleGasProperties [ i ]);
   }

   sort(m_shaleGasProperties.begin(), m_shaleGasProperties.end());
   sort(m_registeredProperties.begin(), m_registeredProperties.end());
}

bool GenexSimulator::isShaleGasProperty (const string & propertyName)  const {
   return std::binary_search( m_shaleGasProperties.begin(), m_shaleGasProperties.end(), propertyName );
}
bool GenexSimulator::isPropertyRegistered(const string & propertyName) 
{
   return std::binary_search( m_registeredProperties.begin(), m_registeredProperties.end(), propertyName );
}
bool GenexSimulator::isPropertyRequested(const string & propertyName) const
{
   return std::binary_search( m_requestedProperties.begin(), m_requestedProperties.end(), propertyName );
}
const vector<string> & GenexSimulator::getRequestedProperties() const 
{
   return m_requestedProperties;
}
const vector<string> & GenexSimulator::getRegisteredProperties() const 
{
   return m_registeredProperties;
}
void GenexSimulator::deleteSourceRockPropertyValues()
{
   
   Interface::MutablePropertyValueList::iterator propertyValueIter;

   propertyValueIter = m_propertyValues.begin ();

   while (propertyValueIter != m_propertyValues.end ())
   {
      Interface::PropertyValue * propertyValue = * propertyValueIter;

      if ( isPropertyRegistered ( propertyValue->getProperty()->getName() ) )
      {
         propertyValueIter = m_propertyValues.erase (propertyValueIter);

         if (propertyValue->getRecord ())
         {
            propertyValue->getRecord ()->getTable ()->deleteRecord (propertyValue->getRecord ());
         }

         delete propertyValue;
      }
      else
      {
         ++propertyValueIter;
      }

   }

}
