#ifndef INVERSION_RUN_TIME_CONFIGURATION_H
#define INVERSION_RUN_TIME_CONFIGURATION_H

#include <string>
#include <vector>

class RuntimeConfiguration
{
public:
   RuntimeConfiguration(const std::string & templateFile, const std::string & directory, const std::string & outputFileName)
      : m_project(templateFile)
      , m_outputdirectory(directory)
      , m_outputfilename(outputFileName)
   {}

   const std::string & getTemplateProjectFile() const
   { return m_project; }

   const std::string & getOutputDirectory() const
   { return m_outputdirectory; }

   const std::string & getOutputFileNamePrefix() const
   { return m_outputfilename; }


private:
   std::string m_project;
   std::string m_outputdirectory;
   std::string m_outputfilename;
};

#endif
