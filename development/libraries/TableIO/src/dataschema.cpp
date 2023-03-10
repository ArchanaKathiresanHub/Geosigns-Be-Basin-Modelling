#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
using namespace std;

#include "datautils.h"

#include "database.h"
#include "dataschema.h"


using namespace database;
using namespace std;


FieldDefinition::FieldDefinition (FieldDefinition::StorageType storageType, const std::string & name, datatype::DataType type,
	    const std::string & unit, const std::string & defaultValue, int outputOrder)
: m_storageType (storageType), m_name (name), m_dataType (type), m_unit (unit), m_defaultValue (defaultValue), m_outputOrder (outputOrder)
{
}

FieldDefinition::~FieldDefinition (void)
{
}

FieldDefinition * FieldDefinition::deepCopy () const
{
   return new FieldDefinition (m_storageType, m_name, m_dataType, m_unit, m_defaultValue, m_outputOrder);
}

bool FieldDefinition::saveNameToStream (ostream & ofile, int & borrowed) const
{
   return saveStringToStream (ofile, name (), borrowed, Database::GetFieldWidth ());
}

bool FieldDefinition::saveUnitToStream (ostream & ofile, int & borrowed) const
{
   string field = unit ();
   field.insert (0, "(");
   field += ")";

   return saveStringToStream (ofile, field, borrowed, Database::GetFieldWidth ());
}

TableDefinition::TableDefinition( const std::string & name, const std::string & description, int version )
                                : m_name( name )
                                , m_description( description )
                                , m_version( version )
                                {}

TableDefinition::~TableDefinition (void)
{
   FieldDefinitionListIterator iter;
   for (iter = m_fieldDefinitionList.begin ();
	 iter != m_fieldDefinitionList.end (); ++iter)
   {
      FieldDefinition * fieldDef = * iter;
      delete fieldDef;
   }
}

int TableDefinition::getIndex (const string & name, int hint) const
{
   if ( hint >= 0 && hint < static_cast<int>( size () ) )
   {
      FieldDefinition * fieldDef = getFieldDefinition (hint);
      if (fieldDef->hasName (name)) return hint;
   }

   for (int i = 0; i < static_cast<int>(size ()); i++)
   {
      FieldDefinition * fieldDef = getFieldDefinition (i);

      if (fieldDef->hasName (name)) return i;
   }

   // cerr << "Error: Field " << name << " not part of Table " << m_name << endl;
   return -1;
}

bool TableDefinition::addFieldDefinition (FieldDefinition * fieldDef)
{
   m_fieldDefinitionList.push_back (fieldDef);

   if (fieldDef->storageType () == FieldDefinition::Volatile) return true;

   ShuffleIterator iter;
   for (iter = m_outputOrdering.begin (); iter != m_outputOrdering.end (); ++iter)
   {
      size_t order = *iter;
      FieldDefinition * localFieldDef = m_fieldDefinitionList[order];
      if (localFieldDef->outputOrder () > fieldDef->outputOrder ()) break;
   }
   m_outputOrdering.insert (iter, m_fieldDefinitionList.size () - 1);

   return true;
}

// add a new field definition with a name, datatype, unit and default value
bool TableDefinition::addFieldDefinition (const std::string & name, datatype::DataType type,
      const std::string & unit, const std::string & defaultValue, int outputOrder)
{
   FieldDefinition * fieldDef = new FieldDefinition (FieldDefinition::Persistent, name, type, unit, defaultValue, outputOrder);
   addFieldDefinition (fieldDef);

   return true;
}

// add a new volatile field definition with a name, datatype, unit and default value
bool TableDefinition::addVolatileFieldDefinition (const std::string & name, datatype::DataType type,
      const std::string & unit, const std::string & defaultValue)
{
   FieldDefinition * fieldDef = new FieldDefinition (FieldDefinition::Volatile, name, type, unit, defaultValue);
   addFieldDefinition (fieldDef);

   return true;
}

TableDefinition * TableDefinition::deepCopy () const
{
   TableDefinition * tableDefCopy = new TableDefinition (name (), description (), version () );

   FieldDefinitionListIterator iter;
   for (iter = m_fieldDefinitionList.begin ();
	 iter != m_fieldDefinitionList.end (); ++iter)
   {
      FieldDefinition * fieldDef = * iter;

      FieldDefinition * fieldDefCopy = fieldDef->deepCopy ();

      if (!fieldDefCopy)
      {
	 delete tableDefCopy;
	 return 0;
      }

      tableDefCopy->addFieldDefinition (fieldDefCopy);
   }

   return tableDefCopy;
}

