#ifndef HPC_FCBENCH_BASIN_MODEL_PARAMETER_H
#define HPC_FCBENCH_BASIN_MODEL_PARAMETER_H

#include <string>
#include <boost/shared_ptr.hpp>

#include "formattingexception.h"

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
 
  

  class BasinModelParameter
  {
  public:
    virtual ~BasinModelParameter() {};

    enum Type { BOOL, INT, LONG, FLOAT, DOUBLE, STRING};

    struct QueryException : formattingexception::BaseException< QueryException > {};
    virtual std::string readValue( const DataAccess::Interface::ProjectHandle * ) const = 0;
    virtual void writeValue( DataAccess::Interface::ProjectHandle *, const std::string & value) const = 0;
  
    // input text can have several forms:
    // 1) Table . Field : Type . RecordNr            , i.e.: Explicit
    // 2) Table . Field : Type . [ Field : Type = Value ]   , i.e.: Implicit
    // all whitespace should be ignored
    struct ParseException : formattingexception::BaseException< ParseException > {};
    static boost::shared_ptr< BasinModelParameter > parse( const std::string & text );

  protected:
    static std::string readValue(Type type, database::Record * record, const std::string & field);
    static void writeValue(Type type, database::Record * record, const std::string & field, const std::string & value);
    static Type parseType(const std::string & type);
  };


  class ImplicitBasinModelParameter : public BasinModelParameter
  {
  public:
    ImplicitBasinModelParameter( const std::string & table, const std::string & field, Type type
	, const std::string & m_conditionField, Type conditionalType, const std::string & m_conditionValue
	);

    virtual std::string readValue( const DataAccess::Interface::ProjectHandle * ) const ;
    virtual void writeValue( DataAccess::Interface::ProjectHandle *, const std::string & value) const ;

  private:
    static database::Record * findRecord(database::Table * table, Type type, const std::string & field, const std::string & value);

    std::string m_table;
    std::string m_field;
    std::string m_conditionField;
    std::string m_conditionValue;
    Type m_type, m_conditionalType;
  };

  class ExplicitBasinModelParameter : public BasinModelParameter
  {
  public:
    ExplicitBasinModelParameter( const std::string & table, const std::string & field, Type type, int record);

    virtual std::string readValue( const DataAccess::Interface::ProjectHandle * ) const ;
    virtual void writeValue( DataAccess::Interface::ProjectHandle *, const std::string & value) const ;

  private:

    std::string m_table;
    std::string m_field;
    int m_recordNumber;
    Type m_type;
  };



}


#endif
