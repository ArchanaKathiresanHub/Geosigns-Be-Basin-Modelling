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

#include <boost/shared_ptr.hpp>
#include <boost/pointer_cast.hpp>

namespace database
{


	class Database;
	class Table;
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
        class AbstractField
        {
        public:
           virtual ~AbstractField() {} 

           AbstractField(const FieldDefinition & fieldDefinition)
              : m_fieldDefinition(fieldDefinition)
           {}

           const FieldDefinition & getFieldDefinition (void) const
           { return m_fieldDefinition; }

           boost::shared_ptr<AbstractField> clone() const
           { return boost::shared_ptr<AbstractField>( doClone() ); }

           virtual bool assignFromString (const std::string & word) = 0;
           virtual bool saveToStream (ostream & ofile, int &borrowed) = 0;

        private:
           virtual AbstractField * doClone() const = 0;

           const FieldDefinition & m_fieldDefinition;
        };

	template < class Type > class Field : public AbstractField
	{
	 public:
		virtual ~Field (void) {}

		/// save the value to a stream.
		bool saveToStream (ostream & ofile, int &borrowed);

		Field (const FieldDefinition & fieldDef)
                   : AbstractField(fieldDef)
                { 
                   assignFromString(fieldDef.defaultValue());
                }


		void setValue (const Type & value)
                {
                   checkType < Type > (value, getFieldDefinition().dataType ());
                   m_value = value;
                }

		const Type & getValue (void) const
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
	class TABLEIO_DLL_EXPORT Record
	{
	 public:
		/// print the record's content
		void printOn (ostream &);

		/// Return the table name
		const std::string & tableName (void) const;

		/// Return the list index of the field with the specified name.
		inline int getIndex (const std::string & fieldName);

		inline const TableDefinition & getTableDefinition (void) const;
		inline Table * getTable (void) const;


                template <typename Type>
                void setValue (int index, const Type & value)
                {
                   boost::dynamic_pointer_cast< Field < Type > >(getField (index))->setValue(value);
                }

                template < class Type > 
                void setValue (const std::string & fieldName, const Type & value, int * cachedIndex = 0) const
                {
                   boost::dynamic_pointer_cast< Field < Type > >( getField (fieldName, cachedIndex) )->setValue(value);
                }

                template <typename Type>
                const Type & getValue (int index) const
                {
                   return boost::dynamic_pointer_cast< const Field < Type > >(getField (index))->getValue();
                }

                template < class Type > 
                const Type & getValue (const std::string & fieldName, int * cachedIndex = 0) const
                {
                   return boost::dynamic_pointer_cast< const Field < Type > >( getField (fieldName, cachedIndex) )->getValue();
                }

		Record (const TableDefinition & tableDefinition, Table * table);
		Record (const Record & record);

	 private:
		friend class Table;
                
                typedef std::vector< boost::shared_ptr< AbstractField > > FieldList;
		typedef FieldList::iterator FieldListIterator;

		~Record() {}
                
                boost::shared_ptr<AbstractField> getField (int index) const
                { return m_fields[index]; }

                boost::shared_ptr<AbstractField> getField(const std::string & name, int * index) const;
 
		// Record (const TableDefinition & tableDefinition, Table * table);
		//      Record (Record & record);
		void createFields (void);

		void destroyYourself (void);
		void addToTable (void);

		
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
	class TABLEIO_DLL_EXPORT Table
	{
		typedef std::vector < Record * >RecordList;
		typedef RecordList::iterator RecordListIterator;

	 public:
		/// Forward iterator type used to iterate through the Records of a Table.
		typedef RecordList::iterator iterator;

		/// Create a new Record in this Table
		Record * createRecord ();

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
		Record * findRecord(const std::string & fieldName, const std::string & value);
                Record * findRecord(const std::string & field1, const std::string & value1, const std::string & field2, const std::string & value2, Record * other = 0);


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

	 private:
		friend class Database;
		friend class Record;

		const TableDefinition & m_tableDefinition;
		RecordList m_records;

		Table (const TableDefinition & tableDefinition);
		~Table (void);

		/// Get the partitioning for the field name
		/// Remove the partitionings
		void resetPartitionings ();
		/// clear the partitionings
		void clearPartitionings ();

		bool loadDataToFieldMapFromLine (std::string & line, std::vector < int >&dataToFieldMap);
		bool loadUnitsFromStream (istream & infile);
		bool loadRecordsFromStream (istream & infile, std::vector < int >&dataToFieldMap);
		bool loadRowBasedRecordsFromStream (istream & infile);

	};


	/// A Database consists of a list of Tables as specified by the DataSchema
	/// it was created from.
	class TABLEIO_DLL_EXPORT Database
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
		~Database (void);

		/// Add text to the database header
		void addHeader (const std::string & text);
		/// Clear the database header
		void clearHeader (void);

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

	int Record::getIndex (const std::string & fieldName)
	{
		return m_tableDefinition.getIndex (fieldName);
	}

	const TableDefinition & Record::getTableDefinition (void) const
	{
		return m_tableDefinition;
	}

	Table * Record::getTable (void) const
	{
		return m_table;
	}



}
#endif
