#ifndef DATABASE_H
#define DATABASE_H

#include "stdafx.h"
// Note: Comments starting with '///' are suitable for parsing by 'doxygen' to produce readable documentation
// in the form of html, latex or rtf files.
#include <stdlib.h>

#include <vector>
#include <map>
#include <string>

#include <fstream>
using std::istream;
using std::ofstream;
using std::ostream;

#include <assert.h>

#include "dataschema.h"
#include "datatype.h"

namespace database
{


   class Transaction;
   class Database;
   class Table;
   class TablePartitioning;
   class Record;


   template < class Type > TABLEIO_DLL_EXPORT void checkType (const Type & a, const datatype::DataType type);
   template <> TABLEIO_DLL_EXPORT void checkType < bool > (const bool & a, const datatype::DataType type);
   template <> TABLEIO_DLL_EXPORT void checkType < int >(const int &a, const datatype::DataType type);
   template <> TABLEIO_DLL_EXPORT void checkType < long >(const long &a, const datatype::DataType type);
   template <> TABLEIO_DLL_EXPORT void checkType < float >(const float &a, const datatype::DataType type);
   template <> TABLEIO_DLL_EXPORT void checkType < double >(const double &a, const datatype::DataType type);
   template <> TABLEIO_DLL_EXPORT void checkType < std::string > (const std::string & a, const datatype::DataType type);

   /// Fields contain the values of a Record.
   /// This class is a template because we need a different Field class for different types,
   /// e.g. int, long, double, float, string.
   /// This class is used only for implementation purposes.
   template < class Type > class Field
   {
    public:
      /// save the value to a stream.
      bool saveToStream (ostream & ofile, int &borrowed);

    private:
      friend class Record;
      
      const FieldDefinition & m_fieldDefinition;
      Type m_value;

      inline Field (const FieldDefinition & fieldDef);
      ~Field (void);

      inline const FieldDefinition & getFieldDefinition (void);

      inline void setValue (const Type & value);
#ifndef sun
      inline const Type & getValue (void);
#endif
      inline const Type & getValue (Type * value);

      bool assignFromString (const std::string & word);

      bool saveValueToStream (ostream & ofile, int &borrowed);
   };

   /// This class describes the entries of a Database Table.
   /// A Record consists of a number of Fields whose values can be set or retrieved.
   class TABLEIO_DLL_EXPORT Record
   {
    public:
      /// Make the record editable under control of a Transaction
      Record * edit (Transaction * transaction = 0);

      /// print the record's content
      void printOn (ostream &);

      /// Return the table name
      inline const std::string & tableName (void);

      /// Find out if the record is locked but not by the specified Transaction
      inline bool isLocked (Transaction * transaction = 0);

      /// Find out if the record is locked by the specified Transaction
      inline bool isLockedBy (Transaction * transaction = 0);

      /// Return the list index of the field with the specified name.
      inline int getIndex (const std::string & fieldName);

      inline const TableDefinition & getTableDefinition (void) const;
      inline Table * getTable (void);

      /// Set the value of the specified field from value.
      inline void setValue (const std::string & fieldName, const char *value);

      /// Set the value of the specified field from value.
      /// Start searching for the field at * cachedIndex.
      /// Afterwards, * cachedIndex holds the index of the field.
      inline void setValue (const std::string & fieldName, const char *value, int *cachedIndex);

      /// Set the value of the specified field to value.
      /// Type must match the type of the field, or a assertion failure will follow.
      template < class Type > inline void setValue (const std::string & fieldName, const Type & value);
      /// Set the value of the specified field to value.
      /// Type must match the type of the field, or a assertion failure will follow.
      /// Start searching for the field at * cachedIndex.
      /// Afterwards, * cachedIndex holds the index of the field.
      template < class Type > inline void setValue (const std::string & fieldName,
                                                    const Type & value, int *cachedIndex);

      /// Extract the value for the field at the specified index from value.
      inline void setValue (int index, const char *value);

      /// Set the value of the field at the specified index to value.
      /// Type must match the type of the field, or a assertion failure will follow.
      template < class Type > inline void setValue (int index, const Type & value);

#ifndef sun
      /// Return the value of the specified field.
      template < class Type > inline const Type & getValue (const std::string & fieldName);
      /// Return the value of the specified field.
      /// Start searching for the field at * cachedIndex.
      /// Afterwards, * cachedIndex holds the index of the field.
      template < class Type > inline const Type & getValue (const std::string & fieldName, int *cachedIndex);

