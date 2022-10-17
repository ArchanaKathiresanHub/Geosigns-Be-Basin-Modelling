#ifndef HPC_FCBENCH_BASIN_MODEL_PARAMETER_H
#define HPC_FCBENCH_BASIN_MODEL_PARAMETER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iosfwd>
#include <tr1/array>

#include "FormattingException.h"

namespace DataAccess { namespace Interface {
class ProjectHandle;
} }


namespace database
{
   class Table;
   class Record;
}

namespace hpc
{



  class Project3DParameter
  {
  public:
    virtual ~Project3DParameter() {}

    enum Type { BOOL, INT, LONG, FLOAT, DOUBLE, STRING};

    struct QueryException : formattingexception::BaseException< QueryException > {};
    virtual std::string readValue( const DataAccess::Interface::ProjectHandle * ) const = 0;
    virtual void writeValue( DataAccess::Interface::ProjectHandle *, const std::string & value) const = 0;
    virtual bool isEqual( const Project3DParameter & ) const = 0;
    virtual void print( std::ostream & out) const = 0;

    // input text can have several forms:
    // 1) Table . Field : Type . RecordNr            , i.e.: Explicit
    // 2) Table . Field : Type . [ Field : Type = Value ]   , i.e.: Implicit
    // 3) Param {  Name = Value, ... }   , i.e.: Choice
    // all whitespace should be ignored
    static std::shared_ptr< Project3DParameter > parse( const std::string & text );

  protected:
    static std::string readValue(Type type, database::Record * record, const std::string & field);
    static void writeValue(Type type, database::Record * record, const std::string & field, const std::string & value);
    static Type parseType(const std::string & type);
    static const std::array< std::string, 6 > typeStrings;
  };

  inline bool operator==( const Project3DParameter & a, const Project3DParameter & b)
  { return a.isEqual(b); }

  inline bool operator!=( const Project3DParameter & a, const Project3DParameter & b)
  { return ! a.isEqual(b); }

  inline std::ostream & operator<<( std::ostream & output, const Project3DParameter & p)
  { p.print(output); return output; }

  class ImplicitProject3DParameter : public Project3DParameter
  {
  public:
    ImplicitProject3DParameter( const std::string & table, const std::string & field, Type type
  , const std::string & m_conditionField, Type conditionalType, const std::string & m_conditionValue
  );

    virtual std::string readValue( const DataAccess::Interface::ProjectHandle * ) const ;
    virtual void writeValue( DataAccess::Interface::ProjectHandle *, const std::string & value) const ;

    virtual bool isEqual(const Project3DParameter & other) const;
    virtual void print( std::ostream & output) const;

  private:
    static database::Record * findRecord(database::Table * table, Type type, const std::string & field, const std::string & value);

    std::string m_table;
    std::string m_field;
    std::string m_conditionField;
    std::string m_conditionValue;
    Type m_type, m_conditionalType;
  };

  class ExplicitProject3DParameter : public Project3DParameter
  {
  public:
    ExplicitProject3DParameter( const std::string & table, const std::string & field, Type type, int record);

    virtual std::string readValue( const DataAccess::Interface::ProjectHandle * ) const ;
    virtual void writeValue( DataAccess::Interface::ProjectHandle *, const std::string & value) const ;

    virtual bool isEqual(const Project3DParameter & other) const;
    virtual void print( std::ostream & output) const;

  private:

    std::string m_table;
    std::string m_field;
    int m_recordNumber; // -1 = all record. 0, 1, 2, etc... are normal records
    Type m_type;
  };


  class ChoiceProject3DParameter : public Project3DParameter
  {
  public:
     typedef std::string Name;
     typedef std::string Value;

     ChoiceProject3DParameter( std::shared_ptr< Project3DParameter> parameter
           , const std::vector< Name > & name, const std::vector< Value > & values);

     virtual std::string readValue( const DataAccess::Interface::ProjectHandle * ) const ;
     virtual void writeValue( DataAccess::Interface::ProjectHandle *, const std::string & value) const ;

     virtual bool isEqual(const Project3DParameter & other) const;
     virtual void print( std::ostream & output) const;

  private:
     std::shared_ptr< Project3DParameter > m_parameter;
     std::vector<Name> m_names;
     std::vector<Value> m_values;
  };


}


#endif
