/// Class DataReader is an abstract Base Class for reading data from Cauldron Project file
/// through the TableIo lib
/// Each class that inherits from it must define its own version of readData
/// Other protected methods are virtual so the inheriting class can redefine them 
/// for its own purposes. 

#include "datareader.h"
#include "createuserrequest.h"

//
// PUBLIC METHODS
//
void DataReader::clearData ()
{
   m_plotLines.erase (m_plotLines.begin(), m_plotLines.end());
}

//
// PROTECTED METHODS
//
/// DataReader::openProject opens a project if it isn't already open
bool DataReader::openProject (const char *filename)
{
   m_readOnlyDatabase = ReadOnlyDatabase::getReadOnlyDatabase (); 
   return m_readOnlyDatabase->openDatabase (filename);
}

/// DataReader::getTable opens a table if it isn't already open
Table* DataReader::getTable (const char *tableName)
{
   return m_readOnlyDatabase->getTable (tableName);
}

/// DataReader::readTable loops through a table calling 
/// protected methods: recordWanted and readRecord, 
/// which can be overwritten by inheriting classes
void DataReader::readTable (Table *table)
{
   int tableSize = table->size();
   Record *rec;

   for ( int i=0; i < tableSize; ++i )
   {
      rec = table->getRecord (i);
      
      if ( recordWanted (rec) )
      {
         readRecord (rec);
      }
   }
}

/// DataReader::recordWanted returns true if a records values
/// match with what is currently in the UserRequest object
bool DataReader::recordWanted (Record *rec)
{
   // returns true if record's reservoir name, age and trap id are found in user request
   bool recordNeeded = false;
   
   // see if record's reservoir name matches a requested reservoir
   m_currReservoir = &getRecordReservoirName (rec);
   m_resIt = m_resData->find (*m_currReservoir);
   if ( m_resIt != m_endResIt )
   {
      // the current record's reservoir is requested, so now loop requested persistent traps
      m_persisTrapData = &(m_resIt->second);
      m_endPersisIt = m_persisTrapData->end();
      
      for ( m_persisIt = m_persisTrapData->begin(); m_persisIt != m_endPersisIt; ++m_persisIt )
      {
         // see if current record's age is requested for this persis trap
         m_transTrapData = &(m_persisIt->second);
         m_transIt = m_transTrapData->find (getRecordAge (rec));
         if ( m_transIt != m_transTrapData->end() )
         {
            // age is requested so now check transiend id
            if ( (m_transIt->second) == getRecordTrapId (rec) )
            {
               // the trans id is also requested, so this record is needed
               recordNeeded = true;
               break;
            }
         }
      }
   }
   
   return recordNeeded;
}

// standard trap id name in project file, can be overwritten by derived class
int DataReader::getRecordTrapId (Record *rec)
{
   return getTrapID (rec);
}

// standard age name in project file, can be overwritten by derived class
double DataReader::getRecordAge (Record *rec)
{
   return getAge (rec);
}

// standard reservoir name in project file, can be overwritten by derived class
const string& DataReader::getRecordReservoirName(Record *rec)
{
   return getReservoirName (rec);
}

void DataReader::setupSearchIterators ()
{ 
   m_resData = &(userRequest ()->getUserRequest ());
   m_endResIt = m_resData->end();
}