      /// Return the value of the field with the specified index.
      template < class Type > inline const Type & getValue (int index);
#endif

      /// Return the value of the specified field.
      /// The value argument is to assist some (Sun) compilers in deducing the proper
      /// function template instance.
      template < class Type > inline const Type & getValue (const std::string & fieldName, Type * value);
      /// Return the value of the specified field.
      /// Start searching for the field at * cachedIndex.
      /// Afterwards, * cachedIndex holds the index of the field.
      /// The value argument is to assist some (Sun) compilers in deducing the proper
      /// function template instance.
      template < class Type > inline const Type & getValue (const std::string & fieldName,
                                                            Type * value, int *cachedIndex);

      template < class Type > inline const Type & getValue (int index, Type * value);

      void copyFrom (Record & record);
      Record (const TableDefinition & tableDefinition, Table * table);
      Record (Record & record);

    private:
      friend class Table;
      friend class Transaction;

      Table * m_table;

      Transaction * m_transaction;

      typedef std::vector< void *> FieldList;
      typedef FieldList::iterator FieldListIterator;

      const TableDefinition & m_tableDefinition;
      FieldList m_fields;

      void *getField (const std::string & name, int *cachedIndex = 0);
      inline void *getField (int index);
 
      // Record (const TableDefinition & tableDefinition, Table * table);
      //      Record (Record & record);
      void createFields (void);

      ~Record (void);
      void removeFields (void);

      //     void copyFrom (Record & record);

      template <class Type> void * copyField (Record * srcRecord, size_t index, Type * ptr);

      void destroyYourself (void);
      void addToTable (void);

      inline void setTransaction (Transaction * transaction);

      inline void lock (Transaction * transaction = 0);
      inline void unlock (Transaction * transaction = 0);

      bool saveToStream (ostream & ofile, bool rowBased);
      bool saveFieldToStream (ostream & ofile, int fieldIndex, int &borrowed);

      bool loadFromLine (std::string & line, std::vector < int >&dataToFieldMap);
      bool assignFromStringToIndex (const std::string & word, int toIndex);

   };

   /// Transaction objects handle ongoing database transactions.
   /// A Transaction consists of a list of old and a list of new Record objects.
   /// Transactions can be used to create, delete or modify Record objects.
   /// You can either commit or roll back a transaction
   class TABLEIO_DLL_EXPORT Transaction
   {
      typedef std::vector < Record * >RecordList;
      typedef RecordList::iterator RecordListIterator;

      typedef std::pair <Record *, Record * > RecordPair;
      typedef std::vector < RecordPair > RecordPairList;
      typedef RecordPairList::iterator RecordPairListIterator;
      typedef RecordPairList::reverse_iterator RecordPairListReverseIterator;

    public:
      /// This method will undo all changes made under control of this Transaction.
      /// It will also destroy the Transaction
      void rollBack (void);
      /// This method will finalize all changes made under control of this Transaction
      /// It will also destroy the Transaction
      void commit (void);

      /// Create a sub transaction for this Transaction
      Transaction * createSubTransaction (void);

    private:
      friend class Database;
      friend class Table;
      friend class Record;

      const Database * m_database;
      Transaction * m_child;
      Transaction * m_parent;

      RecordList m_createdRecords;
      RecordPairList m_deletedRecords;
      RecordPairList m_editedRecords;

      inline void setChild (Transaction * transaction);
      inline void setParent (Transaction * transaction);

      inline Transaction * getChild (void);
      inline Transaction * getParent (void);

      bool descendsFrom (Transaction * transaction);
      bool ascends (Transaction * transaction);

      inline void addCreatedRecord (Record * record);
      inline void addDeletedRecord (Record * record, Record * nextRecord);
      inline void addEditedRecord (Record * record, Record * copiedRecord);

      Transaction (const Database * database);
      ~Transaction (void);
   };

   /// Signature definition of a partial Table ordering function.
   typedef bool (*OrderingFunc) (Record *, Record *);
   /// Signature definition of a Record equality function.
   typedef bool (*EqualityFunc) (Record *, Record *);
   /// Signature definition of a Record merge function.
   typedef bool (*MergeFunc) (Record *, Record *);

   /// An object of this class contains a list of Records with the same TableDefinition
   class TABLEIO_DLL_EXPORT Table
   {
      typedef std::vector < Record * >RecordList;
      typedef RecordList::iterator RecordListIterator;

