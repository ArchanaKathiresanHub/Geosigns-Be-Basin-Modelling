/// Class ComponensReader inherits from DataReader
/// and redefines some of the protected virtual methods
/// that handle tables reading details

#include "componentsreader.h"
#include "createuserrequest.h"

#include "globalstrings.h"
using namespace Graph_Properties;

// library references
#include "ComponentManager.h"
using namespace CBMGenerics;

//
// PUBLIC METHODS
//

/// ComponentsReader::readData opens the current project file (if its not already open),
/// opens the TrapIoTbl if not already open, initialises data for table reading and then reads the table
string ComponentsReader::readData(const char* filename)
{ 
   string error;

   // open project file
   if ( ! openProject (filename) ) 
   {
      error = "Error opening database file: ";
      error += filename;
      error += "\n";
   }

   // get table
   Table *trapIoTbl = getTable (TrapIoTbl);
   if ( trapIoTbl == NULL ) 
   {
      error += "Error opening table TrapIoTbl\n";
   }
   else
   {
      setupSearchIterators ();

      // initialise line data
      initLineData (trapIoTbl->size());

      // read table and retain relevant data
      readTable (trapIoTbl);
   }

   return error;
}

//
// PROTECTED METHODS
//

/// ComponentsReader::readRecord reads components masses from a record
void ComponentsReader::readRecord (Record *rec)
{
   // get reference to trap group
   LineGroup& lineGroup = (plotLines()) [*m_currReservoir][m_persisIt->first];
   ComponentManager &tcm = ComponentManager::getInstance();

   // read properties
   double age = m_transIt->first - (2*m_transIt->first);
   lineGroup[tcm.GetSpeciesName(ComponentManager::C1)].add (age, getMassC1 (rec));
   lineGroup[tcm.GetSpeciesName(ComponentManager::C2)].add (age, getMassC2 (rec));
   lineGroup[tcm.GetSpeciesName(ComponentManager::C3)].add (age, getMassC3 (rec));
   lineGroup[tcm.GetSpeciesName(ComponentManager::C4)].add (age, getMassC4 (rec));
   lineGroup[tcm.GetSpeciesName(ComponentManager::C5)].add (age, getMassC5 (rec));
   lineGroup[tcm.GetSpeciesName(ComponentManager::N2)].add (age, getMassN2 (rec));
   lineGroup[tcm.GetSpeciesName(ComponentManager::COx)].add (age, getMassCOx (rec));
   lineGroup[tcm.GetSpeciesName(ComponentManager::C6Minus14Aro)].add (age, getMassC6_14Aro (rec));
   lineGroup[tcm.GetSpeciesName(ComponentManager::C6Minus14Sat)].add (age, getMassC6_14Sat (rec));
   lineGroup[tcm.GetSpeciesName(ComponentManager::C15PlusAro)].add (age, getMassC15Aro (rec));
   lineGroup[tcm.GetSpeciesName(ComponentManager::C15PlusSat)].add (age, getMassC15Sat (rec));
   lineGroup[tcm.GetSpeciesName(ComponentManager::resin)].add (age, getMassresins  (rec));
   lineGroup[tcm.GetSpeciesName(ComponentManager::asphaltene)].add (age, getMassasphaltenes (rec));

   lineGroup[WetGas].add (age, getWetGas (rec));
   lineGroup[TotalGas].add (age, getTotalGas (rec));
   lineGroup[TotalOil].add (age, getTotalOil (rec)); 
}

/// ComponentsReader::initLineData allocates memory for line data 
/// (one line of data for each mass)
void ComponentsReader::initLineData (int size)
{
   LineGroup* lineGroup;
   ComponentManager &tcm = ComponentManager::getInstance();

   // loop reservoirs and persis Ids initialising group lines within each
   for ( m_resIt = m_resData->begin(); m_resIt != m_endResIt; ++m_resIt )
   {
      m_persisTrapData = &(m_resIt->second);
      m_endPersisIt = m_persisTrapData->end();
      for ( m_persisIt = m_persisTrapData->begin(); m_persisIt != m_endPersisIt; ++m_persisIt )
      {
         lineGroup = &((plotLines()) [m_resIt->first][m_persisIt->first]);

         ((*lineGroup)[tcm.GetSpeciesName(ComponentManager::C1)]).setSize (size);
         ((*lineGroup)[tcm.GetSpeciesName(ComponentManager::C2)]).setSize (size);
         ((*lineGroup)[tcm.GetSpeciesName(ComponentManager::C3)]).setSize (size);
         ((*lineGroup)[tcm.GetSpeciesName(ComponentManager::C4)]).setSize (size);
         ((*lineGroup)[tcm.GetSpeciesName(ComponentManager::C5)]).setSize (size);
         ((*lineGroup)[tcm.GetSpeciesName(ComponentManager::N2)]).setSize (size);
         ((*lineGroup)[tcm.GetSpeciesName(ComponentManager::COx)]).setSize (size);
         ((*lineGroup)[tcm.GetSpeciesName(ComponentManager::C6Minus14Aro)]).setSize (size);
         ((*lineGroup)[tcm.GetSpeciesName(ComponentManager::C6Minus14Sat)]).setSize (size);
         ((*lineGroup)[tcm.GetSpeciesName(ComponentManager::C15PlusSat)]).setSize (size);
         ((*lineGroup)[tcm.GetSpeciesName(ComponentManager::C15PlusAro)]).setSize (size);
         ((*lineGroup)[tcm.GetSpeciesName(ComponentManager::resin)]).setSize (size);
         ((*lineGroup)[tcm.GetSpeciesName(ComponentManager::asphaltene)]).setSize (size);

         ((*lineGroup)[WetGas]).setSize (size);
         ((*lineGroup)[TotalGas]).setSize (size);
         ((*lineGroup)[TotalOil]).setSize (size);        
      }
   }
}

//
// PRIVATE METHODS
//
double ComponentsReader::getTotalGas (Record *rec)
{
   return 
         getMassC1 (rec) +
         getMassC2 (rec) +
         getMassC3 (rec) +
         getMassC4 (rec) +
         getMassC5 (rec);
}

double ComponentsReader::getWetGas (Record *rec)
{
   return          
         getMassC2 (rec) +
         getMassC3 (rec) +
         getMassC4 (rec) +
         getMassC5 (rec);
}

double  ComponentsReader::getTotalOil (Record *rec)
{
  return 
         getMassC6_14Aro (rec) +
         getMassC6_14Sat (rec) +
         getMassC15Aro (rec) +
         getMassC15Sat (rec) +
         getMassresins (rec) +
         getMassasphaltenes (rec);
}
