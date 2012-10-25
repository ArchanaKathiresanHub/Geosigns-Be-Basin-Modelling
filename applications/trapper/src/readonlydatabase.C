/// Class ReadOnlyDatabase is a singleton class used to open a project database
/// and read table info from it
/// One project file should only ever be opened once
/// also each table should only be opened for reading once 
/// so a TableMap is maintained to ensure if a table requested is already
/// opened then it is simply returned to the requesting function

#include "readonlydatabase.h"

//
// CTOR / DTOR (PRIVATE)
//
ReadOnlyDatabase::~ReadOnlyDatabase ()
{
   // don't delete tables within database, as that happens in the database dtor
   if ( m_database ) delete m_database;
}

//
// PUBLIC METHODS
//

/// ReadOnlyDatabase::createDatabase opens a project file, deleteing data relating to 
/// a previous project file first, if it exists. 
bool ReadOnlyDatabase::createDatabase (const char *projectFilename)
{
   // delete old if necessary
   deleteOldDatabase ();
 
   DataSchema *projSchema = createCauldronSchema ();
   m_database = Database::CreateFromFile (projectFilename, *projSchema);
 
   return m_database != NULL;
}

/// ReadOnlyDatabase::openDatabase opens a project file only if it is not already opened.
bool ReadOnlyDatabase::openDatabase (const char *projectFilename)
{ 
   if ( m_database == NULL )
   {
      createDatabase (projectFilename);
   }

   return m_database != NULL;
}

/// ReadOnlyDatabase::getTable opens a table in a project file if it has not already been opened
/// It then returns a pointer to that table
Table* ReadOnlyDatabase::getTable (const char *tableName)
{
  Table *table = 0;

   // try to find table in list of tables already opened  
   const_TableIT tableIt = m_tables.find (tableName);
 
   // if didn't find the table, then create it and add it to the list of tables
   // already opened
   if ( tableIt == m_tables.end() )
   {
      Table *newTable = createTable (tableName);

      // check if table created okay
      if ( newTable != 0 && newTable->size() > 0 )
      {
         // if add table okay then assign to return pointer
         //
         // (note: the new Table memory will be deleted by the database dtor
         // either way)
         if ( addTableToList (tableName, newTable) )
         {
            table = newTable;
         } 
      }
   }
   else
   {
      // table already opened, so just return it
      table = tableIt->second;
   }

   return table;
}

//
// PRIVATE METHODS
//
Table* ReadOnlyDatabase::createTable (const char *tableName)
{
   return m_database->getTable (tableName);
}

/// ReadOnlyDatabase::addTableToList adds a newly opened table to a list of open tables
/// so if a request to open the table occurs a second time, rather than opening it twice, it is returned from this list
bool ReadOnlyDatabase::addTableToList (const char *tableName, Table *table)
{
   pair<string, Table*> newTable (tableName, table);
   
   // return true or false resulting from table insert
   return (m_tables.insert (newTable)).second;
}

void ReadOnlyDatabase::deleteOldDatabase ()
{
   if ( m_database )
   {
      // delete list of opened tables
      m_tables.erase (m_tables.begin(), m_tables.end());
      
      // delete database pointer
      delete m_database;
   }
}
