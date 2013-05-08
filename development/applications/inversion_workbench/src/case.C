#include <string>
#include <iostream>

#include "case.h"
#include "parameter.h"
#include "project.h"

void Case::addParameter(Parameter * parameter)
{
   m_values.push_back( boost::shared_ptr<Parameter>( parameter ));
}

void Case::createProjectFile(const std::string & originalProjectFile, const std::string & workingProjectFile) const
{
   Project project( originalProjectFile, workingProjectFile);
   for (unsigned i = 0; i < m_values.size(); ++i)
   {
      m_values[i]->changeParameter(project);
   }
   project.close();
}

void Case::printParameters(std::ostream & output) const
{
   if (m_values.empty())
   {
      output << "NONE";
   }
   else
   {
      for (int i = 0; i < m_values.size(); ++i)
      {
         if (i > 0)
            output << "; ";
         m_values[i]->print(output);
      }
   }
}


