#include <iostream>

#include "project3dparameter.h"
#include "Interface/ProjectHandle.h"


int main(int argc, char ** argv)
{
   if (argc < 5 )
   {
      std::cerr 
         << "Usage: " << argv[0] << " INPUT_PROJECT OUTPUT_PROJECT PARAMETER VALUE\n"
         << "  where PARAMETER is of the form\n"
         << "      TABLE . FIELD : TYPE . RECORDNR\n"
         << "  or\n"
         << "      TABLE . FIELD : TYPE . [ FIELD : TYPE = VALUE ]\n"
         << "\n"
         << "This will set a value in the specified the table in the project file.\n"
         << "\n"
         << "  TABLE, FIELD, TYPE, and VALUE are to substituted by appropriate value:\n"
         << "  - TABLE: The Table name, e.g. LithotypeIoTbl, BasementIoTbl.\n"
         << "  - FIELD: The name of the field / column in the table, e.g.  StpThCond,\n"
            "            TopCrustHeatProd.\n"
         << "  - TYPE: The type of the field. This must match the actual type, \n"
            "            otherwise the program will abort.\n"
            "            valid types are: bool, int, long, float, double, string.\n"
         << "  - RECORDNR: Records can be specified explicitly by specifying\n"
            "            a record number, or they can be specified implicitly\n"
            "            with the bracket '[' notation.\n"
         << "  - VALUE: When implicit notation is used, a value should be supplied\n"
         << "            to select a record. Note that value needs to be quoted\n"
         << "            with double quotes \" when the value contains spaces.\n"
         << std::endl;
      return 1;
   }

   std::string inputProjectFile = argv[1];
   std::string outputProjectFile = argv[2];
   std::string parameter = argv[3];
   std::string value = argv[4];

   DataAccess::Interface::ProjectHandle * project 
      = DataAccess::Interface::OpenCauldronProject( inputProjectFile, "r");

   hpc::Project3DParameter::parse( parameter)->writeValue(project, value);

   project->saveToFile( outputProjectFile );

   return 0;
}
