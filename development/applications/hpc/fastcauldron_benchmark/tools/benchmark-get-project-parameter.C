#include <iostream>

#include "project3dparameter.h"
#include "Interface/ProjectHandle.h"


int main(int argc, char ** argv)
{
   if (argc < 3 )
   {
      std::cerr 
         << "Usage: " << argv[0] << " PROJECTFILE PARAMETER\n"
         << "  where PARAMETER is of the form\n"
         << "      TABLE . FIELD : TYPE . RECORDNR\n"
         << "  or\n"
         << "      TABLE . FIELD : TYPE . [ FIELD : TYPE = VALUE ]\n"
         << "\n"
         << "This will read a value from the specified table in the project file\n"
         << "and output it to standard output.\n"
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

   std::string projectFile = argv[1];
   std::string parameter = argv[2];

   DataAccess::Interface::ProjectHandle * project 
      = DataAccess::Interface::OpenCauldronProject( projectFile, "r");

   std::cout << hpc::Project3DParameter::parse( parameter)->readValue(project) << std::endl;

   return 0;
}
