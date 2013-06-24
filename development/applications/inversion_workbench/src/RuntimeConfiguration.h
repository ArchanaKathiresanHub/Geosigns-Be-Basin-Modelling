#ifndef INVERSION_RUN_TIME_CONFIGURATION_H
#define INVERSION_RUN_TIME_CONFIGURATION_H

#include <string>
#include <vector>

class RuntimeConfiguration
{
public:
   RuntimeConfiguration(
         const std::string & templateFile,
         const std::string & directory, 
         const std::string & outputFileName,
         const std::string & cauldronVersion,
         const std::string & cauldronRuntimeParams)
      : m_project(templateFile)
      , m_outputdirectory(directory)
      , m_outputfilename(outputFileName)
      , m_cauldronVersion(cauldronVersion)
      , m_cauldronRuntimeParams(cauldronRuntimeParams)                                 
   {}

   const std::string & getTemplateProjectFile() const
   { return m_project; }

   const std::string & getOutputDirectory() const
   { return m_outputdirectory; }

   const std::string & getOutputFileNamePrefix() const
   { return m_outputfilename; }

   const std::string & getCauldronVersion() const
   { return m_cauldronVersion; }

   const std::string & getCauldronRuntimeParams() const
   { return m_cauldronRuntimeParams; }

private:
   std::string m_project;
   std::string m_outputdirectory;
   std::string m_outputfilename;
   std::string m_cauldronVersion;
   std::string m_cauldronRuntimeParams;
};

#endif
