//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
// This utility allow to load and then compare table by table 2 .project3d files

#ifndef DATASCHEMA_H
#define DATASCHEMA_H

// Note: Comments starting with '///' are suitable for parsing by 'doxygen' to produce readable documentation
// in the form of html, latex or rtf files.

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "datatype.h"

using std::istream;
using std::ofstream;
using std::ostream;

namespace database
{
   /// Objects of this class define the properties of a Field object.
   /// FieldDefinition objects are not created directly, but through
   /// the TableDefinition::addFieldDefinition () function
   class FieldDefinition
   {
   public:
      typedef enum { Volatile, Persistent } StorageType;

   private:
      friend class TableDefinition;
      friend class Record;

      const StorageType m_storageType;

      const std::string m_name;
      const datatype::DataType m_dataType;
      const std::string m_unit;
      const std::string m_defaultValue;

      int m_outputOrder;

      /// Construction method for a FieldDefinition
      ///
      /// \param name: the name of the Field
      /// \param type: the type of the field
      /// \param unit: a string describing the unit of the field
      /// \param defaultValue
      FieldDefinition (StorageType storageType, const std::string & name, datatype::DataType type,
            const std::string & unit, const std::string & defaultValue, int outputOrder = 10000);
      ~FieldDefinition (void);

      /// Create a complete copy of this object and its attributes
      FieldDefinition * deepCopy () const;

      /// outputs the m_name value onto the specified stream
      bool saveNameToStream (ostream & ofile, int & borrowed) const;
      /// outputs the unit value onto the specified stream
      bool saveUnitToStream (ostream & ofile, int & borrowed) const;

      public:
      /// returns the storage type (Volatile or Persistent)
      StorageType storageType () const;
      /// returns the name
      const std::string & name () const;
      /// returns the type
      datatype::DataType dataType () const;
      /// returns the unit
      const std::string & unit () const;
      /// returns the defaultValue
      const std::string & defaultValue () const;
      int outputOrder () const;

      /// checks if it has a name
      bool isValid (void) const;
      /// checks if it has the specified name
      bool hasName (const std::string &theName) const;
   };

   /// This class defines what the records of a Table that comforms to a TableDefinition look like.
   /// Objects of this class are created through the DataSchema::addTableDefinition () function.
   class TableDefinition
   {
      typedef std::vector<FieldDefinition *> FieldDefinitionList;
      typedef FieldDefinitionList::const_iterator FieldDefinitionListIterator;
      typedef std::vector<size_t> Shuffle;
      typedef Shuffle::iterator ShuffleIterator;

      friend class Record;
      friend class Table;
      friend class DataSchema;

      const std::string m_name;
      const std::string m_description;
      const int         m_version;

      FieldDefinitionList m_fieldDefinitionList;
      Shuffle m_outputOrdering;

      bool addFieldDefinition (FieldDefinition * fieldDefinition);


      bool isValid (void) const;
      bool hasName (const std::string &theName) const;

      bool saveToStream (ostream & ofile, bool rowBased) const;
      bool saveFieldDefinitionsToStream (ostream & ofile) const;

      /// returns the index of the FieldDefinition that is outputed at the position specified
      /// by order
      size_t getPosition (size_t order) const;

      public:
      /// return the name of a TableDefinition
      const std::string & name() const { return m_name; }

      /// return the description of a TableDefinition
      const std::string & description() const { return m_description; }

      /// return the description of a TableDefinition
      const int & version() const { return m_version; }

      TableDefinition * deepCopy () const;

      /// returns the index in the list of tables of a FieldDefinition with the given name
      int getIndex (const std::string & name, int hint = -1) const;

      /// Creates a new FieldDefinition with the specified attributes
      bool addFieldDefinition (const std::string & name, datatype::DataType type,
            const std::string & unit, const std::string & defaultValue, int outputOrder = 10000);

      /// Creates a new, volatile, FieldDefinition with the specified attributes.
      /// Fields of this FieldDefinition will not be saved.
      bool addVolatileFieldDefinition (const std::string & name, datatype::DataType type,
            const std::string & unit, const std::string & defaultValue);

      /// returns the FieldDefinition at the specified index
      FieldDefinition * getFieldDefinition (size_t i) const;

      /// returns the number of FieldDefinitions
      size_t size (void) const;

      /// returns the number of non-volatile FieldDefinitions
      size_t persistentSize (void) const;

      TableDefinition (const std::string & name, const std::string & description, int version );
     ~TableDefinition ();

   };

   /// Objects of this class are used to specify the structure of a database.
   /// Objects of this class have a name, a description and a list of TableDefinitions.
   class DataSchema
   {
      typedef std::vector<TableDefinition *> TableDefinitionList;
      typedef TableDefinitionList::const_iterator TableDefinitionListIterator;

      friend class Database;

      TableDefinitionList m_tableDefinitionList;

      DataSchema * deepCopy () const;

      public:

      /// Create an empty DataSchema
      DataSchema (void);

      /// Destroy a DataSchema
      ~DataSchema (void);

      /// \brief Add a table definition to this data schema.
      bool addTableDefinition (TableDefinition * tableDefinition);

      /// \brief Remove a table definition from this data schema.
      bool removeTableDefinition (TableDefinition * tableDefinition);

      /// \brief Determine if the data schema contains the table.
      bool hasTableDefinition ( TableDefinition* tableDefinition ) const;

      /// Create an empty TableDefinition with given name and description and add it to this DataSchema
      TableDefinition * addTableDefinition (const std::string & name, const std::string & description, int version );

      /// Find the TableDefinition with the given name
      TableDefinition * getTableDefinition (const std::string & name) const;

      /// Return the TableDefinition at the given index
      TableDefinition * getTableDefinition (size_t i) const { return m_tableDefinitionList[i]; }

      /// Find the index of a TableDefinition with the given name. Returns -1 if not found
      int getIndex (const std::string & name) const;

      /// Return the number of TableDefinitions
      size_t size (void) const { return m_tableDefinitionList.size (); }
   };

   inline FieldDefinition::StorageType FieldDefinition::storageType () const
   {
      return m_storageType;
   }

   inline const std::string & FieldDefinition::name () const
   {
      return m_name;
   }

   inline datatype::DataType FieldDefinition::dataType () const
   {
      return m_dataType;
   }

   inline const std::string & FieldDefinition::unit () const
   {
      return m_unit;
   }

   inline const std::string & FieldDefinition::defaultValue () const
   {
      return m_defaultValue;
   }

   inline int FieldDefinition::outputOrder () const
   {
      return m_outputOrder;
   }

   inline bool FieldDefinition::isValid (void) const
   {
      return m_name.length () != 0;
   }

   inline bool FieldDefinition::hasName (const std::string& theName) const
   {
      return m_name == theName;
   }

   inline bool TableDefinition::isValid (void) const
   {
      return m_name.length () != 0;
   }

   inline bool TableDefinition::hasName (const std::string& theName) const
   {
      return m_name == theName;
   }

   inline size_t TableDefinition::getPosition (size_t order) const
   {
      assert (order < m_outputOrdering.size ());

      return m_outputOrdering[order];
   }

   inline size_t TableDefinition::size (void) const
   {
      return m_fieldDefinitionList.size ();
   }

   inline size_t TableDefinition::persistentSize (void) const
   {
      return m_outputOrdering.size ();
   }

   inline FieldDefinition * TableDefinition::getFieldDefinition  (size_t i) const
   {
      if (i >= size ())
         return 0;
      else
         return m_fieldDefinitionList[i];
   }
}

#endif // DATASCHEMA_H
