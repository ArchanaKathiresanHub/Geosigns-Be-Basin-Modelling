#include "project3dparameter.h"
#include "parser.h"

#include <cstdlib>
#include <cctype>
#include <typeinfo>
#include <algorithm>
#include <limits>

#include "ProjectHandle.h"

namespace hpc 
{


std::shared_ptr<Project3DParameter>
Project3DParameter
   :: parse( const std::string & text )
{
   Parser parser(text);

   std::string table = parser.nextToken();
   parser.expect(".");
   std::string field = parser.nextToken();
   parser.expect(":");
   Type type = parseType( parser.nextToken() );
   parser.expect(".");
   std::string record = parser.nextToken();

   std::shared_ptr< Project3DParameter > param;
   if (record == "[")
   {
      std::string conditionalField = parser.nextToken();
      parser.expect(":");
      Type conditionalType = parseType( parser.nextToken() );
      parser.expect("=");
      std::string conditionalValue = parser.nextToken();
      parser.expect("]");

      param.reset(
            new ImplicitProject3DParameter( table, field, type, conditionalField, conditionalType, conditionalValue)
         );
   }
   else if (record == "*")
   {
      param.reset(
            new ExplicitProject3DParameter( table, field, type, -1)
         );
   }
   else
   {
      const char * string = record.c_str();
      char * ptr = 0;

      long int number = std::strtol(string, &ptr, 0);

      if (*ptr != '\0')
         throw ParseException() << "Expected a number of record number, but encountered '" << record << "' instead";

      if (number > std::numeric_limits<int>::max())
         throw ParseException() << "Overflow while parsing the record number '" << record << "'.";

      if (number < 0)
         throw ParseException() << "The record number can't be negative.";

      param.reset( 
            new ExplicitProject3DParameter( table, field, type, number)
         );
   }

   if (parser.hasNextToken() )
   {
      std::string attribute = parser.nextToken();

      if (attribute == "{")
      {
         std::vector< std::string > names;
         std::vector< std::string > values;
         std::string comma;
         do
         {
            names.push_back( parser.nextToken() );
            parser.expect("=");
            values.push_back(parser.nextToken());

            comma = parser.nextToken();
         } while(comma == ",");

         if (comma != "}")
            throw ParseException() << "Expected '}' to conclude choice of parameter values";

         param.reset( new ChoiceProject3DParameter( param, names, values) );
      }
      else
      {
         throw ParseException() << "Unexpected '" << attribute << "' after parameter.";
      }
   }

   return param;
}

const std::array< std::string, 6 >
Project3DParameter
   :: typeStrings = { { "bool", "int", "long", "float", "double", "string" } };

Project3DParameter :: Type
Project3DParameter
  :: parseType(const std::string & type)
{
  return Type( std::distance( typeStrings.begin(), std::find( typeStrings.begin(), typeStrings.end(), type)) );
}

std::string
Project3DParameter
  :: readValue(Type type, database::Record * record, const std::string & field)
{
   assert( record );

   std::ostringstream result;
   switch(type)
   {
      case BOOL: result << record->getValue<bool>(field); break;
      case INT : result << record->getValue<int>(field); break;
      case LONG: result << record->getValue<long>(field); break;
      case FLOAT: result << record->getValue<float>(field); break;
      case DOUBLE: result << record->getValue<double>(field); break;
      case STRING: result << record->getValue<std::string>(field); break;
      default: throw QueryException() << "Unknown type";
   }
   return result.str();
}

namespace 
{
  template <typename T> void 
  recordSetValue(database::Record * record, const std::string & field, const std::string & value)
  {
     std::istringstream v(value);

     T x;
     v >> x;
     if (v)
        record->setValue(field, x);
     else
        throw Project3DParameter::QueryException() << "The value '" << value << "' cannot be parsed as '"
           << typeid(T).name() << "' for field '" << field << "'";
  }

  template <> void 
  recordSetValue<std::string>(database::Record * record, const std::string & field, const std::string & value)
  {
     record->setValue(field, value);
  }

