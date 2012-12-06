#ifndef INVERSION_OPTIONPARSER_H
#define INVERSION_OPTIONPARSER_H

#include <map>
#include <vector>
#include <string>

#include "generalexception.h"


struct OptionException : BaseException< OptionException > 
{ OptionException() { *this << "Error reading command line parameter: "; } };

class OptionParser
{
public:
  static const std::string PARAMETER;  

  typedef std::map< std::string, std::vector< std::string > > Options;

  OptionParser(int argc, char ** argv, const std::string & optionFlag, const std::string & separators);

  class OptionValues
  { friend class OptionParser;
  public:

    const std::string & operator[]( size_t i);

    size_t size() const;

  private:
    OptionValues(const OptionParser & parser, const std::string & name);

    std::string m_name;
    Options::const_iterator m_it;
  };

  OptionValues operator[]( const std::string & name ) const;

  bool defined( const std::string & name) const;

  bool empty() const;

  void erase( const std::string & name) ;

  std::vector<std::string> definedNames() const;

private:
  Options m_options ;
  int m_pos;
};


#endif
