//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
// This utility allow to load and then compare table by table 2 .project3d files

#ifndef DATABASE_H
#define DATABASE_H

#include "dataschema.h"
#include "datatype.h"

#include <assert.h>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

using std::istream;
using std::ofstream;
using std::ostream;

namespace database
{
   class Database;
   class Table;
   class Record;

   template < class Type > void checkType (     const Type        &a, const datatype::DataType type );
   template <> void checkType < bool > (        const bool        &a, const datatype::DataType type );
   template <> void checkType < int >(          const int         &a, const datatype::DataType type );
   template <> void checkType < long >(         const long        &a, const datatype::DataType type );
   template <> void checkType < float >(        const float       &a, const datatype::DataType type );
   template <> void checkType < double >(       const double      &a, const datatype::DataType type );
   template <> void checkType < std::string > ( const std::string &a, const datatype::DataType type );

   /// Fields contain the values of a Record.
   /// This class is a template because we need a different Field class for different types,
   /// e.g. int, long, double, float, string.
   /// This class is used only for implementation purposes.
   class AbstractField
   {
      public:
         virtual ~AbstractField() {}

         explicit AbstractField( const FieldDefinition & fieldDefinition )
            : m_fieldDefinition(fieldDefinition)
         {}

         const FieldDefinition & getFieldDefinition() const
         { return m_fieldDefinition; }

         std::shared_ptr<AbstractField> clone() const
         { return std::shared_ptr<AbstractField>( doClone() ); }

         virtual bool assignFromString (const std::string & word) = 0;
         virtual bool saveToStream (ostream & ofile, int &borrowed) = 0;

      private:
         virtual AbstractField * doClone() const = 0;

         const FieldDefinition & m_fieldDefinition;
   };

   template < class Type > class Field : public AbstractField
   {
      public:
         virtual ~Field() {}

         /// save the value to a stream.
         bool saveToStream (ostream & ofile, int &borrowed);

         explicit Field (const FieldDefinition & fieldDef)
            : AbstractField(fieldDef)
         {
            assignFromString(fieldDef.defaultValue());
         }


         void setValue (const Type & value)
         {
            checkType < Type > (value, getFieldDefinition().dataType ());
            m_value = value;
         }

         const Type & getValue() const
         {
            checkType < Type > (m_value, getFieldDefinition().dataType ());
            return m_value;
         }

         virtual bool assignFromString (const std::string & word);

      private:
         virtual Field<Type> * doClone() const
         {
            return new Field<Type>( *this );
         }

         Type m_value;
   };


   /// This class describes the entries of a Database Table.
   /// A Record consists of a number of Fields whose values can be set or retrieved.
   class Record
   {
   public:
      // destructor
      ~Record() {}

      /// print the record's content
      void printOn (ostream &);

      /// Return the table name
      const std::string & tableName() const;

      /// Return the list index of the field with the specified name.
      int getIndex (const std::string & fieldName);

      const TableDefinition & getTableDefinition() const;
      Table * getTable() const;


      template <typename Type>
      void setValue (size_t index, const Type & value)
      {
         std::dynamic_pointer_cast< Field < Type > >(getField (index))->setValue(value);
      }

      template < class Type >
      void setValue (const std::string & fieldName, const Type & value, int * cachedIndex = nullptr) const
      {
         std::dynamic_pointer_cast< Field < Type > >( getField (fieldName, cachedIndex) )->setValue(value);
      }

      template <typename Type>
      const Type & getValue (size_t index) const
      {
         return std::dynamic_pointer_cast< const Field < Type > >(getField (index))->getValue();
      }

      template < class Type >
      const Type & getValue (const std::string & fieldName, int * cachedIndex = nullptr) const
      {
         return std::dynamic_pointer_cast< const Field < Type > >( getField (fieldName, cachedIndex) )->getValue();
      }

      Record (const TableDefinition & tableDefinition, Table * table);
      Record (const Record & record);
      Record (Record&& record) = delete;

      Record& operator=(const Record& record) = delete;
      Record& operator=(Record&& record) = delete;

      /// \brief Construct a new record.
      ///
      /// Copy the contets of the old record but assign it to the new table.
      Record (const Record & record, Table * table);

      /// \brief Get a copy of a record, but is associated with another table reference.
      ///
      /// Table must have same name as current record otherwise a nullptr will be returned.
      Record* deepCopy ( Table * table ) const;

   private:
      friend class Table;

      typedef std::vector< std::shared_ptr< AbstractField > > FieldList;
      typedef FieldList::iterator FieldListIterator;

      std::shared_ptr<AbstractField> getField (size_t index) const { return m_fields[index]; }

      std::shared_ptr<AbstractField> getField(const std::string & name, int * index) const;

      // Record (const TableDefinition & tableDefinition, Table * table);
      //      Record (Record & record);
      void createFields();

      void destroyYourself();
      void addToTable();


      bool saveToStream (ostream & ofile, bool rowBased);
      bool saveFieldToStream (ostream & ofile, int fieldIndex, int &borrowed);

