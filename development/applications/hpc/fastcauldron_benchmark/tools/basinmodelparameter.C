#include "basinmodelparameter.h"

#include <cstdlib>
#include <cctype>
#include <typeinfo>

#include "Interface/ProjectHandle.h"

namespace hpc 
{


boost::shared_ptr<BasinModelParameter>
BasinModelParameter
   :: parse( const std::string & text )
{
   struct Parser
   {
      Parser(const std::string & text)
         : m_text(text)
         , m_pos(0)
         , m_ignoreWhiteSpace(true)
      {}

      void skipws()
      {
         while (m_pos < m_text.size() && std::isspace(m_text[m_pos]) )
         {
            ++m_pos;
         }
      }

      void expect(const std::string & string)
      {
         if (m_ignoreWhiteSpace)
            skipws();

         if (string.empty())
            return;

         if ( m_text.find(string, m_pos) == m_pos)
         {
            m_pos += string.size()-1;
            ++m_pos;
         }
         else
         {
            throw ParseException() << "Expected '" << string << "' at position " << m_pos;
         }
      }

      bool hasNextToken()
      {
         skipws();
         return !eof();
      }

      std::string nextToken() 
      {
         if (!hasNextToken())
            throw ParseException() << "Unexpected end of input";
                  
         if (m_text[m_pos] == '"')
         {
            ++m_pos; // skip the start quote character

            std::string token;
            while ( !eof() && m_text[m_pos] != '"')
            {
              token.push_back(m_text[m_pos]);
              ++m_pos;
            }
            
            if (!eof())
               ++m_pos; // skip the end quote 

            return token;
         }
         else if (std::isalnum(m_text[m_pos]) || m_text[m_pos] == '_')
         {
            std::string token;
            while ( !eof() && std::isalnum(m_text[m_pos]))
            {
               token.push_back(m_text[m_pos]);
               ++m_pos;
            }
            
            return token;
         }
         else 
         {
            return std::string(m_text, m_pos++, 1);
         }
      }

      bool eof() const
      { return m_pos == m_text.size(); }

      bool m_ignoreWhiteSpace;
      std::string m_text;
      std::string::size_type m_pos;
   } 
   parser(text);


   std::string table = parser.nextToken();
   parser.expect(".");
   std::string field = parser.nextToken();
   parser.expect(":");
   Type type = parseType( parser.nextToken() );
   parser.expect(".");
   std::string record = parser.nextToken();

   if (record == "[")
   {
      std::string conditionalField = parser.nextToken();
      parser.expect(":");
      Type conditionalType = parseType( parser.nextToken() );
      parser.expect("=");
      std::string conditionalValue = parser.nextToken();
      parser.expect("]");

      return boost::shared_ptr< BasinModelParameter >(
            new ImplicitBasinModelParameter( table, field, type, conditionalField, conditionalType, conditionalValue)
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

      return boost::shared_ptr< BasinModelParameter >(
            new ExplicitBasinModelParameter( table, field, type, number)
         );
   }
}

BasinModelParameter :: Type
BasinModelParameter
  :: parseType(const std::string & type)
{
  const std::string types[]
     = { "bool", "int", "long", "float", "double", "string" };

  return Type( std::distance( types, std::find( types, types + sizeof(types)/sizeof(types[0]), type)));
}

std::string
BasinModelParameter
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
        throw BasinModelParameter::QueryException() << "The value '" << value << "' cannot be parsed as '"
           << typeid(T).name() << "' for field '" << field << "'";
  }

  template <> void 
  recordSetValue<std::string>(database::Record * record, const std::string & field, const std::string & value)
  {
     record->setValue(field, value);
  }

  template <typename T> database::Record * 
  tableFindRecord(database::Table * table, const std::string & field, const std::string & value)
  {
     std::istringstream v(value);

     T x;
     v >> x;
     if (v)
        return table->findRecord(field, x);
     else
        throw BasinModelParameter::QueryException() << "The value '" << value << "' cannot be parsed as '"
           << typeid(T).name() << "' for field '" << field << "'";
  }
  