  template <typename T> database::Record * 
  tableFindRecord(database::Table * table, const std::string & field, const std::string & value);
  
  template <> database::Record *
  tableFindRecord<std::string>(database::Table * table, const std::string & field, const std::string & value)
  {
     return table->findRecord(field, value);
  }

}

void
Project3DParameter
  :: writeValue(Type type, database::Record * record, const std::string & field, const std::string & value)
{
   switch(type)
   {
      case BOOL: recordSetValue<bool>(record, field, value); break;
      case INT :  recordSetValue<int>(record, field, value); break;
      case LONG:  recordSetValue<long>(record, field, value); break;
      case FLOAT:  recordSetValue<float>(record, field, value); break;
      case DOUBLE: recordSetValue<double>(record, field, value); break;
      case STRING: recordSetValue<std::string>(record, field, value); break;

      default: 
         throw QueryException() << "Unknown type";
   }
}

ExplicitProject3DParameter
   :: ExplicitProject3DParameter( const std::string & table, const std::string & field, Type type, int record)
   : m_table(table), m_field(field), m_recordNumber(record), m_type(type)
{}

bool
ExplicitProject3DParameter
   :: isEqual(const Project3DParameter & other) const
{
   const ExplicitProject3DParameter * o = dynamic_cast<const ExplicitProject3DParameter *>(&other);
   return o && this->m_table == o->m_table && this->m_field == o->m_field &&
      this->m_recordNumber == o->m_recordNumber && this->m_type == o->m_type;
}

void
ExplicitProject3DParameter
   :: print( std::ostream & output) const
{
   if (m_recordNumber < 0)
     output << m_table << " . " << m_field << ":" << typeStrings[m_type] << " . *";
   else
     output << m_table << " . " << m_field << ":" << typeStrings[m_type] << " . " << m_recordNumber;
}

std::string
ExplicitProject3DParameter
   :: readValue( const DataAccess::Interface::ProjectHandle * project) const
{
   database::Table * table = project->getTable(m_table);
   if (!table)
      throw QueryException() << "Table '" << m_table << "' not found.";

   if (m_recordNumber == -1)
   {
      if (table->size() == 0)
         throw QueryException() << "Table does not contain any records.";

      std::string value = Project3DParameter::readValue(m_type, table->getRecord(0), m_field);
      for (int i = 1; i < table->size(); ++i)
         if (Project3DParameter::readValue(m_type, table->getRecord(i), m_field) != value)
            throw QueryException() << "Cannot use * as record number, because the field in this table has differing values";
      return value;
   }
   else
   {
      database::Record * record = table->getRecord(m_recordNumber);
    
      if (!record)
         throw QueryException() << "Record " << m_recordNumber << " not found in table '" << m_table << "'.";

      return Project3DParameter::readValue(m_type, record, m_field);
   }
}

void
ExplicitProject3DParameter
   :: writeValue( DataAccess::Interface::ProjectHandle * project, const std::string & value) const
{
   database::Table * table = project->getTable(m_table);
   if (!table)
      throw QueryException() << "Table '" << m_table << "' not found.";

   if (m_recordNumber == -1)
   {
      for (int i = 0 ; i < table->size(); ++i)
         Project3DParameter::writeValue(m_type, table->getRecord(i), m_field, value);
   }
   else
   {
      database::Record * record = table->getRecord(m_recordNumber);
    
      if (!record)
         throw QueryException() << "Record " << m_recordNumber << " not found in table '" << m_table << "'.";

      Project3DParameter::writeValue(m_type, record, m_field, value);
   }
}

ImplicitProject3DParameter
   :: ImplicitProject3DParameter( const std::string & table, const std::string & field, Type type
         , const std::string & conditionField, Type conditionalType, const std::string & conditionValue
         )
   : m_table(table), m_field(field)
   , m_conditionField(conditionField)
   , m_conditionValue(conditionValue)                                         
   , m_type(type)
   , m_conditionalType(conditionalType)
{}

std::string
ImplicitProject3DParameter
   :: readValue( const DataAccess::Interface::ProjectHandle * project) const
{
   database::Table * table = project->getTable(m_table);
   if (!table)
      throw QueryException() << "Table '" << m_table << "' not found.";

   database::Record * record = findRecord(table, m_conditionalType, m_conditionField, m_conditionValue);
 
   if (!record)
      throw QueryException() << "Record with '" << m_conditionField << "' = '" 
         << m_conditionValue << "' not found in table '" << m_table << "'.";

   return Project3DParameter::readValue(m_type, record, m_field);
}

void
ImplicitProject3DParameter
   :: writeValue( DataAccess::Interface::ProjectHandle * project, const std::string & value) const
{
   database::Table * table = project->getTable(m_table);
   if (!table)
      throw QueryException() << "Table '" << m_table << "' not found.";

   database::Record * record = findRecord(table, m_conditionalType, m_conditionField, m_conditionValue);
 
   if (!record)
      throw QueryException() << "Record with '" << m_conditionField << "' = '" 
         << m_conditionValue << "' not found in table '" << m_table << "'.";

   Project3DParameter::writeValue(m_type, record, m_field, value);
}

bool
ImplicitProject3DParameter
   :: isEqual(const Project3DParameter & other) const
{
   const ImplicitProject3DParameter * o = dynamic_cast<const ImplicitProject3DParameter *>(& other);
   return o && this->m_table == o->m_table && this->m_field == o->m_field &&
      this->m_type == o->m_type && this->m_conditionField == o->m_conditionField &&
      this->m_conditionValue == o->m_conditionValue && this->m_conditionalType == o->m_conditionalType;
}

void
ImplicitProject3DParameter
   :: print( std::ostream & output) const
{
   output << m_table << " . " << m_field << ":" << typeStrings[m_type] << " . [ "
      << m_conditionField << ":" << typeStrings[m_conditionalType] << " = " << m_conditionValue << " ]";
}

database::Record *
ImplicitProject3DParameter
  :: findRecord(database::Table * table, Type type, const std::string & field, const std::string & value)
{
   switch(type)
   {
      case STRING:return tableFindRecord<std::string>(table, field, value); 

      default: 
         throw QueryException() << "Unknown type";
   }
}

ChoiceProject3DParameter
  :: ChoiceProject3DParameter(std::shared_ptr< Project3DParameter> parameter
           , const std::vector< Name > & names, const std::vector< Value > & values)
  : m_parameter(parameter)
  , m_names(names)
  , m_values(values)                
{}

std::string
ChoiceProject3DParameter
  :: readValue(const DataAccess::Interface::ProjectHandle * project) const
{
  std::string value = m_parameter->readValue(project);
  unsigned i = std::distance(m_values.begin(), std::find( m_values.begin(), m_values.end(), value));

  if (i == m_values.size())
     throw QueryException() << "Value that was read from parameter was not defined in choice parameter";

  return m_names[i];
}
                                  
void
ChoiceProject3DParameter
  :: writeValue(DataAccess::Interface::ProjectHandle * project, const std::string & value) const
{
  unsigned i = std::distance(m_names.begin(), std::find( m_names.begin(), m_names.end(), value));

  if (i == m_names.size())
     throw QueryException() << "The name for the used to write a choice parameter is not defined";

  m_parameter->writeValue(project, m_values[i]);
}
 
bool
ChoiceProject3DParameter
   :: isEqual(const Project3DParameter & other) const
{
   const ChoiceProject3DParameter * o = dynamic_cast<const ChoiceProject3DParameter *>(&other);
   return o && this->m_parameter->isEqual( *o->m_parameter ) && 
      this->m_names == o->m_names && this->m_values == o->m_values;
} 

void
ChoiceProject3DParameter
   :: print( std::ostream & output ) const
{
   m_parameter->print(output);
   output << " { ";
   for (unsigned i = 0; i < m_names.size(); ++i)
   {
      if (output)
         output << ", ";

      output << m_names[i] << " = " << m_values[i];
   }
   output << " }";
}

}
