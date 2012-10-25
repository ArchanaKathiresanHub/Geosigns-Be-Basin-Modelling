/// Class PropertiesReader inherits from DataReader and
/// creates its own version of readData and readRecord to 
/// specifically read property info

#include "propertiesreader.h"
#include "createuserrequest.h"
#include "globalstrings.h"
using namespace Graph_Properties;

//
// PUBLIC METHODS
//
/// PropertiesReader::readData opens a project file and the trap Io tbl
/// and calls the base class readTable function
string PropertiesReader::readData(const char* filename)
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
// PRIVATE METHODS
//
/// PropertiesReader::readRecord extracts property data from a record
void PropertiesReader::readRecord (Record *rec)
{
   // get reference to trap group
   LineGroup& lineGroup = (plotLines()) [*m_currReservoir][m_persisIt->first];
   
   // read properties
   double age = m_transIt->first - (2*m_transIt->first);
   lineGroup[Pressure].add (age, getPressure (rec));
   lineGroup[Temperature].add (age, getTemperature (rec));
   lineGroup[VolumeOil].add (age, getVolumeOil (rec));
   lineGroup[VolumeGas].add (age, getVolumeGas (rec));
   lineGroup[TrapCapacity].add (age, getTrapCapacity (rec));
}

/// PropertiesReader::initLineData initialise line graph objects with enough
/// memory for property data to be later read
void PropertiesReader::initLineData (int size)
{
   LineGroup* lineGroup;
   
   // loop reservoirs and persis Ids initialising group lines within each
   for ( m_resIt = m_resData->begin(); m_resIt != m_endResIt; ++m_resIt )
   {  
      m_persisTrapData = &(m_resIt->second);
      m_endPersisIt = m_persisTrapData->end();
      m_persisIt = m_persisTrapData->begin();
      
      for ( m_persisIt = m_persisTrapData->begin(); m_persisIt != m_endPersisIt; ++m_persisIt )
     {
         lineGroup = &((plotLines()) [m_resIt->first][m_persisIt->first]);
         ((*lineGroup)[Pressure]).setSize (size);
         ((*lineGroup)[Temperature]).setSize (size);
         ((*lineGroup)[VolumeOil]).setSize (size);
         ((*lineGroup)[VolumeGas]).setSize (size);
         ((*lineGroup)[TrapCapacity]).setSize (size);
      }
   }
}