  template <> database::Record *
  tableFindRecord<std::string>(database::Table * table, const std::string & field, const std::string & value)
  {
     return table->findRecord(field, value);
  }

}

void
BasinModelParameter
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

ExplicitBasinModelParameter
   :: ExplicitBasinModelParameter( const std::string & table, const std::string & field, Type type, int record)
   : m_table(table), m_field(field), m_recordNumber(record), m_type(type)
{}

std::string
ExplicitBasinModelParameter
   :: readValue( const DataAccess::Interface::ProjectHandle * project) const
{
   database::Table * table = project->getTable(m_table);
   if (!table)
      throw QueryException() << "Table '" << m_table << "' not found.";

   database::Record * record = table->getRecord(m_recordNumber);
 
   if (!record)
      throw QueryException() << "Record " << m_recordNumber << " not found in table '" << m_table << "'.";

   return BasinModelParameter::readValue(m_type, record, m_field);
}

void
ExplicitBasinModelParameter
   :: writeValue( DataAccess::Interface::ProjectHandle * project, const std::string & value) const
{
   database::Table * table = project->getTable(m_table);
   if (!table)
      throw QueryException() << "Table '" << m_table << "' not found.";

   database::Record * record = table->getRecord(m_recordNumber);
 
   if (!record)
      throw QueryException() << "Record " << m_recordNumber << " not found in table '" << m_table << "'.";

   BasinModelParameter::writeValue(m_type, record, m_field, value);
}

ImplicitBasinModelParameter
   :: ImplicitBasinModelParameter( const std::string & table, const std::string & field, Type type
         , const std::string & conditionField, Type conditionalType, const std::string & conditionValue
         )
   : m_table(table), m_field(field)
   , m_conditionField(conditionField)
   , m_conditionValue(conditionValue)                                         
   , m_type(type)
   , m_conditionalType(conditionalType)
{}

std::string
ImplicitBasinModelParameter
   :: readValue( const DataAccess::Interface::ProjectHandle * project) const
{
   database::Table * table = project->getTable(m_table);
   if (!table)
      throw QueryException() << "Table '" << m_table << "' not found.";

   database::Record * record = findRecord(table, m_conditionalType, m_conditionField, m_conditionValue);
 
   if (!record)
      throw QueryException() << "Record with '" << m_conditionField << "' = '" 
         << m_conditionValue << "' not found in table '" << m_table << "'.";

   return BasinModelParameter::readValue(m_type, record, m_field);
}

void
ImplicitBasinModelParameter
   :: writeValue( DataAccess::Interface::ProjectHandle * project, const std::string & value) const
{
   database::Table * table = project->getTable(m_table);
   if (!table)
      throw QueryException() << "Table '" << m_table << "' not found.";

   database::Record * record = findRecord(table, m_conditionalType, m_conditionField, m_conditionValue);
 
   if (!record)
      throw QueryException() << "Record with '" << m_conditionField << "' = '" 
         << m_conditionValue << "' not found in table '" << m_table << "'.";

   BasinModelParameter::writeValue(m_type, record, m_field, value);
}

database::Record *
ImplicitBasinModelParameter
  :: findRecord(database::Table * table, Type type, const std::string & field, const std::string & value)
{
   switch(type)
   {
      case BOOL: return tableFindRecord<bool>(table, field, value); 
      case INT :  return tableFindRecord<int>(table, field, value); 
      case LONG:  return tableFindRecord<long>(table, field, value); 
      case FLOAT: return tableFindRecord<float>(table, field, value); 
      case DOUBLE:return tableFindRecord<double>(table, field, value); 
      case STRING:return tableFindRecord<std::string>(table, field, value); 

      default: 
         throw QueryException() << "Unknown type";
   }
}

}