    public:
      /// Forward iterator type used to iterate through the Records of a Table.
      typedef RecordList::iterator iterator;

      /// Create a new Record in this Table, under control of a Transaction
      Record * createRecord (Transaction * transaction = 0);

      /// Find the position of a record
      Table::iterator findRecordPosition (Record * record);

      /// move record1 to the position before record2
      bool moveRecord (Record * record1, Record * record2);
      /// insert record1 into the position before record2
      bool insertRecord (Record * record1, Record * record2);
      /// Erase the specified record without deleting it
      bool eraseRecord (Record * record);

      /// Remove the specified Record from this Table, returns the next record in the table.
      Record * removeRecord (Record * record);

      /// remove the record from the table partitionings
      bool removeRecordFromTablePartitionings (Record * record);

      /// Remove the specified Record from this Table and destroy it, under control of a Transaction.
      bool deleteRecord (Record * record, Transaction * transaction = 0);
      bool deleteRecordTransactional (Record * record, Record * recordNext, Transaction * transaction = 0);

      /// Remove the specified Record from this Table and destroy it, under control of a Transaction.
      Table::iterator deleteRecord (Table::iterator & iter, Transaction * transaction = 0);
      Table::iterator removeRecord (Table::iterator & iter);

      void addRecord (Record * record);

      /// Edit the specified Record under control of a Transaction.
      /// The returned Record is the one to modify.
      Record * editRecord (Record * record, Transaction * transaction);

      /// Sort the records with the specified ordering function.
      void sort (OrderingFunc func);
      void stable_sort (OrderingFunc func);
      /// Remove duplicate records based on the specified equality function after merging them based on the specified merge function
      void unique (EqualityFunc equalityFunc, MergeFunc mergeFunc = 0);
      /// Return the name of the Table.
      inline const std::string & name (void);

      inline Record * getRecord (Table::iterator & iter);

      /// return the table definition
      inline const TableDefinition & getTableDefinition () const;

      /// Return the number of Records in the Table.
      inline size_t size (void);

      /// Return the Record at the specified index
      inline Record *getRecord (int i);
      /// Return the Record at the specified index
      inline Record *operator[] (int i);

      /// Find a record in which the specified field has the specified value
      template < class Type > inline Record * findRecord (const std::string & fieldName, const Type & value);
      Record * findRecord (const std::string & fieldName, const char * value);

      template < class Type > inline Record * findRecord (const std::string & fieldName1, const Type & value1, 
                                                          const std::string & fieldName2, const Type & value2, Record * record1 = 0);
      Record * findRecord (const std::string & fieldName1, const char * value1, const std::string & fieldName2, const char * value2, Record * record1 = 0);

      /// Remove all Records from this Table and destroy them as requested.
      void clear (bool deleteRecords = true);

      /// Get the index of a field in a Record of this Table.
      inline int getIndex (const std::string & fieldName);

      /// Save this Table to a file with the specified name.
      bool saveToFile (const std::string & filename);
      /// Fill this Table from a file with the specified name.
      bool loadFromFile (const std::string & filename);

      /// Save this Table to the specified output stream.
      bool saveToStream (ostream & ofile);
      /// Fill this Table from the specified input stream.
      bool loadFromStream (istream & ifile);

      /// Return an iterator pointing to the beginning of the Table.
      inline Table::iterator begin (void);
      /// Return an iterator pointing to the end of the Table.
      inline Table::iterator end (void);

      /// get subtable based on the specified partitioning by
      /// specifying a value for the field name used in the partitioning
      template < class Type > Table * getSubTable (const std::string& fieldName, Type value);

    private:
      friend class Database;
      friend class Record;
      friend class TablePartitioning;

      const TableDefinition & m_tableDefinition;
      RecordList m_records;

      TablePartitioning ** m_tablePartitionings;

      Table (const TableDefinition & tableDefinition);
      ~Table (void);

      /// Get the partitioning for the field name
      TablePartitioning * getPartitioning (const std::string & fieldName);
      /// Remove the partitionings
      void resetPartitionings ();
      /// clear the partitionings
      void clearPartitionings ();

      bool loadDataToFieldMapFromLine (std::string & line, std::vector < int >&dataToFieldMap);
      bool loadUnitsFromStream (istream & infile);
      bool loadRecordsFromStream (istream & infile, std::vector < int >&dataToFieldMap);
      bool loadRowBasedRecordsFromStream (istream & infile);

   };


