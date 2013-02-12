#ifndef HPC_FCBENCH_PARAMETERDEFINITIONS_H
#define HPC_FCBENCH_PARAMETERDEFINITIONS_H

#include <map>
#include <boost/shared_ptr.hpp>


namespace hpc
{
class Project3DParameter;
class CmdLineParameter;

class ParameterDefinitions
{
public:
   typedef std::string Name;

   typedef std::map< Name, boost::shared_ptr<const Project3DParameter> > ProjectParamMap;
   typedef std::map< Name, boost::shared_ptr<const CmdLineParameter> >   CmdLineParamMap;

   ParameterDefinitions(const std::string & parameterDefinitionsFiles );

   const ProjectParamMap & projectParameters() const
   { return m_projectParams; }

   const CmdLineParamMap  & mpiCmdLineParameters() const
   { return m_mpiCmdLineParams; }

   const CmdLineParamMap & cauldronCmdLineParameters() const
   { return m_cauldronCmdLineParams; }

private:
   ProjectParamMap m_projectParams;
   CmdLineParamMap m_mpiCmdLineParams;
   CmdLineParamMap m_cauldronCmdLineParams;
};


}

#endif
