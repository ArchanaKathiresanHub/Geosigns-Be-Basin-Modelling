#include "optionparser.h"

OptionParser
  :: OptionParser(int argc, char ** argv, const std::string & optionFlag, const std::string & separators)
  : m_options()
  , m_pos(0)
{
  std::string currentOptionName = PARAMETER;

  for (int i = 1; i < argc; ++i)
  {
    std::string tok( argv[i] );

    if ( tok.size() > optionFlag.size() && tok.find( optionFlag ) == 0 )
    { // then we found a new option flag
      currentOptionName = tok.substr( optionFlag.size());
      m_options.insert( std::make_pair( currentOptionName, std::vector<std::string>() ));
      continue;
    }

    // else tokenize the parameters of the option
    size_t pos = 0, previousPos = 0;
    while(pos < tok.size())
    {
      previousPos = pos;
      pos = tok.find_first_of(separators, pos);

      if (pos != std::string::npos)
      {
	if (pos != previousPos)
	{
	  m_options[currentOptionName].push_back( tok.substr(previousPos, pos - previousPos));
	}
	m_options[currentOptionName].push_back( tok.substr(pos, 1));
	pos++;
      }
      else
      {
	m_options[currentOptionName].push_back( tok.substr(previousPos) );
	break;
      }
    }
  }
}


const std::string & 
OptionParser :: OptionValues
  :: operator[]( size_t i)
{
  if (i >= m_it->second.size() )
    throw OptionException() << m_name << " needs an additional parameter";

  return m_it->second[i];
}

size_t
OptionParser :: OptionValues
  :: size() const
{
  return m_it->second.size();
}

OptionParser :: OptionValues
  :: OptionValues(const OptionParser & parser, const std::string & name)
  : m_name( name )
  ,	m_it( parser.m_options.find(name) )
{
    if (m_it == parser.m_options.end() )
      throw OptionException() << m_name;
}

OptionParser :: OptionValues 
OptionParser
  :: operator[]( const std::string & name ) const
{
  return OptionValues(*this, name);
}

bool 
OptionParser
  :: defined( const std::string & name) const
{
  return m_options.find(name) != m_options.end();
}

bool
OptionParser
  :: empty() const
{
  return m_options.empty(); 
}

void
OptionParser
  :: erase( const std::string & name) 
{
  m_options.erase( name );
}

std::vector<std::string>
OptionParser
  :: definedNames() const
{
  std::vector<std::string> result;
  for (Options::const_iterator i = m_options.begin(); i != m_options.end(); ++i )
    result.push_back( i->first );

  return result;
}


const std::string
OptionParser
  :: PARAMETER = " a positional parameter ";