   class TABLEIO_DLL_EXPORT TablePartitioning
   {
      public:
	 TablePartitioning (Table * table, size_t fieldIndex);
	 ~TablePartitioning (void);

	 void clear ();

	 inline Table * getTable ();
	 template < class Type > Table * getSubTable (Type value);

	 bool removeRecord (Record * record);

	 bool syncSubTables (void);

      protected:
	 Table * m_table;
	 void * /* std::map<Type, Table *> * */ m_subTables;

	 template < class Type > Table * getSubTableImpl (Type value);
	 template < class Type > void deleteSubTables ();
	 template < class Type > void clearSubTables ();

	 size_t m_partitioningFieldIndex;
	 FieldDefinition * m_partitioningFieldDefinition;

	 int m_sizeSubTables;
   };

   /// A Database consists of a list of Tables as specified by the DataSchema
   /// it was created from.
   class TABLEIO_DLL_EXPORT Database
   {
      typedef std::vector < Table * >TableList;
      typedef TableList::iterator TableListIterator;

      typedef std::vector < Transaction * >TransactionList;
      typedef TransactionList::iterator TransactionListIterator;

    public:
      /// Forward iterator type used to iterate through the Tables of a Database.
      typedef TableList::iterator iterator;

      /// Create a new Database from the given DataSchema.
      static Database *CreateFromSchema (const DataSchema & dataSchema);
      /// Create a new Database from the given DataSchema and fill its Tables from the specified file.
      static Database *CreateFromFile (const std::string & filename, const DataSchema & dataSchema);

      /// Destroy this Database.
      ~Database (void);

      /// Add text to the database header
      void addHeader (const std::string & text);
      /// Clear the database header
      void clearHeader (void);

      Transaction * createTransaction (void);

      /// Get the Table with the specified name.
      Table *getTable (const std::string & name);
      /// Get the Table at the specified index.
      Table *getTable (int index);

      /// Clear the table with the specified name.
      bool clearTable (const std::string & name, bool deleteRecords = true);

      /// Save the contents of this database to the specified file.
      bool saveToFile (const std::string & filename);

      /// Save the contents of this database back to the file from which it was read.
      bool resave (void);

      /// Fill the tables of this database from the specified file.
      bool loadFromFile (const std::string & filename);

      /// Return an iterator pointing to the beginning of the Database.
      inline Database::iterator begin (void);
      /// Return an iterator pointing to the end of the Database.
      inline Database::iterator end (void);

      inline static void SetFieldWidth (int fieldWidth);
      inline static int GetFieldWidth (void);

      inline static void SetPrecision (int precision);
      inline static int GetPrecision (void);

      inline static void SetMaxFieldsPerLine (int fields);
      inline static int GetMaxFieldsPerLine (void);

      bool saveToStream (ostream & ofile);

    private:
      static int s_maxFieldsPerLine;
      static int s_fieldWidth;
      static int s_precision;

      const DataSchema & m_dataSchema;

      std::string m_fileName;

      TableList m_tables;
      TransactionList m_transactions;

      std::string m_header;

      Database (const DataSchema & dataSchema);

      bool skipTableFromStream (istream & infile);

      bool loadFromStream (istream & ifile);

      bool saveHeader (ostream & ofile);

   };

   ///////////////////////////////////////////////////////////////////////
   // IMPLEMENTATIONS ////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////
   Database::iterator Database::begin (void)
   {
      return m_tables.begin ();
   }

   Database::iterator Database::end (void)
   {
      return m_tables.end ();
   }

   void Database::SetPrecision (int precision)
   {
      s_precision = precision;
   }

   int Database::GetPrecision (void)
   {
      return s_precision;
   }

   void Database::SetMaxFieldsPerLine (int fields)
   {
      s_maxFieldsPerLine = fields;
   }

   int Database::GetMaxFieldsPerLine (void)
   {
      return s_maxFieldsPerLine;
   }

   void Database::SetFieldWidth (int fieldWidth)
   {
      s_fieldWidth = fieldWidth;
   }

   int Database::GetFieldWidth (void)
   {
      return s_fieldWidth;
   }

   Table * TablePartitioning::getTable ()
   {
      return m_table;
   }

   template < class Type >
   Table * TablePartitioning::getSubTable (Type value)
   {
      syncSubTables ();
      return getSubTableImpl (value);
   }

