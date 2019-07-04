#ifndef HPC_FCBENCH_CMDLINEPARAMETER_H
#define HPC_FCBENCH_CMDLINEPARAMETER_H

#include <string>
#include <vector>

#include <memory>

#include "FormattingException.h"

namespace hpc
{
  
class CmdLineParameter
{
public:
   typedef std::string Option;
   typedef std::string Value;
    
   virtual std::vector< Option > getOptions(const std::vector< Value > & values) const = 0;

   // input text can have several forms:
   // 1) { Name : Option, Name : Option, ... } => ChoiceCmdLineParameter
   // 2) "-some text {0} more text {1} ..."    => ParameterizedCmdLineParameter
   static std::shared_ptr< CmdLineParameter > parse(const std::string & text);
};


class ParameterizedCmdLineParameter : public CmdLineParameter
{
public:
   ParameterizedCmdLineParameter( const std::string & m_option);

   virtual std::vector< Option > getOptions(const std::vector< Value > & values) const;
private:
  std::string m_option;
};

class ChoiceCmdLineParameter : public CmdLineParameter
{
public:
   typedef std::string Name;

   ChoiceCmdLineParameter( const std::vector<Name> & names, const std::vector<Option> & options);

   virtual std::vector< Option > getOptions(const std::vector< Value > & values) const;
private:
   std::vector< Name > m_names;
   std::vector< Option > m_options; 
};



}

#endif
