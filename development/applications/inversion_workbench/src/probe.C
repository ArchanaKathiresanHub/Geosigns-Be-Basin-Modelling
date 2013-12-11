#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <sstream>

#include "ProjectResultsReader.h"

void showUsage ( const char* command, const char* message ) 
{ 
   std::cerr << '\n';

   if ( message != 0 ) {
      std::cerr << command << ": "  << message << '\n';
   }

   std::cerr << "Usage: " << command << std::endl
      << "\t-project <cauldron-project-file>  to specify input file.\n" 
      << "\t-property <property-name>         to specify type of measurement, e.g. Temperature or Pressure.\n"
      << "\t-age <snapshot-time>              to specify the snapshot to query.\n"
      << "\t-x <x-coordinate>                 to specify the x-coordinate.\n"
      << "\t-y <y-coordinate>                 to specify the y-coordinate.\n"
      << "\t-z <z-coordinate>                 to specify a z-coordinate, this option can be used multiple times.\n"
      << "\t[-help]                           to print this message.\n"
      << std::endl;
}

int main (int argc, char ** argv)
{
   std::string projectFileName, propertyName;
   double age = 0.0, x = 0.0, y = 0.0;
   std::vector<double> zs;


   for (int arg = 1; arg < argc; arg++)
   {
      if (strcmp (argv[arg], "-project") == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage( argv[0], "Argument for '-project' is missing.");
            return EXIT_FAILURE;
         }

         projectFileName = argv[++arg];
      }
      else if (strcmp(argv[arg], "-property") == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage( argv[0], "Argument for '-property' is missing.");
            return EXIT_FAILURE;
         }

         propertyName = argv[++arg];
      }    
      else if (strcmp(argv[arg], "-age") == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage( argv[0], "Argument for '-age' is missing.");
            return EXIT_FAILURE;
         }

         std::istringstream s( argv[++arg] );
         s >> age;

         if (s.fail())
         {
            showUsage( argv[0], "Argument for '-age' must be a valid floating point number");
            return EXIT_FAILURE;
         }
      }    
      else if (strcmp(argv[arg], "-x") == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage( argv[0], "Argument for '-x' is missing.");
            return EXIT_FAILURE;
         }

         std::istringstream s( argv[++arg] );
         s >> x;
         if (s.fail())
         {
            showUsage( argv[0], "Argument for '-x' must be a valid floating point number");
            return EXIT_FAILURE;
         }
      }
      else if (strcmp(argv[arg], "-y") == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage( argv[0], "Argument for '-y' is missing.");
            return EXIT_FAILURE;
         }

         std::istringstream s( argv[++arg] );
         s >> y;
         if (s.fail())
         {
            showUsage( argv[0], "Argument for '-y' must be a valid floating point number");
            return EXIT_FAILURE;
         }
      }
      else if (strcmp(argv[arg], "-z") == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage( argv[0], "Argument for '-z' is missing.");
            return EXIT_FAILURE;
         }

         double z;
         std::istringstream s( argv[++arg] );
         s >> z;
         if (s.fail())
         {
            showUsage( argv[0], "Argument for '-y' must be a valid floating point number");
            return EXIT_FAILURE;
         }
         zs.push_back(z);
      }
      else if (strcmp(argv[arg], "-help") == 0)
      {
         showUsage ( argv[ 0 ], " Standard usage.");
         return EXIT_SUCCESS;
      }
      else
      {
         showUsage( argv[0], "Unrecognized option.");
      }
   }


   std::vector<double> results;
   ProjectResultsReader(projectFileName).read(propertyName, age, x, y, zs, results);

   for (size_t i = 0; i < results.size(); ++i)
   {
      if (i)
         std::cout << ' ';

      std::cout << results[i];
   }
   std::cout << std::endl;


   return EXIT_SUCCESS;
}



