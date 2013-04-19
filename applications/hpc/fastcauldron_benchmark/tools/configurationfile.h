#ifndef HPC_FCBENCH_CONFIGURATIONFILE_H
#define HPC_FCBENCH_CONFIGURATIONFILE_H

#include <map>
#include <string>

#include "formattingexception.h"

namespace hpc
{

class ConfigurationFile
{
public:
   ConfigurationFile( const std::string & file );

   const std::string & getString(const std::string & name) const;
   int getInt(const std::string & name) const;

   struct Exception : formattingexception::BaseException< Exception > {};

private:
   std::map< std::string, std::string > m_nameValueMap;
};

}

#endif
