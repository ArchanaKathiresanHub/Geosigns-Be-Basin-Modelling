#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "generalexception.h"
#include "Interface/ProjectHandle.h"
#include "DataMiningProjectHandle.h"
#include "CauldronDomain.h"


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

struct ProbeException : BaseException< ProbeException >
{ ProbeException() { *this << "Error while probing Cauldron project: "; } };

/// Probes a Cauldron project3d file. Given:
///  - a physical quantity, as 'propertyName',
///  - a snapshot time,
///  - horizontal coordinates, x and y, and
///  - a series of depths, in zs,
///  this function return the results from the simulation in the output vector 'results'.
void probe( const std::string & project, const std::string & propertyName, double snapshotTime, double x, double y,
    const std::vector<double> & zs, std::vector<double> & results )
{
  // clear the result vector
  results.clear();
  
  // Open project file
  DataAccess::Mining::DomainPropertyFactory factory;
  DataAccess::Interface::ProjectHandle::UseFactory( &factory );

  DataAccess::Mining::ProjectHandle* projectHandle 
    = dynamic_cast< DataAccess::Mining::ProjectHandle * >(
	DataAccess::Interface::OpenCauldronProject(project, "r")
      );

  if (!projectHandle)
    throw ProbeException() << "Could not load project file '" << project << "'";

  // Load property
  const DataAccess::Interface::Property* property = projectHandle->findProperty (propertyName);
  if (!property) 
    throw ProbeException() << "Unknown PropertyName value: " << propertyName;

  // Load snapshot
  if (snapshotTime < 0) 
    throw ProbeException() << "Illegal snapshot time: " << snapshotTime;
  const DataAccess::Interface::Snapshot * snapshot = projectHandle->findSnapshot (snapshotTime);
  DataAccess::Mining::CauldronDomain domain ( projectHandle );
  domain.setSnapshot (snapshot);
  DataAccess::Mining::DomainPropertyCollection* domainProperties = projectHandle->getDomainPropertyCollection ();
  domainProperties->setSnapshot (snapshot);
	
  // Check whether the x and y are in the grid
  unsigned int a, b;
  const DataAccess::Interface::Grid * grid = projectHandle->getLowResolutionOutputGrid ();
  if (!grid->getGridPoint (x, y, a, b)) 
    throw ProbeException() << "Illegal (XCoord, YCoord) pair: (" << x << ", " << y << ")";
       
  // Retrieve the results
  for (size_t i = 0; i < zs.size(); ++i)
  {
    DataAccess::Mining::ElementPosition element;
    if (!domain.findLocation (x, y, zs[i], element))
      throw ProbeException() << "Illegal point coordinates: " << x << ", " << y << ", " << zs[i];

    results.push_back( domainProperties->getDomainProperty (property)->compute (element) );
  }

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
  probe( projectFileName, propertyName, age, x, y, zs, results);

  for (size_t i = 0; i < results.size(); ++i)
  {
    if (i)
      std::cout << ' ';

    std::cout << results[i];
  }
  std::cout << std::endl;
  

  return EXIT_SUCCESS;
}



