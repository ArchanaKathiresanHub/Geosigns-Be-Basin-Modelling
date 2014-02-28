#include <string>
#include <iostream>

#include "formattingexception.h"

#include "Scenario.h"
#include "parameter.h"
#include "project.h"

Scenario::Scenario () :
      m_isValid (true)
{
}

bool Scenario::isValid() const
{
   return m_isValid;
}

void Scenario::addParameter(Parameter * parameter)
{
   m_parameters.push_back( boost::shared_ptr<Parameter>( parameter ));
}

void Scenario::createProjectFile(const std::string & originalProjectFile, const std::string & workingProjectFile) const
{
   Project project( originalProjectFile, workingProjectFile);
   for (unsigned i = 0; i < m_parameters.size(); ++i)
   {
      try
      {
         m_parameters[i]->changeParameter(project);
      }
      catch (formattingexception::GeneralException & fe)
      {
         m_isValid = false;
         throw (fe);
      }
   }
   project.close();
}

void Scenario::printParameters(std::ostream & output) const
{
   if (m_parameters.empty())
   {
      output << "NONE";
   }
   else
   {
      for (int i = 0; i < m_parameters.size(); ++i)
      {
         if (i > 0)
            output << "; ";
         m_parameters[i]->print(output);
      }
   }
}


