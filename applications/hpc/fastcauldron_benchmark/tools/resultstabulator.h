#ifndef HPC_FCBENCH_RESULTSTABULATOR_H
#define HPC_FCBENCH_RESULTSTABULATOR_H

#include <iosfwd> 
#include <vector>
#include <string>

namespace hpc
{

class ParameterDefinitions;
class VariableDefinitions;
class ParameterSettings;

std::ostream & 
writeResultsTabulatorScript( std::ostream & output, const ParameterDefinitions & parameters, const VariableDefinitions & variables, const std::vector< ParameterSettings > & settings, const std::string & dirWithResults);



}

#endif
