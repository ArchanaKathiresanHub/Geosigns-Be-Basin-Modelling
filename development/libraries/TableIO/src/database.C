#include <stdlib.h>
#include <assert.h>
#include <string>
#include <map>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
using namespace std;

#include <sys/types.h>
#include <time.h>

#include "database.h"
#include "dataschema.h"

#include "datautils.h"

using namespace std;
using namespace datatype;

namespace database {

int Database::s_maxFieldsPerLine = 200;		// to support the trapperiotbl
int Database::s_fieldWidth = 4;
int Database::s_precision = 15;



Database::Database (const DataSchema & dataSchema) : m_dataSchema (dataSchema), m_fileName ("")
{
   char *fieldWidthStr = getenv ("IBS_FIELDWIDTH");

   if (fieldWidthStr && strlen (fieldWidthStr) != 0)
   {
      Database::s_fieldWidth = atoi (fieldWidthStr);
   }


   for (size_t i = 0; i < m_dataSchema.size (); i++)
   {
      TableDefinition *tableDef = m_dataSchema.getTableDefinition (i);

      Table *aTable = new Table (*tableDef);

      m_tables.push_back (aTable);
   }
}

Database::~Database (void)
{
   for (TableListIterator iter = m_tables.begin (); iter != m_tables.end (); ++iter)
   {
      Table *table = *iter;
      delete table;
   }

   delete & m_dataSchema;
}

void Database::addHeader (const string & text)
{
   m_header += text;
   m_header += "\n";

   cout << "Header: " << endl << m_header << endl;
}

void Database::clearHeader (void)
{
   m_header = "";
}

Transaction * Database::createTransaction (void)
{
   Transaction * transaction = new Transaction (this);
   // m_transactions.push_back (transaction);

   return transaction;
}

Table *Database::getTable (const string & name)
{
   int index = m_dataSchema.getIndex (name);

   if (index == -1)
      return 0;
   return m_tables[index];
}

Table *Database::getTable (int index)
{
   if (index == -1)
      return 0;
   if (index >= m_tables.size ())
      return 0;
   return m_tables[index];
}

bool Database::clearTable (const string & name, bool deleteRecords)
{
   Table *tbl = getTable (name);

   if (!tbl)
      return false;

   tbl->clear (deleteRecords);
   return true;
}

bool Database::resave (void)
{
   if (m_fileName != "")
   {
      return saveToFile (m_fileName);
   }
   else
   {
      return false;
   }
}

bool Database::saveToFile (const string & filename)
{
   bool result;

   if (filename.length () == 0)
      return false;

   ofstream outfile;

   outfile.open (filename.c_str (), ios::out);

   if (outfile.fail ())
   {
      cerr << "Error occurred during opening file " << filename << endl;
      return false;
   }

   outfile << ";" << endl;
   time_t secs = time (0);

   outfile << "; START - " << filename << " " << ctime (&secs) << ";" << endl;

   result = saveToStream (outfile);

   outfile.close ();
   return result;
}

bool Database::saveToStream (ostream & ofile)
{
   bool result = true;

   saveHeader (ofile);

   for (TableListIterator iter = m_tables.begin (); iter != m_tables.end (); ++iter)
   {
      Table *table = *iter;

      result = table->saveToStream (ofile);

      if (!result)
         return false;
   }

   return true;
}

bool Database::saveHeader (ostream & ofile)
{
   if (m_header.length () == 0) return true;

   string modHeader = m_header;

   int pos = -1;
   do
   {
      modHeader.insert (pos + 1, ";! ");
      // cerr << "modHeader" << modHeader << endl;
   }
   while ((pos = modHeader.find ("\n", pos + 4)) != string::npos && pos < modHeader.length () - 2);

   ofile << modHeader;
   if (modHeader[modHeader.length () - 1] != '\n')
      cerr << endl;

   return true;
}

Database *Database::CreateFromSchema (const DataSchema & dataSchema)
{
   DataSchema *copy = dataSchema.deepCopy ();

   return new Database (*copy);
}

Database *Database::CreateFromFile (const string & filename, const DataSchema & dataSchema)
{
   Database *database = Database::CreateFromSchema (dataSchema);

   if (database->loadFromFile (filename))
      return database;
   else
   {
      // delete database;
      return 0;
   }
}

bool Database::loadFromFile (const string & filename)
{
   bool result;

   if (filename.length () == 0)
      return false;

   ifstream infile;

   infile.open (filename.c_str (), ios::in);

   if (infile.fail ())
   {
      cerr << "Error occurred during opening file " << filename << endl;
      return false;
   }

   result = loadFromStream (infile);
   infile.close ();

   if (result) m_fileName = filename;

   return result;
}

bool Database::loadFromStream (istream & infile)
{
   string line;

   bool checkForHeaders = true;
   while (true)
   {
      if (!loadLine (infile, line, checkForHeaders))
         return true;

      if (line.find (";! ") == 0)
      {
	 addHeader (line.substr (3, string::npos) + "\n");
	 continue;
      }

      findAndRemoveDelimiters (line, "[]");
      checkForHeaders = false;

      Table *table = getTable (line);

      if (!table)               // skip over specification
      {
         if (!skipTableFromStream (infile))
            return false;
      }
      else
      {
         if (!table->loadFromStream (infile))
            return false;
      }
   }
}

bool Database::skipTableFromStream (istream & infile)
{
   string line;

   while (true)
   {
      if (!loadLine (infile, line))
         return false;
      if (findAndRemoveDelimiters (line, "[]") && line == "End")
         return true;
   }
}

TablePartitioning::TablePartitioning (Table * table, size_t fieldIndex) : m_table (table), m_partitioningFieldIndex (fieldIndex)
{
   m_partitioningFieldDefinition = m_table->getTableDefinition().getFieldDefinition (m_partitioningFieldIndex);
   assert (m_partitioningFieldDefinition != 0);
   
   switch (m_partitioningFieldDefinition->dataType ())
   {
         case Bool:
	    m_subTables = (void *) new std::map<bool, Table *>;
            break;
         case Int:
	    m_subTables = (void *) new std::map<int, Table *>;
            break;
         case Long:
	    m_subTables = (void *) new std::map<long, Table *>;
            break;
         case Float:
	    m_subTables = (void *) new std::map<float, Table *>;
            break;
         case Double:
	    m_subTables = (void *) new std::map<double, Table *>;
            break;
         case String:
	    m_subTables = (void *) new std::map<string, Table *>;
            break;
         default:
            assert (false);
   }

   m_sizeSubTables = 0;
}

TablePartitioning::~TablePartitioning ()
{
   switch (m_partitioningFieldDefinition->dataType ())
   {
         case Bool:
	    deleteSubTables<bool> ();
            break;
         case Int:
	    deleteSubTables<int> ();
            break;
         case Long:
	    deleteSubTables<long> ();
            break;
         case Float:
	    deleteSubTables<float> ();
            break;
         case Double:
	    deleteSubTables<double> ();
            break;
         case String:
	    deleteSubTables<string> ();
            break;
         default:
            assert (false);
   }

   m_sizeSubTables = 0;
}

void TablePartitioning::clear ()
{
   switch (m_partitioningFieldDefinition->dataType ())
   {
         case Bool:
	    clearSubTables<bool> ();
            break;
         case Int:
	    clearSubTables<int> ();
            break;
         case Long:
	    clearSubTables<long> ();
            break;
         case Float:
	    clearSubTables<float> ();
            break;
         case Double:
	    clearSubTables<double> ();
            break;
         case String:
	    clearSubTables<string> ();
            break;
         default:
            assert (false);
   }

   m_sizeSubTables = 0;
}

bool TablePartitioning::removeRecord (Record * record)
{
   Table * subTable;
   switch (m_partitioningFieldDefinition->dataType ())
   {
      case Bool:
	 {
	    bool fieldValue = record->getValue<bool> (m_partitioningFieldIndex);
	    subTable = getSubTable (fieldValue);
	 }
	 break;
      case Int:
	 {
	    int fieldValue = record->getValue<int> (m_partitioningFieldIndex);
	    subTable = getSubTable (fieldValue);
	 }
	 break;
      case Long:
	 {
	    long fieldValue = record->getValue<long> (m_partitioningFieldIndex);
	    subTable = getSubTable (fieldValue);
	 }
	 break;
      case Float:
	 {
	    float fieldValue = record->getValue<float> (m_partitioningFieldIndex);
	    subTable = getSubTable (fieldValue);
	 }
	 break;
      case Double:
	 {
	    double fieldValue = record->getValue<double> (m_partitioningFieldIndex);
	    subTable = getSubTable (fieldValue);
	 }
	 break;
      case String:
	 {
	    string fieldValue = record->getValue<string> (m_partitioningFieldIndex);
	    subTable = getSubTable (fieldValue);
	 }
	 break;
      default:
	 assert (false);
   }

   assert (subTable);

   if (subTable->removeRecord (record))
   {
      --m_sizeSubTables;
      return true;
   }
   else
   {
      return false;
   }
}

bool TablePartitioning::syncSubTables (void)
{
   // does not deal with inserted or deleted records yet.
   int i;

   for (i = m_sizeSubTables; i < m_table->size (); ++i)
   {
      Record * record = m_table->getRecord (i);

      Table * subTable;
      switch (m_partitioningFieldDefinition->dataType ())
      {
	 case Bool:
	    {
	       bool fieldValue = record->getValue<bool> (m_partitioningFieldIndex);
	       subTable = getSubTableImpl (fieldValue);
	    }
	    break;
	 case Int:
	    {
	       int fieldValue = record->getValue<int> (m_partitioningFieldIndex);
	       subTable = getSubTableImpl (fieldValue);
	    }
	    break;
	 case Long:
	    {
	       long fieldValue = record->getValue<long> (m_partitioningFieldIndex);
	       subTable = getSubTableImpl (fieldValue);
	    }
	    break;
	 case Float:
	    {
	       float fieldValue = record->getValue<float> (m_partitioningFieldIndex);
	       subTable = getSubTableImpl (fieldValue);
	    }
	    break;
	 case Double:
	    {
	       double fieldValue = record->getValue<double> (m_partitioningFieldIndex);
	       subTable = getSubTableImpl (fieldValue);
	    }
	    break;
	 case String:
	    {
	       string fieldValue = record->getValue<string> (m_partitioningFieldIndex);
	       subTable = getSubTableImpl (fieldValue);
	    }
	    break;
	 default:
	    assert (false);
      }

      subTable->addRecord (record);
      ++m_sizeSubTables;
   }
   return true;
}
      
Transaction::Transaction (const Database * database) :
   m_database (database), m_parent (0), m_child (0)
{
}

Transaction::~Transaction (void)
{
   if (m_parent) m_parent->setChild (0);
}

Transaction * Transaction::createSubTransaction (void)
{
   if (getChild () != 0) return getChild ()->createSubTransaction ();

   Transaction * transaction = new Transaction (m_database);
   transaction->setParent (this);
   setChild (transaction);

   return transaction;
}

void Transaction::rollBack (void)
{
   if (m_child) m_child->rollBack ();

   RecordPairListReverseIterator recordPairIter;
   RecordListIterator recordIter;

   for (recordPairIter = m_deletedRecords.rbegin (); recordPairIter != m_deletedRecords.rend (); ++recordPairIter)
   {
      RecordPair & recordPair = *recordPairIter;
      Record *record = recordPair.first;
      Record *nextRecord = recordPair.second;

      record->getTable ()->insertRecord (record, nextRecord);

      record->unlock (this);
   }

   for (recordPairIter = m_editedRecords.rbegin (); recordPairIter != m_editedRecords.rend (); ++recordPairIter)
   {
      RecordPair & recordPair = *recordPairIter;
      Record *editedRecord = recordPair.first;
      Record *copiedRecord = recordPair.second;

      editedRecord->copyFrom (* copiedRecord);
      editedRecord->unlock (this);
      delete copiedRecord;
   }

   for (recordIter = m_createdRecords.begin (); recordIter != m_createdRecords.end (); ++recordIter)
   {
      Record *record = *recordIter;
      record->destroyYourself ();
   }
   
   delete this;
}

void Transaction::commit (void)
{
   RecordPairListIterator recordPairIter;
   RecordListIterator recordIter;

   if (m_parent) return;

   for (recordPairIter = m_deletedRecords.begin (); recordPairIter != m_deletedRecords.end (); ++recordPairIter)
   {
      RecordPair & recordPair = *recordPairIter;
      Record *record = recordPair.first;
      delete record;
   }

   for (recordPairIter = m_editedRecords.begin (); recordPairIter != m_editedRecords.end (); ++recordPairIter)
   {
      RecordPair & recordPair = *recordPairIter;

      Record *editedRecord = recordPair.first;
      Record *copiedRecord = recordPair.second;

      editedRecord->unlock (this);
      delete copiedRecord;
   }

   for (recordIter = m_createdRecords.begin (); recordIter != m_createdRecords.end (); ++recordIter)
   {
      Record *record = *recordIter;
      record->unlock (this);
   }

   if (m_child)
   {
      m_child->setParent (0);
      m_child->commit ();
   }

   delete this;
}

bool Transaction::descendsFrom (Transaction * transaction)
{
   if (this == transaction) return true;
   if (transaction == 0) return false;
   if (m_parent == 0) return false;
   return m_parent->descendsFrom (transaction);
}

bool Transaction::ascends (Transaction * transaction)
{
   if (this == transaction) return true;
   if (transaction == 0) return false;
   if (m_child == 0) return false;
   return m_child->ascends (transaction);
}

Table::Table (const TableDefinition & tableDefinition):
m_tableDefinition (tableDefinition), m_tablePartitionings (0)
{
}

Table::~Table (void)
{
   for (RecordListIterator iter = m_records.begin (); iter != m_records.end (); ++iter)
   {
      Record *record = *iter;
      delete record;
   }

   resetPartitionings ();
}

void Table::clear (bool deleteRecords)
{
   if (deleteRecords)
   {
      for (RecordListIterator iter = m_records.begin (); iter != m_records.end (); ++iter)
      {
	 Record *record = *iter;
	 delete record;
      }
   }

   clearPartitionings ();

   m_records.clear ();
}


Record *Table::createRecord (Transaction * transaction)
{
   Record *record = new Record (m_tableDefinition, this);

   if (transaction)
   {
      while (transaction->getChild () != 0) transaction = transaction->getChild ();
      record->lock (transaction);
      transaction->addCreatedRecord (record);
   }

   addRecord (record);

   return record;
}

Table::iterator Table::findRecordPosition (Record * record)
{
   if (!record) return end ();

   Table::iterator iter;
   for (iter = begin (); iter != end (); ++iter)
   {
      if (*iter == record)
      {
	 break;
      }
   }

   return iter;
}

bool Table::moveRecord (Record * record1, Record * record2)
{
   if (record1 == record2) return true;

   bool success = false;
   success = eraseRecord (record1);

   if (success) success = insertRecord (record1, record2);

   return success;
}

bool Table::eraseRecord (Record * record)
{
   assert (record);

   Table::iterator iter = findRecordPosition (record);

   if (iter != end ())
   {
      m_records.erase (iter);
      return true;
   }
   else
   {
      return false;
   }
}

bool Table::insertRecord (Record * record1, Record * record2)
{
   assert (record1);

   Table::iterator iter = findRecordPosition (record2);

   m_records.insert (iter, record1);
   return true;
}

void Table::addRecord (Record * record)
{
   // assert (record->getTable () == this);
   m_records.push_back (record);
}


Record * Table::editRecord (Record * record, Transaction * transaction)
{
   if (record->isLockedBy (transaction)) return record;
   if (record->isLocked ()) return 0;

   assert (transaction);

   while (transaction->getChild () != 0) transaction = transaction->getChild ();

   Record * copiedRecord = new Record (* record);
   transaction->addEditedRecord (record, copiedRecord);

   record->lock (transaction);

   return record;
}

bool Table::removeRecordFromTablePartitionings (Record * record)
{
   if (m_tablePartitionings)
   {
      int i;
      for (i = m_tableDefinition.size () -1; i >=0; --i)
      {
	 if (m_tablePartitionings[i])
	 {
	    if (!m_tablePartitionings[i]->removeRecord (record))
	    {
	       return false;
	    }
	 }
      }
   }
   return true;
}

bool Table::deleteRecord (Record * record, Transaction * transaction)
{
   Record * recordNext = removeRecord (record);

   return deleteRecordTransactional (record, recordNext, transaction);
}

bool Table::deleteRecordTransactional (Record * record, Record * recordNext, Transaction * transaction)
{
   if (transaction)
   {
      while (transaction->getChild () != 0) transaction = transaction->getChild ();

      transaction->addDeletedRecord (record, recordNext);
   }
   else
   {
      delete record;
   }
   return true;
}

Record * Table::removeRecord (Record * record)
{
   removeRecordFromTablePartitionings (record);
   Table::iterator iter = findRecordPosition (record);

   iter = removeRecord (iter);
   return getRecord (iter);
}

Table::iterator Table::deleteRecord (Table::iterator & iter, Transaction * transaction)
{
   Record * recordToDelete = getRecord (iter);
   Table::iterator returnIter = removeRecord (iter);
   deleteRecordTransactional (recordToDelete, getRecord (iter), transaction);

   return returnIter;
}

Table::iterator Table::removeRecord (Table::iterator & iter)
{
   if (iter != end ())
   {
      return (m_records.erase (iter));
   }
   else
   return iter;
}

Record * Table::findRecord (const std::string & fieldName, const char * value)
{
   int index = getIndex (fieldName);
   if (index < 0) return 0;

   Table::iterator iter;
   for (iter = begin (); iter != end (); ++iter)
   {
      Record * record = * iter;
      const std::string & foundValue = record->getValue (index, (std::string *) 0);

      if (foundValue == value) return record;
   }
   return 0;
}

Record * Table::findRecord (const std::string & fieldName1, const char * value1, const std::string & fieldName2, const char * value2, Record * record1)
{
   int index1 = getIndex (fieldName1);
   if (index1 < 0) return 0;

   int index2 = getIndex (fieldName2);
   if (index2 < 0) return 0;

   Table::iterator iter;
   for (iter = begin (); iter != end (); ++iter)
   {
      Record * record = * iter;
      const std::string & foundValue1 = record->getValue (index1, (std::string *) 0);

      if (foundValue1 == value1) {
         const std::string & foundValue2 = record->getValue (index2, (std::string *) 0);
         if (foundValue2 == value2) {
            if( record != record1 ) {
               return record;
            }
         }
      }
   }
   return 0;
}

void Table::sort (OrderingFunc func)
{
   std::sort (m_records.begin (), m_records.end (), func);
}

void Table::stable_sort (OrderingFunc func)
{
   std::stable_sort (m_records.begin (), m_records.end (), func);
}

void Table::unique (EqualityFunc equalityFunc, MergeFunc mergeFunc)
{
   for (Table::iterator iter = begin (); iter != end () && (iter + 1) != end ();)
   {
      Record *record = *iter;
      Record *recordNext = *(iter + 1);

      if (equalityFunc (record, recordNext))
      {
	 if (mergeFunc)
      {
	    mergeFunc (record, recordNext);
	 }

         m_records.erase (iter + 1);
         delete recordNext;
      }
      else
         ++iter;
   }
}

bool Table::saveToFile (const string & filename)
{
   bool result;

   if (filename.length () == 0)
      return false;

   ofstream outfile;

   outfile.open (filename.c_str (), ios::out);

   if (outfile.fail ())
   {
      cerr << "Error occurred during opening file " << filename << endl;
      return false;
   }

   result = saveToStream (outfile);

   outfile.close ();

   return result;
}

bool Table::saveToStream (ostream & ofile)
{
   ofile << ";" << endl;

   bool rowBased;

   if (m_tableDefinition.size () > Database::GetMaxFieldsPerLine ())
      rowBased = true;
   else
      rowBased = false;

   if (!m_tableDefinition.saveToStream (ofile, rowBased))
      return false;

   for (Table::iterator iter = begin (); iter != end (); ++iter)
   {
      Record *record = *iter;

      bool result = record->saveToStream (ofile, rowBased);

      if (!result)
         return false;
   }

   ofile << "[End]" << endl;
   ofile << ";" << endl;

   return !ofile.fail ();
}

bool Table::loadFromStream (istream & infile)
{
   vector < int >dataToFieldMap;

   string line;

   if (!loadLine (infile, line))
   {
      return false;
   }

   if (findAndRemoveDelimiters (line, "[]") && line == "End")
   {
      return true;              // empty row-based table without Field-Specs
   }
   else if (findAndRemoveDelimiters (line, "<>") && line == "Row")
   {
      return loadRowBasedRecordsFromStream (infile);
   }
   else
   {
      if (!loadDataToFieldMapFromLine (line, dataToFieldMap))
         return false;

      if (!loadUnitsFromStream (infile)) return false;

      return loadRecordsFromStream (infile, dataToFieldMap);
   }
}

bool Table::loadDataToFieldMapFromLine (string & line, vector < int >&dataToFieldMap)
{
   size_t fieldNameStartPos = 0;
   size_t fieldNameEndPos = 0;
   size_t fieldNameLength = 0;

   int lineIndex = -1;

   const string separators = " \t";

   while ((fieldNameStartPos =
               line.find_first_not_of (separators, fieldNameEndPos)) != string::npos)
   {
      lineIndex++;

      fieldNameEndPos = line.find_first_of (separators, fieldNameStartPos);

      if (fieldNameEndPos == string::npos)
         fieldNameLength = string::npos;
      else
         fieldNameLength = fieldNameEndPos - fieldNameStartPos;

      string fieldName = line.substr (fieldNameStartPos, fieldNameLength);

      dataToFieldMap.push_back (m_tableDefinition.getIndex (fieldName));
   }

   return true;
}

bool Table::loadUnitsFromStream (istream & infile)
{
   string line;

   // remember the position in the stream
   streampos pos = infile.tellg ();

   if (!loadLine (infile, line))
      return false; 

   const string separators = " \t";
   size_t firstNonSpace = line.find_first_not_of (separators, 0);

   if (firstNonSpace == string::npos || line[firstNonSpace] != '(')
   {
      // this is not a unit line, we need to revert to the remembered position
      infile.seekg (pos);
   }
   else
   {
      // Units line, we should really compare with the specified units ....
   }
   return true;
}

bool Table::loadRecordsFromStream (istream & infile, vector < int >&dataToFieldMap)
{
   string line;

   while (true)
   {
      if (!loadLine (infile, line))
         return false;
      if (findAndRemoveDelimiters (line, "[]") && line == "End")
         return true;

      Record *record = createRecord ();

      if (!record->loadFromLine (line, dataToFieldMap))
         return false;
   }
}

bool Table::loadRowBasedRecordsFromStream (istream & infile)
{
   while (true)
   {
      Record *record = createRecord ();

      while (true)
      {
         string line;

         string fieldName;
         string fieldValue;
         int fieldIndex;

         size_t linePos = 0;

         if (!loadLine (infile, line))
            return false;

         if (findAndRemoveDelimiters (line, "[]") && line == "End")
            return true;

         if (findAndRemoveDelimiters (line, "<>") && line == "Row")
            break;

         linePos = loadWordFromLine (line, linePos, fieldName);

         if (linePos == string::npos)
            return false;

         fieldIndex = m_tableDefinition.getIndex (fieldName);
         if (fieldIndex == -1)
            continue;           // unknown field

         linePos = loadWordFromLine (line, linePos, fieldValue);

         if (linePos == string::npos)
            return false;

         if (!record->assignFromStringToIndex (fieldValue, fieldIndex))
            return false;
      }
   }
}

void Table::resetPartitionings ()
{
   if (!m_tablePartitionings) return;

   int i;
   for (i = m_tableDefinition.size () -1; i >=0; --i)
   {
      delete m_tablePartitionings[i];
   }

   delete [] m_tablePartitionings;
   m_tablePartitionings = 0;
}

TablePartitioning * Table::getPartitioning (const std::string & fieldName)
{
   int fieldIndex = getIndex (fieldName);
   assert (fieldIndex >= 0);

   if (!m_tablePartitionings)
   {
      m_tablePartitionings = new TablePartitioning * [m_tableDefinition.size ()];
      int i;
      for (i = m_tableDefinition.size () -1; i >=0; --i)
      {
	 m_tablePartitionings[i] = 0;
      }
   }
   if (!m_tablePartitionings[fieldIndex])
   {
      m_tablePartitionings[fieldIndex] = new TablePartitioning (this, fieldIndex);
   }

   return m_tablePartitionings[fieldIndex];
}

void Table::clearPartitionings ()
{
   if (!m_tablePartitionings) return;

   int i;
   for (i = m_tableDefinition.size () -1; i >=0; --i)
   {
      if (m_tablePartitionings[i]) m_tablePartitionings[i]->clear ();
   }
}

Record::Record (const TableDefinition & tableDefinition, Table * table)
   : m_tableDefinition (tableDefinition), m_table (table), m_transaction (0)
{
   createFields ();
}

Record::Record (Record & record)
   : m_tableDefinition (record.m_tableDefinition), m_table (record.m_table), m_transaction (0)
{
   createFields ();
   copyFrom (record);
}

void Record::createFields (void)
{
   for (size_t i = 0; i < m_tableDefinition.size (); i++)
   {
      FieldDefinition *fieldDef = m_tableDefinition.getFieldDefinition (i);

      void *field = 0;

      switch (fieldDef->dataType())
      {
         case Bool:
            field = (void *) new Field < bool > (*fieldDef);

            break;
         case Int:
            field = (void *) new Field < int >(*fieldDef);

            break;
         case Long:
            field = (void *) new Field < long >(*fieldDef);

            break;
         case Float:
            field = (void *) new Field < float >(*fieldDef);

            break;
         case Double:
            field = (void *) new Field < double >(*fieldDef);

            break;
         case String:
            field = (void *) new Field < string > (*fieldDef);

            break;
         default:
            assert (false);
            field = 0;
      }

      if (field)
      {
         m_fields.push_back (field);
      }
   }
}

void Record::copyFrom (Record & record)
{
   for (size_t i = 0; i < m_tableDefinition.size (); i++)
   {
      FieldDefinition *fieldDef = m_tableDefinition.getFieldDefinition (i);

      switch (fieldDef->dataType())
      {
         case Bool:
	 {
#ifdef sgi
	    Field < bool > *field = (Field < bool > *) getField (i);
	    Field < bool > *fieldToCopy = (Field < bool > *) record.getField (i);

	    const bool & value = fieldToCopy->getValue ((bool *) 0);
	    field->setValue (value);
#else
	    // does not compile on irix
	    copyField (& record, i, (bool *) 0);
#endif
            break;
	 }
         case Int:
	 {
#ifdef sgi
	    Field < int > *field = (Field < int > *) getField (i);
	    Field < int > *fieldToCopy = (Field < int > *) record.getField (i);

	    const int & value = fieldToCopy->getValue ((int *) 0);
	    field->setValue (value);
#else
	    // does not compile on irix
	    copyField (& record, i, (int *) 0);
#endif
            break;
	 }
         case Long:
	 {
#ifdef sgi
	    Field < long > *field = (Field < long > *) getField (i);
	    Field < long > *fieldToCopy = (Field < long > *) record.getField (i);

	    const long & value = fieldToCopy->getValue ((long *) 0);
	    field->setValue (value);
#else
	    // does not compile on irix
	    copyField (& record, i, (long *) 0);
#endif
            break;
	 }
         case Float:
	 {
#ifdef sgi
	    Field < float > *field = (Field < float > *) getField (i);
	    Field < float > *fieldToCopy = (Field < float > *) record.getField (i);

	    const float & value = fieldToCopy->getValue ((float *) 0);
	    field->setValue (value);
#else
	    // does not compile on irix
	    copyField (& record, i, (float *) 0);
#endif
            break;
	 }
         case Double:
	 {
#ifdef sgi
	    Field < double > *field = (Field < double > *) getField (i);
	    Field < double > *fieldToCopy = (Field < double > *) record.getField (i);

	    const double & value = fieldToCopy->getValue ((double *) 0);
	    field->setValue (value);
#else
	    // does not compile on irix
	    copyField (& record, i, (double *) 0);
#endif
            break;
	 }
         case String:
	 {
#ifdef sgi
	    Field < string > *field = (Field < string > *) getField (i);
	    Field < string > *fieldToCopy = (Field < string > *) record.getField (i);

	    const string & value = fieldToCopy->getValue ((string *) 0);
	    field->setValue (value);
#else
	    // does not compile on irix
	    copyField (& record, i, (string *) 0);
#endif
            break;
	 }
         default:
            assert (false);
      }
   }
}

Record::~Record (void)
{
   removeFields ();
}

void Record::removeFields (void)
{
   for (size_t i = 0; i < m_tableDefinition.size (); i++)
   {
      FieldDefinition *fieldDef = m_tableDefinition.getFieldDefinition (i);

      void *field = m_fields[i];

      switch (fieldDef->dataType ())
      {
         case Bool:
            delete (Field < bool > *)field;
            break;
         case Int:
            delete (Field < int >*) field;

            break;
         case Long:
            delete (Field < long >*) field;

            break;
         case Float:
            delete (Field < float >*) field;

            break;
         case Double:
            delete (Field < double >*) field;

            break;
         case String:
            delete (Field < string > *)field;
            break;
         default:
            assert (false);
      }
   }
   m_fields.clear ();
}

template <class Type>
void * Record::copyField (Record * srcRecord, size_t index, Type * ptr)
{
   FieldDefinition *fieldDef = m_tableDefinition.getFieldDefinition (index);

   Field < Type > *field = (Field < Type > *)getField (index);
   Field < Type > *fieldToCopy = (Field < Type > *)srcRecord->getField (index);


   const Type & value = fieldToCopy->getValue ((Type *) 0);
   field->setValue (value);

   return (void *) field;
}

void Record::destroyYourself (void)
{
   getTable ()->deleteRecord (this);
}

void Record::addToTable (void)
{
   getTable ()->addRecord (this);
}

Record * Record::edit (Transaction * transaction)
{
   return getTable ()->editRecord (this, transaction);
}

void *Record::getField (const string & name, int *cachedIndex)
{
   int hint = (cachedIndex ? *cachedIndex : -1);

   int index = m_tableDefinition.getIndex (name, hint);

   if (cachedIndex)
      *cachedIndex = index;

   if (index == -1)
      return 0;
   return m_fields[index];
}

template <> bool Field < bool >::saveToStream (ostream & ofile, int &borrowed)
{

   string valueString = m_value == false ? "false" : "true";

   return saveStringToStream (ofile, valueString, borrowed, Database::GetFieldWidth ());
}

template <> bool Field < string >::saveToStream (ostream & ofile, int &borrowed)
{
   const string quote = "\"";

   string valueString = m_value;

   size_t wordQuotePos;
   size_t wordStartPos = 0;

   do
   {
      wordQuotePos = valueString.find_first_of (quote, wordStartPos);
      if (wordQuotePos != string::npos)
      {
	 valueString.insert (wordQuotePos, "\\");
	 wordStartPos = wordQuotePos + 2;
      }
      else
      {
	 break;
      }
   }
   while (true);

   valueString.insert (0, "\"");
   valueString += "\"";

   return saveStringToStream (ofile, valueString, borrowed, Database::GetFieldWidth ());
}


template < class Type > bool Field < Type >::saveToStream (ostream & ofile, int &borrowed)
{

   ostringstream buf;
   buf.precision (Database::GetPrecision ());
   buf << m_value;
   std::string return_string = buf.str ();

   return saveStringToStream (ofile, return_string, borrowed, Database::GetFieldWidth ());
}


template < class Type > bool Field < Type >::assignFromString (const string & word)
{
   assert (false);
   return false;
}

template <> bool Field < bool >::assignFromString (const string & word)
{
   if (word.length () == 0)
      return false;

   if (word == "false")
      m_value = false;
   else
      m_value = true;

   return true;
}

template <> bool Field < string >::assignFromString (const string & word)
{
   if (word.length () == 0)
      return false;

   m_value = word;
   return true;
}

template <> bool Field < int >::assignFromString (const string & word)
{
   if (word.length () == 0)
      return false;

   m_value = atoi (word.c_str ());

   return true;
}

template <> bool Field < long >::assignFromString (const string & word)
{
   if (word.length () == 0)
      return false;

   m_value = atol (word.c_str ());

   return true;
}

template <> bool Field < float >::assignFromString (const string & word)
{
   if (word.length () == 0)
      return false;

   m_value = (float) atof (word.c_str ());

   return true;
}

template <> bool Field < double >::assignFromString (const string & word)
{
   if (word.length () == 0)
      return false;

   m_value = atof (word.c_str ());

   return true;
}

template <> bool Field < int >::saveValueToStream (ostream & ofile, int &borrowed)
{
   const FieldDefinition & fieldDef = getFieldDefinition ();

   switch (fieldDef.dataType ())
   {
      case Bool:
         ((Field < bool > *)this)->saveToStream (ofile, borrowed);
         return true;
      case Int:
         ((Field < int >*) this)->saveToStream (ofile, borrowed);
         return true;
      case Long:
         ((Field < long >*) this)->saveToStream (ofile, borrowed);
         return true;
      case Float:
         ((Field < float >*) this)->saveToStream (ofile, borrowed);
         return true;
      case Double:
         ((Field < double >*) this)->saveToStream (ofile, borrowed);
         return true;
      case String:
         ((Field < string > *)this)->saveToStream (ofile, borrowed);
         return true;
      default:
         return false;
   }
}

template < class Type > bool Field < Type >::saveValueToStream (ostream & ofile, int &borrowed)
{
   assert (false);
   return false;
}

template < class Type > Field < Type >::Field (const FieldDefinition & fieldDef)
   : m_fieldDefinition (fieldDef)
{
   assignFromString (m_fieldDefinition.defaultValue ());
}

void Record::printOn (ostream & str)
{
   saveToStream (str, false);
}

bool Record::saveToStream (ostream & ofile, bool rowBased)
{
   bool result = true;

   if (rowBased)
      ofile << "<Row>" << endl;

   int borrowed = 0;

   for (size_t order = 0; order < m_tableDefinition.persistentSize (); order++)
   {
      const size_t position = m_tableDefinition.getPosition (order);
      void *field = m_fields[position];

      if (rowBased)
      {
         borrowed = 0;

         const FieldDefinition & fieldDef = ((Field < int >*) field)->getFieldDefinition ();

         fieldDef.saveNameToStream (ofile, borrowed);
         ofile << "      ";

         ((Field < int >*) field)->saveValueToStream (ofile, borrowed);
         ofile << "      ";

         fieldDef.saveUnitToStream (ofile, borrowed);
         ofile << endl;
      }
      else
      {
         if (order != 0)
	 {
            ofile << " ";
	 }

         ((Field < int >*) field)->saveValueToStream (ofile, borrowed);
      }
   }

   if (!rowBased)
      ofile << endl;
   return result;
}


bool Record::loadFromLine (string & line, vector < int >&dataToFieldMap)
{
   size_t linePos = 0;

   for (vector < int >::iterator mapIter = dataToFieldMap.begin ();
         mapIter != dataToFieldMap.end (); ++mapIter)
   {
      string fieldValue;

      if (linePos == string::npos)
         return false;

      linePos = loadWordFromLine (line, linePos, fieldValue);

      int toIndex = *mapIter;

      if (toIndex == -1)
      {
         // data belongs to an unknown field, skip over it
         continue;
      }

      if (!assignFromStringToIndex (fieldValue, toIndex))
         return false;
   }

   return true;
}

bool Record::assignFromStringToIndex (const string & word, int toIndex)
{
   FieldDefinition *fieldDef = m_tableDefinition.getFieldDefinition (toIndex);

   assert (fieldDef->isValid ());


   switch (fieldDef->dataType ())
   {
      case Bool:
         ((Field < bool > *)getField (toIndex))->assignFromString (word);
         break;
      case Int:
         ((Field < int >*) getField (toIndex))->assignFromString (word);
         break;
      case Long:
         ((Field < long >*) getField (toIndex))->assignFromString (word);
         break;
      case Float:
         ((Field < float >*) getField (toIndex))->assignFromString (word);
         break;
      case Double:
         ((Field < double >*) getField (toIndex))->assignFromString (word);
         break;
      case String:
         ((Field < string > *)getField (toIndex))->assignFromString (word);
         break;
      default:
         return false;
   }
   return true;
}

template < class Type > void checkType (const Type & a, const datatype::DataType type)
{
   assert (false);
}

}

template <> void database::checkType < bool > (const bool & a, const datatype::DataType type)
{
   assert (type == datatype::Bool);
}

template <> void database::checkType < int >(const int &a, const datatype::DataType type)
{
   assert (type == datatype::Int);
}

template <> void database::checkType < long >(const long &a, const datatype::DataType type)
{
   assert (type == datatype::Long);
}

template <> void database::checkType < float >(const float &a, const datatype::DataType type)
{
   assert (type == datatype::Float);
}

template <> void database::checkType < double >(const double &a, const datatype::DataType type)
{
   assert (type == datatype::Double);
}

template <> void database::checkType < string > (const string & a, const datatype::DataType type)
{
   assert (type == datatype::String);
}
