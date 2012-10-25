/// Class SourceReader inhertits from DataReader and adds
/// specific functionality for reading component data
/// It also has its own componentsSelected variable that is set from 
/// the components graph when the user selectes a new components

#include "sourcereader.h"
#include "environmentsettings.h"

//
// PUBLIC METHODS
//
/// SourceReader::readData opens the MigrationIoTbl, creats an event tree to hold the migration 
/// event info and then extracts the data from the event tree relation to the user-selected trap
string SourceReader::readData(const char* filename)
{
   string error = "";

   // open project file
   if ( ! openProject (filename) ) 
   {
      error = "Error opening database file: ";
      error += filename;
      error += "\n";
   }

   Table *migIoTbl = getTable (MigrationIoTbl);
   if ( migIoTbl == NULL ) 
   {
      error += "Error opening Migration I/O Table\n";
   }
   else
   {      
      // set up interators to search through user-request
      setupSearchIterators ();
   
      // setup event tree for selected component
      m_eventTree.setSelectedComponent (m_selectedComponent);
      m_eventTree.clear ();
      m_eventTree.setTableSize (migIoTbl->size());
   
      readTable (migIoTbl);
      
      // after all nodes read in, calculate the source 
      // node percentages
      m_eventTree.calculatePercents ();
   
      if ( EnvironmentSettings::debugOn () )
      {
         m_eventTree.printContents ();
      }
 
      // generate trap source info for user-requested trap
      // which will be stored in ReservoirLineGraph
      generateTrapSourceForUserTrap ();                      
   }
   
   return error;
}

//
// PRIVATE METHODS
//
/// SourceReader::readRecord calls EventTree's readRecord, to extract
/// source and destination data from the migration table row
void SourceReader::readRecord (Record *rec)
{
   m_eventTree.readRecord (rec);
}

/// SourceReader::generateTrapSourceForUserTrap collects source data for a selected trap 
/// from the event tree and prepares it for the source graph
void SourceReader::generateTrapSourceForUserTrap ()
{
   // generate Trap Source information based on event tree
   // for user-trap at each timestep
   
   LineGroup *lineGroup;
   
   // for each selected reservoir
   for ( m_resIt = m_resData->begin(); m_resIt != m_endResIt; ++m_resIt )
   {
      m_persisTrapData = &(m_resIt->second);      
      m_endPersisIt = m_persisTrapData->end();
      
      // for each selected persistant trap
      for ( m_persisIt = m_persisTrapData->begin(); m_persisIt != m_endPersisIt;
              ++m_persisIt )
      {
         // get line group for reservoir and persistant trap
         lineGroup = &((plotLines()) [m_resIt->first][m_persisIt->first]);
         
         m_transTrapData = &(m_persisIt->second);
         m_endTransIt = m_transTrapData->end();
         
         // for each transient id at specific age
         for ( m_transIt = m_transTrapData->begin(); m_transIt != m_endTransIt;
                 ++m_transIt )
         {            
            m_trapSource.generateSource (m_resIt->first, 
                                         m_transIt->first, 
                                         m_transIt->second, 
                                         &m_eventTree, 
                                         lineGroup);
         }
      }
   }
}

