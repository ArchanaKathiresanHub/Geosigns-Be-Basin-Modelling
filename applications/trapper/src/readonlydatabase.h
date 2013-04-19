/// Class ReadOnlyDatabase is a singleton class used to open a project database
/// and read table info from it
/// One project file should only ever be opened once
/// also each table should only be opened for reading once 
/// so a TableMap is maintained to ensure if a table requested is already
/// opened then it is simply returned to the requesting function

#ifndef __readonlydatabase__
#define __readonlydatabase__

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
using namespace database;

#include <map>
using namespace std;

class ReadOnlyDatabase
{
public:
   // public methods
   static ReadOnlyDatabase* getReadOnlyDatabase ()
   {
      static ReadOnlyDatabase m_readonly_database; 
      return &m_readonly_database;
   }
   
   bool createDatabase (const char *projectFilename);
   bool openDatabase (const char *projectFilename);
   Table* getTable (const char *tableName);
   bool databaseOpen () { return m_database != NULL; }
   
private:
   // private typedefs
   typedef map<string, Table*> TableMap;
   typedef TableMap::const_iterator const_TableIT;
   typedef TableMap::iterator TableIT;
   
   // ctor / dtor private, as singleton class
   ReadOnlyDatabase () : m_database (0) {} 
   ~ReadOnlyDatabase (); 
   
   // private methods
   Table* createTable (const char *tableName);
   bool addTableToList (const char *tableName, Table *table);
   void deleteOldDatabase ();
   
   // private operatrs - assignment not allowed
   ReadOnlyDatabase& operator= (const ReadOnlyDatabase &rhs);
   
   Database *m_database;
   TableMap m_tables;
};

#endif