bool TableDefinition::saveToStream( ostream & ofile, bool rowBased ) const
{
   ofile << "; " << m_description << endl;
   ofile << ";" << endl;
   ofile << "[" << m_name << "]" << endl;
   ofile << ";v" << version() << endl;

   if ( !rowBased ) { saveFieldDefinitionsToStream( ofile ); }
   return !ofile.fail();
}

bool TableDefinition::saveFieldDefinitionsToStream (ostream & ofile) const
{
   int borrowed = 0;

   size_t order;

   for (order = 0; order < persistentSize (); order++)
   {
      const size_t position = getPosition (order);
      FieldDefinition * fieldDef = getFieldDefinition (position);

      if (order != 0)
      {
	 ofile << " ";
      }

      fieldDef->saveNameToStream (ofile, borrowed);
   }
   ofile << endl;

   borrowed = 0;

   for (order = 0; order < persistentSize (); order++)
   {
      const size_t position = getPosition (order);
      FieldDefinition * fieldDef = getFieldDefinition (position);

      if (order != 0)
      {
	 ofile << " ";
      }

      fieldDef->saveUnitToStream (ofile, borrowed);
   }
   ofile << endl;

   return !ofile.fail ();
}

DataSchema::DataSchema (void)
{
}

DataSchema::~DataSchema (void)
{
   TableDefinitionListIterator iter;
   for (iter = m_tableDefinitionList.begin ();
	 iter != m_tableDefinitionList.end (); ++iter)
   {
      TableDefinition * tableDef = * iter;
      delete tableDef;
   }
}

DataSchema * DataSchema::deepCopy () const
{
   DataSchema * dataSchemaCopy = new DataSchema ();

   TableDefinitionListIterator iter;
   for (iter = m_tableDefinitionList.begin ();
	 iter != m_tableDefinitionList.end (); ++iter)
   {
      TableDefinition * tableDef = * iter;

      TableDefinition * tableDefCopy = tableDef->deepCopy ();
      if (!tableDefCopy)
      {
	 delete dataSchemaCopy;
	 return 0;
      }

      dataSchemaCopy->addTableDefinition (tableDefCopy);
   }

   return dataSchemaCopy;
}

bool DataSchema::addTableDefinition (TableDefinition * tableDefinition) {

   // Dont add a null table definition and dont add one if its already in the schema.
   if ( tableDefinition != nullptr and not hasTableDefinition ( tableDefinition )) {
      m_tableDefinitionList.push_back ( tableDefinition );
   }

   return true;
}

bool DataSchema::removeTableDefinition (TableDefinition * tableDefinition) {

   // Dont try to remove a null table definition and dont remove it if does not exist in the schema.
   if ( tableDefinition != nullptr and hasTableDefinition ( tableDefinition )) {
      TableDefinitionList::iterator iter = std::remove ( m_tableDefinitionList.begin (), m_tableDefinitionList.end (), tableDefinition );
      m_tableDefinitionList.erase ( iter, m_tableDefinitionList.end ());
   }

   return true;
}

bool DataSchema::hasTableDefinition ( TableDefinition* tableDefinition ) const {
   return std::find ( m_tableDefinitionList.begin (), m_tableDefinitionList.end (), tableDefinition ) != m_tableDefinitionList.end ();
}

// create a new table definition with name and description
TableDefinition * DataSchema::addTableDefinition (const std::string & name, const std::string & description, int ver )
{
   TableDefinition * tableDefinition = new TableDefinition( name, description, ver );
   addTableDefinition (tableDefinition);
   return tableDefinition;
}

// Retrieve a data definition
TableDefinition * DataSchema::getTableDefinition (const string & name) const
{
   TableDefinitionListIterator iter;

   for (iter = m_tableDefinitionList.begin ();
	 iter != m_tableDefinitionList.end (); ++iter)
   {
      TableDefinition * tableDef = * iter;

      if (tableDef->hasName (name)) return tableDef;
   }

   return 0;
}

int DataSchema::getIndex (const string & name) const
{
   int i;
   TableDefinitionListIterator iter;

   for (i = 0, iter = m_tableDefinitionList.begin ();
	 iter != m_tableDefinitionList.end (); ++i, ++iter)
   {
      TableDefinition * tableDef = * iter;

      if (tableDef->hasName (name)) return i;
   }

   return -1;
}