   template < class Type >
   Table * TablePartitioning::getSubTableImpl (Type value)
   {
      checkType < Type > (value, m_partitioningFieldDefinition->dataType ());

      Table * subTable = (* (std::map<Type, Table *> *) m_subTables)[value];
      if (!subTable)
      {
	 subTable = new Table (m_table->getTableDefinition ());
	 (* (std::map<Type, Table *> *) m_subTables)[value] = subTable;
      }
      return subTable;
   }

   template < class Type >
   void TablePartitioning::deleteSubTables ()
   {
      std::map<Type, Table *> * subTables = (std::map<Type, Table *> *) m_subTables;
      if (!subTables) return;

      typename std::map<Type,Table *>::iterator mapIterator;
      for (mapIterator = subTables->begin (); mapIterator != subTables->end (); ++mapIterator)
      {
	 Table * subTable = (*mapIterator).second;
	 subTable->clear (false);
	 delete subTable;
      }

      delete subTables;
      m_subTables = 0;
   }

   template < class Type >
   void TablePartitioning::clearSubTables ()
   {
      std::map<Type, Table *> * subTables = (std::map<Type, Table *> *) m_subTables;
      if (!subTables) return;

      typename std::map<Type,Table *>::iterator mapIterator;
      for (mapIterator = subTables->begin (); mapIterator != subTables->end (); ++mapIterator)
      {
	 Table * subTable = (*mapIterator).second;
	 subTable->clear (false);
      }
   }

   void Transaction::setChild (Transaction * transaction)
   {
      m_child = transaction;
   }

   void Transaction::setParent (Transaction * transaction)
   {
      m_parent = transaction;
   }

   Transaction * Transaction::getChild (void)
   {
      return m_child;
   }

   Transaction * Transaction::getParent (void)
   {
      return m_parent;
   }

   void Transaction::addCreatedRecord (Record * record)
   {
      m_createdRecords.push_back (record);
   }

   void Transaction::addEditedRecord (Record * record, Record * copiedRecord)
   {
      m_editedRecords.push_back (RecordPair (record, copiedRecord));
   }

   void Transaction::addDeletedRecord (Record * record, Record * nextRecord)
   {
      m_deletedRecords.push_back (RecordPair (record, nextRecord));
   }

   Record * Table::getRecord (Table::iterator & iter)
   {
      return (iter == m_records.end () ? 0 : * iter);
   }

   size_t Table::size (void)
   {
      return m_records.size ();
   }

   const std::string & Table::name (void)
   {
      return m_tableDefinition.name ();
   }

   const TableDefinition & Table::getTableDefinition () const
   {
      return m_tableDefinition;
   }

   Record *Table::operator[] (int i)
   {
      return getRecord (i);
   }

   Record *Table::getRecord (int i)
   {
      if (i >= 0 && i < (int) size ())
	 return m_records[i];
      else
	 return 0;
   }

   template < class Type > Record * Table::findRecord (const std::string & fieldName, const Type & value)
   {
      int index = getIndex (fieldName);
      if (index < 0) return 0;

      Table::iterator recordIter;
      for (recordIter = begin (); recordIter != end (); ++recordIter)
      {
	 Record * record = * recordIter;
	 const Type & foundValue = record->getValue (index, (Type *) 0);

	 if (value == foundValue) return record;
      }
      return 0;
   }

   template < class Type > Record * Table::findRecord (const std::string & fieldName1, const Type & value1, 
                                                       const std::string & fieldName2, const Type & value2, Record * record1)
   {
      int index1 = getIndex (fieldName1);
      if (index1 < 0) return 0;

      int index2 = getIndex (fieldName2);
      if (index2 < 0) return 0;

      Table::iterator recordIter;
      for (recordIter = begin (); recordIter != end (); ++recordIter)
      {
	 Record * record = * recordIter;
	 const Type & foundValue1 = record->getValue (index1, (Type *) 0);

         if (value1 == foundValue1) {
            const Type & foundValue2 = record->getValue (index2, (Type *) 0);
            if (value2 == foundValue2 ) {
               if( record != record1 ) {
                  return record;
               }
            }
         }
      }
      return 0;
   }

   int Table::getIndex (const std::string & fieldName)
   {
      return m_tableDefinition.getIndex (fieldName);
   }

   Table::iterator Table::begin (void)
   {
      return m_records.begin ();
   }

   Table::iterator Table::end (void)
   {
      return m_records.end ();
   }

   template < class Type > Table * Table::getSubTable (const std::string & fieldName, Type value)
   {
      TablePartitioning * tablePartitioning = getPartitioning (fieldName);
      assert (tablePartitioning);
      return tablePartitioning->getSubTable (value);
   }

