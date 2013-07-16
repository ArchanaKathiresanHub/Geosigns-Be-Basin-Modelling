#include "parameterdefinitions.h"
#include "parser.h"
#include "project3dparameter.h"
#include "cmdlineparameter.h"
#include "system.h"

#include <fstream>
#include <string>
#include <iostream>

namespace hpc
{

ParameterDefinitions
   :: ParameterDefinitions(const Path & parameterDefinitionFile )
   : m_projectParams()
   , m_mpiCmdLineParams()
   , m_cauldronCmdLineParams()
{                              
   // parse parameter definition file
   boost::shared_ptr<std::istream> paramDefs = parameterDefinitionFile.readFile();
   std::string line;
   int lineNr = 0;
   while ( ++lineNr, getline(*paramDefs, line) )
   {
      // skip lines that are empty, only contain white space and/or start with a #
      {  std::string::size_type i;
         for (i = 0; i < line.size(); ++i)
            if ( !std::isspace( line[i] ) )
               break;
         if (i == line.size() || line[i] == '#')
            continue;
      }

      try
      {
         Parser parser(line);

         std::string paramName = parser.nextToken();
         parser.expect("-");
         std::string paramType = parser.nextToken();
         parser.expect("-");

         if (paramType == "Project3D")
            m_projectParams[paramName] = Project3DParameter::parse(parser.remaining());
         else if (paramType == "MpiCmdLine")
            m_mpiCmdLineParams[paramName] = CmdLineParameter::parse(parser.remaining());
         else if (paramType == "CauldronCmdLine")
            m_cauldronCmdLineParams[paramName] = CmdLineParameter::parse(parser.remaining());
         else
            throw ParseException() << "Unknown parameter type '" << paramType << "' in parameter definition file";
      }
      catch(std::exception & e)
      {
         std::cerr << "Warning: Syntax error on line " << lineNr << " in file '" << parameterDefinitionFile << "'; "
            << e.what() << std::endl;
      }
   }
}
 

}