      bool loadFromLine (const std::string & line, std::vector < int >&dataToFieldMap);
      bool assignFromStringToIndex (const std::string & word, int toIndex);


      Table * m_table;
      const TableDefinition & m_tableDefinition;
      FieldList m_fields;
   };


   /// Signature definition of a partial Table ordering function.
   typedef bool (*OrderingFunc) (Record *, Record *);
   /// Signature definition of a Record equality function.
   typedef bool (*EqualityFunc) (Record *, Record *);
   /// Signature definition of a Record merge function.
   typedef bool (*MergeFunc) (Record *, Record *);

   /// An object of this class contains a list of Records with the same TableDefinition
   class Table
   {
      typedef std::vector < Record * >RecordList;
      typedef RecordList::iterator RecordListIterator;

   public:
      /// Forward iterator type used to iterate through the Records of a Table.
      typedef RecordList::iterator iterator;
      typedef RecordList::const_iterator const_iterator;

      /// Create a new Record in this Table
      Record * createRecord (bool addToFile = true);

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

      /// Remove the specified Record from this Table and destroy it
      bool deleteRecord (Record * record);

      /// Remove the specified Record from this Table and destroy it
      Table::iterator removeRecord (Table::iterator & iter);

      void addRecord (Record * record);

      /// Sort the records with the specified ordering function.
      void sort (OrderingFunc func);
      void stable_sort (OrderingFunc func);
      /// Sort the records according to the given fields list
      void stable_sort (const std::vector<std::string> & fldList);

      /// Remove duplicate records based on the specified equality function after merging them based on the specified merge function
      void unique (EqualityFunc equalityFunc, MergeFunc mergeFunc = nullptr);

      /// return the name of the Table.
      const std::string & name() { return m_tableDefinition.name(); }

      /// return verision of data schema in which this table was written. Used in table upgrade scheme
      int version() { return m_version; }

      Record * getRecord (const Table::iterator & iter) const;

      /// return the table definition
      const TableDefinition & getTableDefinition () const;

      /// Return the number of Records in the Table.
      size_t size() { return m_records.size(); }

      /// Return the Record at the specified index
      Record *getRecord (int i);
      /// Return the Record at the specified index
      Record *operator[] (int i);

      /// Find a record in which the specified field has the specified value
      Record * findRecord( const std::string & fieldName, const std::string & value );
      Record * findRecord( const std::string & field1, const std::string & value1, const std::string & field2,
                           const std::string & value2, Record * other = nullptr);

      /// Remove all Records from this Table and destroy them as requested.
      void clear (bool deleteRecords = true);

      /// Get the index of a field in a Record of this Table.
      int getIndex (const std::string & fieldName);

      /// Save this Table to a file with the specified name.
      bool saveToFile (const std::string & filename);
      /// Fill this Table from a file with the specified name.
      bool loadFromFile (const std::string & filename);

      /// Save this Table to the specified output stream.
      bool saveToStream (ostream & ofile);
      /// Fill this Table from the specified input stream.
      bool loadFromStream (istream & ifile);

      /// Return an iterator pointing to the beginning of the Table.
      Table::iterator begin();
      /// Return an iterator pointing to the end of the Table.
      Table::iterator end();
      /// Return an const_iterator pointing to the beginning of the Table.
      Table::const_iterator begin() const;
      /// Return an const_iterator pointing to the end of the Table.
      Table::const_iterator end() const;

      /// \brief Copy the records in the current table to the table passed as a parameter.
      ///
      /// Contents will be copied only if the table is not a nullptr and the table
      /// name matches the current table.
      void copyTo ( Table* table ) const;

   private:

      friend class Database;
      friend class Record;

      const TableDefinition & m_tableDefinition;
      RecordList m_records;
      int        m_version;

      explicit Table (const TableDefinition & tableDefinition);
      ~Table();

      /// Get the partitioning for the field name
      /// Remove the partitionings
      void resetPartitionings ();
      /// clear the partitionings
      void clearPartitionings ();

      bool loadDataToFieldMapFromLine (std::string & line, std::vector < int >&dataToFieldMap);
      bool loadUnitsFromStream (istream & infile);
      bool loadRecordsFromStream (istream & infile, std::vector < int >&dataToFieldMap);
      bool loadRowBasedRecordsFromStream (istream & infile);

      void setVersion( int ver ) { m_version = ver; }
   };


   /// A Database consists of a list of Tables as specified by the DataSchema
   /// it was created from.
   class Database
   {
      typedef std::vector < Table * >TableList;
      typedef TableList::iterator TableListIterator;

   public:
      /// Forward iterator type used to iterate through the Tables of a Database.
      typedef TableList::iterator iterator;

      /// Create a new Database from the given DataSchema.
      static Database *CreateFromSchema (const DataSchema & dataSchema);
      /// Create a new Database from the given DataSchema and fill its Tables from the specified file.
      static Database *CreateFromFile (const std::string & filename, const DataSchema & dataSchema);

      /// Destroy this Database.
      ~Database();