   const std::string & Record::tableName (void)
   {
      return getTable()->name ();
   }

   void * Record::getField (int index)
   {
      return m_fields[index];
   }

   void Record::setTransaction (Transaction * transaction)
   {
      m_transaction = transaction;
   }

   void Record::lock (Transaction * transaction)
   {
      m_transaction = transaction;
   }

   void Record::unlock (Transaction * transaction)
   {
      if (m_transaction == transaction) m_transaction = 0;
   }

   bool Record::isLocked (Transaction * transaction)
   {
      return m_transaction != 0 && !(m_transaction->ascends (transaction));
   }

   bool Record::isLockedBy (Transaction * transaction)
   {
      return m_transaction != 0 && m_transaction->ascends (transaction);
   }

   int Record::getIndex (const std::string & fieldName)
   {
      return m_tableDefinition.getIndex (fieldName);
   }

   const TableDefinition & Record::getTableDefinition (void) const
   {
      return m_tableDefinition;
   }

   Table * Record::getTable (void)
   {
      return m_table;
   }

   template < class Type > void Record::setValue (const std::string & fieldName, const Type & value)
   {
      Field < Type > *field = (Field < Type > *)getField (fieldName, 0);
      assert (field);

      field->setValue (value);
   }

   template < class Type > void Record::setValue (const std::string & fieldName, const Type & value, int *cachedIndex)
   {
      Field < Type > *field = (Field < Type > *)getField (fieldName, cachedIndex);
      assert (field);

      field->setValue (value);
   }

   void Record::setValue (const std::string & fieldName, const char *value)
   {
      Field < std::string > *field = (Field < std::string > *)getField (fieldName, 0);
      assert (field);

      field->setValue (value);
   }

   void Record::setValue (const std::string & fieldName, const char *value, int *cachedIndex)
   {
      Field < std::string > *field = (Field < std::string > *)getField (fieldName, cachedIndex);
      assert (field);

      field->setValue (value);
   }

   template < class Type > void Record::setValue (int index, const Type & value)
   {
      Field < Type > *field = (Field < Type > *)getField (index);
      assert (field);

      field->setValue (value);
   }

   void Record::setValue (int index, const char *value)
   {
      Field < std::string > *field = (Field < std::string > *)getField (index);
      assert (field);

      field->setValue (value);
   }

#ifndef sun
   template < class Type > const Type & Record::getValue (const std::string & fieldName)
   {
      Field < Type > *field = (Field < Type > *)getField (fieldName, 0);
      assert (field);

      return field->getValue ();
   }

   template < class Type > const Type & Record::getValue (const std::string & fieldName, int *cachedIndex)
   {
      Field < Type > *field = (Field < Type > *)getField (fieldName, cachedIndex);
      assert (field);

      return field->getValue ();
   }

   template < class Type > const Type & Record::getValue (int index)
   {
      Field < Type > *field = (Field < Type > *)getField (index);
      assert (field);

      return field->getValue ();
   }
#endif

   template < class Type > const Type & Record::getValue (const std::string & fieldName, Type * value)
   {
      Field < Type > *field = (Field < Type > *)getField (fieldName, 0);
      assert (field);

      return field->getValue (value);
   }

   template < class Type > const Type & Record::getValue (const std::string & fieldName, Type * value, int *cachedIndex)
   {
      Field < Type > *field = (Field < Type > *)getField (fieldName, cachedIndex);
      assert (field);

      return field->getValue (value);
   }

   template < class Type > const Type & Record::getValue (int index, Type * value)
   {
      Field < Type > *field = (Field < Type > *)getField (index);
      assert (field);

      return field->getValue (value);
   }

   template < class Type > Field < Type >::~Field (void)
   {
   }

   template < class Type > const FieldDefinition & Field < Type >::getFieldDefinition (void)
   {
      return m_fieldDefinition;
   }

   template < class Type > void Field < Type >::setValue (const Type & value)
   {
      checkType < Type > (value, m_fieldDefinition.dataType ());

      m_value = value;
   }

#ifndef sun
   template < class Type > const Type & Field < Type >::getValue (void)
   {
      checkType < Type > (m_value, m_fieldDefinition.dataType ());

      return m_value;
   }
#endif

   template < class Type > const Type & Field < Type >::getValue (Type * value)
   {
      checkType < Type > (m_value, m_fieldDefinition.dataType ());

      if (value)
         *value = m_value;
      return m_value;
   }
}
#endif