      /// Return the number of Tables in the Database.
      size_t size() { return m_tables.size(); }

      /// Add text to the database header
      void addHeader (const std::string & text);
      /// Clear the database header
      void clearHeader();

      /// \brief Add a table to both the Cauldron schema and the database.
      ///
      /// If the database has a table with the same name already then it will not be added.
      /// The return value indicates whether or not the table was added (true it was added, false it otherwise).
      bool addTableDefinition ( TableDefinition* tblDef );

      /// \brief Remove a table from the database
      ///
      /// The table reference is also removed from the schema.
      void removeTable ( Table* table );

      /// \brief Remove a table from the database and delete it.
      ///
      /// The table reference is also removed from the schema.
      void deleteTable ( const std::string& tableName );

      /// \brief Determine whether or not the database has a table with the name.
      bool hasTable ( const std::string& name ) const;

      /// \brief Determine whether or not the database has a table.
      bool hasTable ( const Table* table ) const;

      /// Get the Table with the specified name.
      Table *getTable (const std::string & name) const;
      /// Get the Table at the specified index.
      Table *getTable (int index) const;

      /// Clear the table with the specified name.
      bool clearTable (const std::string & name, bool deleteRecords = true);

      /// Save the contents of this database to the specified file.
      bool saveToFile (const std::string & filename);

      /// Save the contents of this database back to the file from which it was read.
      bool resave();

      /// Fill the tables of this database from the specified file.
      bool loadFromFile (const std::string & filename);

      /// Reread file again, or if tblName is given, just this table
      bool reload( std::string tblName = "" );

      /// Get the file name
      const std::string& getFileName() const { return m_fileName; }

      /// Return an iterator pointing to the beginning of the Database.
      Database::iterator begin() { return m_tables.begin (); }

      /// Return an iterator pointing to the end of the Database.
      Database::iterator end() { return m_tables.end (); }

      static void SetFieldWidth (int fieldWidth);
      static int GetFieldWidth();

      static void SetPrecision (int precision);
      static int GetPrecision();

      static void SetMaxFieldsPerLine (int fields);
      static int GetMaxFieldsPerLine();

      bool saveToStream (ostream & ofile);

      /// \brief Get a constant reference to the data schema.
      const DataSchema& getDataSchema () const;

      /// @brief Get list of table which were recognized in file
      const std::map<std::string, bool> & getTablesInFile()  { return m_tablesInFile; }

   private:

      static int s_maxFieldsPerLine;
      static int s_fieldWidth;
      static int s_precision;

      std::map<std::string,bool> m_tablesInFile; // tables list in file: true for loaded, false for skipped

      DataSchema& m_dataSchema;

      std::string m_fileName;

      TableList m_tables;

      std::string m_header;

      explicit Database (DataSchema& dataSchema);

      bool skipTableFromStream (istream & infile);

      bool loadFromStream (istream & ifile);

      bool saveHeader (ostream & ofile);

   };

   ///////////////////////////////////////////////////////////////////////
   // IMPLEMENTATIONS ////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////

   inline void Database::SetPrecision (int precision)
   {
      s_precision = precision;
   }

   inline int Database::GetPrecision()
   {
      return s_precision;
   }

   inline void Database::SetMaxFieldsPerLine (int fields)
   {
      s_maxFieldsPerLine = fields;
   }

   inline int Database::GetMaxFieldsPerLine()
   {
      return s_maxFieldsPerLine;
   }

   inline void Database::SetFieldWidth (int fieldWidth)
   {
      s_fieldWidth = fieldWidth;
   }

   inline int Database::GetFieldWidth()
   {
      return s_fieldWidth;
   }

   inline const DataSchema& Database::getDataSchema () const {
      return m_dataSchema;
   }

   inline Record * Table::getRecord (const iterator& iter) const
   {
      return (iter == m_records.end () ? nullptr : * iter);
   }

   inline const TableDefinition & Table::getTableDefinition () const
   {
      return m_tableDefinition;
   }

   inline Record *Table::operator[] (int i)
   {
      return getRecord (i);
   }

   inline Record *Table::getRecord (int i)
   {
      if (i >= 0 && i < static_cast<int>(size ()))
         return m_records[i];
      else
         return nullptr;
   }

   inline int Table::getIndex (const std::string & fieldName)
   {
      return m_tableDefinition.getIndex (fieldName);
   }

   inline Table::iterator Table::begin()
   {
      return m_records.begin ();
   }

   inline Table::iterator Table::end()
   {
      return m_records.end ();
   }

   inline Table::const_iterator Table::begin() const
   {
      return m_records.begin();
   }

   inline Table::const_iterator Table::end() const
   {
      return m_records.end();
   }

   inline int Record::getIndex (const std::string & fieldName)
   {
      return m_tableDefinition.getIndex (fieldName);
   }

   inline const TableDefinition & Record::getTableDefinition() const
   {
      return m_tableDefinition;
   }

   inline Table * Record::getTable() const
   {
      return m_table;
   }
}
#endif
